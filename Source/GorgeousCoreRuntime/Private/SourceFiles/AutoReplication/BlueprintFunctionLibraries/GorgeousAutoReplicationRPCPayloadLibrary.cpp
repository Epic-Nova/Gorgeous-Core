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
#include "AutoReplication/BlueprintFunctionLibraries/GorgeousAutoReplicationRPCPayloadLibrary.h"

#include "Serialization/MemoryWriter.h"
#include "Serialization/StructuredArchive.h"
#include "UObject/UnrealType.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"

//-----------------------------------------------------------------------------
// Internal helpers
//-----------------------------------------------------------------------------
namespace GorgeousRPCPayloadLibrary_Private
{
	/**
	 * Serialize a value described by @p Property / @p ValuePtr into a byte array.
	 * - Struct / scalar types that are trivially copyable: raw memcpy.
	 * - FString / FText / FName / dynamic containers: FMemoryWriter archive path.
	 */
	static bool SerializeValueToBytes(const FProperty* Property, const void* ValuePtr, TArray<uint8>& OutBytes, FName& OutPropertyClassName, FName& OutStructTypeName)
	{
		if (!Property || !ValuePtr)
		{
			return false;
		}

		OutPropertyClassName = Property->GetClass()->GetFName();
		OutStructTypeName    = NAME_None;

		if (const FStructProperty* AsStruct = CastField<FStructProperty>(Property))
		{
			if (!AsStruct->Struct)
			{
				return false;
			}
			OutStructTypeName = AsStruct->Struct->GetFName();
			// Structs: use FMemoryWriter so nested FStrings / TArrays inside the struct
			// are properly serialized, not shallow-copied.
			FMemoryWriter Writer(OutBytes);
			AsStruct->Struct->SerializeTaggedProperties(Writer, const_cast<uint8*>(static_cast<const uint8*>(ValuePtr)), AsStruct->Struct, nullptr);
			return true;
		}

		if (CastField<FStrProperty>(Property) || CastField<FTextProperty>(Property))
		{
			// Heap-allocated string/text: archive path.
			FMemoryWriter Writer(OutBytes);
			Property->SerializeItem(FStructuredArchiveFromArchive(Writer).GetSlot(), const_cast<void*>(ValuePtr));
			return true;
		}

		// All remaining types (int32, float, double, bool, FName, int64, byte, etc.) are
		// either POD or have a fixed in-place representation that is safe to memcpy.
		GORGEOUS_55_HIGHER(const int32 PropSize = Property->GetElementSize();)
		GORGEOUS_54_LOWER(const int32 PropSize  = Property->GetSize();)
		OutBytes.SetNumUninitialized(PropSize);
		FMemory::Memcpy(OutBytes.GetData(), ValuePtr, PropSize);
		return true;
	}
}

//-----------------------------------------------------------------------------
FGorgeousRPCPayload UGorgeousAutoReplicationRPCPayloadLibrary::MakeAutoReplicationRPCPayload(const FName HandlerName)
{
	FGorgeousRPCPayload Payload;
	Payload.HandlerName = HandlerName;
	return Payload;
}

//-----------------------------------------------------------------------------
bool UGorgeousAutoReplicationRPCPayloadLibrary::AddArgumentFromProperty(
	FGorgeousRPCPayload& Payload, FName ArgumentName,
	const FProperty* Property, const void* ValuePtr)
{
	if (ArgumentName.IsNone() || !Property || !ValuePtr)
	{
		return false;
	}

	FGorgeousRPCArgumentContainer Arg(ArgumentName);
	if (!GorgeousRPCPayloadLibrary_Private::SerializeValueToBytes(Property, ValuePtr, Arg.ValueBytes, Arg.PropertyClassName, Arg.StructTypeName))
	{
		return false;
	}

	// Replace any existing entry with the same name so callers can overwrite.
	const int32 ExistingIdx = Payload.Arguments.IndexOfByPredicate(
		[&](const FGorgeousRPCArgumentContainer& C) { return C.ArgumentName == ArgumentName; });
	if (ExistingIdx != INDEX_NONE)
	{
		Payload.Arguments[ExistingIdx] = MoveTemp(Arg);
	}
	else
	{
		Payload.Arguments.Add(MoveTemp(Arg));
	}
	return true;
}

