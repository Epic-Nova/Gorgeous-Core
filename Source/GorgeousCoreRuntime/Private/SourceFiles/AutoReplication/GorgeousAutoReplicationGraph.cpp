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

#include "AutoReplication/GorgeousAutoReplicationGraph.h"

#include "AutoReplication/GorgeousAutoReplicationCoordinator.h"
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"

UGorgeousAutoReplicationGraph::UGorgeousAutoReplicationGraph()
{
}

void UGorgeousAutoReplicationGraph::InitGlobalGraphNodes()
{
	Super::InitGlobalGraphNodes();

	AutoReplicationNode = CreateNewNode<UReplicationGraphNode_ActorList>();
	if (AutoReplicationNode)
	{
		AddGlobalGraphNode(AutoReplicationNode);
		RefreshAutoReplicationOwners();
	}
}

void UGorgeousAutoReplicationGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager)
{
	Super::InitConnectionGraphNodes(ConnectionManager);
}

void UGorgeousAutoReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
{
	Super::RouteAddNetworkActorToNodes(ActorInfo, GlobalInfo);
}

void UGorgeousAutoReplicationGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
	Super::RouteRemoveNetworkActorToNodes(ActorInfo);
	OnActorRemoved(ActorInfo.GetActor());
}

void UGorgeousAutoReplicationGraph::RouteRenameNetworkActorToNodes(const FRenamedReplicatedActorInfo& ActorInfo)
{
	Super::RouteRenameNetworkActorToNodes(ActorInfo);
	HandleOwnerRenamed(ActorInfo);
}

void UGorgeousAutoReplicationGraph::RegisterObjectVariable(UGorgeousObjectVariable* Variable)
{
	if (!Variable)
	{
		return;
	}

	RegisteredVariables.Add(Variable);

	if (!AutoReplicationNode)
	{
		return;
	}

	if (AActor* OwningActor = Cast<AActor>(Variable->GetAutoReplicationOwner()))
	{
		const FObjectKey OwnerKey(OwningActor);
		FOwnerRegistration& Registration = OwnerRegistrations.FindOrAdd(OwnerKey);
		Registration.Actor = OwningActor;
		++Registration.RefCount;
		AddOwnerToAutoReplication(OwningActor, Registration);
	}
}

void UGorgeousAutoReplicationGraph::UnregisterObjectVariable(UGorgeousObjectVariable* Variable)
{
	if (!Variable)
	{
		return;
	}

	RegisteredVariables.Remove(Variable);

	if (!AutoReplicationNode)
	{
		return;
	}

	if (AActor* OwningActor = Cast<AActor>(Variable->GetAutoReplicationOwner()))
	{
		const FObjectKey OwnerKey(OwningActor);
		if (FOwnerRegistration* Registration = OwnerRegistrations.Find(OwnerKey))
		{
			Registration->RefCount = FMath::Max(0, Registration->RefCount - 1);
			if (Registration->RefCount == 0)
			{
				RemoveOwnerFromAutoReplication(OwningActor, *Registration);
				OwnerRegistrations.Remove(OwnerKey);
			}
		}
	}
}

void UGorgeousAutoReplicationGraph::AddOwnerToAutoReplication(AActor* OwnerActor, FOwnerRegistration& Registration)
{
	if (!OwnerActor || !AutoReplicationNode)
	{
		return;
	}

	if (Registration.bIsRegistered)
	{
		return;
	}

	FNewReplicatedActorInfo ActorInfo(OwnerActor);
	AutoReplicationNode->NotifyAddNetworkActor(ActorInfo);
	Registration.bIsRegistered = true;
}

void UGorgeousAutoReplicationGraph::RemoveOwnerFromAutoReplication(AActor* OwnerActor, FOwnerRegistration& Registration)
{
	if (!OwnerActor || !AutoReplicationNode || !Registration.bIsRegistered)
	{
		return;
	}

	FNewReplicatedActorInfo ActorInfo(OwnerActor);
	AutoReplicationNode->NotifyRemoveNetworkActor(ActorInfo);
	Registration.bIsRegistered = false;
	SetActorDestructionInfoToIgnoreDistanceCulling(OwnerActor);
}

void UGorgeousAutoReplicationGraph::OnActorRemoved(AActor* OwnerActor)
{
	if (!OwnerActor)
	{
		return;
	}

	const FObjectKey OwnerKey(OwnerActor);
	if (FOwnerRegistration* Registration = OwnerRegistrations.Find(OwnerKey))
	{
		Registration->RefCount = 0;
		RemoveOwnerFromAutoReplication(OwnerActor, *Registration);
		OwnerRegistrations.Remove(OwnerKey);
	}
}

void UGorgeousAutoReplicationGraph::ResetAutoReplicationOwners()
{
	if (!AutoReplicationNode)
	{
		OwnerRegistrations.Reset();
		return;
	}

	for (TPair<FObjectKey, FOwnerRegistration>& Pair : OwnerRegistrations)
	{
		if (Pair.Value.bIsRegistered && Pair.Value.Actor.IsValid())
		{
			FNewReplicatedActorInfo ActorInfo(Pair.Value.Actor.Get());
			AutoReplicationNode->NotifyRemoveNetworkActor(ActorInfo);
		}
	}

	OwnerRegistrations.Reset();
}

void UGorgeousAutoReplicationGraph::HandleOwnerRenamed(const FRenamedReplicatedActorInfo& ActorInfo)
{
	if (!AutoReplicationNode)
	{
		return;
	}

	const FObjectKey OldKey(ActorInfo.OldActorInfo.Actor);
	const FObjectKey NewKey(ActorInfo.NewActorInfo.Actor);

	if (FOwnerRegistration* Registration = OwnerRegistrations.Find(OldKey))
	{
		FOwnerRegistration Temp = *Registration;
		OwnerRegistrations.Remove(OldKey);
		Temp.Actor = ActorInfo.NewActorInfo.Actor;
		OwnerRegistrations.Add(NewKey, Temp);
	}
}

void UGorgeousAutoReplicationGraph::ResetGameWorldState()
{
	ResetAutoReplicationOwners();
	RegisteredVariables.Reset();
	Super::ResetGameWorldState();
}

void UGorgeousAutoReplicationGraph::TearDown()
{
	ResetAutoReplicationOwners();
	RegisteredVariables.Reset();
	Super::TearDown();
}

void UGorgeousAutoReplicationGraph::RefreshAutoReplicationOwners()
{
	if (!AutoReplicationNode)
	{
		return;
	}

	for (TPair<FObjectKey, FOwnerRegistration>& Pair : OwnerRegistrations)
	{
		FOwnerRegistration& Registration = Pair.Value;
		if (!Registration.Actor.IsValid() || Registration.RefCount <= 0)
		{
			Registration.bIsRegistered = false;
			continue;
		}

		AddOwnerToAutoReplication(Registration.Actor.Get(), Registration);
	}
}
