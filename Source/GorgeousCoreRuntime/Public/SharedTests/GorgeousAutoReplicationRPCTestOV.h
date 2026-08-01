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
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/NativeObjectVariableDefinitions.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousAutoReplicationRPCTestOV.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Automation Test OV
| Functional Name: UGorgeousAutomationTestOV
| Parent Class: UGorgeousObjectVariable
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Test-only OV subclass that holds the universal
| Automation_HandleRPC_WithReturnOV UFUNCTION for EObjectVariable / EAuto
| dispatch paths. Only compiled when the automation-test infrastructure is
| available; the UCLASS itself is unconditional so UHT can reflect it in all
| configurations.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(NotBlueprintable, NotBlueprintType, Hidden,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/SharedTests/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/SharedTests/GorgeousAutomationTestOV",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/SharedTests/Examples/"
		)
)
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