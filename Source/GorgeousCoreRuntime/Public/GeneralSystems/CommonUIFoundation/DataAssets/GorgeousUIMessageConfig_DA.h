// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GorgeousUIMessageConfig_DA.generated.h"

class UCommonActivatableWidget;
class UCommonUIState_DA;
class UGorgeousUITheme_DA;

/**
 * Configuration for a Gorgeous Message/Dialog template.
 */
UCLASS(BlueprintType)
class GORGEOUSCORERUNTIME_API UGorgeousUIMessageConfig_DA : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The widget class to spawn for this message. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TSoftClassPtr<UCommonActivatableWidget> MessageWidgetClass;

	/** The UI state to switch to while this message is active (e.g. Pause, SlowMo). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UCommonUIState_DA> MessageState;

	/** The theme/skin to apply to the message dialog. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UGorgeousUITheme_DA> MessageTheme;

	/** 
	 * The buttons to display on the dialog. 
	 * Key: The Result Tag returned when clicked (e.g. "Confirm").
	 * Value: The localized text to display on the button.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TMap<FName, FText> Buttons;

	/** The layer to push the message onto (e.g. Modal, Overlay). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FGameplayTag LayerTag;
};
