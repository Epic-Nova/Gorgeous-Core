// Copyright (c) 2026 Simsalabim Studios. All rights reserved.
#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GorgeousDevelopmentSuiteSettings.generated.h"
/** Runtime opt-in. The optional suite and explicit target approval are also required in Shipping/Test. */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Development Suite"))
class GORGEOUSCORERUNTIME_API UGorgeousDevelopmentSuiteSettings : public UDeveloperSettings
{
    GENERATED_BODY()
public:
    UPROPERTY(Config, EditAnywhere, Category="Insight Matrix")
    bool bEnableInsightMatrix = false;
    virtual FName GetCategoryName() const override { return TEXT("Gorgeous Things"); }
    virtual FName GetSectionName() const override { return TEXT("DevelopmentSuite"); }
};
