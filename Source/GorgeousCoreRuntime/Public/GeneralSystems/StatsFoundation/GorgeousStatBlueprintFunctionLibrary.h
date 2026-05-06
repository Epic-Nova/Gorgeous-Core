// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "GorgeousStatBlueprintFunctionLibrary.generated.h"

class UGorgeousStatComponent_AC;
class UGorgeousStatStorage_OV;
class AGorgeousPlayerController;

/**
 * Blueprint Function Library for the Gorgeous Stat System.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousStatBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Resolves the global Stat storage object. 
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats", meta = (WorldContext = "WorldContextObject"))
	static UGorgeousStatStorage_OV* GetGorgeousStatStorage(UObject* WorldContextObject, bool bRequireNetworking = true);

	/** Finds the stat component on the controlled pawn of the player controller. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Stats", meta = (WorldContext = "WorldContextObject"))
	static UGorgeousStatComponent_AC* GetStatComponent(UObject* WorldContextObject);

	/** Gets a stat value from an actor. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Stats", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "Actor", CompactNodeTitle="Get Stat"))
	static float GetStatValue(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag);

	/** Sets a stat value on an actor (Authority Only). */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "Actor", CompactNodeTitle="Set Stat"))
	static void SetStatValue(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag, float Value);

	/** Modifies a stat value on an actor (Authority Only). */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "Actor", CompactNodeTitle="Modify Stat"))
	static void ModifyStatValue(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag, float Delta);

	/** Adds a specific controller to the allowed list for a stat on a specific actor. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "Actor"))
	static void AddAllowedController(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag, AGorgeousPlayerController* Controller);

	/** Removes a specific controller from the allowed list for a stat on a specific actor. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Stats", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "Actor"))
	static void RemoveAllowedController(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag, AGorgeousPlayerController* Controller);
};
