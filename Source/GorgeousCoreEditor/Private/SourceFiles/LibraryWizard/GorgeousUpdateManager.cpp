#include "LibraryWizard/GorgeousUpdateManager.h"

#include "LibraryWizard/GorgeousLibrarySettings.h"

#include "HttpModule.h"
#include "LibraryWizard/GorgeousNetworkConsent.h"
#include "NudgeHelper/GorgeousNudgeSubsystem.h"
#include "NudgeHelper/GorgeousNudgeDeveloperSettings.h"
#include "Editor.h"
#include "LibraryWizard/GorgeousReleaseVerification.h"
#include "FileHelpers.h"
#include "Widgets/Input/SButton.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Misc/MessageDialog.h"
#include "Misc/ScopeExit.h"
#include "Algo/AllOf.h"
#include "Helpers/GorgeousVersionComparison.h"
#include "IGorgeousThingsModuleInterface.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/SecureHash.h"
#include "Misc/Guid.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "LibraryWizard/GorgeousApiEndpoints.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Helpers/GorgeousLoggingHelper.h"
#include "Helpers/GorgeousPluginHelper.h"
#include "Helpers/GorgeousPersistentData.h"
#include "Interfaces/IHttpResponse.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SWindow.h"
#include "openssl/sha.h"

THIRD_PARTY_INCLUDES_START
#include "openssl/sha.h"
THIRD_PARTY_INCLUDES_END

void UGorgeousUpdateManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    GorgeousNetworkConsent::ScheduleStartupPrompt(FSimpleDelegate::CreateWeakLambda(this, [this]
    {
        ProbeConnection();
        if (GEditor && UGorgeousNudgeDeveloperSettings::Get()->bEnableAutomaticNudges)
        {
            if (auto* Nudges = GEditor->GetEditorSubsystem<UGorgeousNudgeSubsystem>()) Nudges->CheckForNudgesNow();
        }
    }));
    ProbeConnection();
}

void UGorgeousUpdateManager::ProbeConnection()
{
    if (!GorgeousNetworkConsent::IsAllowed()) return;

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UGorgeousUpdateManager::OnProbeConnectionResponse);
    Request->SetURL(GorgeousApiEndpoints::GetEndpoint());
    Request->SetVerb("GET");
    Request->ProcessRequest();
}

void UGorgeousUpdateManager::OnProbeConnectionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!GorgeousNetworkConsent::IsAllowed()) return;
    if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode())
        || !GorgeousApiEndpoints::IsAllowedURL(Response->GetEffectiveURL())) return;

    if (Request->GetURL().StartsWith(TEXT("http://"), ESearchCase::IgnoreCase))
    {
        GT_W_LOG("GT.DevelopmentMode.Activated", TEXT("Gorgeous API development origin uses HTTP."));
    }
    FetchSystemsCatalog();
}

