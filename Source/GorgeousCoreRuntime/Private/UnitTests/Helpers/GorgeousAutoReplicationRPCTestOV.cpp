// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|    Gorgeous Core - AutoReplication RPC Test Object Variable Helper       |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

#include "UnitTests/Helpers/GorgeousAutoReplicationRPCTestOV.h"

#include "ObjectVariables/NativeObjectVariableDefinitions.h"

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousAutomationTestOV, Log, All);

void UGorgeousAutomationTestOV::Automation_HandleRPC_WithReturnOV(
	UInteger_SOV* ReturnValue,
	int32 TestInputInt,
	const FString& TestInputString,
	int32 Sequence,
	const FString& Origin,
	const FString& Timestamp)
{
	constexpr int32 RPC_TRANSFORM_MULTIPLY = 7;
	constexpr int32 RPC_TRANSFORM_ADD = 42;

	if (ReturnValue)
	{
		ReturnValue->Value = TestInputInt * RPC_TRANSFORM_MULTIPLY + RPC_TRANSFORM_ADD;
	}

	UE_LOG(LogGorgeousAutomationTestOV, Log,
		TEXT("[AutomationRPC] OV handler on %s: TestInputInt=%d -> ReturnOV.Value=%d seq=%d from=%s"),
		*GetName(), TestInputInt, ReturnValue ? ReturnValue->Value : -1, Sequence, *Origin);
}
