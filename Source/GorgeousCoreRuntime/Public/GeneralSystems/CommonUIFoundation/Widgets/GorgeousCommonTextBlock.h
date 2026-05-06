// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonTextBlock.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
#include "GorgeousCommonTextBlock.generated.h"

/**
 * Overridden CommonTextBlock with Signal Bridge support.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousCommonTextBlock : public UCommonTextBlock, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

public:
	UE_UI_WIDGET_INTERFACE_BOILERPLATE()
	/** Tag used to identify this text block for Signal Bridge updates. */
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
