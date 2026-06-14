// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonRotator.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
#include "GorgeousCommonRotator.generated.h"

/**
 * AAA Rotator with Gorgeous Foundation support.
 */
UCLASS(BlueprintType, Blueprintable)
class GORGEOUSCORERUNTIME_API UGorgeousCommonRotator : public UCommonRotator, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

public:
	UE_UI_WIDGET_INTERFACE_BOILERPLATE()

	UGorgeousCommonRotator(const FObjectInitializer& ObjectInitializer);

	/** Binding Tag for Signal Bridge routing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FGameplayTag BindingTag;

	/** Enables the per-widget style allow list. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Style")
	bool bUseStylePropertyAllowList = true;

	/** Properties that can be styled by themes or Signal Bridge payloads. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Style")
	TSet<FName> StylePropertyAllowList;

	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous UI", meta = (DisplayName = "On Theme Applied"))
	void OnThemeApplied_BP(const UGorgeousUITheme_DA* Theme);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Signal-driven index selection. */
	UFUNCTION()
	void OnRotatorSignalReceived(FGameplayTag SignalTag, const struct FInstancedStruct& Payload);
};
