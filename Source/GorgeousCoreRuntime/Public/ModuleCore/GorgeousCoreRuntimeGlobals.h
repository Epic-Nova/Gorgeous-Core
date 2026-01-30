// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "ReplicationGraph.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousCoreRuntimeGlobals.generated.h"
//<-------------------------------------------------------------------------->

class UGorgeousAutoReplicationRPCRequestAsyncAction;

/**
 * Class extended by all other classes that are part of the Gorgeous Things ecosystem.
 *
 * Used to provide a static way to extend the functionality of the Gorgeous Things ecosystem.
 * Although the accessing of these functionalities is mostly done through wrapper classes that are widely accessible in the engine (blueprint).
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeous : public UObject
{
	GENERATED_BODY()
};

/**
 * Interface class extended by all other interface classes that are part of the Gorgeous Things ecosystem.
 *
 * Used to provide a static way to extend the functionality of the Gorgeous Things ecosystem for interfaces.
 *
 * @note This class is intended to be used as a base for interface implementations within the Gorgeous Things ecosystem.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousInterface : public UGorgeous
{
	GENERATED_BODY()

protected:

	/**
	 * Protected constructor to prevent direct instantiation.
	 */
	explicit UGorgeousInterface() {}
};

/**
 * Globals class for the Gorgeous Core Runtime module.
 *
 * Exposes static functions and properties that are used throughout the module.
 */
UCLASS(BlueprintType, DisplayName = "Gorgeous Core Globals")
class UGorgeousCoreRuntimeGlobals : public UObject
{
	GENERATED_BODY()

public:
	
	/**
	 *	Returns the reference to the Gorgeous World Settings,
	 *	Requires to set AGorgeousWorldSettings or a child of it to be set in the Project Settings to work.
	 * 
	 * @return The current instance of the Gorgeous World Settings.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals", meta = (WorldContext = "WorldContextObject"))
	static class AGorgeousWorldSettings* GetGorgeousWorldSettings(const UObject* WorldContextObject);

	/** Resolves an object variable by its globally registered display name. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals|Object Variables")
	static UGorgeousObjectVariable* GetNamedObjectVariable(FName DisplayName, bool bLogWarning = true);

	/** Resolves a UObject reference stored inside a named object variable (expects an Object Single OV). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals|Object Variables")
	static UObject* GetNamedObjectReference(FName DisplayName, bool bLogWarning = true);

	/** Fetches a registered QoL object reference using a constrained dropdown that updates the return type. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals|Quality Of Life", meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "QualityOfLifeClass", CompactNodeTitle = "Get QoL Reference"))
	static UObject* GetQualityOfLifeReference(const UObject* WorldContextObject,
		UPARAM(meta = (MustImplement = "/Script/GorgeousCoreRuntime.GorgeousQualityOfLifeNodeTarget_I", AllowAbstract = "false"))
		TSubclassOf<UObject> QualityOfLifeClass);





#pragma region AutoReplication_Networking_Functions
	/** Fetches the network-aware AutoReplication value stored under the provided key on the given AutoReplication context. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|AutoReplication|Networking", meta = (WorldContext = "WorldContextObject"))
	static bool GetNetGorgeousAutoReplicationValue(UObject* WorldContextObject, FName Key, UGorgeousObjectVariable*& OutValue, UObject* AutoReplicationOwner = nullptr);

	/** Sets a AutoReplication value through the mixin, routing through replication when available. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|AutoReplication|Networking", meta = (WorldContext = "WorldContextObject"))
	static bool SetNetGorgeousAutoReplicationValue(UObject* WorldContextObject, FName Key, UGorgeousObjectVariable* NewValue, UObject* AutoReplicationOwner = nullptr);

	/** Queues an asynchronous AutoReplication RPC request driven by the owning mixin. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|AutoReplication|Networking", meta = (WorldContext = "WorldContextObject", DeprecatedFunction, DeprecationMessage = "Use the Request AutoReplication RPC async action node to receive dispatcher callbacks."))
	static bool RequestAutoReplicationRPC(UObject* WorldContextObject, FName Key, EGorgeousAutoReplicationRPCType Type, const FGorgeousRPCPayload& Payload, EGorgeousAutoReplicationTargetKind TargetKind = EGorgeousAutoReplicationTargetKind::EAuto, UObject* AutoReplicationOwner = nullptr);

	/** Returns true when any RPC requests are waiting to be processed on the provided context. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|Networking", meta = (WorldContext = "WorldContextObject"))
	static bool HasPendingAutoReplicationRPC(UObject* WorldContextObject, UObject* AutoReplicationOwner = nullptr);

	/** Pops the next queued RPC so Blueprint logic can respond asynchronously. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|Networking", meta = (WorldContext = "WorldContextObject"))
	static bool DequeuePendingAutoReplicationRPC(UObject* WorldContextObject, FGorgeousQueuedRPC& OutRPC, UObject* AutoReplicationOwner = nullptr);

	/** Returns the default AutoReplication stream config currently applied to newly created variables. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals|AutoReplication|Configuration")
	static FGorgeousAutoReplicationStreamConfig GetDefaultAutoReplicationStreamConfig();

	/** Overrides the default stream config applied to variables that do not specify their own settings. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|AutoReplication|Configuration")
	static void SetDefaultAutoReplicationStreamConfig(const FGorgeousAutoReplicationStreamConfig& NewConfig);

	/** Adds or updates a developer-settings override for a specific AutoReplication entry key. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|AutoReplication|Configuration")
	static void SetAutoReplicationStreamOverride(FName EntryKey, const FGorgeousAutoReplicationStreamConfig& NewConfig);

	/** Reads a developer-settings override for a specific AutoReplication entry key. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals|AutoReplication|Configuration")
	static bool GetAutoReplicationStreamOverride(FName EntryKey, FGorgeousAutoReplicationStreamConfig& OutConfig);

	/** Removes the developer-settings override for the provided entry key. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|AutoReplication|Configuration")
	static void ClearAutoReplicationStreamOverride(FName EntryKey);

	/** Enables or disables Iris/ReplicationGraph backends at runtime via developer settings. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|AutoReplication|Configuration", meta = (WorldContext = "WorldContextObject"))
	static void SetAutoReplicationBackendsEnabled(const UObject* WorldContextObject, bool bEnableIris, bool bEnableReplicationGraph);

	/** Returns true if Iris initialization is currently enabled for AutoReplication. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals|AutoReplication|Configuration")
	static bool IsAutoReplicationIrisEnabled();

	/** Returns true if the AutoReplication-specific replication graph is enabled. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals|AutoReplication|Configuration")
	static bool IsAutoReplicationReplicationGraphEnabled();

	/** Blueprint: Force-enable or -disable Iris at runtime (override developer settings). */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|AutoReplication|Configuration", meta = (WorldContext = "WorldContextObject"))
	static void SetAutoReplicationUseIrisOverride(const UObject* WorldContextObject, bool bEnable);

