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
#include "ObjectVariables/GorgeousObjectVariable.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousInsightBlueprintStats_OV.generated.h"
//<-------------------------------------------------------------------------->

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousBlueprintStatsInsightAction
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Insight Stats")
	FName ActionName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Insight Stats")
	FGameplayTag SignalBridgeTag;
};

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousBlueprintSystemStatsData
{
	GENERATED_BODY()

	// Map of stat names to numeric values.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Insight Stats")
	TMap<FString, double> NumericStats;

	// Clickable UI actions registered by this Blueprint system.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Insight Stats")
	TArray<FGorgeousBlueprintStatsInsightAction> Actions;
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Insight Blueprint Stats
| Functional Name: UGorgeousInsightBlueprintStats_OV
| Parent Class: UGorgeousObjectVariable
| Class Suffix: _OV
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Registry Object Variable that holds Insight Matrix stats from Blueprint
| systems such as playlists, teams, and VDG.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType, Blueprintable,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/GorgeousInsightBlueprintStats_OV",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousInsightBlueprintStats_OV : public UGorgeousObjectVariable
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Insight Stats")
	TMap<FName, FGorgeousBlueprintSystemStatsData> SystemStatsMap;

	/**
	 * Registers a Blueprint system and returns the GUID of this registry Object Variable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|InsightMatrix", meta = (WorldContext = "WorldContextObject"))
	static FGuid RegisterBlueprintSystemStats(UObject* WorldContextObject, FName SystemName);

	/**
	 * Registers a clickable UI action for a specific Blueprint system.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|InsightMatrix", meta = (WorldContext = "WorldContextObject"))
	static void RegisterBlueprintSystemAction(UObject* WorldContextObject, const FGuid& RegistryGuid, FName SystemName, FName ActionName, FGameplayTag SignalBridgeTag);

	/**
	 * Unregisters a specific clickable UI action for a Blueprint system.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|InsightMatrix", meta = (WorldContext = "WorldContextObject"))
	static void UnregisterBlueprintSystemAction(UObject* WorldContextObject, const FGuid& RegistryGuid, FName SystemName, FName ActionName);

	/**
	 * Completely removes a registered Blueprint system and all of its stats/actions from the registry.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|InsightMatrix", meta = (WorldContext = "WorldContextObject"))
	static void UnregisterBlueprintSystemStats(UObject* WorldContextObject, const FGuid& RegistryGuid, FName SystemName);

	/**
	 * Sets a numeric stat for a specific Blueprint system.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Gorgeous|InsightMatrix", meta = (CustomStructureParam = "StatValue"))
	static void SetBlueprintSystemStat(const FGuid& RegistryGuid, FName SystemName, FString StatName, int32 StatValue);

	DECLARE_FUNCTION(execSetBlueprintSystemStat);
};
