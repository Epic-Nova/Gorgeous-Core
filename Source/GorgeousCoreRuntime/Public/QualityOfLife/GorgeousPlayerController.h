// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "GameFramework/PlayerController.h"
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousObjectVariableTrunk.h"
#include "ObjectVariables/NativeObjectVariableDefinitions.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRelayComponent.h"
#include "AutoReplication/GorgeousAutoReplicationRPCResponder_I.h"
#include "QualityOfLife/GorgeousQualityOfLifeNodeTarget_I.h"
#include "QualityOfLife/GorgeousPlayerConnectionInfo_I.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousPlayerController.generated.h"
//<-------------------------------------------------------------------------->

#if WITH_DEV_AUTOMATION_TESTS
struct FGorgeousAutomationRPCWitnessEntry
{
	FName HandlerName = NAME_None;
	int32 Sequence = 0;
	FString Stamp;
	TEnumAsByte<ENetRole> NetRole = ROLE_None;
	TEnumAsByte<ENetMode> NetMode = NM_Standalone;
	double TimestampSeconds = 0.0;
};
#endif

/**
 * A custom subclass of APlayerController used to manage player input and data.
 * 
 * This class extends APlayerController to provide additional functionality for managing player-related data 
 * and input. The `AGorgeousPlayerController` class allows the handling of extra data, such as input configurations 
 * or custom player attributes, through the `AdditionalGorgeousData` map. It also provides custom behavior for the 
 * `BeginPlay()` and `PostEditChangeProperty()` functions, which are used to initialize and modify the player controller 
 * in different scenarios, such as gameplay start or when properties are edited in the Unreal Editor.
 */
UCLASS(Blueprintable, BlueprintType)
class GORGEOUSCORERUNTIME_API AGorgeousPlayerController : public APlayerController
	, public IGorgeousAutoReplicationRPCResponder_I
	, public IGorgeousQualityOfLifeNodeTarget_I
	, public IGorgeousPlayerConnectionInfo_I
{
	GENERATED_BODY()
	
public:

	AGorgeousPlayerController();
	FGorgeousAutoReplicationMixin& GetAutoReplicationMixin() { return AutoReplicationMixin; }
	const FGorgeousAutoReplicationMixin& GetAutoReplicationMixin() const { return AutoReplicationMixin; }

	/** Registers or updates an AutoReplication entry at runtime. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Player Controller|Networking")
	bool RegisterAutoReplicationEntry(FName Key, TSubclassOf<UGorgeousObjectVariable> DefaultClass, bool bReplicate, bool bOverrideStreamConfig, FGorgeousAutoReplicationStreamConfig StreamConfigOverride);

	virtual void HandleAutoReplicationRPC_Implementation(const FGorgeousQueuedRPC& QueuedRPC) override;

#if WITH_DEV_AUTOMATION_TESTS
	const TArray<FGorgeousAutomationRPCWitnessEntry>& GetAutomationRPCWitnessEntries() const { return AutomationRPCWitnessEntries; }
	void ResetAutomationRPCWitnessEntries();
#endif
	
	//<============================--- Overrides ---=============================>
	
	/** 
	 * Called when the player controller begins play.
	 * 
	 * This function is called when the player controller starts. It can be overridden to initialize any player-specific
	 * functionality or data, such as setting up player input or other gameplay-related elements.
	 */
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	/** Removes this controller from the shared SelfReference OV when it is destroyed. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// IGorgeousPlayerConnectionInfo_I
	virtual bool IsRemoteNetConnection_Implementation() const override;
	virtual FUniqueNetIdRepl GetPlayerNetId_Implementation() const override;
	virtual FString GetGorgeousStablePlayerId_Implementation() const override;
	virtual int32 GetPlayerConnectionIndex_Implementation() const override;
	
	//<-------------------------------------------------------------------------->

	UFUNCTION()
	void Automation_ServerToClient(int32 Sequence, const FString& Stamp);

	UFUNCTION()
	void Automation_ClientToServer(int32 Sequence, const FString& Stamp);

	UFUNCTION()
	void Automation_Multicast(int32 Sequence, const FString& Stamp);

	/**
	 * Universal owner-side RPC handler used by ALL async-action unit tests.
	 * Implements the first-parameter OV return-value convention: the backend constructs a
	 * UInteger_SOV*, passes it as ReturnValue, and this handler populates it with
	 *   ReturnValue->Value = TestInputInt * RPC_TRANSFORM_MULTIPLY + RPC_TRANSFORM_ADD
	 * Parameter names match the arguments added by BuildVerifiablePayload exactly so that
	 * InvokeNativeAutoReplicationRPCHandlerOnObject can map them without error.
	 * The E9 assertion in RunAsyncRPCActionTest verifies the computed value round-trips.
	 */
	UFUNCTION()
	void Automation_HandleRPC_WithReturnOV(UInteger_SOV* ReturnValue, int32 TestInputInt, const FString& TestInputString, int32 Sequence, const FString& Origin, const FString& Timestamp);

	/** Enables networking features for this controller's AutoReplication data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Player Controller|Networking")
	bool bActivateNetworkingCapabilities;

	/**
	 * Additional data for the current class.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gorgeous Player Controller")
	TMap<FName, FGorgeousObjectVariableEntry> AdditionalGorgeousData;

	/** Trunk that stores serialized default payloads for this controller's authored object variables. */
	UPROPERTY(EditDefaultsOnly, Category = "Gorgeous Player Controller|Defaults", meta = (ShowOnlyInnerProperties))
	FGorgeousObjectVariableTrunk DefaultObjectVariableTrunk;

protected:

	/** Component responsible for relaying RPC results back to the authority. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gorgeous Player Controller|Networking", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGorgeousAutoReplicationRPCRelayComponent> AutoReplicationRPCRelay;

	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous Player Controller|Networking")
	void OnAutoReplicationRPCReceived(const FGorgeousQueuedRPC& QueuedRPC, bool bWasHandled);

	UPROPERTY(ReplicatedUsing = OnRep_GorgeousAutoReplicationVariables)
	TArray<FGorgeousReplicatedVariableEntry> ReplicatedAutoReplicationVariables;

	FGorgeousAutoReplicationMixin AutoReplicationMixin;
	
	UFUNCTION()
	void OnRep_GorgeousAutoReplicationVariables();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#if WITH_DEV_AUTOMATION_TESTS
	void RecordAutomationRPCWitness(FName HandlerName, int32 Sequence, const FString& Stamp);
	TArray<FGorgeousAutomationRPCWitnessEntry> AutomationRPCWitnessEntries;
#endif
};
