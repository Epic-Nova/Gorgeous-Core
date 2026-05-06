// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "SignalBridgeStructures.generated.h"

class AGorgeousPlayerController;

DECLARE_DYNAMIC_DELEGATE_OneParam(FSignalBridgeEventDelegate, const FInstancedStruct&, Payload);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSignalBridgeEventMulticastDelegate, const FInstancedStruct&, Payload);

/**
 * Rules for controlling access to a specific Signal Bridge tag.
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousSignalBridgeAccessRules_S
{
	GENERATED_BODY()

	FGorgeousSignalBridgeAccessRules_S()
		: bNetworked(true)
		, AccessPolicy(EGorgeousObjectVariableAccessPolicy::Everyone)
	{}

	/** Whether dispatches for this tag should be replicated across the network. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Signal Bridge")
	bool bNetworked;

	/** Who is allowed to listen to or receive dispatches for this tag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Signal Bridge")
	EGorgeousObjectVariableAccessPolicy AccessPolicy;

	/** Explicit controller classes allowed if AccessPolicy is Custom (optional filter). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Signal Bridge", meta = (EditCondition = "AccessPolicy == EGorgeousObjectVariableAccessPolicy::Custom"))
	TArray<TSubclassOf<AGorgeousPlayerController>> AllowedClasses;

	/** Specific controller instances allowed if AccessPolicy is Custom. Managed by Authority at runtime. */
	UPROPERTY(BlueprintReadOnly, Category = "Signal Bridge")
	TArray<TObjectPtr<AGorgeousPlayerController>> AllowedControllers;

	/** The controller that registered this signal. Used for 'OwningControllerOnly' policy. */
	UPROPERTY(BlueprintReadOnly, Category = "Signal Bridge")
	TObjectPtr<AGorgeousPlayerController> RegisteredBy;
};
