// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "GorgeousAutoReplicationRPCExamples.generated.h"

/** Blueprint helpers that showcase the AutoReplication RPC flow end-to-end. */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationRPCExamples : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Builds a sample payload that routes into UGorgeousObjectVariable::ExampleAutoReplicationRPCHandler. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|Examples", meta = (WorldContext = "Context"))
	static FGorgeousRPCPayload BuildExampleDamagePayload(UObject* Context, float DamageValue, const FString& DamageLabel);

	/** Sends the example payload immediately using the provided AutoReplication key and reliability. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|Examples", meta = (WorldContext = "Context"))
	static bool SendExampleDamageAutoReplicationRPC(UObject* Context, FName AutoReplicationKey, float DamageValue, const FString& DamageLabel, EGorgeousAutoReplicationRPCType Reliability = EGorgeousAutoReplicationRPCType::EReliableServer);

	/** Logs a short explanation of how object variables participate in AutoReplication RPCs. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|Examples", meta = (WorldContext = "Context"))
	static void DescribeObjectVariableUsage(UObject* Context);
};

