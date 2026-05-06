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

	/** Tag used to identify this rotator for signal-driven updates. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FGameplayTag BindingTag;

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();

	/** Signal-driven index selection. */
	UFUNCTION()
	void OnRotatorSignalReceived(const FInstancedStruct& Payload);
};