//-----------------------------------------------------------------------------
// CustomThunk – wildcard "add any value as RPC argument"
//-----------------------------------------------------------------------------
DEFINE_FUNCTION(UGorgeousAutoReplicationRPCPayloadLibrary::execAddAutoReplicationRPCArgument)
{
	// Param 1: Payload (ref)
	P_GET_STRUCT_REF(FGorgeousRPCPayload, Payload);
	// Param 2: ArgumentName
	P_GET_PROPERTY(FNameProperty, ArgumentName);
	// Param 3: Value – wildcard, harvested via MostRecentProperty
	Stack.MostRecentProperty        = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FProperty>(nullptr);
	const FProperty* ValueProperty = Stack.MostRecentProperty;
	void*            ValueAddr     = Stack.MostRecentPropertyAddress;
	P_FINISH;

	P_NATIVE_BEGIN;
	UGorgeousAutoReplicationRPCPayloadLibrary::AddArgumentFromProperty(Payload, ArgumentName, ValueProperty, ValueAddr);
	*(FGorgeousRPCPayload*)RESULT_PARAM = Payload;
	P_NATIVE_END;
}

// Native stub required by DECLARE_FUNCTION – never called directly.
FGorgeousRPCPayload& UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCArgument(
	FGorgeousRPCPayload& Payload, FName /*ArgumentName*/, int32 /*Value*/)
{
	check(false && "AddAutoReplicationRPCArgument: call via Blueprint (CustomThunk) only.");
	return Payload;
}

//-----------------------------------------------------------------------------
// Typed convenience helpers
//-----------------------------------------------------------------------------
FGorgeousRPCPayload& UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCBoolArgument(
	FGorgeousRPCPayload& Payload, FName ArgumentName, bool bValue)
{
	FGorgeousRPCArgumentContainer Arg(ArgumentName);
	Arg.PropertyClassName = FName("BoolProperty");
	Arg.ValueBytes.SetNumUninitialized(sizeof(bool));
	FMemory::Memcpy(Arg.ValueBytes.GetData(), &bValue, sizeof(bool));
	Payload.Arguments.Add(MoveTemp(Arg));
	return Payload;
}

FGorgeousRPCPayload& UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCIntArgument(
	FGorgeousRPCPayload& Payload, FName ArgumentName, int32 Value)
{
	FGorgeousRPCArgumentContainer Arg(ArgumentName);
	Arg.PropertyClassName = FName("IntProperty");
	Arg.ValueBytes.SetNumUninitialized(sizeof(int32));
	FMemory::Memcpy(Arg.ValueBytes.GetData(), &Value, sizeof(int32));
	Payload.Arguments.Add(MoveTemp(Arg));
	return Payload;
}

FGorgeousRPCPayload& UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCFloatArgument(
	FGorgeousRPCPayload& Payload, FName ArgumentName, double Value)
{
	FGorgeousRPCArgumentContainer Arg(ArgumentName);
	// Blueprint float pins are doubles in UE5.
	Arg.PropertyClassName = FName("DoubleProperty");
	Arg.ValueBytes.SetNumUninitialized(sizeof(double));
	FMemory::Memcpy(Arg.ValueBytes.GetData(), &Value, sizeof(double));
	Payload.Arguments.Add(MoveTemp(Arg));
	return Payload;
}

FGorgeousRPCPayload& UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCStringArgument(
	FGorgeousRPCPayload& Payload, FName ArgumentName, const FString& Value)
{
	FGorgeousRPCArgumentContainer Arg(ArgumentName);
	Arg.PropertyClassName = FName("StrProperty");
	FMemoryWriter Writer(Arg.ValueBytes);
	FString Mutable = Value;
	Writer << Mutable;
	Payload.Arguments.Add(MoveTemp(Arg));
	return Payload;
}

FGorgeousRPCPayload& UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCNameArgument(
	FGorgeousRPCPayload& Payload, FName ArgumentName, FName Value)
{
	FGorgeousRPCArgumentContainer Arg(ArgumentName);
	Arg.PropertyClassName = FName("NameProperty");
	Arg.ValueBytes.SetNumUninitialized(sizeof(FName));
	FMemory::Memcpy(Arg.ValueBytes.GetData(), &Value, sizeof(FName));
	Payload.Arguments.Add(MoveTemp(Arg));
	return Payload;
}

//-----------------------------------------------------------------------------
FGorgeousRPCPayload& UGorgeousAutoReplicationRPCPayloadLibrary::SetAutoReplicationRPCTimeout(
	FGorgeousRPCPayload& Payload, float TimeoutSeconds)
{
	Payload.TimeoutSeconds = TimeoutSeconds;
	return Payload;
}
