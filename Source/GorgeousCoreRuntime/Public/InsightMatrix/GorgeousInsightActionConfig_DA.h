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
#include "Blueprint/UserWidget.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousInsightActionConfig_DA.generated.h"
//<-------------------------------------------------------------------------->

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousBlueprintInsightAction
{
	GENERATED_BODY()

	// Unique Action ID.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FName ActionId;

	// Display name in the Insight Matrix.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FText DisplayName;

	// Detailed description of the action.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FText Description;

	// Category under which this action appears.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FName Category;

	// Editor Utility Widget class to spawn in Editor mode.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets", meta = (MetaClass = "/Script/Blutility.EditorUtilityWidget"))
	TSoftClassPtr<UUserWidget> EditorWidgetClass;

	// Runtime User Widget class to spawn in PIE or standalone mode.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSoftClassPtr<UUserWidget> RuntimeWidgetClass;
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Insight Action Config
| Functional Name: UGorgeousInsightActionConfig_DA
| Parent Class: UPrimaryDataAsset
| Class Suffix: _DA
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Data asset used to register Insight Matrix actions and bind them to Editor
| Utility Widgets and Runtime Debug Widgets.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/GorgeousInsightActionConfig_DA",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousInsightActionConfig_DA : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Which provider/system this action config belongs to (e.g., "TeamSystem", "Inventory").
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FName ProviderName;

	// The list of actions defined by this configuration.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TArray<FGorgeousBlueprintInsightAction> Actions;
};
