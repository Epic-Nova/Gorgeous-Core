// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "GorgeousAutoReplicationRPCPayloadLibrary.generated.h"

/** Helper library for constructing FGorgeousRPCPayload structures in Blueprint and C++. */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationRPCPayloadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Initializes a payload with the provided handler name. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|AutoReplication|RPC")
	static FGorgeousRPCPayload MakeAutoReplicationRPCPayload(FName HandlerName);

	/**
	 * Universal wildcard argument – accepts any Blueprint-compatible value type.
	 * The value is serialized by-value (no UObject pointer, no GC risk).
	 * ArgumentName must match the handler function parameter name exactly.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Gorgeous Core|AutoReplication|RPC",
		meta = (CustomStructureParam = "Value",
			DisplayName = "Add RPC Argument (Wildcard)",
			Keywords = "rpc argument wildcard any"))
	static FGorgeousRPCPayload& AddAutoReplicationRPCArgument(UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, int32 Value);
	DECLARE_FUNCTION(execAddAutoReplicationRPCArgument);

	/** Convenience helper – add a bool argument. Returns the payload for chaining. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC")
	static FGorgeousRPCPayload& AddAutoReplicationRPCBoolArgument(UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, bool bValue);

	/** Convenience helper – add an int32 argument. Returns the payload for chaining. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC")
	static FGorgeousRPCPayload& AddAutoReplicationRPCIntArgument(UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, int32 Value);

	/** Convenience helper – add a float argument. Stored as double (native Blueprint float). Returns the payload for chaining. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC")
	static FGorgeousRPCPayload& AddAutoReplicationRPCFloatArgument(UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, double Value);

	/** Convenience helper – add a string argument. Returns the payload for chaining. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC")
	static FGorgeousRPCPayload& AddAutoReplicationRPCStringArgument(UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, const FString& Value);

	/** Convenience helper – add an FName argument. Returns the payload for chaining. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|AutoReplication|RPC")
	static FGorgeousRPCPayload& AddAutoReplicationRPCNameArgument(UPARAM(ref) FGorgeousRPCPayload& Payload, FName ArgumentName, FName Value);

	/** Sets the per-request timeout in seconds. 0 means use the global default. Returns the payload for chaining. */
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
};

