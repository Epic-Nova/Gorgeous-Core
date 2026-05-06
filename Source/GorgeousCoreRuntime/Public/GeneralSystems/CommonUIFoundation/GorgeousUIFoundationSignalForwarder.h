// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "GorgeousUIFoundationSignalForwarder.generated.h"

class UGorgeousUIFoundationSubsystem;

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousSignalBridgeTagForwarder : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGameplayTag Tag;

	UPROPERTY()
	TObjectPtr<UGorgeousUIFoundationSubsystem> OwningSubsystem;

	UFUNCTION()
	void OnBridgePayload(FGameplayTag SignalTag, const struct FInstancedStruct& Payload);
};
