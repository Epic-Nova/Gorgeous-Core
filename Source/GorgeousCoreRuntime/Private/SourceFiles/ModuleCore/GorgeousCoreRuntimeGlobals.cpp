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
#include "GorgeousCoreRuntimeGlobals.h"
#include "ModuleCore/GorgeousAutoReplicationSettings.h"
#include "AutoReplication/GorgeousAutoReplicationCoordinator.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
#include "ObjectVariables/NativeObjectVariableDefinitions.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "QualityOfLife/GorgeousLocalPlayerRegistry_GIS.h"
#include "QualityOfLife/GorgeousGameMode.h"
#include "QualityOfLife/GorgeousGameState.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "QualityOfLife/GorgeousPlayerState.h"
#include "QualityOfLife/GorgeousWorldSettings.h"
#include "QualityOfLife/GorgeousQualityOfLifeNodeTarget_I.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

// Forward declaration for helper that applies developer settings to the AutoReplication coordinator.
static void ApplyAutoReplicationSettingsToCoordinator(UWorld* World);

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousCoreRuntimeGlobals, All, All);

namespace GorgeousCoreRuntimeGlobals_Private
{
	template <typename TAutoReplicationClass>
	FGorgeousAutoReplicationMixin* ResolveFromContext(UObject* Context)
	{
		if (TAutoReplicationClass* TypedObject = Cast<TAutoReplicationClass>(Context))
		{
			return &TypedObject->GetAutoReplicationMixin();
		}
		return nullptr;
	}

	FGorgeousAutoReplicationMixin* ResolveAutoReplicationMixin(UObject* Context, UObject* ExplicitOwner)
	{
		auto ResolveForObject = [](UObject* Object) -> FGorgeousAutoReplicationMixin*
		{
			if (!Object)
			{
				return nullptr;
			}

			if (FGorgeousAutoReplicationMixin* GameModeMixin = ResolveFromContext<AGorgeousGameMode>(Object))
			{
				return GameModeMixin;
			}
			if (FGorgeousAutoReplicationMixin* GameStateMixin = ResolveFromContext<AGorgeousGameState>(Object))
			{
				return GameStateMixin;
			}
			if (FGorgeousAutoReplicationMixin* PlayerControllerMixin = ResolveFromContext<AGorgeousPlayerController>(Object))
			{
				return PlayerControllerMixin;
			}
			if (FGorgeousAutoReplicationMixin* PlayerStateMixin = ResolveFromContext<AGorgeousPlayerState>(Object))
			{
				return PlayerStateMixin;
			}
			if (FGorgeousAutoReplicationMixin* WorldSettingsMixin = ResolveFromContext<AGorgeousWorldSettings>(Object))
			{
				return WorldSettingsMixin;
			}

			return nullptr;
		};

		if (FGorgeousAutoReplicationMixin* ExplicitMixin = ResolveForObject(ExplicitOwner))
		{
			return ExplicitMixin;
		}

		if (FGorgeousAutoReplicationMixin* ContextMixin = ResolveForObject(Context))
		{
			return ContextMixin;
		}

		UObject* WorldContext = ExplicitOwner ? ExplicitOwner : Context;
		if (!WorldContext)
		{
			return nullptr;
		}

		// Fallback: derive AutoReplication owner from world context.
		if (AGorgeousGameState* WorldGameState = Cast<AGorgeousGameState>(UGorgeousCoreRuntimeGlobals::GetQualityOfLifeReference(WorldContext, AGorgeousGameState::StaticClass())))
		{
			return &WorldGameState->GetAutoReplicationMixin();
		}
		if (AGorgeousGameMode* WorldGameMode = Cast<AGorgeousGameMode>(UGorgeousCoreRuntimeGlobals::GetQualityOfLifeReference(WorldContext, AGorgeousGameMode::StaticClass())))
		{
			return &WorldGameMode->GetAutoReplicationMixin();
		}
		if (AGorgeousWorldSettings* WorldSettings = UGorgeousCoreRuntimeGlobals::GetGorgeousWorldSettings(WorldContext))
		{
			return &WorldSettings->GetAutoReplicationMixin();
		}

		return nullptr;
	}
}