FString UGorgeousUpdateManager::HashDirectory(const FString& DirectoryPath)
{
    TArray<FString> Files;
    IFileManager::Get().FindFilesRecursive(Files, *DirectoryPath, TEXT("*.h"), true, false, false);
    
    Files.Sort();

    FMD5 MD5Gen;
    for (const FString& FilePath : Files)
    {
        FString FileContent;
        if (FFileHelper::LoadFileToString(FileContent, *FilePath))
        {
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
    FString SourceDir = FPaths::Combine(PluginBaseDir, TEXT("Source"));
    TArray<FString> ModuleDirs;
    
    IFileManager::Get().FindFiles(ModuleDirs, *(SourceDir / TEXT("*")), false, true);
    
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

            if (bShouldExclude)
            {
                FilesToExclude.Add(File);
            }
        }
        TArray<FString> FinalFilesToHash = AllFilesToHash.FilterByPredicate([&](const FString& File)
        {
            return !FilesToExclude.Contains(File);
        });
        
        FinalFilesToHash.Sort();
        
        for (const FString& File : FinalFilesToHash)
        {
            IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

            
            if (TUniquePtr<FArchive> FileReader(IFileManager::Get().CreateFileReader(*File)); FileReader)            
            {
                const int64 FileSize = FileReader->TotalSize();
                TArray<uint8> FileData;
                
                FileData.SetNumUninitialized(FileSize);
                FileReader->Serialize(FileData.GetData(), FileSize);
                
                SHA256_CTX FileHash;
                SHA256_Init(&FileHash);
                SHA256_Update(&FileHash, FileData.GetData(), FileSize);
                
                FSHA256Signature HashDigest;
                SHA256_Final(HashDigest.Signature, &FileHash);
                FinalFileHashes.Add(HashDigest.ToString().ToLower());
            }
        }
    }
    
    FString CombinedHashes;
    for (const FString& Hash : FinalFileHashes)
    {
        CombinedHashes += Hash;
    }
    
    FTCHARToUTF8 Utf8String(*CombinedHashes);
    
    SHA256_CTX FinalHash;
    SHA256_Init(&FinalHash);
    SHA256_Update(&FinalHash, (const uint8*)Utf8String.Get(), Utf8String.Length());
    
    FSHA256Signature FinalDigest;
    SHA256_Final(FinalDigest.Signature, &FinalHash);
    return FinalDigest.ToString().ToLower();
}

void UGorgeousUpdateManager::Deinitialize()
{
    if (ActiveDownloadRequest.IsValid()) { ActiveDownloadRequest->OnProcessRequestComplete().Unbind(); ActiveDownloadRequest->OnRequestProgress64().Unbind(); ActiveDownloadRequest->CancelRequest(); ActiveDownloadRequest.Reset(); }
    Super::Deinitialize();
    if (ActiveProgressWindow.IsValid())
    {
        ActiveProgressWindow->RequestDestroyWindow();
        ActiveProgressWindow.Reset();
    }
    ActiveProgressBar.Reset();
    ActiveProgressStatusText.Reset();
}

TSharedRef<FJsonObject> UGorgeousUpdateManager::BuildPluginUpdateEntry(const FString& PluginName,
    const FString& PluginBaseDir, const FString& CurrentVersion)
{
    const FString BaseFootprint = CalculatePluginModuleCoreHash(PluginName, PluginBaseDir);
    const FString Salt = FGuid::NewGuid().ToString(EGuidFormats::Digits);
    const FString SaltedFootprint = BaseFootprint + Salt;
    const FTCHARToUTF8 Utf8Footprint(*SaltedFootprint);
    const FString LicenseProof = FMD5::HashBytes(reinterpret_cast<const uint8*>(Utf8Footprint.Get()), Utf8Footprint.Length());

    const TSharedRef<FJsonObject> Entry = MakeShared<FJsonObject>();
    Entry->SetStringField(TEXT("PluginName"), PluginName);
    Entry->SetStringField(TEXT("Checksum"), LicenseProof);
    Entry->SetStringField(TEXT("Salt"), Salt);
    // Source fingerprints are diagnostic metadata, not the license or archive hash.
    Entry->SetStringField(TEXT("SourceChecksumSha256"), CalculatePluginChecksum(PluginName, PluginBaseDir));
    Entry->SetStringField(TEXT("CurrentVersion"), CurrentVersion);
    return Entry;
}

