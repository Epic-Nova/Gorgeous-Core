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

	/** Binding Tag for Signal Bridge routing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FGameplayTag BindingTag;

	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous UI", meta = (DisplayName = "On Theme Applied"))
	void OnThemeApplied_BP(const UGorgeousUITheme_DA* Theme);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/**
	 * Signal-driven tab selection handler.
	 * Payload should contain "TabIndex" (int) or "TabID" (FName).
	 */
	UFUNCTION()
	void OnTabSelectSignalReceived(FGameplayTag SignalTag, const struct FInstancedStruct& Payload);
};