//=============================================================================
// UGorgeousCoreRuntimeGlobals Implementation
//=============================================================================

AGorgeousWorldSettings* UGorgeousCoreRuntimeGlobals::GetGorgeousWorldSettings(const UObject* WorldContextObject)
{
	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		// Frequently queried before a world exists; keep the log quiet unless verbose is enabled.
		GT_I_LOG("GT.RuntimeGlobals.InvalidWorldContext", TEXT("Invalid WorldContextObject passed to GetGorgeousWorldSettings."));
		return nullptr;
	}
	return Cast<AGorgeousWorldSettings>(WorldContextObject->GetWorld()->GetWorldSettings());
}

UGorgeousObjectVariable* UGorgeousCoreRuntimeGlobals::GetNamedObjectVariable(const FName DisplayName, const bool bLogWarning)
{
	if (DisplayName.IsNone())
	{
		if (bLogWarning)
		{
			GT_W_LOG("GT.RuntimeGlobals.NamedOV.InvalidName", TEXT("GetNamedObjectVariable was called with an invalid display name."));
		}
		return nullptr;
	}

	if (UGorgeousObjectVariable* Variable = UGorgeousRootObjectVariable::FindVariableByDisplayName(DisplayName))
	{
		return Variable;
	}

	if (bLogWarning)
	{
		GT_W_LOG("GT.RuntimeGlobals.NamedOV.NotFound", TEXT("No object variable registered under the display name '%s'."), *DisplayName.ToString());
	}

	return nullptr;
}

UObject* UGorgeousCoreRuntimeGlobals::GetNamedObjectReference(const FName DisplayName, const bool bLogWarning)
{
	if (UGorgeousObjectVariable* Variable = GetNamedObjectVariable(DisplayName, bLogWarning))
	{
		if (const UObject_SOTOV* ObjectVariable = Cast<UObject_SOTOV>(Variable))
		{
			return ObjectVariable->Execute_GetObjectObjectSingleObjectVariable(ObjectVariable, NAME_None);
		}
		if (const UObject_AOTOV* ObjectVariable = Cast<UObject_AOTOV>(Variable))
		{
			return ObjectVariable->Execute_GetObjectObjectArrayObjectVariable(ObjectVariable, NAME_None).Num() > 0
				       ? ObjectVariable->Execute_GetObjectObjectArrayObjectVariable(ObjectVariable, NAME_None)[0]
				       : nullptr; //@TODO Comply with the possibility of multiple entries to ensure that the splitscreen logic works
		}

		if (bLogWarning)
		{
			GT_W_LOG("GT.RuntimeGlobals.NamedOV.NotObjectSingle", TEXT("Named object variable '%s' is not an Object Single OV."), *DisplayName.ToString());
		}
	}

	return nullptr;
}

UObject* UGorgeousCoreRuntimeGlobals::GetQualityOfLifeReference(const UObject* WorldContextObject, const TSubclassOf<UObject> QualityOfLifeClass)
{
	const TArray<UObject*> All = GetQualityOfLifeReferences(WorldContextObject, QualityOfLifeClass);
	return All.Num() > 0 ? All[0] : nullptr;
}

void UGorgeousCoreRuntimeGlobals::RefreshQualityOfLifeReplication(UObject* WorldContextObject,
	TSubclassOf<UObject> QualityOfLifeClass)
{
	UObject* QoLInstance = GetQualityOfLifeReference(WorldContextObject, QualityOfLifeClass); // Ensure references are resolved and cached.
	if (!QoLInstance)
	{
		GT_W_LOG("GT.RuntimeGlobals.QoL.RefreshReplication.NoInstance", TEXT("Unable to refresh replication for QoL class %s because no instance could be found."), *QualityOfLifeClass->GetName());
		return;
	}
	
if (FGorgeousAutoReplicationMixin* AutoReplicationMixin = GorgeousCoreRuntimeGlobals_Private::ResolveAutoReplicationMixin(WorldContextObject, QoLInstance))
	{
		AutoReplicationMixin->InitializeAdditionalData(true);
	}
	else
	{
		GT_W_LOG("GT.RuntimeGlobals.QoL.RefreshReplication.NoMixin", TEXT("Unable to refresh replication for QoL class %s because no AutoReplication mixin could be found on the instance."), *QualityOfLifeClass->GetName());
	}
}

