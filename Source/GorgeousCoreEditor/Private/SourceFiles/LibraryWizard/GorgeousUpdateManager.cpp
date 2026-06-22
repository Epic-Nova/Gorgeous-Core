#include "LibraryWizard/GorgeousUpdateManager.h"
#include "LibraryWizard/GorgeousLibrarySettings.h"

#include "HttpModule.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/SecureHash.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Helpers/Macros/GorgeousConnectionHelperMacros.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Helpers/GorgeousLoggingHelper.h"
#include "Helpers/GorgeousPluginHelper.h"
#include "Interfaces/IHttpResponse.h"

void UGorgeousUpdateManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ProbeConnection();
}

void UGorgeousUpdateManager::ProbeConnection()
{
    if (UGorgeousPluginHelper* Helper = UGorgeousPluginHelper::GetSingleton())
    {
        if (Helper->GetForceDevMode())
        {
            bIsDevMode = true;
            GT_W_LOG_FULL_EX(
                TEXT("Gorgeous Dev Mode Activated"),
                TEXT("Operating in HTTP is dangerous."),
                GT_DURATION,
                true, true, true, true, nullptr, nullptr
            );
            FetchSystemsCatalog();
            return;
        }
    }

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UGorgeousUpdateManager::OnProbeConnectionResponse);
    Request->SetURL(TEXT(GORGEOUS_API_ENDPOINT));
    Request->SetVerb("GET");
    Request->ProcessRequest();
}

void UGorgeousUpdateManager::OnProbeConnectionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    FString RequestURL = Request->GetURL();

    if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
    {
        if (RequestURL.StartsWith(TEXT("http://")))
        {
            bIsDevMode = true;
            GT_W_LOG_FULL_EX(
                TEXT("Gorgeous Dev Mode Activated"),
                TEXT("Operating in HTTP is dangerous."),
                GT_DURATION,
                true,
                true,
                true,
                true,
                nullptr,
                nullptr
            );
        }
        else
        {
            bIsDevMode = false;
        }

        FetchSystemsCatalog();
    }
    else
    {
        if (RequestURL.StartsWith(TEXT("https://")))
        {
            // Fallback to HTTP probe
            FString FallbackURL = RequestURL.Replace(TEXT("https://"), TEXT("http://"));
            TSharedRef<IHttpRequest, ESPMode::ThreadSafe> FallbackRequest = FHttpModule::Get().CreateRequest();
            FallbackRequest->OnProcessRequestComplete().BindUObject(this, &UGorgeousUpdateManager::OnProbeConnectionResponse);
            FallbackRequest->SetURL(FallbackURL);
            FallbackRequest->SetVerb(TEXT("GET"));
            FallbackRequest->ProcessRequest();
        }
    }
}

void UGorgeousUpdateManager::Deinitialize()
{
    Super::Deinitialize();
}

FString UGorgeousUpdateManager::HashDirectory(const FString& DirectoryPath)
{
    TArray<FString> Files;
    IFileManager::Get().FindFilesRecursive(Files, *DirectoryPath, TEXT("*.h"), true, false, false);
    
    // Sort files to ensure deterministic hashing
    Files.Sort();

    FMD5 MD5Gen;
    for (const FString& FilePath : Files)
    {
        FString FileContent;
        if (FFileHelper::LoadFileToString(FileContent, *FilePath))
        {
            // Normalize path for consistent hashing across platforms
            FString RelPath = FilePath.RightChop(DirectoryPath.Len() + 1);
            RelPath.ReplaceInline(TEXT("\\"), TEXT("/"));
            
            MD5Gen.Update((const uint8*)TCHAR_TO_UTF8(*RelPath), RelPath.Len());
            MD5Gen.Update((const uint8*)TCHAR_TO_UTF8(*FileContent), FileContent.Len());
        }
    }

    FMD5Hash Hash;
    Hash.Set(MD5Gen);
    return LexToString(Hash);
}

