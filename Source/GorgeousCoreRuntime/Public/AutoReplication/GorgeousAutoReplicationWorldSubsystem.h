// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "GorgeousAutoReplicationWorldSubsystem.generated.h"

class UNetDriver;

/**
 * World subsystem that ensures the auto-replication coordinator is initialized
 * for every gameplay world so Iris and replication graph backends are
 * available before automation harnesses begin validation.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

#if WITH_EDITOR
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
#endif

private:
	void HandleNetDriverCreated(UWorld* World, UNetDriver* NetDriver);

	FDelegateHandle NetDriverCreatedHandle;
};
