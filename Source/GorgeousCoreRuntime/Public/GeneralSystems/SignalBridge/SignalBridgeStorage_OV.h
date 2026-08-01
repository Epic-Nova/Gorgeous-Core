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
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SignalBridgeListenerStructures.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
#include "SignalBridgeStructures.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "SignalBridgeStorage_OV.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Signal Bridge Storage
| Functional Name: USignalBridgeStorage_OV
| Parent Class: UGorgeousObjectVariable
| Class Suffix: _OV
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| The network-aware storage for the Signal Bridge system.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
//<=================--- Forward Declarations ---=================>
class AGorgeousPlayerController;
//<------------------------------------------------------------->
UCLASS(BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/SignalBridge/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/SignalBridge/SignalBridgeStorage_OV",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/SignalBridge/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API USignalBridgeStorage_OV : public UGorgeousObjectVariable
{
	GENERATED_BODY()

public:

	// Constructs the Signal Bridge storage object.
	USignalBridgeStorage_OV();

	//<============================--- Overrides ---============================>
	#pragma region Overrides
protected:

	// Activates replication-specific Signal Bridge state.
	virtual void OnReplicationActivated_Implementation(const FGorgeousAutoReplicationContext& Context) override;

	/**
	 * Determines whether a controller may access an AutoReplication property.
	 *
	 * @param Controller The controller requesting access.
	 * @param PropertyName The property the controller is requesting.
	 * @return True when the controller may access the property, false otherwise.
	 */
	virtual bool CanControllerAccessVariable_Implementation(AGorgeousPlayerController* Controller, FName PropertyName) const override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Configures access rules for a specific gameplay tag.
	 *
	 * @param Tag The tag to configure.
	 * @param Rules The access rules to apply.
	 * @param Requester The controller that is registering this signal (becomes the 'Owner').
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge")
	void RegisterSignal(FGameplayTag Tag, const FGorgeousSignalBridgeAccessRules_S& Rules, AGorgeousPlayerController* Requester);

	/**
	 * Listens to a signal identified by a tag using a delegate handle.
	 *
	 * @param Tag The signal tag to listen for.
	 * @param Controller The controller that owns the listener.
	 * @param Delegate The delegate invoked when the signal is dispatched.
	 * @return True when the listener was registered, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge")
	bool Listen(FGameplayTag Tag, AGorgeousPlayerController* Controller, const FSignalBridgeEventDelegate& Delegate);

	/**
	 * Listens to a signal for a specific actor.
	 *
	 * @param Tag The signal tag to listen for.
	 * @param TargetActor The actor whose signals should be received.
	 * @param Controller The controller that owns the listener.
	 * @param Delegate The delegate invoked when the signal is dispatched.
	 * @return True when the listener was registered, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge")
	bool ListenToActor(FGameplayTag Tag, AActor* TargetActor, AGorgeousPlayerController* Controller, const FSignalBridgeEventDelegate& Delegate);

	/**
	 * Dispatches a signal with the given tag and payload.
	 *
	 * @param Tag The signal tag to dispatch.
	 * @param Payload The payload delivered to listeners.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge")
	void Dispatch(FGameplayTag Tag, const FInstancedStruct& Payload);

	/**
	 * Clears local listeners for a specific tag.
	 *
	 * @param Tag The signal tag whose listeners are cleared.
	 * @param Controller The controller whose listeners are cleared.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge")
	void Clear(FGameplayTag Tag, AGorgeousPlayerController* Controller);

	/**
	 * Fires the local bindings for a tag. Called by the RPC relay.
	 *
	 * @param Tag The signal tag being delivered locally.
	 * @param Payload The payload delivered to local listeners.
	 */
	void FireLocalSignal(FGameplayTag Tag, const FInstancedStruct& Payload);

	/**
	 * Dynamically adds an allowed controller for a signal (only if policy is Custom).
	 *
	 * @param Tag The signal tag whose allowed list is updated.
	 * @param Controller The controller to allow.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge")
	void AddAllowedController(FGameplayTag Tag, AGorgeousPlayerController* Controller);

	/**
	 * Dynamically removes an allowed controller for a signal.
	 *
	 * @param Tag The signal tag whose allowed list is updated.
	 * @param Controller The controller to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge")
	void RemoveAllowedController(FGameplayTag Tag, AGorgeousPlayerController* Controller);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
public:

	/**
	 * Returns the number of local signals fired since the process started.
	 *
	 * @return The total number of local signals fired.
	 */
	static int64 GetTotalLocalSignalsFired();

	/**
	 * Returns the number of local signals that had no listeners since the process started.
	 *
	 * @return The total number of local signals with no listeners.
	 */
	static int64 GetTotalNoListenersFound();

	/**
	 * Returns the number of registered tags with active listeners.
	 *
	 * @return The number of tags with active listeners.
	 */
	int32 GetTotalActiveListeners() const;
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only


	//<============================--- Callbacks ---============================>
	#pragma region Callbacks
protected:

	/**
	 * Registers a remote listener through the server.
	 *
	 * @param Tag The signal tag to listen for.
	 * @param Controller The controller that owns the listener.
	 */
	UFUNCTION(Server, Reliable)
	void Server_RegisterListener(FGameplayTag Tag, AGorgeousPlayerController* Controller);

	/**
	 * Registers a remote actor-scoped listener through the server.
	 *
	 * @param Tag The signal tag to listen for.
	 * @param TargetActor The actor whose signals should be received.
	 * @param Controller The controller that owns the listener.
	 */
	UFUNCTION(Server, Reliable)
	void Server_RegisterActorListener(FGameplayTag Tag, AActor* TargetActor, AGorgeousPlayerController* Controller);

	/**
	 * Unregisters a remote listener through the server.
	 *
	 * @param Tag The signal tag to stop listening for.
	 * @param Controller The controller that owns the listener.
	 */
	UFUNCTION(Server, Reliable)
	void Server_UnregisterListener(FGameplayTag Tag, AGorgeousPlayerController* Controller);

	/**
	 * Requests a server-authoritative signal dispatch from a client.
	 *
	 * @param Tag The signal tag to dispatch.
	 * @param Payload The payload delivered to listeners.
	 * @param Requester The controller requesting the dispatch.
	 */
	UFUNCTION(Server, Reliable)
	void Server_RequestDispatch(FGameplayTag Tag, FInstancedStruct Payload, AGorgeousPlayerController* Requester);
	//<------------------------------------------------------------------------->
	#pragma endregion Callbacks


	//<============================--- Variables ---============================>
	#pragma region Variables
protected:
	// Replicated access rules for each tag. Managed by AutoReplication.
	UPROPERTY()
	TMap<FGameplayTag, FGorgeousSignalBridgeAccessRules_S> AccessRules;

	/** Local-only map of bound delegates. */
	TMap<FGameplayTag, FSignalBridgeEventMulticastDelegate> LocalBindings;

	// Authority-only dictionary of listener tracking objects.
	UPROPERTY(Transient)
	TMap<FGameplayTag, FGorgeousSignalBridgeListener_S> DictionaryAssociations;
	//<------------------------------------------------------------------------->
	#pragma endregion Variables


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
private:

	// Friend access for Insight Matrix unit tests.
	friend struct FGorgeousSignalBridgeTestAccess;
	/** Helper to find or create a listener registry for a tag (Authority only). */
	FGorgeousSignalBridgeListener_S& GetOrCreateListenerRegistry(FGameplayTag Tag);

	/** Helper to evaluate access for a specific controller and tag. */
	bool EvaluateTagAccess(AGorgeousPlayerController* Controller, FGameplayTag Tag) const;
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only
};