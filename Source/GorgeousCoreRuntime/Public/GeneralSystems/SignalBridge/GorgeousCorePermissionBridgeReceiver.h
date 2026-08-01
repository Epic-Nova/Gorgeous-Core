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

//<=============================--- Includes ---============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include "Templates/GorgeousBaseWorldContextUObject.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousCorePermissionBridgeReceiver.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Payload sent via the Signal Bridge to request or respond to a permission check.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousPermissionSignalPayload_S
{
	GENERATED_BODY()

	// Name of the permission being evaluated (e.g., "com.gorgeous.simsalabim.chest.open").
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permission")
	FString PermissionName;

	// The character/player attempting the action.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permission")
	TObjectPtr<AActor> Interactor = nullptr;

	// The container or world entity being accessed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permission")
	TObjectPtr<AActor> TargetObject = nullptr;

	// Tag used to dispatch the response back.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permission")
	FGameplayTag ResponseTag;

	// The outcome of the evaluation. Set by the receiver before responding.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permission")
	bool bGranted = false;
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Core Permission Bridge Receiver
| Functional Name: UGorgeousCorePermissionBridgeReceiver
| Parent Class: UGorgeousBaseWorldContextUObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Receives Signal Bridge permission requests, evaluates them through the
| runtime permission systems, and dispatches the resulting response.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType, Blueprintable,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/SignalBridge/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/SignalBridge/GorgeousCorePermissionBridgeReceiver",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/SignalBridge/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousCorePermissionBridgeReceiver : public UGorgeousBaseWorldContextUObject
{
	GENERATED_BODY()

public:

	// Constructs the permission bridge receiver.
	UGorgeousCorePermissionBridgeReceiver();

	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Initializes the receiver and binds it to Signal Bridge listeners.
	 *
	 * @param WorldContextObject The context used to resolve the active world.
	 */
	UFUNCTION(BlueprintCallable, Category = "Permission")
	void InitializeReceiver(UObject* WorldContextObject);

	/**
	 * Handles a permission check request dispatched through the Signal Bridge.
	 *
	 * @param SignalTag The signal tag identifying the permission request.
	 * @param Payload The request payload to evaluate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Permission")
	void OnReceivePermissionRequest(FGameplayTag SignalTag, const FInstancedStruct& Payload);

	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
protected:
	/**
	 * Evaluates a permission request through the configured runtime permission systems.
	 *
	 * @param Request The permission request to evaluate.
	 * @return True when the request is granted, false otherwise.
	 */
	virtual bool EvaluatePermissionRequest(const FGorgeousPermissionSignalPayload_S& Request) const;
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only
};