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
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GorgeousCoreRuntimeGlobals.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousStatFoundation.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Stat Foundation
| Functional Name: UGorgeousStatFoundation
| Parent Class: UGorgeous
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Blueprint Function Library for the Gorgeous Stat System.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
//<=================--- Forward Declarations ---=================>
class UGorgeousStatComponent_AC;
class UGorgeousStatFoundationStorage_OV;
class AGorgeousPlayerController;
//<------------------------------------------------------------->
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/StatsFoundation/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/StatsFoundation/GorgeousStatFoundation",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/StatsFoundation/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousStatFoundation : public UGorgeous
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:
	/**
	 * Resolves the global Stat Foundation storage object.
	 *
	 * @param WorldContextObject The context used to resolve the active world.
	 * @param bRequireNetworking Whether the resolved storage must support networking.
	 * @return The Stat Foundation storage object, or nullptr when unavailable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats", meta = (WorldContext = "WorldContextObject"))
	static UGorgeousStatFoundationStorage_OV* GetGorgeousStatStorage(UObject* WorldContextObject, bool bRequireNetworking = true);

	/**
	 * Finds the Stat Foundation component on the controlled pawn.
	 *
	 * @param WorldContextObject The context used to resolve the active world.
	 * @return The controlled pawn's stat component, or nullptr when unavailable.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Stats", meta = (WorldContext = "WorldContextObject"))
	static UGorgeousStatComponent_AC* GetStatComponent(UObject* WorldContextObject);

	/**
	 * Gets a statistic value from an actor.
	 *
	 * @param WorldContextObject The context used to resolve the active world.
	 * @param Actor The actor that owns the statistic.
	 * @param StatTag The statistic to read.
	 * @return The current statistic value.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Stats", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "Actor", CompactNodeTitle="Get Stat"))
	static float GetStatValue(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag);

	/**
	 * Sets a statistic value on an actor. Authority only.
	 *
	 * @param WorldContextObject The context used to resolve the active world.
	 * @param Actor The actor that owns the statistic.
	 * @param StatTag The statistic to set.
	 * @param Value The value to assign.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "Actor", CompactNodeTitle="Set Stat"))
	static void SetStatValue(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag, float Value);

	/**
	 * Modifies a statistic value on an actor. Authority only.
	 *
	 * @param WorldContextObject The context used to resolve the active world.
	 * @param Actor The actor that owns the statistic.
	 * @param StatTag The statistic to modify.
	 * @param Delta The amount to add to the current value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "Actor", CompactNodeTitle="Modify Stat"))
	static void ModifyStatValue(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag, float Delta);

	/**
	 * Adds a controller to a statistic's allowed list on an actor.
	 *
	 * @param WorldContextObject The context used to resolve the active world.
	 * @param Actor The actor that owns the statistic.
	 * @param StatTag The statistic whose access list is updated.
	 * @param Controller The controller to allow.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "Actor"))
	static void AddAllowedController(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag, AGorgeousPlayerController* Controller);

	/**
	 * Removes a controller from a statistic's allowed list on an actor.
	 *
	 * @param WorldContextObject The context used to resolve the active world.
	 * @param Actor The actor that owns the statistic.
	 * @param StatTag The statistic whose access list is updated.
	 * @param Controller The controller to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "Actor"))
	static void RemoveAllowedController(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag, AGorgeousPlayerController* Controller);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions
};