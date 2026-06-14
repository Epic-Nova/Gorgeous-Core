// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - System developer settings                   |
<==========================================================================*/
#pragma once

// Includes
#include "GorgeousSystemDeveloperSettings.generated.h"

class UGeneralSystemConfiguration_PDA;

UCLASS(Config = Editor, DefaultConfig, DisplayName = "Core ↪ System Overrides")
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousSystemDeveloperSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UGorgeousSystemDeveloperSettings();

    // Returns the project settings singleton
    static UGorgeousSystemDeveloperSettings* Get();

#if WITH_EDITOR
    virtual FName GetCategoryName() const override { return TEXT("Gorgeous Things"); }
    virtual FText GetSectionText() const override;
#endif

    /** Map SystemIdentifier -> explicit PDA asset override. If set, the validator will treat the referenced PDA as canonical. */
    UPROPERTY(EditAnywhere, Config, Category = "System Overrides")
    TMap<FName, TSoftObjectPtr<UGeneralSystemConfiguration_PDA>> PDAOverrides;

};
