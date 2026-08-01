// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "AutoReplication/GorgeousAutoReplicationWorldSubsystem.h"

#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "Engine/World.h"
#include "Engine/NetDriver.h"

namespace
{
	bool IsGameLikeWorld(const UWorld* World)
	{
		return World && World->IsGameWorld();
	}
}

void UGorgeousAutoReplicationWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UWorld* World = GetWorld())
	{
		if (!IsGameLikeWorld(World))
		{
			return;
		}

		NetDriverCreatedHandle = FWorldDelegates::OnNetDriverCreated.AddUObject(this, &UGorgeousAutoReplicationWorldSubsystem::HandleNetDriverCreated);
		UGorgeousCoreRuntimeGlobals::InitializeAutoReplicationForWorld(World);
	}
}

void UGorgeousAutoReplicationWorldSubsystem::Deinitialize()
{
	if (NetDriverCreatedHandle.IsValid())
	{
		FWorldDelegates::OnNetDriverCreated.Remove(NetDriverCreatedHandle);
		NetDriverCreatedHandle.Reset();
	}

	Super::Deinitialize();
}

#if WITH_EDITOR
bool UGorgeousAutoReplicationWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return IsGameLikeWorld(Cast<UWorld>(Outer));
}
#endif

void UGorgeousAutoReplicationWorldSubsystem::HandleNetDriverCreated(UWorld* World, UNetDriver* NetDriver)
{
	if (!NetDriver || World != GetWorld() || !IsGameLikeWorld(World))
	{
		return;
	}

	UGorgeousCoreRuntimeGlobals::InitializeAutoReplicationForWorld(World);
}
