// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonTabListWidgetBase.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
#include "GorgeousCommonTabList.generated.h"

/**
 * AAA Tab List with Gorgeous Foundation support.
 * Automatically themes its tab buttons and supports signal-driven tab switching.
 */
UCLASS(BlueprintType, Blueprintable)
class GORGEOUSCORERUNTIME_API UGorgeousCommonTabList : public UCommonTabListWidgetBase, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

public:
	UE_UI_WIDGET_INTERFACE_BOILERPLATE()

	/** Tag used to identify this tab list for signal-driven updates (e.g. switching tabs via ID). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FGameplayTag BindingTag;

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);

	/** Theme interpolation state + maps used by UE_UI_TICK_THEME_INTERP */
	TMap<FName, FLinearColor> CurrentThemeColors;
	TMap<FName, FLinearColor> TargetThemeColors;

	/** Interp speed and flag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Juicy")
	float ThemeInterpSpeed = 5.0f;
	bool bIsInterpTheme = false;

	/**
	 * Signal-driven tab selection handler.
	 * Payload should contain "TabIndex" (int) or "TabID" (FName).
	 */
	UFUNCTION()
	void OnTabSelectSignalReceived(const FInstancedStruct& Payload);
};
