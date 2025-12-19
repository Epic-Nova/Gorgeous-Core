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
#include "AutoReplication/BlueprintFunctionLibraries/GorgeousAutoReplicationRPCExamples.h"
#include "AutoReplication/BlueprintFunctionLibraries/GorgeousAutoReplicationRPCPayloadLibrary.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "ObjectVariables/GorgeousObjectVariable.h"

FGorgeousRPCPayload UGorgeousAutoReplicationRPCExamples::BuildExampleDamagePayload(UObject* Context, const float DamageValue, const FString& DamageLabel)
{
	FGorgeousRPCPayload Payload = UGorgeousAutoReplicationRPCPayloadLibrary::MakeAutoReplicationRPCPayload(TEXT("ExampleAutoReplicationRPCHandler"));
	UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCFloatArgument(Context, Payload, TEXT("ExampleValue"), DamageValue);
	UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCStringArgument(Context, Payload, TEXT("ExampleLabel"), DamageLabel);
	return Payload;
}

bool UGorgeousAutoReplicationRPCExamples::SendExampleDamageAutoReplicationRPC(UObject* Context, const FName AutoReplicationKey, const float DamageValue, const FString& DamageLabel, const EGorgeousAutoReplicationRPCType Reliability)
{
	FGorgeousRPCPayload Payload = BuildExampleDamagePayload(Context, DamageValue, DamageLabel);
	return UGorgeousCoreRuntimeGlobals::RequestAutoReplicationRPC(Context, AutoReplicationKey, Reliability, Payload);
}

void UGorgeousAutoReplicationRPCExamples::DescribeObjectVariableUsage(UObject* Context)
{
	UE_LOG(LogTemp, Log, TEXT("AutoReplication RPC examples: Each argument is encapsulated inside a Gorgeous Object Variable instance. Use the payload helper nodes to author arguments, bind OnAutoReplicationRPCPayload to inspect them, and implement ExampleAutoReplicationRPCHandler (or your own handler) on the target object variable to consume typed parameters."));
}

