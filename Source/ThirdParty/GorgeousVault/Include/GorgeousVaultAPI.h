// Copyright (c) 2026-2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|              GorgeousVault — C-Linkage Export Table                        |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026-2026 Gorgeous Things by Simsalabim Studios,    |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|  This header defines the flat C function table exported by the Vault DLL. |
|  No C++ name mangling — no vtables cross the DLL boundary.               |
|                                                                           |
|  This file ships with the open-source GorgeousCore headers.              |
|  The IMPLEMENTATIONS live inside the Vault DLL (never distributed as     |
|  source).                                                                 |
<==========================================================================*/
#pragma once

#include "GorgeousVaultPlatform.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// ═══════════════════════════════════════════════════════════════════════════
//  Vault Lifecycle
// ═══════════════════════════════════════════════════════════════════════════

/** Initialize the Vault. Called once on DLL load. Returns GVault_OK on success. */
VAULT_API int32_t GVault_Initialize(void);

/** Shutdown and cleanup the Vault. Called on module unload. */
VAULT_API void GVault_Shutdown(void);

/** Returns the Vault version as (major * 10000 + minor * 100 + patch). */
VAULT_API int32_t GVault_GetVersion(void);

// ═══════════════════════════════════════════════════════════════════════════
//  Object Variable — Core Lifecycle
// ═══════════════════════════════════════════════════════════════════════════

/** Invoke instanced functionality on an Object Variable. */
VAULT_API int32_t GVault_OV_InvokeInstancedFunctionality(GVaultOVHandle Self, const void* NewGuid);

/** BeginDestroy hook for Object Variables. */
VAULT_API void GVault_OV_BeginDestroy(GVaultOVHandle Self);

/** Register a child variable into the parent's registry. */
VAULT_API int32_t GVault_OV_RegisterWithRegistry(GVaultOVHandle Self, GVaultOVHandle NewChild);

/** Set the parent of an Object Variable. */
VAULT_API int32_t GVault_OV_SetParent(GVaultOVHandle Self, GVaultOVHandle NewParent);

/** Apply a replicated identifier to an Object Variable. */
VAULT_API int32_t GVault_OV_ApplyReplicatedIdentifier(GVaultOVHandle Self, const void* InIdentifier);

// ═══════════════════════════════════════════════════════════════════════════
//  Object Variable — Serialization
// ═══════════════════════════════════════════════════════════════════════════

/** Serialize an Object Variable into a payload buffer. */
VAULT_API int32_t GVault_OV_SerializeToPayload(GVaultOVHandle Self, GVaultPayloadHandle OutPayload);

/** Deserialize an Object Variable from a payload buffer. */
VAULT_API int32_t GVault_OV_DeserializeFromPayload(GVaultOVHandle Self, GVaultPayloadHandle InPayload);

// ═══════════════════════════════════════════════════════════════════════════
//  Object Variable — Replication
// ═══════════════════════════════════════════════════════════════════════════

/** Activate replication for an Object Variable with the given context. */
VAULT_API int32_t GVault_OV_ActivateReplication(GVaultOVHandle Self, const void* ReplicationContext);

/** Register a property for AutoReplication. */
VAULT_API int32_t GVault_OV_RegisterReplicatedProperty(GVaultOVHandle Self, const void* PropertyName,
                                                        int32_t ReplicationMode, int32_t bSendInitialState,
                                                        const void* AdvancedConfig);

/** Bind an RPC handler for AutoReplication. */
VAULT_API int32_t GVault_OV_BindRPCHandler(GVaultOVHandle Self, const void* RPCName, int32_t Reliability);

/** Request an AutoReplication RPC. */
VAULT_API int32_t GVault_OV_RequestAutoReplicationRPC(GVaultOVHandle Self, int32_t Type,
                                                       const void* Payload, const void* OverrideKey,
                                                       GVaultHandle OverrideContext, int32_t TargetKind);