TArray<UObject*> UGorgeousCoreRuntimeGlobals::GetQualityOfLifeReferences(const UObject* WorldContextObject, const TSubclassOf<UObject> QualityOfLifeClass, const FString& StablePlayerId)
{
	if (!*QualityOfLifeClass)
	{
		GT_W_LOG("GT.RuntimeGlobals.QoL.InvalidClass", TEXT("GetQualityOfLifeReferences requires a valid QoL class selection."));
		return {};
	}

	if (!QualityOfLifeClass->ImplementsInterface(UGorgeousQualityOfLifeNodeTarget_I::StaticClass()))
	{
		GT_W_LOG("GT.RuntimeGlobals.QoL.InvalidInterface", TEXT("Class %s does not implement IGorgeousQualityOfLifeNodeTarget_I."), *QualityOfLifeClass->GetName());
		return {};
	}

	return FGorgeousQualityOfLifeStatics::ResolveSelfReferences(WorldContextObject, QualityOfLifeClass, StablePlayerId);
}

// ── Local Player Stable ID ────────────────────────────────────────────────────

bool UGorgeousCoreRuntimeGlobals::RegisterLocalPlayerStableId(const UObject* WorldContextObject, APlayerController* PlayerController, const FString& StableId)
{
	return FGorgeousQualityOfLifeStatics::RegisterLocalPlayerStableId(PlayerController, StableId);
}

bool UGorgeousCoreRuntimeGlobals::RenameLocalPlayerStableId(const UObject* WorldContextObject, APlayerController* PlayerController, const FString& NewStableId)
{
	return FGorgeousQualityOfLifeStatics::RenameLocalPlayerStableId(PlayerController, NewStableId);
}

FString UGorgeousCoreRuntimeGlobals::GetLocalPlayerStableId(const UObject* WorldContextObject, const APlayerController* PlayerController)
{
	return FGorgeousQualityOfLifeStatics::GetLocalPlayerStableId(PlayerController);
}

APlayerController* UGorgeousCoreRuntimeGlobals::GetPlayerControllerForStableId(const UObject* WorldContextObject, const FString& StableId)
{
	return FGorgeousQualityOfLifeStatics::GetPlayerControllerForStableId(WorldContextObject, StableId);
}

void UGorgeousCoreRuntimeGlobals::GetAllRegisteredLocalPlayers(const UObject* WorldContextObject, TArray<FString>& OutStableIds, TArray<int32>& OutPlayerIndices)
{
	FGorgeousQualityOfLifeStatics::GetAllRegisteredLocalPlayers(WorldContextObject, OutStableIds, OutPlayerIndices);
}

#pragma region AutoReplication_Networking_Functions

bool UGorgeousCoreRuntimeGlobals::GetNetGorgeousAutoReplicationValue(UObject* WorldContextObject, const FName Key, UGorgeousObjectVariable*& OutValue, UObject* AutoReplicationOwner)
{
	OutValue = nullptr;
	FGorgeousAutoReplicationMixin* AutoReplicationMixin = GorgeousCoreRuntimeGlobals_Private::ResolveAutoReplicationMixin(WorldContextObject, AutoReplicationOwner);
	const UObject* Context = AutoReplicationOwner ? AutoReplicationOwner : WorldContextObject;
 
	if (!AutoReplicationMixin)
	{
		GT_W_LOG("GT.RuntimeGlobals.AutoReplication.ResolveRead", TEXT("Unable to resolve AutoReplication mixin while reading key %s."), *Key.ToString());
		return false;
	}

	if (AutoReplicationMixin->TryGetValue(Key, OutValue))
	{
		return true;
	}

	GT_I_LOG("GT.RuntimeGlobals.AutoReplication.NotFound", TEXT("AutoReplication entry %s not found on context %s."), *Key.ToString(), Context ? *Context->GetName() : TEXT("<null>"));
	return false;
}

