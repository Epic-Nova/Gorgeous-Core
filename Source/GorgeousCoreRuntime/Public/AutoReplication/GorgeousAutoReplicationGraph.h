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

#include "CoreMinimal.h"
//#include "BasicReplicationGraph.h"
#include "ReplicationGraph.h"
#include "UObject/ObjectKey.h"
#include "GorgeousAutoReplicationGraph.generated.h"

class UGorgeousObjectVariable;
class UReplicationGraphNode_ActorList;
class AActor;

/** Dedicated replication graph that can be installed to broadcast auto-replicated object variables efficiently. */
UCLASS(Config = Game)
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()

public:
	UGorgeousAutoReplicationGraph();

	virtual void InitGlobalGraphNodes() override;
	virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager) override;
	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override;
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;
	virtual void RouteRenameNetworkActorToNodes(const FRenamedReplicatedActorInfo& ActorInfo) override;
	virtual void ResetGameWorldState() override;
	virtual void TearDown() override;

	void RegisterObjectVariable(UGorgeousObjectVariable* Variable);
	void UnregisterObjectVariable(UGorgeousObjectVariable* Variable);

private:
	struct FOwnerRegistration
	{
		FOwnerRegistration()
			: RefCount(0)
			, bIsRegistered(false)
		{
		}

		TWeakObjectPtr<AActor> Actor;
		int32 RefCount;
		bool bIsRegistered;
	};

	void AddOwnerToAutoReplication(AActor* OwnerActor, FOwnerRegistration& Registration);
	void RemoveOwnerFromAutoReplication(AActor* OwnerActor, FOwnerRegistration& Registration);
	void OnActorRemoved(AActor* OwnerActor);
	void ResetAutoReplicationOwners();
	void HandleOwnerRenamed(const FRenamedReplicatedActorInfo& ActorInfo);
	void RefreshAutoReplicationOwners();

	/** Global node that keeps QoL owners always relevant when required by their streams. */
	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_ActorList> AutoReplicationNode;

	UPROPERTY()
	TSet<TWeakObjectPtr<UGorgeousObjectVariable>> RegisteredVariables;

	/** Tracks how many object variables reference a given QoL owner so we can add/remove actors deterministically. */
	TMap<FObjectKey, FOwnerRegistration> OwnerRegistrations;
};
