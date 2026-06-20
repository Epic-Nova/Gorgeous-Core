// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GorgeousInstallableSystem_DA.generated.h"

/**
 * Editor-only descriptor for a unified installable system (e.g. Core Systems).
 * These assets are populated from the Offline Cache (or eventually the API)
 * and dictate whether the system can be downloaded, updated, or managed.
 */
UCLASS(BlueprintType)
class GORGEOUSCOREEDITOR_API 
UGorgeousInstallableSystem_DA : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    virtual bool IsEditorOnly() const override { return true; }

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System|Identity")
    FString SystemId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System|Identity")
    FString TargetPluginName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System|Identity")
    FText SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System|Identity", meta = (MultiLine = "true"))
    FText SystemDescription;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System|Identity")
    FName SystemIconName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System|Identity")
    FString SystemIconUrl;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System|Remote")
    FString Version;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System|Remote")
    FString DownloadUrl;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System|Structure")
    TArray<FString> SourcePaths;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System|Structure")
    TArray<FString> ContentPaths;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System|State")
    bool bIsPreShipped;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System|State")
    bool bIsInstalled;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System|State")
    FString InstalledVersion;

    /**
     * Helper to determine if a system is installed by checking if its source or content paths exist on disk.
     */
    bool CheckIsInstalled() const;
};
