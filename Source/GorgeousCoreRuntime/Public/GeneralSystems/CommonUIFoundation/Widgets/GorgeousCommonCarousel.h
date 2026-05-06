// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonAnimatedSwitcher.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
#include "GorgeousCommonCarousel.generated.h"

/**
 * A "Gorgeous" version of the Animated Switcher that acts as a carousel.
 * Supports signal-driven child injection and remote-controlled navigation.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousCommonCarousel : public UCommonAnimatedSwitcher, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

public:
	UE_UI_WIDGET_INTERFACE_BOILERPLATE()

	/** Tag used to identify this carousel for Signal Bridge updates. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FGameplayTag BindingTag;

	/** Switches to the next child in the carousel. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI")
	void Next();

	/** Switches to the previous child in the carousel. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI")
	void Previous();

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();
};
