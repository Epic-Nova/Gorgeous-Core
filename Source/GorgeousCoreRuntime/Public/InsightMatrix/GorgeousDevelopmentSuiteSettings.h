// Copyright (c) 2026 Simsalabim Studios. All rights reserved.
#pragma once
#include "CoreMinimal.h"
#include "ModuleCore/GorgeousCoreDeveloperSettings.h"
#include "GorgeousDevelopmentSuiteSettings.generated.h"
/** Runtime opt-in. The optional suite and explicit target approval are also required in Shipping/Test. */
UCLASS(Config=Game, DefaultConfig, meta=(GorgeousSettingsPage = "General", DisplayName="Development Suite"))
class GORGEOUSCORERUNTIME_API UGorgeousDevelopmentSuiteSettings : public UGorgeousCoreDeveloperSettings
{
    GENERATED_BODY()
public:
    UPROPERTY(Config, EditAnywhere, Category="Insight Matrix")
    bool bEnableInsightMatrix = false;
    virtual FName GetSectionName() const override { return TEXT("DevelopmentSuite"); }
};
