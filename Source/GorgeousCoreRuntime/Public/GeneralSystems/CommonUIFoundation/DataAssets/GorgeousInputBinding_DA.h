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
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/GorgeousPrimaryDataAsset.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousInputBinding_DA.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Input Binding
| Functional Name: UGorgeousInputBinding_DA
| Parent Class: UGorgeousPrimaryDataAsset
| Class Suffix: _DA
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Maps Gameplay Tags to Enhanced Input Action assets and metadata. Allows
| the Gorgeous logic kernel to remain tag-based while leveraging Enhanced
| Input features.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/DataAssets/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/DataAssets/GorgeousInputBinding_DA",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/DataAssets/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousInputBinding_DA : public UGorgeousPrimaryDataAsset
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:
	// Returns the primary asset type for input bindings.
	virtual FPrimaryAssetType GetPrimaryAssetType() const override { return TEXT("InputBinding"); }
	// Returns the content paths scanned for input bindings.
	virtual TArray<FString> GetPreferredScanPaths() const override { return { TEXT("Input/Bindings") }; }
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:
	// Stores mapping contexts activated with this binding.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TArray<FGorgeousInputMappingConfig_S> InputMappingContexts;

	// Map of conceptual action tags to input action metadata.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (Categories = "UI.Action", ForceInlineRow, ShowOnlyInnerProperties))
	TMap<FGameplayTag, FGorgeousInputBindingInfo_S> Bindings;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
public:
#if WITH_EDITOR
	// Validates this asset's configured input binding data.
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only
};