	/** Blueprint: Clear the Iris runtime override (revert to developer settings). */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|AutoReplication|Configuration", meta = (WorldContext = "WorldContextObject"))
	static void ClearAutoReplicationUseIrisOverride(const UObject* WorldContextObject);

	/** Blueprint: Force-enable or -disable the replication graph at runtime (override developer settings). */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|AutoReplication|Configuration", meta = (WorldContext = "WorldContextObject"))
	static void SetAutoReplicationReplicationGraphOverride(const UObject* WorldContextObject, bool bEnable);

	/** Blueprint: Clear the replication graph runtime override (revert to developer settings). */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|AutoReplication|Configuration", meta = (WorldContext = "WorldContextObject"))
	static void ClearAutoReplicationReplicationGraphOverride(const UObject* WorldContextObject);

	/** Blueprint: Set the auto-replication graph class override. Pass a class derived from UReplicationGraph. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|AutoReplication|Configuration", meta = (WorldContext = "WorldContextObject"))
	static void SetAutoReplicationGraphClassOverride(const UObject* WorldContextObject, TSubclassOf<UReplicationGraph> GraphClass);

	/** Blueprint: Clear the auto-replication graph class override. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|AutoReplication|Configuration", meta = (WorldContext = "WorldContextObject"))
	static void ClearAutoReplicationGraphClassOverride(const UObject* WorldContextObject);

	/** Resolves the effective stream config for a given AutoReplication entry, factoring in Blueprint overrides and developer settings. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|AutoReplication|Configuration", meta = (WorldContext = "WorldContextObject"))
	static bool GetEffectiveAutoReplicationStreamConfig(UObject* WorldContextObject, FName EntryKey, FGorgeousAutoReplicationStreamConfig& OutConfig, UObject* AutoReplicationOwner = nullptr);

	/** Applies configured AutoReplication settings and forces the coordinator to initialize for the provided world. */
	static void InitializeAutoReplicationForWorld(class UWorld* World);

#pragma endregion AutoReplication_Networking_Functions
};

