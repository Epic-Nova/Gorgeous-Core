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

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousCoreRuntimeGlobals.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousAutoReplicationRPCPayloadGlobals.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Auto Replication RPC Payload Globals
| Functional Name: UGorgeousAutoReplicationRPCPayloadGlobals
| Parent Class: UGorgeous
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Static helper functions for constructing FGorgeousRPCPayload structures 
| in Blueprint and C++.
|  These functions are used when creating RPC payloads to be sent via 
| AutoReplication, allowing you to specify the handler function name 
| and add arguments in a flexible way.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationRPCPayloadGlobals : public UGorgeous
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
public:

	/**
	 * HandlerName must match the name of the Blueprint or Native event function that will process this payload, e.g. "OnReceiveDamageRPC".
	 * 
	 * @param HandlerName The name of the handler function that will process this RPC on the receiving end. Must match the function name exactly.
	 * @return An initialized payload struct with the handler name set. You can then add arguments to this payload using the AddAutoReplicationRPCArgument functions before sending it via AutoReplication.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC")
	static FGorgeousRPCPayload MakeAutoReplicationRPCPayload(FName HandlerName);

	/**
	 * Universal wildcard argument – accepts any Blueprint-compatible value type.
	 * The value is serialized by-value (no UObject pointer, no GC risk).
	 * ArgumentName must match the handler function parameter name exactly.
	 * 
	 * @param Payload The payload to add the argument to. This is passed by reference and also returned for chaining.
	 * @param ArgumentName The name of the argument, which must match the parameter name in the handler function that will receive this payload. The match is case-insensitive.
	 * @param Value The value of the argument. This can be any type that is compatible with Blueprint. The function will handle serialization based on the value's type.
	 * @return The modified payload with the new argument added, returned by reference for chaining multiple calls.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Gorgeous Core|AutoReplication|RPC",
		meta = (CustomStructureParam = "Value",
			DisplayName = "Add RPC Argument (Wildcard)",
			Keywords = "rpc argument wildcard any"))
	static FGorgeousRPCPayload& AddAutoReplicationRPCArgument(UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, int32 Value);
	DECLARE_FUNCTION(execAddAutoReplicationRPCArgument);

	/**
	 * Convenience helper for adding a bool argument to the payload.
	 * 
	 * @param Payload The payload to add the argument to. This is passed by reference and also returned for chaining.
	 * @param ArgumentName The name of the argument, which must match the parameter name in the handler function that will receive this payload. The match is case-insensitive.
	 * @param bValue The bool value of the argument.
	 * @return The modified payload with the new argument added, returned by reference for chaining multiple calls.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC")
	static FGorgeousRPCPayload& AddAutoReplicationRPCBoolArgument(UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, bool bValue);

	/**
	 * Convenience helper for adding an int32 argument to the payload.
	 * 
	 * @param Payload The payload to add the argument to. This is passed by reference and also returned for chaining.
	 * @param ArgumentName The name of the argument, which must match the parameter name in the handler function that will receive this payload. The match is case-insensitive.
	 * @param Value The int32 value of the argument.
	 * @return The modified payload with the new argument added, returned by reference for chaining multiple calls.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC")
	static FGorgeousRPCPayload& AddAutoReplicationRPCIntArgument(UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, int32 Value);

	/**
	 * Convenience helper for adding a float argument to the payload.
	 * 
	 * @param Payload The payload to add the argument to. This is passed by reference and also returned for chaining.
	 * @param ArgumentName The name of the argument, which must match the parameter name in the handler function that will receive this payload. The match is case-insensitive.
	 * @param Value The float value of the argument.
	 * @return The modified payload with the new argument added, returned by reference for chaining multiple calls.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC")
	static FGorgeousRPCPayload& AddAutoReplicationRPCFloatArgument(UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, double Value);

	/**
	 * Convenience helper for adding a string argument to the payload.
	 * 
	 * @param Payload The payload to add the argument to. This is passed by reference and also returned for chaining.
	 * @param ArgumentName The name of the argument, which must match the parameter name in the handler function that will receive this payload. The match is case-insensitive.
	 * @param Value The string value of the argument.
	 * @return The modified payload with the new argument added, returned by reference for chaining multiple calls.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC")
	static FGorgeousRPCPayload& AddAutoReplicationRPCStringArgument(UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, const FString& Value);

	/**
	 * Convenience helper for adding an FName argument to the payload.
	 * 
	 * @param Payload The payload to add the argument to. This is passed by reference and also returned for chaining.
	 * @param ArgumentName The name of the argument, which must match the parameter name in the handler function that will receive this payload. The match is case-insensitive.
	 * @param Value The FName value of the argument.
	 * @return The modified payload with the new argument added, returned by reference for chaining multiple calls.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC")
	static FGorgeousRPCPayload& AddAutoReplicationRPCNameArgument(UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, FName Value);

	/**
	 * Sets the per-request timeout in seconds. 0 means use the global default.
	 * @param Payload The payload to set the timeout for.
	 * @param TimeoutSeconds The timeout in seconds.
	 * @return The modified payload with the new timeout set, returned by reference for chaining multiple calls.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC",
		meta = (DisplayName = "Set RPC Timeout"))
	static FGorgeousRPCPayload& SetAutoReplicationRPCTimeout(UPARAM(ref) FGorgeousRPCPayload& Payload, float TimeoutSeconds);

	/**
	 * C++ helper – store any scalar/struct value via its FProperty descriptor.
	 * This is the shared path used by the typed helpers and the CustomThunk.
	 *
	 * @param Payload         Target payload to add the argument to.
	 * @param ArgumentName    Must match the handler function parameter name.
	 * @param Property        The FProperty describing Value's type and size.
	 * @param ValuePtr        Pointer to the raw value memory.
	 * @return true on success.
	 */
	static bool AddArgumentFromProperty(FGorgeousRPCPayload& Payload, FName ArgumentName,
	                                    const FProperty* Property, const void* ValuePtr);
//<------------------------------------------------------------------------->
};

using GT_AR_RPCPayloadLibrary = UGorgeousAutoReplicationRPCPayloadGlobals;