// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Interfaces/IHttpRequest.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "EditorSubsystem.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousUpdateManager.generated.h"
//<-------------------------------------------------------------------------->

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGorgeousUpdateCheckCompleted, bool, bUpdatesAvailable);

class SProgressBar;
class STextBlock;
class SWindow;

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

    /**
     * Downloads a plugin update using the provided download token.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous|Updates")
    void DownloadPluginUpdate(const FString& PluginName, const FString& DownloadToken);

private:
    bool bIsDevMode = false;

    void ProbeConnection();
    void OnProbeConnectionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    void OnUpdateCheckResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void OnDownloadPluginUpdateResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString PluginName);
    void OnFetchSystemsCatalogResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    FString HashDirectory(const FString& DirectoryPath);

    FString CalculatePluginModuleCoreHash(const FString& PluginName, const FString& PluginBaseDir);

    FString CalculatePluginChecksum(const FString& PluginName, const FString& PluginBaseDir);

    TSharedPtr<class SWindow> ActiveProgressWindow;
    TSharedPtr<class SProgressBar> ActiveProgressBar;
    TSharedPtr<class STextBlock> ActiveProgressStatusText;
};