bool UGorgeousCoreRuntimeGlobals::SetNetGorgeousAutoReplicationValue(UObject* WorldContextObject, const FName Key, UGorgeousObjectVariable* NewValue, UObject* AutoReplicationOwner)
{
	FGorgeousAutoReplicationMixin* AutoReplicationMixin = GorgeousCoreRuntimeGlobals_Private::ResolveAutoReplicationMixin(WorldContextObject, AutoReplicationOwner);
	if (!AutoReplicationMixin)
	{
		GT_W_LOG("GT.RuntimeGlobals.AutoReplication.ResolveWrite", TEXT("Unable to resolve AutoReplication mixin while writing key %s."), *Key.ToString());
		return false;
	}

	if (AutoReplicationMixin->IsNetworkingEnabled())
	{
		const bool bResult = AutoReplicationMixin->TrySetReplicatedValue(Key, NewValue);
		if (!bResult)
		{
			GT_W_LOG("GT.RuntimeGlobals.AutoReplication.EnqueueFail", TEXT("Failed to enqueue replicated update for AutoReplication key %s."), *Key.ToString());
		}
		return bResult;
	}

	if (FGorgeousObjectVariableEntry* Entry = AutoReplicationMixin->FindEntry(Key))
	{
		Entry->DefaultValue = NewValue;
		Entry->Handle.CacheValue(NewValue);
		return true;
	}

	const FString ContextLabel = AutoReplicationOwner ? AutoReplicationOwner->GetName() : (WorldContextObject ? WorldContextObject->GetName() : TEXT("<null>"));
	GT_W_LOG("GT.RuntimeGlobals.AutoReplication.EntryMissing", TEXT("Unable to locate AutoReplication entry %s for context %s."), *Key.ToString(), *ContextLabel);
	return false;
}

FGorgeousAutoReplicationStreamConfig UGorgeousCoreRuntimeGlobals::GetDefaultAutoReplicationStreamConfig()
{
	if (const UGorgeousAutoReplicationSettings* Settings = UGorgeousAutoReplicationSettings::Get())
	{
		return Settings->DefaultStreamConfig;
	}
	return FGorgeousAutoReplicationStreamConfig();
}

void UGorgeousCoreRuntimeGlobals::SetDefaultAutoReplicationStreamConfig(const FGorgeousAutoReplicationStreamConfig& NewConfig)
{
	if (UGorgeousAutoReplicationSettings* Settings = GetMutableDefault<UGorgeousAutoReplicationSettings>())
	{
		Settings->DefaultStreamConfig = NewConfig;
		Settings->SaveConfig();
	}
}

void UGorgeousCoreRuntimeGlobals::SetAutoReplicationStreamOverride(const FName EntryKey, const FGorgeousAutoReplicationStreamConfig& NewConfig)
{
	if (EntryKey.IsNone())
	{
		GT_W_LOG("GT.RuntimeGlobals.AutoReplication.StreamKeyInvalid", TEXT("Cannot set AutoReplication stream override because the entry key is invalid."));
		return;
	}

	if (UGorgeousAutoReplicationSettings* Settings = GetMutableDefault<UGorgeousAutoReplicationSettings>())
	{
		Settings->StreamOverrides.Add(EntryKey, NewConfig);
		Settings->SaveConfig();
	}
}

bool UGorgeousCoreRuntimeGlobals::GetAutoReplicationStreamOverride(const FName EntryKey, FGorgeousAutoReplicationStreamConfig& OutConfig)
{
	if (EntryKey.IsNone())
	{
		return false;
	}

	if (const UGorgeousAutoReplicationSettings* Settings = UGorgeousAutoReplicationSettings::Get())
	{
		if (const FGorgeousAutoReplicationStreamConfig* Override = Settings->StreamOverrides.Find(EntryKey))
		{
			OutConfig = *Override;
			return true;
		}
	}

	return false;
}

