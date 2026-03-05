// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/ObjectKey.h"
#include "GorgeousRootNetworkStackSubsystem.generated.h"

class UGorgeousObjectVariable;
class AController;

/** Lightweight world subsystem that tracks root network stack registrations. */
UCLASS()
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

	/** Registered variables that requested exposure through the root network stack. */
	UPROPERTY()
	TSet<TWeakObjectPtr<UGorgeousObjectVariable>> RegisteredVariables;

	/** State buckets keyed by controller for quick channel lookup. */
	mutable TMap<FObjectKey, FRootNetworkChannelControllerState> ControllerStates;

	/** Reverse lookup that groups controllers by channel assignments. */
	mutable TMap<FName, TSet<FObjectKey>> ChannelMembership;
};
