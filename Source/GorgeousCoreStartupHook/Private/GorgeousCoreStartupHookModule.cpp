#include "GenericPlatform/GenericPlatformSplash.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "Misc/MessageDialog.h"
#include "HAL/PlatformProcess.h"
#include "HAL/FileManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformSplash.h"
#include "Logging/LogMacros.h"
#include "Misc/SecureHash.h"

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousStartupHook, Log, All);

class FGorgeousCoreStartupHookModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		// ================================================================
		// This fires at PostConfigInit, BEFORE GorgeousCoreRuntime loads
		// (which is PreDefault). We do all scanning AND blocking right here
		// so the binary swap can happen before any Gorgeous module loads.
		// ================================================================

		// --- 0. Allow explicit bypass ---
		if (FParse::Param(FCommandLine::Get(), TEXT("BypassGorgeousHook")))
		{
			UE_LOG(LogGorgeousStartupHook, Log, TEXT("BypassGorgeousHook flag detected. Skipping mismatch checks."));
			return;
		}

		FPlatformSplash::SetSplashText(SplashTextType::StartupProgress,
			TEXT("Cross-checking minimum required core version against Gorgeous Plugins..."));

		// --- 1. Resolve GorgeousCore dynamically via IPluginManager ---
		// Never hardcode the folder name, the plugin directory can be named anything.
		TSharedPtr<IPlugin> CorePlugin = IPluginManager::Get().FindPlugin(TEXT("GorgeousCore"));
		if (!CorePlugin.IsValid())
		{
			UE_LOG(LogGorgeousStartupHook, Warning, TEXT("GorgeousCore not found via IPluginManager, skipping mismatch check."));
			return;
		}

		const FString CurrentCoreVersion = CorePlugin->GetDescriptor().VersionName;
		UE_LOG(LogGorgeousStartupHook, Log, TEXT("GorgeousCore installed version: %s"), *CurrentCoreVersion);

		// --- 2. Load GorgeousPersistentData.json ---
		// ProjectSavedDir() is reliably populated at PostConfigInit.
		const FString CacheFilePath = FPaths::ConvertRelativePathToFull(
			FPaths::ProjectSavedDir() / TEXT("GorgeousThings") / TEXT("GorgeousPersistentData.json")
		);
		UE_LOG(LogGorgeousStartupHook, Log, TEXT("Cache path: %s"), *CacheFilePath);

		FString FileContents;
		TSharedPtr<FJsonObject> PersistentDataObject;

		if (FPaths::FileExists(CacheFilePath) && FFileHelper::LoadFileToString(FileContents, *CacheFilePath))
		{
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContents);
			FJsonSerializer::Deserialize(Reader, PersistentDataObject);
		}
		else
		{
			UE_LOG(LogGorgeousStartupHook, Log, TEXT("No cache file found, skipping mismatch check."));
			return;
		}

		if (!PersistentDataObject.IsValid())
		{
			UE_LOG(LogGorgeousStartupHook, Warning, TEXT("Failed to parse GorgeousPersistentData.json, skipping."));
			return;
		}

		// --- 3. Parse PluginUpdateCache entries ---
		TMap<FString, FString> MinimumCoreRequirements;
		const TArray<TSharedPtr<FJsonValue>>* CacheArray = nullptr;
		if (PersistentDataObject->TryGetArrayField(TEXT("PluginUpdateCache"), CacheArray) && CacheArray)
		{
			for (const TSharedPtr<FJsonValue>& Value : *CacheArray)
			{
				const TSharedPtr<FJsonObject> Obj = Value->AsObject();
				if (Obj.IsValid())
				{
					const FString PluginName = Obj->GetStringField(TEXT("PluginName"));
					const FString MinCoreVer = Obj->GetStringField(TEXT("MinimumCoreVersion"));
					MinimumCoreRequirements.Add(PluginName, MinCoreVer);
					UE_LOG(LogGorgeousStartupHook, Log, TEXT("Cache: %s requires core %s"), *PluginName, *MinCoreVer);
				}
			}
		}

		// --- 4. Walk discovered plugins and compare against current core version ---
		bool bMismatchDetected = false;
		FString MismatchPlugin;
		FString RequiredVersion;

		for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetDiscoveredPlugins())
		{
			const FString PluginName = Plugin->GetName();
			if (const FString* RequiredVer = MinimumCoreRequirements.Find(PluginName))
			{
				if (CurrentCoreVersion != *RequiredVer)
				{
					bMismatchDetected = true;
					MismatchPlugin    = PluginName;
					RequiredVersion   = *RequiredVer;
					UE_LOG(LogGorgeousStartupHook, Warning,
						TEXT("MISMATCH: '%s' needs core '%s' but '%s' is installed."),
						*PluginName, **RequiredVer, *CurrentCoreVersion);
					break;
				}
			}
		}

		if (!bMismatchDetected)
		{
			UE_LOG(LogGorgeousStartupHook, Log, TEXT("All plugins compatible with core %s. No action needed."), *CurrentCoreVersion);
			return;
		}

		// --- 5. Verify against cached binary checksum ---
		TSharedPtr<FJsonObject> ChecksumsObj;
		if (PersistentDataObject.IsValid() && PersistentDataObject->HasField(TEXT("VerifiedBinaryChecksums")))
		{
			ChecksumsObj = PersistentDataObject->GetObjectField(TEXT("VerifiedBinaryChecksums"));
			if (ChecksumsObj.IsValid() && ChecksumsObj->HasField(TEXT("GorgeousCore")))
			{
				const FString SavedChecksum = ChecksumsObj->GetStringField(TEXT("GorgeousCore"));
				
				const FString BinariesDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(
					CorePlugin->GetBaseDir(),
					TEXT("Binaries"),
					FPlatformProcess::GetBinariesSubdirectory()
				));

				TArray<FString> FoundFiles;
				// Only hash the actual compiled module binaries (.so / .dll).
				// Deliberately exclude: gorgeous-installer, .desktop, .sym, .debug,
				// .modules and any other auxiliary files so the checksum is stable
				// across installer updates, debug info changes, etc.
#if PLATFORM_WINDOWS
				IFileManager::Get().FindFilesRecursive(FoundFiles, *BinariesDir, TEXT("*.dll"), true, false);
#else
				IFileManager::Get().FindFilesRecursive(FoundFiles, *BinariesDir, TEXT("*.so"), true, false);
#endif
				FoundFiles.Sort();

				FString CombinedHashes;
				for (const FString& FilePath : FoundFiles)
				{
					FMD5Hash FileHash = FMD5Hash::HashFile(*FilePath);
					CombinedHashes += LexToString(FileHash);
				}

				FMD5 FinalMD5;
				FinalMD5.Update((const uint8*)TCHAR_TO_UTF8(*CombinedHashes), CombinedHashes.Len());
				FMD5Hash FinalHash;
				FinalHash.Set(FinalMD5);
				const FString CurrentChecksum = LexToString(FinalHash);

				if (CurrentChecksum == SavedChecksum)
				{
					UE_LOG(LogGorgeousStartupHook, Log, TEXT("Current binaries exactly match verified checksum for this version mismatch. Bypassing installer."));
					return;
				}
				
				UE_LOG(LogGorgeousStartupHook, Log, TEXT("Binary checksum mismatch. Expecting %s, got %s. Launching installer."), *SavedChecksum, *CurrentChecksum);
			}
		}

		// --- 6. Mismatch! Update splash and block RIGHT HERE ---
		// We are still at PostConfigInit, GorgeousCoreRuntime has NOT loaded yet.
		// Blocking here prevents any subsequent module from loading, enabling a clean binary swap.
		FPlatformSplash::SetSplashText(SplashTextType::StartupProgress,
			TEXT("Gorgeous Installer is resolving binary offset mismatches, please wait..."));

		// Build absolute installer path via GorgeousCore's actual base dir (dynamic, never hardcoded).
		const FString InstallerPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
			CorePlugin->GetBaseDir(),
			TEXT("Binaries"),
			FPlatformProcess::GetBinariesSubdirectory(),