void UGorgeousUpdateManager::CheckForUpdates()
{
    if (!GorgeousNetworkConsent::IsAllowed(true)) return;
    TArray<TSharedRef<IPlugin>> EnabledPlugins = IPluginManager::Get().GetEnabledPlugins();
    TArray<TSharedPtr<FJsonValue>> PluginJsonArray;

    for (const TSharedRef<IPlugin>& Plugin : EnabledPlugins)
    {
        FString PluginName = Plugin->GetName();

        // Include the plugin itself and all Gorgeous plugins
        if (PluginName.StartsWith(TEXT("Gorgeous"), ESearchCase::IgnoreCase) || PluginName == TEXT("GorgeousCore"))
        {
            PluginJsonArray.Add(MakeShared<FJsonValueObject>(
                BuildPluginUpdateEntry(PluginName, Plugin->GetBaseDir(), Plugin->GetDescriptor().VersionName)));
        }
    }

    TSharedPtr<FJsonObject> PayloadObj = MakeShareable(new FJsonObject());
    PayloadObj->SetArrayField(TEXT("Plugins"), PluginJsonArray);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(PayloadObj.ToSharedRef(), Writer);

    const FString Endpoint = GorgeousApiEndpoints::GetV1Endpoint(TEXT("/plugins/update-check"));
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UGorgeousUpdateManager::OnUpdateCheckResponse);
    Request->SetURL(Endpoint);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(OutputString);
    Request->ProcessRequest();
}

void UGorgeousUpdateManager::OnUpdateCheckResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!GorgeousNetworkConsent::IsAllowed()) return;
    bool bUpdatesAvailable = false;

    if (bWasSuccessful && Response.IsValid() && GorgeousApiEndpoints::IsAllowedURL(Response->GetEffectiveURL()))
    {
        if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
        {
            GT_I_LOG("GT.Updates", TEXT("Successfully checked for Gorgeous Plugin updates."));
            
            FString CurrentCoreVersion = TEXT("0.0.0");
            if (TSharedPtr<IPlugin> CorePlugin = IPluginManager::Get().FindPlugin(TEXT("GorgeousCore")); CorePlugin.IsValid())
            {
                CurrentCoreVersion = CorePlugin->GetDescriptor().VersionName;
            }
            
            TSharedPtr<FJsonObject> JsonObject;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
            
            if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
            {
                TArray<FGorgeousPluginUpdateCacheEntry> PluginCache;
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
                                if (GorgeousVersionComparison::IsOlder(CurrentCoreVersion, MinCoreVer))
                                {
                                    //@TODO: Replace with the gorgeous toast system
                                    FString ToastMsg = FString::Printf(TEXT("Update blocked for %s: Minimum Core Version %s required."), *Elem.Key, *MinCoreVer);
                                    GT_W_LOG("GT.Updates", TEXT("%s"), *ToastMsg);
                                    FNotificationInfo Info(FText::FromString(ToastMsg));
                                    Info.ExpireDuration = 5.0f;
                                    FSlateNotificationManager::Get().AddNotification(Info);
                                    continue;
                                }
                            }
                            
                            bUpdatesAvailable = true;
                            
                            FGorgeousPluginUpdateCacheEntry Entry;
                            Entry.PluginName = Elem.Key;
                            
                            FString LatestVer, ChangelogUrlStr;
                            if (PluginData->TryGetStringField(TEXT("LatestVersion"), LatestVer))
                            {
                                Entry.AvailableVersion = LatestVer;
                            }
                            Entry.MinimumCoreVersion = MinCoreVer;
                            if (PluginData->TryGetStringField(TEXT("ChangelogUrl"), ChangelogUrlStr) && ChangelogUrlStr != TEXT("null"))
                            {
                                Entry.ChangelogUrl = ChangelogUrlStr;
                            }
                            Entry.DownloadToken = PluginData->GetStringField(TEXT("DownloadToken"));
                            PluginData->TryGetStringField(TEXT("ChecksumSha256"), Entry.ChecksumSha256);
                            PluginData->TryGetStringField(TEXT("SignatureEd25519"), Entry.SignatureEd25519);
                            PluginCache.Add(Entry);
                            
                            GT_I_LOG("GT.Updates", TEXT("Update available for plugin '%s': latest=%s, minCore=%s"), 
                                *Elem.Key, *Entry.AvailableVersion, *Entry.MinimumCoreVersion);
                        }
                    }
                }

                FGorgeousPersistentData::Get().SetPluginUpdateCache(PluginCache);
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

