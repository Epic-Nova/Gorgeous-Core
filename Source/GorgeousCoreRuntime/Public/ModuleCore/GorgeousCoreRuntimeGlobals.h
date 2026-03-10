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
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "ReplicationGraph.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousCoreRuntimeGlobals.generated.h"
//<-------------------------------------------------------------------------->

class UGorgeousAutoReplicationRPCRequestAsyncAction;
class APlayerController;

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

	/**
	 * Fetches a registered QoL object reference using a constrained dropdown that updates the return type.
	 * Intended for singleton QoL classes (GameMode, GameState, PlayerState, WorldSettings, etc.) where
	 * exactly one instance exists. For PlayerController classes in splitscreen use GetQualityOfLifeReferences.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals|Quality Of Life", meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "QualityOfLifeClass", CompactNodeTitle = "Get QoL Reference"))
	static UObject* GetQualityOfLifeReference(const UObject* WorldContextObject,
		UPARAM(meta = (MustImplement = "/Script/GorgeousCoreRuntime.GorgeousQualityOfLifeNodeTarget_I", AllowAbstract = "false"))
		TSubclassOf<UObject> QualityOfLifeClass);

	/**
	 * Shorthand function to refresh the replication entries for GorgeousAdditionalData of a QoL class, intended for use in Blueprint after registering new AutoReplication entries at runtime.
	 * This function will soon be deprecated as the Auto Replication system currently does not have functionality for detecting a newly registered entry and refreshing the replication state accordingly, 
	 * but we plan to add that in the near future. Therefore, we provide this function as a temporary workaround to allow Blueprint users to manually trigger the replication refresh when they know they've registered new entries at runtime
	 * 
	 * @param WorldContextObject The world context object used to find the relevant QoL class instance.
	 * @param QualityOfLifeClass The QoL class to refresh replication for. Must implement the GorgeousQualityOfLifeNodeTarget_I interface.
	 * @see FGorgeousAutoReplicationMixin::InitializeAdditionalData() for the underlying logic that iterates AdditionalGorgeousData and sets up replication for entries with bReplicate=true, which is what this function calls under the hood.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|Quality Of Life", meta = (WorldContext = "WorldContextObject"))
	static void RefreshQualityOfLifeReplication(UObject* WorldContextObject,
		UPARAM(meta = (MustImplement = "/Script/GorgeousCoreRuntime.GorgeousQualityOfLifeNodeTarget_I", AllowAbstract = "false"))
		TSubclassOf<UObject> QualityOfLifeClass);

	/**
	 * Returns ALL objects stored in the shared SelfReference OV for this class.
	 * This variant is only meaningfully different from GetQualityOfLifeReference for
	 * PlayerController QoL classes in splitscreen: each local PC self-registers into
	 * the same shared OV, so the array contains one entry per active local player.
	 * For all singleton classes (GameMode, GameState, etc.) the array will always
	 * contain at most one element — use GetQualityOfLifeReference instead.
	 * If StablePlayerId is non-empty only the entry whose owning PlayerController is
	 * registered under that stable ID is returned (at most one element).
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals|Quality Of Life",
		meta = (WorldContext = "WorldContextObject",
			   DeterminesOutputType = "QualityOfLifeClass",
			   CompactNodeTitle = "Get QoL References",
			   AdvancedDisplay = "StablePlayerId"))
	static TArray<UObject*> GetQualityOfLifeReferences(
		const UObject* WorldContextObject,
		UPARAM(meta = (MustImplement = "/Script/GorgeousCoreRuntime.GorgeousQualityOfLifeNodeTarget_I", AllowAbstract = "false"))
		TSubclassOf<UObject> QualityOfLifeClass,
		const FString& StablePlayerId = TEXT(""));

#pragma region LocalPlayer_StableId

	/** Explicitly registers (or re-registers) a PlayerController under a custom stable string ID. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|Quality Of Life|Local Players",
		meta = (WorldContext = "WorldContextObject", ReturnDisplayName = "Success"))
	static bool RegisterLocalPlayerStableId(const UObject* WorldContextObject,
		APlayerController* PlayerController, const FString& StableId);

	/**
	 * Renames the stable ID already associated with a PlayerController.
	 * Returns false when NewStableId is taken by a different live player.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|Quality Of Life|Local Players",
		meta = (WorldContext = "WorldContextObject", ReturnDisplayName = "Success"))
	static bool RenameLocalPlayerStableId(const UObject* WorldContextObject,
		APlayerController* PlayerController, const FString& NewStableId);

	/** Returns the stable string ID registered for a PlayerController, or an empty string. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals|Quality Of Life|Local Players",
		meta = (WorldContext = "WorldContextObject"))
	static FString GetLocalPlayerStableId(const UObject* WorldContextObject,
		const APlayerController* PlayerController);

	/** Returns the PlayerController currently registered under StableId, or nullptr. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals|Quality Of Life|Local Players",
		meta = (WorldContext = "WorldContextObject"))
	static APlayerController* GetPlayerControllerForStableId(const UObject* WorldContextObject,
		const FString& StableId);

	/**
	 * Returns all registered stable IDs (OutStableIds) and their corresponding
	 * auto-assigned numeric player indices (OutPlayerIndices) as two parallel arrays.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Globals|Quality Of Life|Local Players",
		meta = (WorldContext = "WorldContextObject"))
	static void GetAllRegisteredLocalPlayers(const UObject* WorldContextObject,
		TArray<FString>& OutStableIds, TArray<int32>& OutPlayerIndices);

#pragma endregion LocalPlayer_StableId

#pragma region AutoReplication_Networking_Functions
	/** Fetches the network-aware AutoReplication value stored under the provided key on the given AutoReplication context. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|AutoReplication|Networking", meta = (WorldContext = "WorldContextObject"))
	static bool GetNetGorgeousAutoReplicationValue(UObject* WorldContextObject, FName Key, UGorgeousObjectVariable*& OutValue, UObject* AutoReplicationOwner = nullptr);

	/** Sets a AutoReplication value through the mixin, routing through replication when available. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Globals|AutoReplication|Networking", meta = (WorldContext = "WorldContextObject"))
	static bool SetNetGorgeousAutoReplicationValue(UObject* WorldContextObject, FName Key, UGorgeousObjectVariable* NewValue, UObject* AutoReplicationOwner = nullptr);

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

