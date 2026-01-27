// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "Delegates/DelegateCombinations.h"

#include "GorgeousAutoReplicationRPCTransporter.generated.h"

class FGorgeousAutoReplicationMixin;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGorgeousAutoReplicationRequestClientTargetSignature, UGorgeousAutoReplicationRPCTransporter*, Transporter, const FGorgeousQueuedRPC&, QueuedRPC);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGorgeousAutoReplicationRPCRoutedSignature, const FGorgeousQueuedRPC&, QueuedRPC, UObject*, Target);

/** Component responsible for routing AutoReplication RPC payloads across the desired network target. Can be subclassed for custom routing rules. */
UCLASS(ClassGroup = (GorgeousCore), Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent))
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationRPCTransporter : public UActorComponent
{
	GENERATED_BODY()

public:
	UGorgeousAutoReplicationRPCTransporter();

	/** Binds this transporter to a mixin so routed payloads can be delivered locally. */
	void InitializeTransporter(FGorgeousAutoReplicationMixin* InOwningMixin);

	/** Routes the provided RPC payload using the network direction encoded on the struct. */
	bool RouteRPC(const FGorgeousQueuedRPC& QueuedRPC);

	/** Routes a serialized property payload using the desired network direction. */
	bool RoutePropertyPayload(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType);

	/** Returns true when this transporter already services the provided mixin. */
	bool IsLinkedToMixin(const FGorgeousAutoReplicationMixin* InOwningMixin) const;

	/** Blueprint hook invoked when a client target is needed for RPC routing. */
	UPROPERTY(BlueprintAssignable, Category = "Gorgeous Core|AutoReplication|Networking")
	FGorgeousAutoReplicationRequestClientTargetSignature OnResolveClientTarget;

	/** Broadcast after an RPC has been forwarded to a remote endpoint. */
	UPROPERTY(BlueprintAssignable, Category = "Gorgeous Core|AutoReplication|Networking")
	FGorgeousAutoReplicationRPCRoutedSignature OnRPCForwarded;

	/** Provides a temporary override used the next time a client RPC needs an explicit target. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|Networking")
	void SetClientTargetOverride(APlayerController* InPlayerController);

	/** Clears any pending client target override. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|Networking")
	void ClearClientTargetOverride();

protected:
	virtual void OnRegister() override;

	/** Determines which mixin handles local delivery so subclasses/tests can override timing. */
	virtual void DeliverRPCLocally(const FGorgeousQueuedRPC& QueuedRPC);

	virtual bool RouteServerBoundRPC(const FGorgeousQueuedRPC& QueuedRPC, bool bReliable);
	virtual bool RouteClientBoundRPC(const FGorgeousQueuedRPC& QueuedRPC, bool bReliable);
	virtual bool RouteMulticastRPC(const FGorgeousQueuedRPC& QueuedRPC, bool bReliable);
	virtual bool ForwardRPCToClient(const FGorgeousQueuedRPC& QueuedRPC, bool bReliable);
	virtual void ForwardRPCToMulticast(const FGorgeousQueuedRPC& QueuedRPC, bool bReliable);
	virtual bool ForwardPropertyPayloadToClient(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType);
	virtual void ForwardPropertyPayloadToMulticast(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType);
	virtual bool IsOwnerLocallyAuthoritative() const;
	virtual bool IsOwnerLocallyControlledClient() const;

	UFUNCTION(Server, Reliable)
	void ServerReceiveRPCReliable(const FGorgeousQueuedRPC& QueuedRPC);

	UFUNCTION(Server, Unreliable)
	void ServerReceiveRPCUnreliable(const FGorgeousQueuedRPC& QueuedRPC);

	UFUNCTION(Client, Reliable)
	void ClientReceiveRPCReliable(const FGorgeousQueuedRPC& QueuedRPC);

	UFUNCTION(Client, Unreliable)
	void ClientReceiveRPCUnreliable(const FGorgeousQueuedRPC& QueuedRPC);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastReceiveRPCReliable(const FGorgeousQueuedRPC& QueuedRPC);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastReceiveRPCUnreliable(const FGorgeousQueuedRPC& QueuedRPC);

	UFUNCTION(Server, Reliable)
	void ServerReceivePropertyPayloadReliable(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType);

	UFUNCTION(Server, Unreliable)
	void ServerReceivePropertyPayloadUnreliable(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType);

	UFUNCTION(Client, Reliable)
	void ClientReceivePropertyPayloadReliable(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType);

	UFUNCTION(Client, Unreliable)
	void ClientReceivePropertyPayloadUnreliable(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastReceivePropertyPayloadReliable(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastReceivePropertyPayloadUnreliable(const FGorgeousAutoReplicationPropertyEnvelope& Envelope, EGorgeousAutoReplicationRPCType RouteType);

	void DeliverPropertyPayload(const FGorgeousAutoReplicationPropertyEnvelope& Envelope);

private:
	FGorgeousAutoReplicationMixin* OwningMixin;
	TWeakObjectPtr<APlayerController> PendingClientTargetOverride;
};
