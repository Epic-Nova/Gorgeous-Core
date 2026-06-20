#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "GorgeousUpdateManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGorgeousUpdateCheckCompleted, bool, bUpdatesAvailable);

/**
 * Subsystem responsible for communicating with the Gorgeous API
 * to verify plugin version hashes and detect available updates.
 */
UCLASS()
class GORGEOUSCOREEDITOR_API UGorgeousUpdateManager : public UEditorSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Scans all installed Gorgeous plugins (including Core), calculates the MD5
     * hash of their ModuleCore directory, and polls the remote API for updates.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous|Updates")
    void CheckForUpdates();

    /** Fetches the public catalog of available plugins and saves it offline. */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous|Updates")
    void FetchSystemsCatalog();

    /** Fired when the update check finishes. */
    UPROPERTY(BlueprintAssignable, Category = "Gorgeous|Updates")
    FOnGorgeousUpdateCheckCompleted OnUpdateCheckCompleted;

private:
    bool bIsDevMode = false;
    
    void ProbeConnection();
    void OnProbeConnectionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    void OnUpdateCheckResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void OnFetchSystemsCatalogResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    FString CalculatePluginModuleCoreHash(const FString& PluginName, const FString& PluginBaseDir);
    FString HashDirectory(const FString& DirectoryPath);
};