void UGorgeousCoreRuntimeGlobals::ClearAutoReplicationStreamOverride(const FName EntryKey)
{
	if (EntryKey.IsNone())
	{
		return;
	}

	if (UGorgeousAutoReplicationSettings* Settings = GetMutableDefault<UGorgeousAutoReplicationSettings>())
	{
		if (Settings->StreamOverrides.Remove(EntryKey) > 0)
		{
			Settings->SaveConfig();
		}
	}
}

void UGorgeousCoreRuntimeGlobals::SetAutoReplicationBackendsEnabled(const UObject* WorldContextObject, const bool bEnableIris, const bool bEnableReplicationGraph)
{
    if (UGorgeousAutoReplicationSettings* Settings = GetMutableDefault<UGorgeousAutoReplicationSettings>())
    {
        const bool bChanged = Settings->bEnableIris != bEnableIris || Settings->bEnableReplicationGraph != bEnableReplicationGraph;
        if (!bChanged)
        {
            return;
        }

        Settings->bEnableIris = bEnableIris;
        Settings->bEnableReplicationGraph = bEnableReplicationGraph;
        Settings->SaveConfig();

        if (UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr)
        {
            // Apply developer settings/persistent overrides to the coordinator before reinitializing.
            ApplyAutoReplicationSettingsToCoordinator(World);

            FGorgeousAutoReplicationCoordinator& Coordinator = FGorgeousAutoReplicationCoordinator::Get(World);
            Coordinator.TearDown();
            Coordinator.Initialize(World);
        }
    }
}

void UGorgeousCoreRuntimeGlobals::SetAutoReplicationUseIrisOverride(const UObject* WorldContextObject, bool bEnable)
{
    if (UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr)
    {
        FGorgeousAutoReplicationCoordinator& Coordinator = FGorgeousAutoReplicationCoordinator::Get(World);
        Coordinator.SetUseIrisOverride(bEnable);

        // Reinitialize Coordinator to apply change immediately.
        Coordinator.TearDown();
        Coordinator.Initialize(World);
    }
}

void UGorgeousCoreRuntimeGlobals::ClearAutoReplicationUseIrisOverride(const UObject* WorldContextObject)
{
    if (UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr)
    {
        FGorgeousAutoReplicationCoordinator& Coordinator = FGorgeousAutoReplicationCoordinator::Get(World);
        Coordinator.ClearUseIrisOverride();

        Coordinator.TearDown();
        Coordinator.Initialize(World);
    }
}

void UGorgeousCoreRuntimeGlobals::SetAutoReplicationReplicationGraphOverride(const UObject* WorldContextObject, bool bEnable)
{
    if (UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr)
    {
        FGorgeousAutoReplicationCoordinator& Coordinator = FGorgeousAutoReplicationCoordinator::Get(World);
        Coordinator.SetReplicationGraphEnabledOverride(bEnable);

        Coordinator.TearDown();
        Coordinator.Initialize(World);
    }
}

void UGorgeousCoreRuntimeGlobals::ClearAutoReplicationReplicationGraphOverride(const UObject* WorldContextObject)
{
    if (UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr)
    {
        FGorgeousAutoReplicationCoordinator& Coordinator = FGorgeousAutoReplicationCoordinator::Get(World);
        Coordinator.ClearReplicationGraphEnabledOverride();

        Coordinator.TearDown();
        Coordinator.Initialize(World);
    }
}

void UGorgeousCoreRuntimeGlobals::SetAutoReplicationGraphClassOverride(const UObject* WorldContextObject, TSubclassOf<UReplicationGraph> GraphClass)
{
    if (UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr)
    {
        FGorgeousAutoReplicationCoordinator& Coordinator = FGorgeousAutoReplicationCoordinator::Get(World);
        Coordinator.SetAutoReplicationGraphClassOverride(GraphClass);

        Coordinator.TearDown();
        Coordinator.Initialize(World);
    }
}

