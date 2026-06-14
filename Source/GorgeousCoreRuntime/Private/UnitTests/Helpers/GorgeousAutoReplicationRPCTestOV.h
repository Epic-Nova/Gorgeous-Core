// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|    Gorgeous Core - AutoReplication RPC Test Object Variable Helper       |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

/**
 * A minimal UGorgeousObjectVariable subclass used exclusively by the async-RPC
 * test scenarios to exercise the "first-parameter OV as return value" dispatch
 * convention without polluting the production base class with test UFUNCTIONs.
 *
 * SetupAsyncRPCTestOV() registers an instance of this class under the entry key
 * so that:
 *   - EObjectVariable / EAuto paths dispatch via InvokeNativeHandler on this OV
 *     and find Automation_HandleRPC_WithReturnOV by name.
 *   - The handler sets ReturnValue->Value = TestInputInt * 7 + 42, which the
 *     backend threads back as TargetVariable so E9 can verify the round-trip.
 */

#pragma once

#include "CoreMinimal.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/NativeObjectVariableDefinitions.h"
#include "GorgeousAutoReplicationRPCTestOV.generated.h"

/**
 * Test-only OV subclass that holds the universal Automation_HandleRPC_WithReturnOV
 * UFUNCTION for EObjectVariable / EAuto dispatch paths.
 *
 * Only compiled when the automation-test infrastructure is available; the UCLASS
 * itself is unconditional so UHT can reflect it in all configurations.
 */
UCLASS(NotBlueprintable, NotBlueprintType, Hidden)
class UGorgeousAutomationTestOV : public UGorgeousObjectVariable
{
	GENERATED_BODY()

public:
	/**
	 * Universal native RPC handler exercising the first-parameter OV return-value
	 * convention.  Parameter names match BuildVerifiablePayload exactly so the
	 * argument-bridge in InvokeNativeHandler can map them without error.
	 *
	 * The backend constructs a UInteger_SOV* and injects it as ReturnValue before
	 * calling ProcessEvent.  This handler populates it with:
	 *   ReturnValue->Value = TestInputInt * RPC_TRANSFORM_MULTIPLY + RPC_TRANSFORM_ADD
	 * After ProcessEvent, ExecuteAutoReplicationRPC threads the OV back as
	 * TargetVariable.  The E9 assertion verifies the computed value.
	 */
	UFUNCTION()
	void Automation_HandleRPC_WithReturnOV(UInteger_SOV* ReturnValue, int32 TestInputInt, const FString& TestInputString,
		int32 Sequence, const FString& Origin, const FString& Timestamp);
};
