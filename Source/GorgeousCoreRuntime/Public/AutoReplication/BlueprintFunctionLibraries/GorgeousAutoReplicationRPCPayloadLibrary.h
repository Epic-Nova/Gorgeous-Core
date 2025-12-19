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
#include "GorgeousAutoReplicationRPCPayloadLibrary.generated.h"

class UGorgeousObjectVariable;

/** Helper library for constructing FGorgeousRPCPayload structures in Blueprint and C++. */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationRPCPayloadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Initializes a payload with the provided handler name. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC")
	static FGorgeousRPCPayload MakeAutoReplicationRPCPayload(FName HandlerName);

	/** Adds a pre-built object variable argument to the payload. Optionally duplicates the object into the supplied outer. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC", meta = (WorldContext = "WorldContextObject"))
	static bool AddAutoReplicationRPCObjectArgument(UObject* WorldContextObject, UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, UGorgeousObjectVariable* ArgumentValue, bool bDuplicateForContext = true);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC", meta = (WorldContext = "WorldContextObject"))
	static bool AddAutoReplicationRPCBoolArgument(UObject* WorldContextObject, UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, bool bValue);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC", meta = (WorldContext = "WorldContextObject"))
	static bool AddAutoReplicationRPCIntegerArgument(UObject* WorldContextObject, UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC", meta = (WorldContext = "WorldContextObject"))
	static bool AddAutoReplicationRPCFloatArgument(UObject* WorldContextObject, UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, float Value);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC", meta = (WorldContext = "WorldContextObject"))
	static bool AddAutoReplicationRPCStringArgument(UObject* WorldContextObject, UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, const FString& Value);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC", meta = (WorldContext = "WorldContextObject"))
	static bool AddAutoReplicationRPCVectorArgument(UObject* WorldContextObject, UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, const FVector& Value);

private:
	static UGorgeousObjectVariable* PrepareArgumentForPayload(UObject* DesiredOuter, UGorgeousObjectVariable* ArgumentValue, bool bDuplicateForContext);

	template<typename TObjectVariableType, typename TValueType>
	static UGorgeousObjectVariable* CreateLiteralArgument(UObject* DesiredOuter, const TValueType& Value);
};

