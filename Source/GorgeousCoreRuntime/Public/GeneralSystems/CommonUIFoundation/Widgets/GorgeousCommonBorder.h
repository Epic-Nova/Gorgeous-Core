// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/Border.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
#include "GorgeousCommonBorder.generated.h"

/**
 * Overridden Border with Signal Bridge support.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousCommonBorder : public UBorder, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

public:
	UE_UI_WIDGET_INTERFACE_BOILERPLATE()

	/** Tag used to identify this border for Signal Bridge updates. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FGameplayTag BindingTag;

	/** Interp speed for theme color transitions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Juicy")
	float ThemeInterpSpeed = 5.0f;

	/** Map of colors currently being interpolated. */
	TMap<FName, FLinearColor> CurrentThemeColors;
	TMap<FName, FLinearColor> TargetThemeColors;
	/** Whether we are currently interpolating theme colors. */
	bool bIsInterpTheme = false;

	/** Called when a new theme is applied. Implement in Blueprint for custom styling. */

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);
};