FString UGorgeousUpdateManager::CalculatePluginModuleCoreHash(const FString& PluginName, const FString& PluginBaseDir)
{
    FString SourceDir = FPaths::Combine(PluginBaseDir, TEXT("Source"));
    
    TArray<FString> ModuleDirs;
    // Find all module directories inside Source/
    IFileManager::Get().FindFiles(ModuleDirs, *(SourceDir / TEXT("*")), false, true);

    for (const FString& ModuleDirName : ModuleDirs)
    {
        FString ModuleCorePath = FPaths::Combine(SourceDir, ModuleDirName, TEXT("Public"), TEXT("ModuleCore"));
        if (IFileManager::Get().DirectoryExists(*ModuleCorePath))
        {
            return HashDirectory(ModuleCorePath);
        }
    }
    
    return TEXT("");
}

FString UGorgeousUpdateManager::CalculatePluginChecksum(const FString& PluginName, const FString& PluginBaseDir)
{ 
    // Calculate the plugin checksum as per api spec hashing the WHOLE plugin source, excpluding systems that are per SystemManifest.json not a core system
    
    FString SourceDir = FPaths::Combine(PluginBaseDir, TEXT("Source"));
    TArray<FString> ModuleDirs;
    
    IFileManager::Get().FindFiles(ModuleDirs, *SourceDir, false, true);
    
    TArray<FString> FinalFileHashes;
    
    for (const FString& ModuleDirName : ModuleDirs)
    {
        TArray<FString> SystemManifestPaths;
        IFileManager::Get().FindFilesRecursive(SystemManifestPaths, *(SourceDir / ModuleDirName), TEXT("SystemManifest.json"), true, false, false);
        
        TArray<FString> ExcludePaths;
        for (const FString& ManifestPath : SystemManifestPaths)
        {
            if (FString ManifestContent; FFileHelper::LoadFileToString(ManifestContent, *ManifestPath))
            {
                TSharedPtr<FJsonObject> ManifestJson;

                if (TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ManifestContent); FJsonSerializer::Deserialize(Reader, ManifestJson) && ManifestJson.IsValid())
                {
                    bool bIsCoreSystem = false;
                    if (ManifestJson->TryGetBoolField(TEXT("bIsCoreSystem"), bIsCoreSystem) && !bIsCoreSystem)
                    {
                        const TArray<TSharedPtr<FJsonValue>>* PayloadPaths;
                        if (ManifestJson->TryGetArrayField(TEXT("PayloadPaths"), PayloadPaths))
                        {
                            for (const TSharedPtr<FJsonValue>& Value : *PayloadPaths)
                            {
                                FString Path = Value->AsString();
                                ExcludePaths.Add(Path);
                            }
                        }
                    }
                }
            }
        }
         
        TArray<FString> AllFilesToHash;
        TArray<FString> FilesToExclude;
        
        IFileManager::Get().FindFilesRecursive(AllFilesToHash, *(SourceDir / ModuleDirName), TEXT("*.*"), true, false, false); 
        
        for (const FString& File : AllFilesToHash)
        {
            bool bShouldExclude = false;
            for (const FString& ExcludePath : ExcludePaths)
            {
                if (File.Contains(ExcludePath))
                {
                    bShouldExclude = true;
                    break;
                }
            }

            if (!bShouldExclude)
            {
                FilesToExclude.Add(File);
            }
        }
        TArray<FString> FinalFilesToHash = AllFilesToHash.FilterByPredicate([&](const FString& File)
        {
            return !FilesToExclude.Contains(File);
        });
        
        FinalFilesToHash.Sort();
        
        // Now SHA256 every FinalFilesToHash content
        for (const FString& File : FinalFilesToHash)
        {
            IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
            PlatformFile.OpenRead(*File);
            
            if (TUniquePtr<FArchive> FileReader(IFileManager::Get().CreateFileReader(*File)); FileReader)            
            {
                const int64 FileSize = FileReader->TotalSize();
                TArray<uint8> FileData;
                
                FileData.SetNumUninitialized(FileSize);
                FileReader->Serialize(FileData.GetData(), FileSize);

                FSHA256Signature Signature;
                if (FPlatformMisc::GetSHA256Signature(FileData.GetData(), FileSize, Signature))
                {
                    FinalFileHashes.Add(Signature.ToString().ToLower());
                }
            }
        }
    }
    
    // Now hash all hashes to a single hash
    FString CombinedHashes;
    for (const FString& Hash : FinalFileHashes)
    {
        CombinedHashes += Hash;
    }
    
    FTCHARToUTF8 Utf8String(*CombinedHashes);
    FSHA256Signature FinalSignature;
    if (FPlatformMisc::GetSHA256Signature(Utf8String.Get(), Utf8String.Length(), FinalSignature))
    {
        return FinalSignature.ToString().ToLower();
    }
    return TEXT("");
        
}

