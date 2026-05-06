// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
#include "SignalBridgeStructures.h"
#include "SignalBridgeBlueprintFunctionLibrary.generated.h"

class USignalBridgeStorage_OV;
class AGorgeousPlayerController;

/**
 * Blueprint Function Library for interacting with the Signal Bridge system.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API USignalBridgeBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Resolves the Signal Bridge storage object. 
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge", meta = (WorldContext = "WorldContextObject"))
	static USignalBridgeStorage_OV* GetSignalBridgeStorage(UObject* WorldContextObject, bool bRequireNetworking = true);

	/**
	 * Configures access rules for a specific gameplay tag.
	 * 
	 * @param WorldContextObject Context for world resolution.
	 * @param Tag The tag to configure.
	 * @param Rules The access rules to apply.
	 * @param Requester The controller that is registering this signal (optional, used for 'OwningControllerOnly').
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Register Signal"))
	static void RegisterSignal(UObject* WorldContextObject, FGameplayTag Tag, const FGorgeousSignalBridgeAccessRules_S& Rules, AGorgeousPlayerController* Requester = nullptr);

	/**
	 * Listens to a signal identified by a tag using a delegate handle.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Listen to Signal"))
	static bool Listen(UObject* WorldContextObject, FGameplayTag Tag, AGorgeousPlayerController* Controller, const FSignalBridgeEventDelegate& Delegate);

	/**
	 * Listens to a signal for a specific actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Listen Signal To Actor"))
	static bool ListenToActor(UObject* WorldContextObject, FGameplayTag Tag, AActor* TargetActor, AGorgeousPlayerController* Controller, const FSignalBridgeEventDelegate& Delegate);

	/**
	 * Dispatches a signal with the given tag and payload.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Dispatch Signal"))
	static void Dispatch(UObject* WorldContextObject, FGameplayTag Tag, const FInstancedStruct& Payload);

	/**
	 * Dispatches a signal locally only. No networking occurs.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Dispatch Signal Local"))
	static void DispatchLocal(UObject* WorldContextObject, FGameplayTag Tag, const FInstancedStruct& Payload);

	/**
	 * Clears local listeners for a specific tag.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Clear Listeners"))
	static void Clear(UObject* WorldContextObject, FGameplayTag Tag, AGorgeousPlayerController* Controller);

	/**
	 * Adds a specific controller to the allowed list for a custom signal.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Allow Controller"))
	static void AddAllowedController(UObject* WorldContextObject, FGameplayTag Tag, AGorgeousPlayerController* Controller);

	/**
	 * Removes a specific controller from the allowed list for a custom signal.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Revoke Controller"))
	static void RemoveAllowedController(UObject* WorldContextObject, FGameplayTag Tag, AGorgeousPlayerController* Controller);
};
