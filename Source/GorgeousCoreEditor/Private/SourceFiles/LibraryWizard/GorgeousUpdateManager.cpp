#include "LibraryWizard/GorgeousUpdateManager.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/SecureHash.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Helpers/Macros/GorgeousConnectionHelperMacros.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"

void UGorgeousUpdateManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ProbeConnection();
}

void UGorgeousUpdateManager::ProbeConnection()
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UGorgeousUpdateManager::OnProbeConnectionResponse);
    Request->SetURL(TEXT(GORGEOUS_API_ENDPOINT));
    Request->SetVerb("GET");
    Request->ProcessRequest();
}

void UGorgeousUpdateManager::OnProbeConnectionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        FString CurrentURL = Response->GetURL();
        if (CurrentURL.StartsWith(TEXT("http://")))
        {
            bIsDevMode = true;
            GT_E_LOG("GT.Updates", TEXT("Entering credentials is dangerous as we are in HTTP mode (possible dev mode)"));
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
            FString Hash = CalculatePluginModuleCoreHash(PluginName, Plugin->GetBaseDir());
            
            TSharedPtr<FJsonObject> PluginObj = MakeShareable(new FJsonObject());
            PluginObj->SetStringField(TEXT("PluginName"), PluginName);
            PluginObj->SetStringField(TEXT("ModuleCoreHash"), Hash);
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
    Request->SetURL(TEXT(GORGEOUS_API_ENDPOINT_UPDATE_CHECK));
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
                JsonObject->TryGetBoolField(TEXT("UpdatesAvailable"), bUpdatesAvailable);
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
        FString CachePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("GorgeousPersistentData.json"));
        FFileHelper::SaveStringToFile(Response->GetContentAsString(), *CachePath);
        GT_I_LOG("GT.Updates", TEXT("Successfully fetched and cached Gorgeous Systems Catalog."));
    }
    else
    {
        GT_E_LOG("GT.Updates", TEXT("Failed to fetch Gorgeous Systems Catalog."));
    }
}
