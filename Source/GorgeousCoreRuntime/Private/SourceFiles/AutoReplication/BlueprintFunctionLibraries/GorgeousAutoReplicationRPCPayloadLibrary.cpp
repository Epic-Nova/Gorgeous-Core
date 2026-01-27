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

#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/NativeObjectVariableDefinitions.h"

namespace GorgeousAutoReplicationRPCPayloadLibrary_Private
{
	template <typename TObjectVariableType>
	TObjectVariableType* NewLiteral(UObject* DesiredOuter)
	{
		UObject* Outer = DesiredOuter ? DesiredOuter : GetTransientPackage();
		return NewObject<TObjectVariableType>(Outer);
	}
}

template <typename TObjectVariableType, typename TValueType>
UGorgeousObjectVariable* UGorgeousAutoReplicationRPCPayloadLibrary::CreateLiteralArgument(UObject* DesiredOuter, const TValueType& Value)
{
	TObjectVariableType* Literal = GorgeousAutoReplicationRPCPayloadLibrary_Private::NewLiteral<TObjectVariableType>(DesiredOuter);
	if (!Literal)
	{
		return nullptr;
	}

	Literal->Value = Value;
	return Literal;
}

FGorgeousRPCPayload UGorgeousAutoReplicationRPCPayloadLibrary::MakeAutoReplicationRPCPayload(const FName HandlerName)
{
	FGorgeousRPCPayload Payload;
	Payload.HandlerName = HandlerName;
	return Payload;
}

bool UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCObjectArgument(UObject* WorldContextObject, FGorgeousRPCPayload& Payload, const FName ArgumentName, UGorgeousObjectVariable* ArgumentValue, const bool bDuplicateForContext)
{
	if (ArgumentName.IsNone() || !ArgumentValue)
	{
		return false;
	}

	if (UGorgeousObjectVariable* UsableArgument = PrepareArgumentForPayload(WorldContextObject, ArgumentValue, bDuplicateForContext))
	{
		
		Payload.Arguments.Add(FGorgeousRPCArgumentContainer(ArgumentName, UsableArgument));
		return true;
	}

	return false;
}

bool UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCBoolArgument(UObject* WorldContextObject, FGorgeousRPCPayload& Payload, const FName ArgumentName, const bool bValue)
{
	if (UGorgeousObjectVariable* Literal = CreateLiteralArgument<UBoolean_SOV>(WorldContextObject, bValue))
	{
		return AddAutoReplicationRPCObjectArgument(WorldContextObject, Payload, ArgumentName, Literal, false);
	}
	return false;
}

bool UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCIntegerArgument(UObject* WorldContextObject, FGorgeousRPCPayload& Payload, const FName ArgumentName, const int32 Value)
{
	if (UGorgeousObjectVariable* Literal = CreateLiteralArgument<UInteger_SOV>(WorldContextObject, Value))
	{
		return AddAutoReplicationRPCObjectArgument(WorldContextObject, Payload, ArgumentName, Literal, false);
	}
	return false;
}

bool UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCFloatArgument(UObject* WorldContextObject, FGorgeousRPCPayload& Payload, const FName ArgumentName, const float Value)
{
	if (UGorgeousObjectVariable* Literal = CreateLiteralArgument<UFloat_SOV>(WorldContextObject, static_cast<double>(Value)))
	{
		return AddAutoReplicationRPCObjectArgument(WorldContextObject, Payload, ArgumentName, Literal, false);
	}
	return false;
}

bool UGorgeousAutoReplicationRPCPayloadLibrary::AddAutoReplicationRPCStringArgument(UObject* WorldContextObject, FGorgeousRPCPayload& Payload, const FName ArgumentName, const FString& Value)
{
	if (UGorgeousObjectVariable* Literal = CreateLiteralArgument<UString_SOV>(WorldContextObject, Value))
	{
		return AddAutoReplicationRPCObjectArgument(WorldContextObject, Payload, ArgumentName, Literal, false);
	}
	return false;
}

UGorgeousObjectVariable* UGorgeousAutoReplicationRPCPayloadLibrary::PrepareArgumentForPayload(UObject* DesiredOuter, UGorgeousObjectVariable* ArgumentValue, const bool bDuplicateForContext)
{
	if (!ArgumentValue)
	{
		return nullptr;
	}

	UObject* Outer = DesiredOuter ? DesiredOuter : GetTransientPackage();

	if (ArgumentValue->GetOuter() == Outer || !bDuplicateForContext)
	{
		return ArgumentValue;
	}

	return DuplicateObject<UGorgeousObjectVariable>(ArgumentValue, Outer);
}