void UGorgeousUpdateManager::FetchSystemsCatalog()
{
    if (!GorgeousNetworkConsent::IsAllowed(false)) return;
    const FString Endpoint = GorgeousApiEndpoints::GetV1Endpoint(TEXT("/systems"));
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UGorgeousUpdateManager::OnFetchSystemsCatalogResponse);
    Request->SetURL(Endpoint);
    Request->SetVerb("GET");
    Request->ProcessRequest();
}

void UGorgeousUpdateManager::OnFetchSystemsCatalogResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!GorgeousNetworkConsent::IsAllowed()) return;
    if (bWasSuccessful && Response.IsValid() && GorgeousApiEndpoints::IsAllowedURL(Response->GetEffectiveURL())
        && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
        
        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            const TArray<TSharedPtr<FJsonValue>>* OfflineCacheArray;
            if (JsonObject->TryGetArrayField(TEXT("OfflineSystemCache"), OfflineCacheArray))
            {
                TArray<FGorgeousOfflineSystemCacheEntry> ExistingCache = FGorgeousPersistentData::Get().GetOfflineSystemCache();
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
                    const TSharedPtr<FJsonObject> Obj = Value.IsValid() && Value->Type == EJson::Object ? Value->AsObject() : nullptr;
                    if (Obj.IsValid())
                    {
                        const auto ReadString = [&Obj](const TCHAR* FieldName)
                        {
                            FString Value;
                            Obj->TryGetStringField(FieldName, Value);
                            return Value;
                        };
                        const FString SysId = ReadString(TEXT("SystemId"));
                        if (SysId.IsEmpty()) continue;
                        FGorgeousOfflineSystemCacheEntry Entry;

                        // Inherit properties like bIsCoreSystem from existing cache if it exists
                        if (MergedCacheMap.Contains(SysId))
                        {
                            Entry = MergedCacheMap[SysId];
                        }

                        Entry.SystemId = SysId;
                        Entry.TargetPluginName = ReadString(TEXT("TargetPluginName"));
                        Entry.DisplayName = ReadString(TEXT("DisplayName"));
                        Entry.Description = ReadString(TEXT("Description"));
                        Entry.Version = ReadString(TEXT("Version"));
                        Entry.DownloadUrl = ReadString(TEXT("DownloadUrl"));
                        Entry.ChecksumSha256 = ReadString(TEXT("ChecksumSha256"));
                        Entry.SignatureEd25519 = ReadString(TEXT("SignatureEd25519"));
                        Entry.DownloadToken = ReadString(TEXT("DownloadToken"));

                        Entry.ChangelogUrl = ReadString(TEXT("ChangelogUrl"));

                        Entry.SourcePaths.Empty();
                        const TArray<TSharedPtr<FJsonValue>>* SourcePaths;
                        if (Obj->TryGetArrayField(TEXT("SourcePaths"), SourcePaths))
                        {
                            for (const TSharedPtr<FJsonValue>& PathVal : *SourcePaths) { if (PathVal.IsValid() && PathVal->Type == EJson::String) Entry.SourcePaths.Add(PathVal->AsString()); }
                        }

                        Entry.ContentPaths.Empty();
                        const TArray<TSharedPtr<FJsonValue>>* ContentPaths;
                        if (Obj->TryGetArrayField(TEXT("ContentPaths"), ContentPaths))
                        {
                            for (const TSharedPtr<FJsonValue>& PathVal : *ContentPaths) { if (PathVal.IsValid() && PathVal->Type == EJson::String) Entry.ContentPaths.Add(PathVal->AsString()); }
                        }

                        MergedCacheMap.Add(SysId, Entry);
                    }
                }

                TArray<FGorgeousOfflineSystemCacheEntry> FinalCache;
                MergedCacheMap.GenerateValueArray(FinalCache);
                FGorgeousPersistentData::Get().SetOfflineSystemCache(FinalCache);
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

void UGorgeousUpdateManager::DownloadPluginUpdate(const FString& PluginName, const FString& DownloadToken)
{
    if (!GorgeousNetworkConsent::IsAllowed(true)) return;
    if (DownloadToken.IsEmpty())
    {
        GT_E_LOG("GT.Updates", TEXT("Download token is empty for plugin %s"), *PluginName);
        return;
    }

    const FString URL = GorgeousApiEndpoints::GetV1Endpoint(TEXT("/downloads/") + FGenericPlatformHttp::UrlEncode(DownloadToken));
    for (const auto& Entry : FGorgeousPersistentData::Get().GetPluginUpdateCache())
    {
        if (Entry.PluginName==PluginName && Entry.DownloadToken==DownloadToken)
        {
            DownloadZipAndInstall(URL,PluginName,Entry.ChecksumSha256,DownloadToken,Entry.AvailableVersion,Entry.SignatureEd25519);
            return;
        }
    }
    GT_E_LOG("GT.Updates", TEXT("Download rejected: no matching signed catalog entry."));
}

void UGorgeousUpdateManager::DownloadZipAndInstall(const FString& URL, const FString& PluginName, const FString& ExpectedSHA256, const FString& Token, const FString& Version, const FString& SignatureEd25519)
{
    if (!GorgeousNetworkConsent::IsAllowed(true)) return;
    const bool bValidPluginName = !PluginName.IsEmpty() && Algo::AllOf(PluginName, [](TCHAR C) { return (C >= 'A' && C <= 'Z') || (C >= 'a' && C <= 'z') || (C >= '0' && C <= '9') || C=='_' || C=='-'; });
    if (!GorgeousReleaseVerification::VerifyWithPinnedKey(Token,Version,ExpectedSHA256,SignatureEd25519) ||
        !bValidPluginName || !GorgeousApiEndpoints::IsAllowedURL(URL) || ExpectedSHA256.Len()!=64 ||
        PluginName.IsEmpty() || PluginName.Contains(TEXT("/")) || PluginName.Contains(TEXT("\\")) || PluginName.Contains(TEXT(":")))
    {
        FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("GorgeousUpdates", "InvalidCatalog", "Download unavailable: configure the trusted release public key and use a valid signed catalog entry with an allowed URL and target plugin name."));
        return;
    }
    if (ActiveDownloadRequest.IsValid()) return;
    ActiveDownloadChecksum=ExpectedSHA256;
    FString WindowTitle = FString::Printf(TEXT("Downloading %s Update"), *PluginName);

    TSharedRef<SWindow> ProgressWindow = SNew(SWindow)
        .Title(FText::FromString(WindowTitle))
        .SizingRule(ESizingRule::FixedSize)
        .ClientSize(FVector2D(420.f, 125.f))
        .SupportsMinimize(false)
        .SupportsMaximize(false)
        .IsTopmostWindow(true)
        .AutoCenter(EAutoCenter::PreferredWorkArea)
        [
            SNew(SBorder)
            .Padding(20.f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                [
                    SAssignNew(ActiveProgressStatusText, STextBlock)
                    .Text(FText::FromString(TEXT("Initializing...")))
                ]
                + SVerticalBox::Slot()
                [
                    SAssignNew(ActiveProgressBar, SProgressBar)
                    .Percent(0.f)

                ]
                + SVerticalBox::Slot().AutoHeight()
                [ SNew(SButton).Text(NSLOCTEXT("GorgeousUpdates", "Cancel", "Cancel"))
                    .OnClicked_Lambda([WeakThis=TWeakObjectPtr<UGorgeousUpdateManager>(this)] { if (auto* Self=WeakThis.Get()) { if (Self->ActiveDownloadRequest.IsValid()) { Self->ActiveDownloadRequest->OnProcessRequestComplete().Unbind(); Self->ActiveDownloadRequest->OnRequestProgress64().Unbind(); Self->ActiveDownloadRequest->CancelRequest(); Self->ActiveDownloadRequest.Reset(); } if (Self->ActiveProgressWindow.IsValid()) Self->ActiveProgressWindow->RequestDestroyWindow(); Self->ActiveProgressWindow.Reset(); Self->ActiveProgressBar.Reset(); Self->ActiveProgressStatusText.Reset(); } return FReply::Handled(); }) ]
            ]
        ];

    ActiveProgressWindow = ProgressWindow;
    FSlateApplication::Get().AddWindow(ProgressWindow);
    ProgressWindow->SetWindowMode(EWindowMode::Windowed);

    const FString Endpoint = URL;

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UGorgeousUpdateManager::OnDownloadPluginUpdateResponse, PluginName);
    Request->OnRequestProgress64().BindLambda([WeakThis = TWeakObjectPtr<UGorgeousUpdateManager>(this), PluginName](FHttpRequestPtr Request, int64 Sent, int64 Received)
    {
        const auto Response = Request->GetResponse();
        const int64 Total = Response.IsValid() ? Response->GetContentLength() : 0;
        AsyncTask(ENamedThreads::GameThread, [WeakThis, Request, PluginName, Received, Total]()
        {
            auto* Self = WeakThis.Get();
            if (!Self || Self->ActiveDownloadRequest!=Request || !Self->ActiveProgressBar.IsValid() || !Self->ActiveProgressStatusText.IsValid()) return;
            const FString Status = Total > 0
                ? FString::Printf(TEXT("Downloading %s: %.1f / %.1f MB"), *PluginName, Received/(1024.0*1024.0), Total/(1024.0*1024.0))
                : FString::Printf(TEXT("Downloading %s: %.1f MB"), *PluginName, Received/(1024.0*1024.0));
            Self->ActiveProgressStatusText->SetText(FText::FromString(Status));
            Self->ActiveProgressBar->SetPercent(Total > 0 ? TOptional<float>(float(Received)/float(Total)) : TOptional<float>());
        });
    });
    Request->SetURL(Endpoint);
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Accept"), TEXT("application/zip"));
    ActiveDownloadRequest=Request;
    if (!Request->ProcessRequest()) { ActiveDownloadRequest.Reset(); if (ActiveProgressWindow.IsValid()) ActiveProgressWindow->RequestDestroyWindow(); }
}