/** Build an AutoReplication property payload. */
VAULT_API int32_t GVault_OV_BuildAutoReplicationPropertyPayload(GVaultOVHandle Self,
                                                                 const void* ConditionContext,
                                                                 void* OutPayload);

/** Apply an AutoReplication property payload. */
VAULT_API int32_t GVault_OV_ApplyAutoReplicationPropertyPayload(GVaultOVHandle Self,
                                                                 const void* Payload,
                                                                 GVaultHandle PackageMap);

/** Execute a queued AutoReplication RPC. */
VAULT_API int32_t GVault_OV_ExecuteAutoReplicationRPC(GVaultOVHandle Self, const void* QueuedRPC,
                                                       GVaultOVHandle* OutReturnVariable);

/** Check if executing on the replication owner. */
VAULT_API int32_t GVault_OV_IsExecutingOnReplicationOwner(GVaultOVHandle Self);

/** Set display name. */
VAULT_API int32_t GVault_OV_SetDisplayName(GVaultOVHandle Self, const void* InDisplayName);

/** Set networking enabled/disabled. */
VAULT_API void GVault_OV_SetNetworkingEnabled(GVaultOVHandle Self, int32_t bShouldReplicate);

/** Validate a variable assignment. */
VAULT_API int32_t GVault_OV_ValidateVariableAssignment(GVaultOVHandle Self, const void* PropertyName,
                                                        const void* ValueProperty, const void* ValueAddress);

/** Get lifetime replicated properties. */
VAULT_API void GVault_OV_GetLifetimeReplicatedProps(GVaultOVHandle Self, void* OutLifetimeProps);

/** GetFunctionCallspace override. */
VAULT_API int32_t GVault_OV_GetFunctionCallspace(GVaultOVHandle Self, GVaultHandle Function, GVaultHandle Stack);

/** CallRemoteFunction override. */
VAULT_API int32_t GVault_OV_CallRemoteFunction(GVaultOVHandle Self, GVaultHandle Function,
                                                void* Parameters, void* OutParms, GVaultHandle Stack);

/** Static: Invoke a native AutoReplication RPC handler on any UObject. */
VAULT_API int32_t GVault_OV_InvokeNativeRPCHandlerOnObject(GVaultHandle Target, const void* QueuedRPC,
                                                             GVaultOVHandle* OutReturnVariable);

// ═══════════════════════════════════════════════════════════════════════════
//  Object Variable — Network Stack Control
// ═══════════════════════════════════════════════════════════════════════════

VAULT_API void GVault_OV_SetRootNetworkStackEnabled(GVaultOVHandle Self, int32_t bEnable);
VAULT_API void GVault_OV_SetSharedNetworkStackEnabled(GVaultOVHandle Self, int32_t bEnable);
VAULT_API void GVault_OV_SetAutoReplicationRespectAccessPolicy(GVaultOVHandle Self, int32_t bEnable);
VAULT_API void GVault_OV_SetNetworkAccessPolicy(GVaultOVHandle Self, int32_t NewPolicy, int32_t bApplyToSharedStack);
VAULT_API void GVault_OV_EnsureSharedNetworkStackOwner(GVaultOVHandle Self, GVaultHandle NewOwner);

// ═══════════════════════════════════════════════════════════════════════════
//  Root Object Variable
// ═══════════════════════════════════════════════════════════════════════════

/** Get or lazily create the named root variable. */
VAULT_API GVaultOVHandle GVault_Root_GetRootObjectVariable(const void* RootName);

/** Try get an existing root without creating one. */
VAULT_API GVaultOVHandle GVault_Root_TryGetExistingRoot(const void* RootName);

/** Get all registered root names. */
VAULT_API int32_t GVault_Root_GetRegisteredRootNames(void* OutNames);

/** Find a variable by its GUID. */
VAULT_API GVaultOVHandle GVault_Root_FindVariableByIdentifier(const void* Identifier);