#if PLATFORM_WINDOWS
			TEXT("gorgeous-installer.exe")
#else
			TEXT("gorgeous-installer")
#endif
		));

		// Project file must also be absolute so the installer can find it from its own working dir.
		const FString ProjectFilePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(
			*FPaths::GetProjectFilePath()
		);

		UE_LOG(LogGorgeousStartupHook, Log, TEXT("InstallerPath (abs): %s"), *InstallerPath);
		UE_LOG(LogGorgeousStartupHook, Log, TEXT("ProjectFilePath (abs): %s"), *ProjectFilePath);

		if (!FPaths::FileExists(InstallerPath))
		{
			UE_LOG(LogGorgeousStartupHook, Error, TEXT("gorgeous-installer not found at: %s, cannot resolve mismatch."), *InstallerPath);
			// Let the editor continue anyway, a missing installer shouldn't hard-block startup.
			return;
		}

		const FString Params = FString::Printf(
			TEXT("--verify-compatibility --project \"%s\""),
			*ProjectFilePath
		);

		UE_LOG(LogGorgeousStartupHook, Log, TEXT("Launching: %s %s"), *InstallerPath, *Params);

		// Launch the installer as a visible child process and BLOCK this thread.
		// Because we are inside StartupModule() at PostConfigInit, Unreal's module
		// loader cannot proceed to PreDefault (GorgeousCoreRuntime) until we return.
		// This gives gorgeous-installer the window to recompile binaries before they are loaded.
		uint32 ProcessID = 0;
		FProcHandle Handle = FPlatformProcess::CreateProc(
			*InstallerPath,
			*Params,
			/*bLaunchDetached=*/false,
			/*bLaunchHidden=*/false,
			/*bLaunchReallyHidden=*/false,
			&ProcessID,
			0,
			nullptr,
			nullptr
		);

		if (Handle.IsValid())
		{
			UE_LOG(LogGorgeousStartupHook, Log, TEXT("Installer running (PID %u), blocking module load until complete..."), ProcessID);
			FPlatformProcess::WaitForProc(Handle);
			
			int32 ReturnCode = 0;
			FPlatformProcess::GetProcReturnCode(Handle, &ReturnCode);
			FPlatformProcess::CloseProc(Handle);
			
			UE_LOG(LogGorgeousStartupHook, Log, TEXT("Installer exited with code %d."), ReturnCode);
			
			if (ReturnCode != 0)
			{
				UE_LOG(LogGorgeousStartupHook, Fatal, TEXT("Gorgeous Installer failed with return code %d. Aborting engine startup to prevent running with mismatched binaries."), ReturnCode);
			}
			else
			{
				UE_LOG(LogGorgeousStartupHook, Log, TEXT("Installer succeeded. Resuming module loading."));
			}
		}
		else
		{
			UE_LOG(LogGorgeousStartupHook, Error,
				TEXT("CreateProc failed for gorgeous-installer.\nPath: %s\nParams: %s"),
				*InstallerPath, *Params);
		}
	}

	virtual void ShutdownModule() override
	{
	}
};

IMPLEMENT_MODULE(FGorgeousCoreStartupHookModule, GorgeousCoreStartupHook)