void UGorgeousUpdateManager::OnDownloadPluginUpdateResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString PluginName)
{
    if (Request!=ActiveDownloadRequest) return;
    ActiveDownloadRequest.Reset();
    ON_SCOPE_EXIT
    {
        if (ActiveProgressWindow.IsValid()) ActiveProgressWindow->RequestDestroyWindow();
        ActiveProgressWindow.Reset();
        ActiveProgressBar.Reset();
        ActiveProgressStatusText.Reset();
        ActiveDownloadChecksum.Reset();
    };
    if (!GorgeousNetworkConsent::IsAllowed()) return;
    const bool bDownloadSucceeded = bWasSuccessful && Response.IsValid()
        && GorgeousApiEndpoints::IsAllowedURL(Response->GetEffectiveURL()) && EHttpResponseCodes::IsOk(Response->GetResponseCode());
    if (ActiveProgressBar.IsValid())
    {
        ActiveProgressBar->SetPercent(1.0f);
    }
    if (ActiveProgressStatusText.IsValid())
    {
        ActiveProgressStatusText->SetText(FText::FromString(
            bDownloadSucceeded ? TEXT("Download complete. Preparing installer...") : TEXT("Download failed.")
        ));
    }

    if (!bDownloadSucceeded)
    {
        FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("GorgeousUpdates", "DownloadFailed", "The update download failed. Your project was not changed."));
        GT_E_LOG("GT.Updates", TEXT("Failed to download update for plugin %s. Response code: %d"), *PluginName, Response.IsValid() ? Response->GetResponseCode() : 0);
        if (ActiveProgressWindow.IsValid())
        {
            ActiveProgressWindow->RequestDestroyWindow();
            ActiveProgressWindow.Reset();
        }
        return;
    }

    const FString ZipPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Temp") / FString::Printf(TEXT("Update_%s.zip"), *PluginName));
    const FString Directory = FPaths::GetPath(ZipPath);
    IFileManager::Get().MakeDirectory(*Directory, true);

    const FString ExpectedHash = ActiveDownloadChecksum;
    const TArray<uint8>& Content = Response->GetContent();
    uint8 Digest[SHA256_DIGEST_LENGTH];
    SHA256(Content.GetData(), Content.Num(), Digest);
    FString ActualHash;
    for (uint8 Byte : Digest) ActualHash += FString::Printf(TEXT("%02x"), Byte);
    if (ExpectedHash.Len() != 64 || !ActualHash.Equals(ExpectedHash, ESearchCase::IgnoreCase))
    {
        GT_E_LOG("GT.Updates", TEXT("Update rejected: missing or mismatched SHA-256 from the authenticated catalog."));
        return;
    }
    if (!FFileHelper::SaveArrayToFile(Content, *ZipPath))
    {
        GT_E_LOG("GT.Updates", TEXT("Failed to save update zip to %s"), *ZipPath);
        if (ActiveProgressWindow.IsValid())
        {
            ActiveProgressWindow->RequestDestroyWindow();
            ActiveProgressWindow.Reset();
        }
        return;
    }

    GT_I_LOG("GT.Updates", TEXT("Saved plugin update zip to %s"), *ZipPath);

    if (UGorgeousPluginHelper* DiscoveryHelper = UGorgeousPluginHelper::GetSingleton())
    {
        FGorgeousPersistentData::Get().InvalidatePersistedChecksum(DiscoveryHelper->GetKnownGorgeousPlugins(false));
    }
    
    const FString InstallerPath = IPluginManager::Get().FindPlugin("GorgeousCore")->GetBaseDir() / "Binaries" / FPlatformProcess::GetBinariesSubdirectory() / TEXT("gorgeous-installer");
    FString FullInstallerPath = FPaths::ConvertRelativePathToFull(InstallerPath);
