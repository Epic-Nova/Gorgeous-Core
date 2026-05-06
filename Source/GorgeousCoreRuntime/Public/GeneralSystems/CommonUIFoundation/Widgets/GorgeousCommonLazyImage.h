// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonLazyImage.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
#include "GorgeousCommonLazyImage.generated.h"

/**
 * Overridden CommonLazyImage with Signal Bridge support.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousCommonLazyImage : public UCommonLazyImage, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

public:
	UE_UI_WIDGET_INTERFACE_BOILERPLATE()

	/** Tag used to identify this image for Signal Bridge updates. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FGameplayTag BindingTag;

	/** Optional Action Tag for this image (e.g. UI.Action.Confirm). Used for dynamic icon swapping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	FGameplayTag ActionTag;

	/** Updates the image's brush based on the current theme and input method. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI")
	void UpdateActionIcon();

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();
};
