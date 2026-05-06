// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CommonButtonBase.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
#include "GorgeousCommonButton.generated.h"

/**
 * A "Gorgeous" button that automatically skins itself and plays themed audio.
 */
UCLASS(Abstract, Blueprintable)
class GORGEOUSCORERUNTIME_API UGorgeousCommonButton : public UCommonButtonBase, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

public:
	UE_UI_WIDGET_INTERFACE_BOILERPLATE()

	/** Tag used to identify this button for Signal Bridge updates. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FGameplayTag BindingTag;

	/** Interp speed for theme color transitions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Juicy")
	float ThemeInterpSpeed = 5.0f;

	/** Called when a new theme is applied. Implement in Blueprint for custom styling. */

	/** Sound tag for Hover event. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI|Audio")
	FGameplayTag HoverSoundTag;

	/** Sound tag for Click event. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI|Audio")
	FGameplayTag ClickSoundTag;

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();
	virtual void NativeOnHovered() override;
	virtual void NativeOnUnhovered() override;
	virtual void NativeOnClicked() override;

	/** Plays a themed sound by tag. */
	void PlayThemedSound(FGameplayTag SoundTag);
};