#if PLATFORM_WINDOWS
    FullInstallerPath += TEXT(".exe");
#endif

    FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
    uint32 EditorProcessID = FPlatformProcess::GetCurrentProcessId();

    uint32 InstallerProcessID = 0;
    FString Params = FString::Printf(TEXT("--install-zip \"%s\" --project \"%s\" --wait-for-pid %u --reopen-project --target-plugin \"%s\""), *ZipPath, *ProjectPath, EditorProcessID, *PluginName);

    if (!FEditorFileUtils::SaveDirtyPackages(true, true, true, false, false, false)) return;
    FProcHandle ProcHandle = FPlatformProcess::CreateProc(*FullInstallerPath, *Params, true, false, false, &InstallerProcessID, 0, nullptr, nullptr);
    if (ProcHandle.IsValid())
    {
        GT_I_LOG("GT.Updates", TEXT("Launched gorgeous-installer with PID %u for plugin update"), InstallerProcessID);
        if (ActiveProgressWindow.IsValid())
        {
            ActiveProgressWindow->RequestDestroyWindow();
            ActiveProgressWindow.Reset();
        }
        FPlatformMisc::RequestExit(false);
    }
    else
    {
        GT_E_LOG("GT.Updates", TEXT("Failed to launch gorgeous-installer at %s"), *FullInstallerPath);
        if (ActiveProgressWindow.IsValid())
        {
            ActiveProgressWindow->RequestDestroyWindow();
            ActiveProgressWindow.Reset();
        }
    }
}