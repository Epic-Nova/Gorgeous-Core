// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GeneralSystems/GorgeousPrimaryDataAsset.h"
#include "GorgeousUIMessageConfig_DA.generated.h"

class UCommonActivatableWidget;
class UGorgeousUIState_DA;
class UGorgeousUITheme_DA;

/**
 * Configuration for a Gorgeous Message/Dialog template.
 */
UCLASS(BlueprintType)
class GORGEOUSCORERUNTIME_API UGorgeousUIMessageConfig_DA : public UGorgeousPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** UGorgeousPrimaryDataAsset Interface */
	virtual FPrimaryAssetType GetPrimaryAssetType() const override { return TEXT("UI_Message"); }
	virtual TArray<FString> GetDefaultScanPaths() const override { return { TEXT("UserInterfaces/Messages") }; }

	/** The widget class to spawn for this message. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TSoftClassPtr<UCommonActivatableWidget> MessageWidgetClass;

	/** The UI state to switch to when this message is active. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
	TObjectPtr<UGorgeousUIState_DA> State;

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (Categories = "GT.UI.Layer"))
	FGameplayTag LayerTag;
};