/** Find a variable by display name. */
VAULT_API GVaultOVHandle GVault_Root_FindVariableByDisplayName(const void* InDisplayName);

/** Register a variable into a root's registry. */
VAULT_API int32_t GVault_Root_RegisterWithRegistry(GVaultOVHandle Self, GVaultOVHandle NewVariable);

/** Remove a variable from the registry. */
VAULT_API void GVault_Root_RemoveVariableFromRegistry(GVaultOVHandle Variable);

/** Check if a variable is registered. */
VAULT_API int32_t GVault_Root_IsVariableRegistered(GVaultOVHandle Variable);

/** Cleanup the registry. */
VAULT_API void GVault_Root_CleanupRegistry(int32_t bFullCleanup);

/** Claim root registry ownership. */
VAULT_API int32_t GVault_Root_ClaimOwnership(const void* RootName, const void* StableIdentifier,
                                              GVaultHandle OwningContext, void* OutHandle);

/** Release root registry ownership. */
VAULT_API void GVault_Root_ReleaseOwnership(const void* Handle, GVaultHandle CachedOwner);

// ═══════════════════════════════════════════════════════════════════════════
//  Object Variable Trunk
// ═══════════════════════════════════════════════════════════════════════════

/** Serialize: Write an OV to a payload. */
VAULT_API int32_t GVault_Trunk_WriteObjectToPayload(GVaultOVHandle Source, GVaultPayloadHandle OutPayload);

/** Deserialize: Load an OV from a payload. */
VAULT_API int32_t GVault_Trunk_LoadObjectFromPayload(GVaultOVHandle Destination, GVaultPayloadHandle InPayload);

/** Compute payload hash for integrity/cache invalidation. */
VAULT_API uint32_t GVault_Trunk_ComputePayloadHash(const void* Bytes, int32_t ByteCount);

// ═══════════════════════════════════════════════════════════════════════════
//  Object Variable Registry Subsystem
// ═══════════════════════════════════════════════════════════════════════════

/** Initialize the registry subsystem. */
VAULT_API int32_t GVault_Registry_Initialize(GVaultHandle Subsystem, GVaultHandle Collection);

/** Deinitialize the registry subsystem. */
VAULT_API void GVault_Registry_Deinitialize(GVaultHandle Subsystem);

// ═══════════════════════════════════════════════════════════════════════════
//  AutoReplication — Mixin
// ═══════════════════════════════════════════════════════════════════════════

/** Bind the mixin to an owner and its storage containers. */
VAULT_API int32_t GVault_Mixin_Bind(GVaultHandle Self, GVaultHandle InOwner,
                                     void* InAdditionalData, void* InReplicatedVariables);

/** Initialize additional data and optionally activate networking. */
VAULT_API int32_t GVault_Mixin_InitializeAdditionalData(GVaultHandle Self,
                                                         int32_t bActivateNetworkingCapabilities);

/** Set a replicated value. */
VAULT_API int32_t GVault_Mixin_TrySetReplicatedValue(GVaultHandle Self, const void* Key, GVaultOVHandle NewValue);

/** Get a value preferring replicated data. */
VAULT_API int32_t GVault_Mixin_TryGetValue(GVaultHandle Self, const void* Key, GVaultOVHandle* OutValue);

/** Refresh cached values after replication. */
VAULT_API void GVault_Mixin_RefreshCachedValues(GVaultHandle Self);

/** Request an RPC through the mixin. */
VAULT_API int32_t GVault_Mixin_RequestRPC(GVaultHandle Self, const void* Key, int32_t Type,
                                           const void* Payload, int32_t TargetKind, void* OutRequestGuid);

/** Execute a queued RPC. */
VAULT_API int32_t GVault_Mixin_ExecuteAutoReplicationRPC(GVaultHandle Self, const void* QueuedRPC);

/** Dispatch all pending RPCs. */
VAULT_API void GVault_Mixin_DispatchPendingRPCs(GVaultHandle Self);

