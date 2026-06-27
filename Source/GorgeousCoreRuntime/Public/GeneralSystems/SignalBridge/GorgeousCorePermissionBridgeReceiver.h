// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/GorgeousBaseWorldContextUObject.h"
#include "GameplayTagContainer.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
#include "GorgeousCorePermissionBridgeReceiver.generated.h"

/**
 * Payload sent via the Signal Bridge to request or respond to a permission check.
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousPermissionSignalPayload_S
{
	GENERATED_BODY()

	/** Name of the permission being evaluated (e.g., "com.gorgeous.simsalabim.chest.open"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permission")
	FString PermissionName;

	/** The character/player attempting the action. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permission")
	TObjectPtr<AActor> Interactor = nullptr;

	/** The container or world entity being accessed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permission")
	TObjectPtr<AActor> TargetObject = nullptr;

	/** Tag used to dispatch the response back. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permission")
	FGameplayTag ResponseTag;

	/** The outcome of the evaluation. Set by the receiver before responding. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permission")
	bool bGranted = false;
};

/**
 * Receives and processes permission callbacks from the Signal Bridge.
 * Links the Inventory/World systems to the Core Dev Settings and runtime Team system.
 */
UCLASS(BlueprintType, Blueprintable)
class GORGEOUSCORERUNTIME_API UGorgeousCorePermissionBridgeReceiver : public UGorgeousBaseWorldContextUObject
{
	GENERATED_BODY()

public:
	UGorgeousCorePermissionBridgeReceiver();

	/** Initializes the receiver and binds listeners to the Signal Bridge. */
	UFUNCTION(BlueprintCallable, Category = "Permission")
	void InitializeReceiver(UObject* WorldContextObject);

	/** Callback invoked when a permission check request is dispatched via the Signal Bridge. */
	UFUNCTION(BlueprintCallable, Category = "Permission")
	void OnReceivePermissionRequest(FGameplayTag SignalTag, const FInstancedStruct& Payload);

protected:
	/**
	 * Internal evaluation function that hooks into Dev Settings and the Team System.
	 */
	virtual bool EvaluatePermissionRequest(const FGorgeousPermissionSignalPayload_S& Request) const;
};
