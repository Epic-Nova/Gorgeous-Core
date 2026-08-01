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
#include "Subsystems/WorldSubsystem.h"
#include "UObject/ObjectKey.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousRootNetworkStackSubsystem.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGorgeousObjectVariable;
class AController;
//<------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Root Network Stack Subsystem
| Functional Name: UGorgeousRootNetworkStackSubsystem
| Parent Class: UWorldSubsystem
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Lightweight world subsystem that tracks root network stack registrations.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ObjectVariables/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ObjectVariables/GorgeousRootNetworkStackSubsystem",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ObjectVariables/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousRootNetworkStackSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UGorgeousRootNetworkStackSubsystem* Get(UWorld* World);
	static FName GetDefaultChannelName();

	void RegisterVariable(UGorgeousObjectVariable* Variable);
	void UnregisterVariable(UGorgeousObjectVariable* Variable);
	bool CanControllerAccess(const UGorgeousObjectVariable* Variable, AController* Controller) const;

	/** Automatically subscribes Controller to Variable's channel when policy allows. */
	bool TryAutoSubscribeController(const UGorgeousObjectVariable* Variable, AController* Controller) const;
	bool HasChannelSubscribers(FName ChannelName) const;

	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	void SubscribeControllerToChannel(AController* Controller, FName ChannelName);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	void UnsubscribeControllerFromChannel(AController* Controller, FName ChannelName);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	void ClearControllerSubscriptions(AController* Controller);

	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	bool IsControllerSubscribedToChannel(AController* Controller, FName ChannelName) const;

	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
	TArray<FName> GetControllerSubscriptions(AController* Controller) const;

private:
	struct FRootNetworkChannelControllerState
	{
		TWeakObjectPtr<AController> Controller;
		TSet<FName> Subscriptions;
	};

	void CleanupControllerEntry(const FObjectKey& ControllerKey) const;
	void RemoveControllerEntry(const FObjectKey& ControllerKey) const;
	void AddControllerSubscription(AController* Controller, FName ChannelName) const;
	void RemoveControllerSubscription(AController* Controller, FName ChannelName) const;

	// Registered variables that requested exposure through the root network stack.
	UPROPERTY()
	TSet<TWeakObjectPtr<UGorgeousObjectVariable>> RegisteredVariables;

	/** State buckets keyed by controller for quick channel lookup. */
	mutable TMap<FObjectKey, FRootNetworkChannelControllerState> ControllerStates;

	/** Reverse lookup that groups controllers by channel assignments. */
	mutable TMap<FName, TSet<FObjectKey>> ChannelMembership;
};