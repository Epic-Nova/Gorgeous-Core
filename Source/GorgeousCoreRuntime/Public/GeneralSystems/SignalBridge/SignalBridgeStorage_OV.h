// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "GameplayTagContainer.h"
#include "SignalBridgeListenerStructures.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
#include "SignalBridgeStructures.h"
#include "SignalBridgeStorage_OV.generated.h"

class AGorgeousPlayerController;

/**
 * The network-aware storage for the Signal Bridge system.
 */
UCLASS(BlueprintType)
class GORGEOUSCORERUNTIME_API USignalBridgeStorage_OV : public UGorgeousObjectVariable
{
	GENERATED_BODY()

public:
	USignalBridgeStorage_OV();

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
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge")
	bool Listen(FGameplayTag Tag, AGorgeousPlayerController* Controller, const FSignalBridgeEventDelegate& Delegate);

	/**
	 * Listens to a signal for a specific actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge")
	bool ListenToActor(FGameplayTag Tag, AActor* TargetActor, AGorgeousPlayerController* Controller, const FSignalBridgeEventDelegate& Delegate);

	/**
	 * Dispatches a signal with the given tag and payload.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge")
	void Dispatch(FGameplayTag Tag, const FInstancedStruct& Payload);

	/**
	 * Clears local listeners for a specific tag.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge")
	void Clear(FGameplayTag Tag, AGorgeousPlayerController* Controller);

	/**
	 * Fires the local bindings for a tag. Called by the RPC relay.
	 */
	void FireLocalSignal(FGameplayTag Tag, const FInstancedStruct& Payload);

	/**
	 * Dynamically adds an allowed controller for a signal (only if policy is Custom).
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge")
	void AddAllowedController(FGameplayTag Tag, AGorgeousPlayerController* Controller);

	/**
	 * Dynamically removes an allowed controller for a signal.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Signal Bridge")
	void RemoveAllowedController(FGameplayTag Tag, AGorgeousPlayerController* Controller);

protected:
	//~ Begin UGorgeousObjectVariable Interface
	virtual void OnReplicationActivated_Implementation(const FGorgeousAutoReplicationContext& Context) override;
	virtual bool CanControllerAccessVariable_Implementation(AGorgeousPlayerController* Controller, FName PropertyName) const override;
	//~ End UGorgeousObjectVariable Interface

	/** Server RPC to register a remote listener. */
	UFUNCTION(Server, Reliable)
	void Server_RegisterListener(FGameplayTag Tag, AGorgeousPlayerController* Controller);

	/** Server RPC to register a remote listener for a specific actor. */
	UFUNCTION(Server, Reliable)
	void Server_RegisterActorListener(FGameplayTag Tag, AActor* TargetActor, AGorgeousPlayerController* Controller);

	/** Server RPC to unregister a remote listener. */
	UFUNCTION(Server, Reliable)
	void Server_UnregisterListener(FGameplayTag Tag, AGorgeousPlayerController* Controller);

	/** Server RPC to request a dispatch from a client. */
	UFUNCTION(Server, Reliable)
	void Server_RequestDispatch(FGameplayTag Tag, FInstancedStruct Payload, AGorgeousPlayerController* Requester);

	/** Replicated access rules for each tag. Managed by AutoReplication. */
	UPROPERTY()
	TMap<FGameplayTag, FGorgeousSignalBridgeAccessRules_S> AccessRules;

	/** Local-only map of bound delegates. */
	TMap<FGameplayTag, FSignalBridgeEventMulticastDelegate> LocalBindings;

	/** Authority-only dictionary of listener tracking objects. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FGorgeousSignalBridgeListener_S> DictionaryAssociations;

private:
	/** Helper to find or create a listener registry for a tag (Authority only). */
	FGorgeousSignalBridgeListener_S& GetOrCreateListenerRegistry(FGameplayTag Tag);

	/** Helper to evaluate access for a specific controller and tag. */
	bool EvaluateTagAccess(AGorgeousPlayerController* Controller, FGameplayTag Tag) const;
};
