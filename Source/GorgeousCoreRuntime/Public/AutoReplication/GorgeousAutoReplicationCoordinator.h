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
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "Containers/Ticker.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGorgeousObjectVariable;
class UWorld;
class UReplicationGraph;
//<------------------------------------------------------------->

#if GORGEOUSCORE_WITH_IRIS
class FGorgeousAutoReplicationIrisBackend;
#endif

/** Coordinates auto-replication features such as Iris data streams and replication graph integration. */
class GORGEOUSCORERUNTIME_API FGorgeousAutoReplicationCoordinator
{
	friend class FGorgeousAutoReplicationMixin;
	friend class UGorgeousObjectVariable;
public:
	static FGorgeousAutoReplicationCoordinator& Get(UWorld* World);

	/**
	 * Tears down the coordinator only if its currently cached world matches DyingWorld.
	 * Safe to call from world-cleanup callbacks; does NOT re-initialize the coordinator.
	 * Used by FGorgeousCoreRuntimeModule to clean up before GC sweeps a PIE world.
	 */
	static void TearDownForWorld(UWorld* DyingWorld);

	void Initialize(UWorld* World);
	void TearDown();

	// Explicit out-of-line destructor so that destruction of TUniquePtr<FGorgeousAutoReplicationIrisBackend>
	// happens in the .cpp where FGorgeousAutoReplicationIrisBackend is a complete type. This avoids
	// MSVC C4150 errors when compilation units compile this header without Iris being enabled.
	~FGorgeousAutoReplicationCoordinator();

	// Register/unregister streams
	void RegisterObjectVariable(UGorgeousObjectVariable* Variable, const FGorgeousAutoReplicationStreamConfig& Config);
	void UnregisterObjectVariable(UGorgeousObjectVariable* Variable);

	void NotifyRPCBroadcast(const FGorgeousQueuedRPC& QueuedRPC, UGorgeousObjectVariable* TargetVariable);

	/** Called once per frame by the internal ticker to keep deferred initialization up to date. */
	void Tick(float DeltaSeconds);

	UReplicationGraph* GetOrCreateReplicationGraph(UWorld* World);

	// --- QoL override API (TODOs implementation) ---
	// Override whether Iris should be used at runtime. When set, this takes precedence
	// over the developer settings value until cleared.
	void SetUseIrisOverride(bool bEnable);
	void ClearUseIrisOverride();

	// Override whether the replication graph should be enabled at runtime. When set,
	// this takes precedence over developer settings until cleared.
	void SetReplicationGraphEnabledOverride(bool bEnable);
	void ClearReplicationGraphEnabledOverride();

	// Override the replication-graph class to use when creating the auto-replication
	// graph instance. This should be called before Initialize()/GetOrCreateReplicationGraph.
	void SetAutoReplicationGraphClassOverride(TSubclassOf<UReplicationGraph> InGraphClass);
	void ClearAutoReplicationGraphClassOverride();

private:
	FGorgeousAutoReplicationCoordinator();

	void InitializeReplicationGraph(UWorld* World);
	void InitializeIris(UWorld* World);
	void RefreshReplicationGraphStreams();
	bool HandleTickerTick(float DeltaSeconds);
	void MarkStreamDirty(UGorgeousObjectVariable* Variable);

private:
	TWeakObjectPtr<UWorld> CachedWorld;
	TWeakObjectPtr<UReplicationGraph> AutoReplicationGraph;
	TMap<FObjectKey, FGorgeousAutoReplicationActiveStream> ActiveStreams;
	bool bIrisInitialized;
	bool bReplicationGraphInitialized;
	bool bWarnedReplicationGraphIrisConflict;
	FTSTicker::FDelegateHandle TickHandle;
#if GORGEOUSCORE_WITH_IRIS
	TUniquePtr<FGorgeousAutoReplicationIrisBackend> IrisBackend;
#endif

	// QoL override state (if set, overrides developer settings at runtime)
	bool bUseIrisOverrideSet = false;
	bool bUseIrisOverrideValue = false;

	bool bReplicationGraphEnabledOverrideSet = false;
	bool bReplicationGraphEnabledOverrideValue = false;

	bool bIsInitializingReplicationGraph;
	TSubclassOf<UReplicationGraph> AutoReplicationGraphClassOverride;
};