void UGorgeousUpdateManager::CheckForUpdates()
{
    TArray<TSharedRef<IPlugin>> EnabledPlugins = IPluginManager::Get().GetEnabledPlugins();
    TArray<TSharedPtr<FJsonValue>> PluginJsonArray;

    for (const TSharedRef<IPlugin>& Plugin : EnabledPlugins)
    {
        FString PluginName = Plugin->GetName();
        
        // Include Gorgeous plugins
        if (PluginName.StartsWith(TEXT("Gorgeous"), ESearchCase::IgnoreCase))
        {
            FString Hash = CalculatePluginChecksum(PluginName, Plugin->GetBaseDir());
            
            TSharedPtr<FJsonObject> PluginObj = MakeShareable(new FJsonObject());
            PluginObj->SetStringField(TEXT("PluginName"), PluginName);
            PluginObj->SetStringField(TEXT("Checksum"), Hash);
            PluginObj->SetStringField(TEXT("CurrentVersion"), Plugin->GetDescriptor().VersionName);

            PluginJsonArray.Add(MakeShareable(new FJsonValueObject(PluginObj)));
        }
    }

    TSharedPtr<FJsonObject> PayloadObj = MakeShareable(new FJsonObject());
    PayloadObj->SetArrayField(TEXT("Plugins"), PluginJsonArray);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(PayloadObj.ToSharedRef(), Writer);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UGorgeousUpdateManager::OnUpdateCheckResponse);
    FString Endpoint = bIsDevMode ? FString(TEXT("http://api.gorgeous.simsalabim.studio/api/v1/plugins/update-check")) : FString(TEXT(GORGEOUS_API_V1_ENDPOINT) TEXT("/plugins/update-check"));
    Request->SetURL(Endpoint);
    Request->SetVerb("POST");
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(OutputString);
    Request->ProcessRequest();
}

void UGorgeousUpdateManager::OnUpdateCheckResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	bool bUpdatesAvailable = false;

	if (bWasSuccessful && Response.IsValid())
	{
		if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
		{
			GT_I_LOG("GT.Updates", TEXT("Successfully checked for Gorgeous Plugin updates."));
			
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
			
			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				// Core version check
				FString CurrentCoreVersion = TEXT("0.0.0"); // Fallback
				TArray<FGorgeousPluginUpdateCacheEntry> PluginCache;
				// Since it's a map of PluginName -> UpdateData
				for (auto& Elem : JsonObject->Values)
				{
					TSharedPtr<FJsonObject> PluginData = Elem.Value->AsObject();
					if (PluginData.IsValid())
					{
						bool bAvail = false;
						if (PluginData->TryGetBoolField(TEXT("UpdateAvailable"), bAvail) && bAvail)
						{
							FString MinCoreVer;
							if (PluginData->TryGetStringField(TEXT("MinimumCoreVersion"), MinCoreVer) && !MinCoreVer.IsEmpty())
							{
								// A simplistic version check. In a real scenario you'd compare segments.
								if (MinCoreVer > CurrentCoreVersion)
								{
									// Block update
									FNotificationInfo Info(FText::FromString(FString::Printf(TEXT("Update blocked for %s: Minimum Core Version %s required."), *Elem.Key, *MinCoreVer)));
									Info.ExpireDuration = 5.0f;
									FSlateNotificationManager::Get().AddNotification(Info);
									continue;
								}
							}
							
							bUpdatesAvailable = true;
							
							FGorgeousPluginUpdateCacheEntry Entry;
							Entry.PluginName = Elem.Key;
							Entry.AvailableVersion = PluginData->GetStringField(TEXT("LatestVersion"));
							Entry.MinimumCoreVersion = MinCoreVer;
							Entry.ChangelogUrl = PluginData->GetStringField(TEXT("ChangelogUrl"));
							Entry.DownloadToken = PluginData->GetStringField(TEXT("DownloadToken"));
							PluginCache.Add(Entry);
						}
					}
				}

				if (UGorgeousPluginHelper* Helper = UGorgeousPluginHelper::GetSingleton())
				{
					Helper->SetPluginUpdateCache(PluginCache);
				}
			}
		}
		else
		{
			GT_W_LOG("GT.Updates", TEXT("Failed to check for updates. Response code: %d"), Response->GetResponseCode());
		}
	}
	else
	{
		GT_E_LOG("GT.Updates", TEXT("Failed to connect to Gorgeous API."));
	}

	OnUpdateCheckCompleted.Broadcast(bUpdatesAvailable);
}