/** Get the replicated entry count. */
VAULT_API int32_t GVault_Mixin_GetReplicatedEntryCount(GVaultHandle Self);

// ═══════════════════════════════════════════════════════════════════════════
//  AutoReplication — Coordinator
// ═══════════════════════════════════════════════════════════════════════════

/** Get or create the coordinator for a given world. */
VAULT_API GVaultHandle GVault_Coordinator_Get(GVaultWorldHandle World);

/** Initialize the coordinator. */
VAULT_API int32_t GVault_Coordinator_Initialize(GVaultHandle Self, GVaultWorldHandle World);

/** Tear down the coordinator. */
VAULT_API void GVault_Coordinator_TearDown(GVaultHandle Self);

/** Register an OV for auto-replication. */
VAULT_API int32_t GVault_Coordinator_RegisterVariable(GVaultHandle Self, GVaultOVHandle Variable,
                                                       const void* StreamConfig);

/** Unregister an OV from auto-replication. */
VAULT_API void GVault_Coordinator_UnregisterVariable(GVaultHandle Self, GVaultOVHandle Variable);

/** Notify the coordinator of an RPC broadcast. */
VAULT_API void GVault_Coordinator_NotifyRPCBroadcast(GVaultHandle Self, const void* QueuedRPC,
                                                      GVaultOVHandle TargetVariable);

/** Per-frame tick. */
VAULT_API void GVault_Coordinator_Tick(GVaultHandle Self, float DeltaSeconds);

// ═══════════════════════════════════════════════════════════════════════════
//  AutoReplication — Graph
// ═══════════════════════════════════════════════════════════════════════════

/** Initialize global graph nodes. */
VAULT_API int32_t GVault_Graph_InitGlobalGraphNodes(GVaultHandle Self);

/** Initialize per-connection graph nodes. */
VAULT_API int32_t GVault_Graph_InitConnectionGraphNodes(GVaultHandle Self, GVaultHandle ConnectionManager);

/** Route a newly added network actor. */
VAULT_API int32_t GVault_Graph_RouteAddNetworkActor(GVaultHandle Self, const void* ActorInfo,
                                                     void* GlobalInfo);

/** Route a removed network actor. */
VAULT_API int32_t GVault_Graph_RouteRemoveNetworkActor(GVaultHandle Self, const void* ActorInfo);

/** Register an OV with the graph. */
VAULT_API int32_t GVault_Graph_RegisterObjectVariable(GVaultHandle Self, GVaultOVHandle Variable);

/** Unregister an OV from the graph. */
VAULT_API int32_t GVault_Graph_UnregisterObjectVariable(GVaultHandle Self, GVaultOVHandle Variable);

/** Reset game world state. */
VAULT_API void GVault_Graph_ResetGameWorldState(GVaultHandle Self);

/** Tear down the graph. */
VAULT_API void GVault_Graph_TearDown(GVaultHandle Self);

// ═══════════════════════════════════════════════════════════════════════════
//  AutoReplication — RPC Transporter
// ═══════════════════════════════════════════════════════════════════════════

/** Initialize the transporter with a mixin. */
VAULT_API int32_t GVault_RPCTransporter_Initialize(GVaultHandle Self, GVaultHandle OwningMixin);

/** Route an RPC payload. */
VAULT_API int32_t GVault_RPCTransporter_RouteRPC(GVaultHandle Self, const void* QueuedRPC);

/** Route a property payload. */
VAULT_API int32_t GVault_RPCTransporter_RoutePropertyPayload(GVaultHandle Self, const void* Envelope,
                                                              int32_t RouteType);

// ═══════════════════════════════════════════════════════════════════════════
//  AutoReplication — Iris Backend
// ═══════════════════════════════════════════════════════════════════════════

/** Initialize the Iris backend for a world. */
VAULT_API int32_t GVault_Iris_Initialize(GVaultHandle Self, GVaultWorldHandle World);

