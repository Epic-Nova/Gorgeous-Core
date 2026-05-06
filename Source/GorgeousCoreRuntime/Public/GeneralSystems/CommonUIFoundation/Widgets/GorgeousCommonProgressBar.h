// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ProgressBar.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
#include "GorgeousCommonProgressBar.generated.h"

/**
 * Overridden ProgressBar with Signal Bridge support.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousCommonProgressBar : public UProgressBar, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

public:
	UE_UI_WIDGET_INTERFACE_BOILERPLATE()
	/** Tag used to identify this progress bar for Signal Bridge updates. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FGameplayTag BindingTag;

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();
	/** Map of colors currently being interpolated. */
	TMap<FName, FLinearColor> CurrentThemeColors;
	TMap<FName, FLinearColor> TargetThemeColors;
	/** Whether we are currently interpolating theme colors. */
	bool bIsInterpTheme = false;
};