void UGorgeousCoreRuntimeGlobals::ClearAutoReplicationGraphClassOverride(const UObject* WorldContextObject)
{
    if (UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr)
    {
        FGorgeousAutoReplicationCoordinator& Coordinator = FGorgeousAutoReplicationCoordinator::Get(World);
        Coordinator.ClearAutoReplicationGraphClassOverride();

        Coordinator.TearDown();
        Coordinator.Initialize(World);
    }
}

bool UGorgeousCoreRuntimeGlobals::IsAutoReplicationIrisEnabled()
{
	if (const UGorgeousAutoReplicationSettings* Settings = UGorgeousAutoReplicationSettings::Get())
	{
		return Settings->bEnableIris;
	}
	return false;
}

bool UGorgeousCoreRuntimeGlobals::IsAutoReplicationReplicationGraphEnabled()
{
	if (const UGorgeousAutoReplicationSettings* Settings = UGorgeousAutoReplicationSettings::Get())
	{
		return Settings->bEnableReplicationGraph;
	}
	return false;
}

bool UGorgeousCoreRuntimeGlobals::GetEffectiveAutoReplicationStreamConfig(UObject* WorldContextObject, const FName EntryKey, FGorgeousAutoReplicationStreamConfig& OutConfig, UObject* AutoReplicationOwner)
{
	const UGorgeousAutoReplicationSettings* Settings = UGorgeousAutoReplicationSettings::Get();
	OutConfig = Settings ? Settings->DefaultStreamConfig : FGorgeousAutoReplicationStreamConfig();
	bool bResolvedEntry = false;

	if (EntryKey.IsNone())
	{
		return false;
	}

	if (Settings)
	{
		if (const FGorgeousAutoReplicationStreamConfig* Override = Settings->StreamOverrides.Find(EntryKey))
		{
			OutConfig = *Override;
			bResolvedEntry = true;
		}
	}

	if (FGorgeousAutoReplicationMixin* AutoReplicationMixin = GorgeousCoreRuntimeGlobals_Private::ResolveAutoReplicationMixin(WorldContextObject, AutoReplicationOwner))
	{
		if (FGorgeousObjectVariableEntry* Entry = AutoReplicationMixin->FindEntry(EntryKey))
		{
			if (Entry->bOverrideStreamConfig)
			{
				OutConfig = Entry->StreamConfigOverride;
			}
			bResolvedEntry = true;
		}
	}

	return bResolvedEntry;
}

void ApplyAutoReplicationSettingsToCoordinator(UWorld* World)
{
	if (!World)
	{
		return;
	}

	if (const UGorgeousAutoReplicationSettings* Settings = UGorgeousAutoReplicationSettings::Get())
	{
		FGorgeousAutoReplicationCoordinator& Coordinator = FGorgeousAutoReplicationCoordinator::Get(World);

		// Apply force overrides from developer settings if requested.
		if (Settings->bForceIrisAtRuntime)
		{
			Coordinator.SetUseIrisOverride(true);
		}
		else
		{
			Coordinator.ClearUseIrisOverride();
		}

		if (Settings->bForceReplicationGraphAtRuntime)
		{
			Coordinator.SetReplicationGraphEnabledOverride(true);
		}
		else
		{
			Coordinator.ClearReplicationGraphEnabledOverride();
		}

		// If a default auto-replication graph class is configured in dev settings, apply it.
		if (Settings->AutoReplicationGraphClass.IsValid())
		{
			if (UClass* GraphClass = Settings->AutoReplicationGraphClass.LoadSynchronous())
			{
				Coordinator.SetAutoReplicationGraphClassOverride(GraphClass);
			}
		}
		else
		{
			Coordinator.ClearAutoReplicationGraphClassOverride();
		}
	}
}

void UGorgeousCoreRuntimeGlobals::InitializeAutoReplicationForWorld(UWorld* World)
{
	if (!World)
	{
		return;
	}

	ApplyAutoReplicationSettingsToCoordinator(World);

	// Touch the coordinator to ensure desired backends (Iris/ReplicationGraph) spin up.
	FGorgeousAutoReplicationCoordinator::Get(World);
}

#pragma endregion AutoReplication_Networking_Functions