/** Reset all tracked streams. */
VAULT_API void GVault_Iris_Reset(GVaultHandle Self);

/** Register a stream for Iris replication. */
VAULT_API int32_t GVault_Iris_RegisterStream(GVaultHandle Self, GVaultOVHandle Variable,
                                              const void* Config, const void* StreamGuid);

/** Unregister a stream. */
VAULT_API void GVault_Iris_UnregisterStream(GVaultHandle Self, GVaultOVHandle Variable);

/** Mark a stream as dirty. */
VAULT_API void GVault_Iris_MarkStreamDirty(GVaultHandle Self, GVaultOVHandle Variable);

/** Per-frame Iris tick. */
VAULT_API void GVault_Iris_Tick(GVaultHandle Self, float DeltaSeconds);

// ═══════════════════════════════════════════════════════════════════════════
//  AutoReplication — World Subsystem
// ═══════════════════════════════════════════════════════════════════════════

/** Initialize the world subsystem. */
VAULT_API int32_t GVault_WorldSub_Initialize(GVaultHandle Self, GVaultHandle Collection);

/** Deinitialize the world subsystem. */
VAULT_API void GVault_WorldSub_Deinitialize(GVaultHandle Self);

// ═══════════════════════════════════════════════════════════════════════════
//  AutoReplication — RPC Relay Component
// ═══════════════════════════════════════════════════════════════════════════

/** Relay a result to the server. */
VAULT_API int32_t GVault_Relay_RelayResultToServer(GVaultHandle Self, const void* Result);

/** Relay a property payload to the server. */
VAULT_API int32_t GVault_Relay_RelayPropertyPayloadToServer(GVaultHandle Self, const void* Envelope,
                                                             GVaultHandle TargetMixin);

/** Relay a property payload to the client. */
VAULT_API int32_t GVault_Relay_RelayPropertyPayloadToClient(GVaultHandle Self, const void* Envelope);

/** Relay an RPC to the server. */
VAULT_API int32_t GVault_Relay_RelayRPCToServer(GVaultHandle Self, const void* QueuedRPC,
                                                 int32_t bReliable, GVaultHandle TargetMixin);

// ═══════════════════════════════════════════════════════════════════════════
//  Anti-Tamper / Integrity
// ═══════════════════════════════════════════════════════════════════════════

/** Validate the Vault binary integrity. Returns GVault_OK on success. */
VAULT_API int32_t GVault_ValidateIntegrity(void);

// ═══════════════════════════════════════════════════════════════════════════
//  License Helper — File Operations (drawn from GorgeousCoreLicenseHelper)
// ═══════════════════════════════════════════════════════════════════════════
//  The real encrypted license file logic lives here in the Vault.
//  The open-source FGorgeousCoreLicenseHelper becomes a thin proxy.

/** Create an encrypted per-plugin license file. */
VAULT_API int32_t GVault_License_CreateEncryptedFile(const char* LicenseData, int32_t DataLen,
                                                      const char* ProjectId, const char* PluginName);

/** Read and decrypt a per-plugin license file. */
VAULT_API int32_t GVault_License_ReadAndDecryptFile(const char* ProjectId, const char* PluginName,
                                                     char* OutData, int32_t* OutDataLen);

/** Append a timestamped entry to a plugin's license file. */
VAULT_API int32_t GVault_License_AddEntry(const char* EntryString, const char* ProjectId,
                                           const char* PluginName);

/** Read all timestamped entries from a plugin's license file. */
VAULT_API int32_t GVault_License_ReadAllEntries(const char* ProjectId, const char* PluginName,
                                                 void* OutEntries, int32_t* OutEntryCount);

// ═══════════════════════════════════════════════════════════════════════════
//  Sibling Plugin Discovery & License Registry
// ═══════════════════════════════════════════════════════════════════════════
//  The proxy side queries UGorgeousPluginHelper for known sibling plugins
//  and passes the list here. The Vault builds the Plugin License Registry.

