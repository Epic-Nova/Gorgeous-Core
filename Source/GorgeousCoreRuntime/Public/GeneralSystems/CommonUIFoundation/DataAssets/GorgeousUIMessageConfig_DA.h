// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GeneralSystems/GorgeousPrimaryDataAsset.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousUIMessageConfig_DA.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UCommonActivatableWidget;
class UGorgeousUIState_DA;
class UGorgeousUITheme_DA;
//<------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous UIMessage Config
| Functional Name: UGorgeousUIMessageConfig_DA
| Parent Class: UGorgeousPrimaryDataAsset
| Class Suffix: _DA
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Configuration for a Gorgeous Message/Dialog template.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/DataAssets/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIMessageConfig_DA",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/DataAssets/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousUIMessageConfig_DA : public UGorgeousPrimaryDataAsset
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:
	// Returns the primary asset type for UI message configurations.
	virtual FPrimaryAssetType GetPrimaryAssetType() const override { return TEXT("UI_Message"); }
	// Returns the content paths scanned for UI message configurations.
	virtual TArray<FString> GetDefaultScanPaths() const override { return { TEXT("UserInterfaces/Messages") }; }
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:
	// The widget class to spawn for this message.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TSoftClassPtr<UCommonActivatableWidget> MessageWidgetClass;

	// The UI state to switch to when this message is active.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
	TObjectPtr<UGorgeousUIState_DA> State;

	// The theme/skin to apply to the message dialog.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UGorgeousUITheme_DA> MessageTheme;

	// Maps result tags to the localized button labels displayed by the dialog.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TMap<FName, FText> Buttons;

	// The layer to push the message onto (e.g. Modal, Overlay).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (Categories = "GT.UI.Layer"))
	FGameplayTag LayerTag;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};