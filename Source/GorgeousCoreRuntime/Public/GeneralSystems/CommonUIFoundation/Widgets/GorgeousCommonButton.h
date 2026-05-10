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

	UGorgeousCommonButton(const FObjectInitializer& ObjectInitializer);

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

	/** Sound tag for Hover event. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI|Audio")
	FGameplayTag HoverSoundTag;

	/** Sound tag for Click event. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI|Audio")
	FGameplayTag ClickSoundTag;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnHovered() override;
	virtual void NativeOnUnhovered() override;
	virtual void NativeOnClicked() override;

	/** Plays a themed sound by tag. */
	void PlayThemedSound(FGameplayTag SoundTag);
};