void UGorgeousUpdateManager::DownloadPluginUpdate(const FString& PluginName, const FString& DownloadToken)
{
	if (DownloadToken.IsEmpty())
	{
		GT_E_LOG("GT.Updates", TEXT("Download token is empty for plugin %s"), *PluginName);
		return;
	}

	FString Endpoint = bIsDevMode 
		? FString::Printf(TEXT("http://api.gorgeous.simsalabim.studio/api/v1/downloads/%s"), *DownloadToken)
		: FString::Printf(TEXT(GORGEOUS_API_V1_ENDPOINT) TEXT("/downloads/%s"), *DownloadToken);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UGorgeousUpdateManager::OnDownloadPluginUpdateResponse, PluginName);
	Request->SetURL(Endpoint);
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("Accept"), TEXT("application/zip"));
	Request->ProcessRequest();
}

void UGorgeousUpdateManager::OnDownloadPluginUpdateResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString PluginName)
{
	if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		GT_E_LOG("GT.Updates", TEXT("Failed to download update for plugin %s. Response code: %d"), *PluginName, Response.IsValid() ? Response->GetResponseCode() : 0);
		return;
	}

	// Save the zip payload to <ProjectDir>/Saved/Temp/Update_{PluginName}.zip
	const FString ZipPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Temp") / FString::Printf(TEXT("Update_%s.zip"), *PluginName));
	const FString Directory = FPaths::GetPath(ZipPath);
	IFileManager::Get().MakeDirectory(*Directory, true);

	const TArray<uint8>& Content = Response->GetContent();
	if (!FFileHelper::SaveArrayToFile(Content, *ZipPath))
	{
		GT_E_LOG("GT.Updates", TEXT("Failed to save update zip to %s"), *ZipPath);
		return;
	}

	GT_I_LOG("GT.Updates", TEXT("Saved plugin update zip to %s"), *ZipPath);

	// Determine the gorgeous-installer executable path
	const FString InstallerPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Plugins/Gorgeous-Core/Binaries") / FPlatformProcess::GetBinariesSubdirectory() / TEXT("gorgeous-installer"));
	FString FullInstallerPath = InstallerPath;
#if PLATFORM_WINDOWS
	FullInstallerPath += TEXT(".exe");
#endif

	FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
	uint32 EditorProcessID = FPlatformProcess::GetCurrentProcessId();

	// Launch the installer detached using FPlatformProcess::CreateProc
	uint32 InstallerProcessID = 0;
	FString Params = FString::Printf(TEXT("--cli --install-zip \"%s\" --project \"%s\" --wait-for-pid %u --reopen-project"), *ZipPath, *ProjectPath, EditorProcessID);

	FProcHandle ProcHandle = FPlatformProcess::CreateProc(*FullInstallerPath, *Params, true, false, false, &InstallerProcessID, 0, nullptr, nullptr);
	if (ProcHandle.IsValid())
	{
		GT_I_LOG("GT.Updates", TEXT("Launched gorgeous-installer with PID %u for plugin update"), InstallerProcessID);
		FPlatformMisc::RequestExit(false);
	}
	else
	{
		GT_E_LOG("GT.Updates", TEXT("Failed to launch gorgeous-installer at %s"), *FullInstallerPath);
	}
}