/**
 * Register sibling plugins discovered by UGorgeousPluginHelper.
 * Called by the proxy at editor startup with the list of installed
 * Gorgeous sibling plugins (NOT including GorgeousCore itself).
 *
 * @param PluginNames    Array of null-terminated plugin name strings
 * @param PluginCount    Number of plugins in the array
 * @return GVault_OK on success
 */
VAULT_API int32_t GVault_License_RegisterSiblingPlugins(const char** PluginNames, int32_t PluginCount);

/** Get the number of registered sibling plugins. */
VAULT_API int32_t GVault_License_GetPluginCount(void);

/** Get the license status of a specific plugin. Returns status code. */
VAULT_API int32_t GVault_License_GetPluginStatus(const char* PluginName);

// ═══════════════════════════════════════════════════════════════════════════
//  Per-Plugin License Validation
// ═══════════════════════════════════════════════════════════════════════════

/** Validate an online license for a specific sibling plugin. */
VAULT_API int32_t GVault_License_ValidatePlugin(const char* EncryptedLicPath, const char* ProjectId,
                                                  const char* PluginName);

/** Validate an offline license for a specific sibling plugin. */
VAULT_API int32_t GVault_License_ValidatePluginOffline(const char* OfflineLicPath, const char* ProjectId,
                                                        const char* PluginName);

// ═══════════════════════════════════════════════════════════════════════════
//  License Registry Evaluation & Cascading Death
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Evaluate the Plugin License Registry after all per-plugin validations.
 * If ANY plugin has an invalid/expired/missing license, returns
 * GVault_Error_LicenseInvalid — the caller (VaultLoader) must then
 * clear the entire CachedFunctions table (cascading death).
 *
 * @return GVault_OK if all plugins are licensed, GVault_Error_LicenseInvalid otherwise
 */
VAULT_API int32_t GVault_License_EvaluateRegistry(void);

/** Check if cascading death has been triggered. Returns 1 if active, 0 otherwise. */
VAULT_API int32_t GVault_License_IsCascadingDeathActive(void);

/**
 * Get a snapshot of the Plugin License Registry.
 *
 * @param OutNames     Array of const char* pointers (caller-allocated, at least OutCount entries)
 * @param OutStatuses  Array of int32_t statuses (caller-allocated, at least OutCount entries)
 * @param OutCount     On input: ignored. On output: number of entries written.
 * @return GVault_OK on success
 */
VAULT_API int32_t GVault_License_GetRegistrySnapshot(void* OutNames, int32_t* OutStatuses, int32_t* OutCount);

// ═══════════════════════════════════════════════════════════════════════════
//  Challenge-Response Protocol
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Process a challenge from the proxy. The Vault computes an HMAC response
 * using its .text section hash and internal epoch counter.
 *
 * @param Challenge       32-byte random challenge from the proxy
 * @param ChallengeLen    Length of the challenge (must be 32)
 * @param ResponseOut     Buffer to receive the 32-byte HMAC response
 * @param ResponseLenOut  Set to 32 on success
 * @return GVault_OK on success
 */
VAULT_API int32_t GVault_ChallengeResponse(const uint8_t* Challenge, int32_t ChallengeLen,
                                            uint8_t* ResponseOut, int32_t* ResponseLenOut);

/**
 * Verify a previously issued challenge-response pair.
 * Called by the proxy to ensure DLL consistency.
 *
 * @param Challenge   The original 32-byte challenge
 * @param Response    The 32-byte response to verify
 * @return GVault_OK if the response matches, GVault_Error_ChallengeFailed otherwise
 */
VAULT_API int32_t GVault_VerifyChallengeEcho(const uint8_t* Challenge, const uint8_t* Response);

/**
 * Canary function: returns a build-specific CRC that the proxy can verify.
 * The expected value changes with every Vault release.
 */
VAULT_API uint32_t GVault_Canary_SerializeCRC(void);

#ifdef __cplusplus
} // extern "C"
#endif
