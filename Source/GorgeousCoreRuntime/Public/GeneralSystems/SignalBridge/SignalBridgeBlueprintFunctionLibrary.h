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
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
#include "SignalBridgeStructures.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "SignalBridgeBlueprintFunctionLibrary.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Signal Bridge Blueprint Function Library
| Functional Name: USignalBridgeBlueprintFunctionLibrary
| Parent Class: UBlueprintFunctionLibrary
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Blueprint Function Library for interacting with the Signal Bridge system.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
//<=================--- Forward Declarations ---=================>
class USignalBridgeStorage_OV;
class AGorgeousPlayerController;
//<------------------------------------------------------------->
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/SignalBridge/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/SignalBridge/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API USignalBridgeBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Resolves the Signal Bridge storage object.
	 *
	 * @param WorldContextObject The context used to resolve the active world.
	 * @param bRequireNetworking Whether the resolved storage must support networking.
	 * @return The Signal Bridge storage object, or nullptr when unavailable.
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
	 *
	 * @param WorldContextObject The context used to resolve the active world.
	 * @param Tag The signal tag to listen for.
	 * @param Controller The controller that owns the listener.
	 * @param Delegate The delegate invoked when the signal is dispatched.
	 * @return True when the listener was registered, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Listen to Signal"))
	static bool Listen(UObject* WorldContextObject, FGameplayTag Tag, AGorgeousPlayerController* Controller, const FSignalBridgeEventDelegate& Delegate);

	/**
	 * Listens to a signal for a specific actor.
	 *
	 * @param WorldContextObject The context used to resolve the active world.
	 * @param Tag The signal tag to listen for.
	 * @param TargetActor The actor whose signals should be received.
	 * @param Controller The controller that owns the listener.
	 * @param Delegate The delegate invoked when the signal is dispatched.
	 * @return True when the listener was registered, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Listen Signal To Actor"))
	static bool ListenToActor(UObject* WorldContextObject, FGameplayTag Tag, AActor* TargetActor, AGorgeousPlayerController* Controller, const FSignalBridgeEventDelegate& Delegate);

	/**
	 * Dispatches a signal with the given tag and payload.
	 *
	 * @param WorldContextObject The context used to resolve the active world.
	 * @param Tag The signal tag to dispatch.
	 * @param Payload The payload delivered to listeners.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Dispatch Signal"))
	static void Dispatch(UObject* WorldContextObject, FGameplayTag Tag, const FInstancedStruct& Payload);

	/**
	 * Dispatches a signal locally only. No networking occurs.
	 *
	 * @param WorldContextObject The context used to resolve the active world.
	 * @param Tag The signal tag to dispatch.
	 * @param Payload The payload delivered to local listeners.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Dispatch Signal Local"))
	static void DispatchLocal(UObject* WorldContextObject, FGameplayTag Tag, const FInstancedStruct& Payload);

	/**
	 * Clears local listeners for a specific tag.
	 *
	 * @param WorldContextObject The context used to resolve the active world.
	 * @param Tag The signal tag whose listeners are cleared.
	 * @param Controller The controller whose listeners are cleared.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Clear Listeners"))
	static void Clear(UObject* WorldContextObject, FGameplayTag Tag, AGorgeousPlayerController* Controller);

	/**
	 * Adds a specific controller to the allowed list for a custom signal.
	 *
	 * @param WorldContextObject The context used to resolve the active world.
	 * @param Tag The signal tag whose allowed list is updated.
	 * @param Controller The controller to allow.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Allow Controller"))
	static void AddAllowedController(UObject* WorldContextObject, FGameplayTag Tag, AGorgeousPlayerController* Controller);

	/**
	 * Removes a specific controller from the allowed list for a custom signal.
	 *
	 * @param WorldContextObject The context used to resolve the active world.
	 * @param Tag The signal tag whose allowed list is updated.
	 * @param Controller The controller to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge", meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "Revoke Controller"))
	static void RemoveAllowedController(UObject* WorldContextObject, FGameplayTag Tag, AGorgeousPlayerController* Controller);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions
};