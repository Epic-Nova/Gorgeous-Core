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
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "QualityOfLife/GorgeousPlayerController.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "SignalBridgeStructures.generated.h"
//<-------------------------------------------------------------------------->

DECLARE_DYNAMIC_DELEGATE_TwoParams(FSignalBridgeEventDelegate, FGameplayTag, SignalTag, const FInstancedStruct&, Payload);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSignalBridgeEventMulticastDelegate, FGameplayTag, SignalTag, const FInstancedStruct&, Payload);

/**
 * Simple int32 payload for Signal Bridge.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousInt32Payload_S
{
	GENERATED_BODY()

	// Integer value carried by the signal.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Signal Bridge")
	int32 Value = 0;

	FGorgeousInt32Payload_S() {}
	FGorgeousInt32Payload_S(int32 InValue) : Value(InValue) {}
};

/**
 * Simple float payload for Signal Bridge.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousFloatPayload_S
{
	GENERATED_BODY()

	// Floating-point value carried by the signal.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Signal Bridge")
	float Value = 0.0f;

	FGorgeousFloatPayload_S() {}
	FGorgeousFloatPayload_S(float InValue) : Value(InValue) {}
};

/**
 * Simple FName payload for Signal Bridge.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousFNamePayload_S
{
	GENERATED_BODY()

	// Name value carried by the signal.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Signal Bridge")
	FName Value = NAME_None;

	FGorgeousFNamePayload_S() {}
	FGorgeousFNamePayload_S(FName InValue) : Value(InValue) {}
};

/**
 * Rules for controlling access to a specific Signal Bridge tag.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousSignalBridgeAccessRules_S
{
	GENERATED_BODY()

	FGorgeousSignalBridgeAccessRules_S()
		: bNetworked(true)
		, AccessPolicy(EGorgeousObjectVariableAccessPolicy::Everyone)
	{}

	// Whether dispatches for this tag should be replicated across the network.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Signal Bridge")
	bool bNetworked;

	// Who is allowed to listen to or receive dispatches for this tag.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Signal Bridge")
	EGorgeousObjectVariableAccessPolicy AccessPolicy;

	// Explicit controller classes allowed if AccessPolicy is Custom (optional filter).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Signal Bridge", meta = (EditCondition = "AccessPolicy == EGorgeousObjectVariableAccessPolicy::Custom"))
	TArray<TSubclassOf<AGorgeousPlayerController>> AllowedClasses;

	// Specific controller instances allowed if AccessPolicy is Custom. Managed by Authority at runtime.
	UPROPERTY(BlueprintReadOnly, Category = "Signal Bridge")
	TArray<TObjectPtr<AGorgeousPlayerController>> AllowedControllers;

	// The controller that registered this signal. Used for 'OwningControllerOnly' policy.
	UPROPERTY(BlueprintReadOnly, Category = "Signal Bridge")
	TObjectPtr<AGorgeousPlayerController> RegisteredBy;

	bool operator==(const FGorgeousSignalBridgeAccessRules_S& Other) const
	{
		return bNetworked == Other.bNetworked &&
			AccessPolicy == Other.AccessPolicy &&
			AllowedClasses == Other.AllowedClasses &&
			AllowedControllers == Other.AllowedControllers &&
			RegisteredBy == Other.RegisteredBy;
	}
};