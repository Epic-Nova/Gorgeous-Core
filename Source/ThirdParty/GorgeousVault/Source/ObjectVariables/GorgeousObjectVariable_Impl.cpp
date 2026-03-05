// Copyright (c) 2025-2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
// GorgeousVault — GorgeousObjectVariable Implementation (Stub)
// ============================================================================
// This file will contain the REAL Object Variable implementation.
// Currently a skeleton — every export returns GVault_Error_NotImplemented.
// ============================================================================

#include "GorgeousVaultAPI.h"

// TODO: #include Unreal Engine headers once CMake linking is configured
// #include "ObjectVariables/GorgeousObjectVariable.h"

// ═══════════════════════════════════════════════════════════════════════════
//  Object Variable — Core Lifecycle
// ═══════════════════════════════════════════════════════════════════════════

int32_t GVault_OV_InvokeInstancedFunctionality(GVaultOVHandle Self, const void* NewGuid)
{
    // TODO: Cast Self to UGorgeousObjectVariable*, invoke real logic
    return GVault_Error_NotImplemented;
}

void GVault_OV_BeginDestroy(GVaultOVHandle Self)
{
    // TODO: Real teardown
}

int32_t GVault_OV_RegisterWithRegistry(GVaultOVHandle Self, GVaultOVHandle NewChild)
{
    return GVault_Error_NotImplemented;
}

int32_t GVault_OV_SetParent(GVaultOVHandle Self, GVaultOVHandle NewParent)
{
    return GVault_Error_NotImplemented;
}

int32_t GVault_OV_ApplyReplicatedIdentifier(GVaultOVHandle Self, const void* InIdentifier)
{
    return GVault_Error_NotImplemented;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Object Variable — Serialization
// ═══════════════════════════════════════════════════════════════════════════

int32_t GVault_OV_SerializeToPayload(GVaultOVHandle Self, GVaultPayloadHandle OutPayload)
{
    return GVault_Error_NotImplemented;
}

int32_t GVault_OV_DeserializeFromPayload(GVaultOVHandle Self, GVaultPayloadHandle InPayload)
{
    return GVault_Error_NotImplemented;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Object Variable — Replication
// ═══════════════════════════════════════════════════════════════════════════

int32_t GVault_OV_ActivateReplication(GVaultOVHandle Self, const void* ReplicationContext)
{
    return GVault_Error_NotImplemented;
}

int32_t GVault_OV_RegisterReplicatedProperty(GVaultOVHandle Self, const void* PropertyName,
                                              int32_t ReplicationMode, int32_t bSendInitialState,
                                              const void* AdvancedConfig)
{
    return GVault_Error_NotImplemented;
}

int32_t GVault_OV_BindRPCHandler(GVaultOVHandle Self, const void* RPCName, int32_t Reliability)
{
    return GVault_Error_NotImplemented;
}

int32_t GVault_OV_RequestAutoReplicationRPC(GVaultOVHandle Self, int32_t Type,
                                             const void* Payload, const void* OverrideKey,
                                             GVaultHandle OverrideContext, int32_t TargetKind)
{
    return GVault_Error_NotImplemented;
}

int32_t GVault_OV_BuildAutoReplicationPropertyPayload(GVaultOVHandle Self,
                                                       const void* ConditionContext,
                                                       void* OutPayload)
{
    return GVault_Error_NotImplemented;
}

int32_t GVault_OV_ApplyAutoReplicationPropertyPayload(GVaultOVHandle Self,
                                                       const void* Payload,
                                                       GVaultHandle PackageMap)
{
    return GVault_Error_NotImplemented;
}

int32_t GVault_OV_ExecuteAutoReplicationRPC(GVaultOVHandle Self, const void* QueuedRPC,
                                             GVaultOVHandle* OutReturnVariable)
{
    return GVault_Error_NotImplemented;
}

int32_t GVault_OV_IsExecutingOnReplicationOwner(GVaultOVHandle Self)
{
    return GVault_Error_NotImplemented;
}

int32_t GVault_OV_SetDisplayName(GVaultOVHandle Self, const void* InDisplayName)
{
    return GVault_Error_NotImplemented;
}

void GVault_OV_SetNetworkingEnabled(GVaultOVHandle Self, int32_t bShouldReplicate)
{
    // TODO: Implement
}

int32_t GVault_OV_ValidateVariableAssignment(GVaultOVHandle Self, const void* PropertyName,
                                              const void* ValueProperty, const void* ValueAddress)
{
    return GVault_Error_NotImplemented;
}

void GVault_OV_GetLifetimeReplicatedProps(GVaultOVHandle Self, void* OutLifetimeProps)
{
    // TODO: Implement
}

int32_t GVault_OV_GetFunctionCallspace(GVaultOVHandle Self, GVaultHandle Function, GVaultHandle Stack)
{
    return GVault_Error_NotImplemented;
}

int32_t GVault_OV_CallRemoteFunction(GVaultOVHandle Self, GVaultHandle Function,
                                      void* Parameters, void* OutParms, GVaultHandle Stack)
{
    return GVault_Error_NotImplemented;
}

int32_t GVault_OV_InvokeNativeRPCHandlerOnObject(GVaultHandle Target, const void* QueuedRPC,
                                                   GVaultOVHandle* OutReturnVariable)
{
    return GVault_Error_NotImplemented;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Object Variable — Network Stack Control
// ═══════════════════════════════════════════════════════════════════════════

void GVault_OV_SetRootNetworkStackEnabled(GVaultOVHandle Self, int32_t bEnable) {}
void GVault_OV_SetSharedNetworkStackEnabled(GVaultOVHandle Self, int32_t bEnable) {}
void GVault_OV_SetAutoReplicationRespectAccessPolicy(GVaultOVHandle Self, int32_t bEnable) {}
void GVault_OV_SetNetworkAccessPolicy(GVaultOVHandle Self, int32_t NewPolicy, int32_t bApplyToSharedStack) {}
void GVault_OV_EnsureSharedNetworkStackOwner(GVaultOVHandle Self, GVaultHandle NewOwner) {}