void UGorgeousUpdateManager::FetchSystemsCatalog()
{
    FString Endpoint = bIsDevMode ? FString(TEXT("http://api.gorgeous.simsalabim.studio/api/v1/systems")) : FString(TEXT(GORGEOUS_API_V1_ENDPOINT) TEXT("/systems"));
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UGorgeousUpdateManager::OnFetchSystemsCatalogResponse);
    Request->SetURL(Endpoint);
    Request->SetVerb("GET");
    Request->ProcessRequest();
}

void UGorgeousUpdateManager::OnFetchSystemsCatalogResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
        
        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            const TArray<TSharedPtr<FJsonValue>>* OfflineCacheArray;
            if (JsonObject->TryGetArrayField(TEXT("OfflineSystemCache"), OfflineCacheArray))
            {
                if (UGorgeousPluginHelper* Helper = UGorgeousPluginHelper::GetSingleton())
                {
                    TArray<FGorgeousOfflineSystemCacheEntry> ExistingCache = Helper->GetOfflineSystemCache();
                    TMap<FString, FGorgeousOfflineSystemCacheEntry> MergedCacheMap;
                    
                    // Retain existing core systems from cache
                    for (const FGorgeousOfflineSystemCacheEntry& Existing : ExistingCache)
                    {
                        if (Existing.bIsCoreSystem)
                        {
                            MergedCacheMap.Add(Existing.SystemId, Existing);
                        }
                    }

                    // Parse the API results and merge
                    for (const TSharedPtr<FJsonValue>& Value : *OfflineCacheArray)
                    {
                        TSharedPtr<FJsonObject> Obj = Value->AsObject();
                        if (Obj.IsValid())
                        {
                            FString SysId = Obj->GetStringField(TEXT("SystemId"));
                            FGorgeousOfflineSystemCacheEntry Entry;
                            
                            // Inherit properties like bIsCoreSystem from existing cache if it exists
                            if (MergedCacheMap.Contains(SysId))
                            {
                                Entry = MergedCacheMap[SysId];
                            }
                            
                            Entry.SystemId = SysId;
                            Entry.TargetPluginName = Obj->GetStringField(TEXT("TargetPluginName"));
                            Entry.DisplayName = Obj->GetStringField(TEXT("DisplayName"));
                            Entry.Description = Obj->GetStringField(TEXT("Description"));
                            Entry.Version = Obj->GetStringField(TEXT("Version"));
                            Entry.DownloadUrl = Obj->GetStringField(TEXT("DownloadUrl"));
                            
                            FString ChangelogUrl;
                            if (Obj->TryGetStringField(TEXT("ChangelogUrl"), ChangelogUrl)) { Entry.ChangelogUrl = ChangelogUrl; }

                            Entry.SourcePaths.Empty();
                            const TArray<TSharedPtr<FJsonValue>>* SourcePaths;
                            if (Obj->TryGetArrayField(TEXT("SourcePaths"), SourcePaths))
                            {
                                for (const TSharedPtr<FJsonValue>& PathVal : *SourcePaths) { Entry.SourcePaths.Add(PathVal->AsString()); }
                            }
                            
                            Entry.ContentPaths.Empty();
                            const TArray<TSharedPtr<FJsonValue>>* ContentPaths;
                            if (Obj->TryGetArrayField(TEXT("ContentPaths"), ContentPaths))
                            {
                                for (const TSharedPtr<FJsonValue>& PathVal : *ContentPaths) { Entry.ContentPaths.Add(PathVal->AsString()); }
                            }
                            
                            MergedCacheMap.Add(SysId, Entry);
                        }
                    }
                    
                    TArray<FGorgeousOfflineSystemCacheEntry> FinalCache;
                    MergedCacheMap.GenerateValueArray(FinalCache);
                    Helper->SetOfflineSystemCache(FinalCache);
                }
            }
        }
        GT_I_LOG("GT.Updates", TEXT("Successfully fetched and cached Gorgeous Systems Catalog."));

        // Automatically chain into plugin update check so PluginUpdateCache
        // is populated on every editor startup (not just on manual refresh).
        CheckForUpdates();
    }
    else
    {
        GT_E_LOG("GT.Updates", TEXT("Failed to fetch Gorgeous Systems Catalog."));
    }
}
