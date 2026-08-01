// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"

// Set to 1 to enable experimental memory leak fixes for PIE teardown and replication crashes.
#ifndef GORGEOUS_EXPERIMENTAL_MEMORY_FIXES
#define GORGEOUS_EXPERIMENTAL_MEMORY_FIXES 0
#endif

#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "ModuleCore/GorgeousObjectVariableRootSettings.h"
#include "ObjectVariables/NativeObjectVariableDefinitions.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "QualityOfLife/GorgeousLocalPlayerRegistry_GIS.h"
#include "QualityOfLife/GorgeousPlayerConnectionInfo_I.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerState.h"

namespace
{
	static const FName DefaultLocalRootName(TEXT("DefaultRoot"));
	static const FName DefaultNetworkRootName(TEXT("DefaultNetworkRoot"));

	FName ResolvePreferredRootName(const bool bWantsNetworking)
	{
		if (const UGorgeousObjectVariableRootSettings* Settings = UGorgeousObjectVariableRootSettings::Get())
		{
			if (const FName DesiredName = bWantsNetworking ? DefaultNetworkRootName : DefaultLocalRootName; Settings->FindRootEntry(DesiredName))
			{
				return DesiredName;
			}

			if (bWantsNetworking)
			{
				for (const FGorgeousObjectVariableRootEntry& Entry : Settings->RegisteredRoots)
				{
					if (Entry.bSupportsNetworking && !Entry.bHandlesParentlessReplicatedVariables)
					{
						return Entry.RootName;
					}
				}
			}
			else
			{
				return Settings->GetDefaultRootName();
			}
		}

		return bWantsNetworking ? DefaultNetworkRootName : UGorgeousRootObjectVariable::GetDefaultRootName();
	}

	FName MakeSubobjectName(const UObject* Owner)
	{
#if GORGEOUS_EXPERIMENTAL_MEMORY_FIXES
		return FName(*FString::Printf(TEXT("%s_SelfReferenceOV_%s"), *Owner->GetName(), *FGuid::NewGuid().ToString(EGuidFormats::Digits)));
#else
		return FName(*FString::Printf(TEXT("%s_SelfReferenceOV"), *Owner->GetClass()->GetName()));
#endif
	}

}

namespace FGorgeousQualityOfLifeStatics
{
	void SanitizeCDOAdditionalData(UObject* Owner, TMap<FName, FGorgeousObjectVariableEntry>& AdditionalData)
	{
		if (!Owner || !Owner->HasAnyFlags(RF_ClassDefaultObject) 
			|| !Owner->HasAnyFlags(RF_ArchetypeObject)
			|| !Owner->GetOutermost()->HasAnyPackageFlags(PKG_EditorOnly))
		{
			return;
		}

		if (FGorgeousObjectVariableEntry* SelfEntry = AdditionalData.Find("SelfReference"))
		{
			if (SelfEntry->DefaultValue)
			{
				UGorgeousRootObjectVariable::RemoveVariableFromRegistry(SelfEntry->DefaultValue);
				SelfEntry->DefaultValue = nullptr;
			}
			SelfEntry->bReplicate = false;
			SelfEntry->Handle = FGorgeousAutoReplicationHandle();
			AdditionalData.Remove("SelfReference");
		}
	}

	void SanitizeCDOAdditionalDataOnLevelSwitch(const FString& LevelName)
	{
		
	}

	UObject_AOTOV* EnsureSelfReference(UObject* Owner, TMap<FName, FGorgeousObjectVariableEntry>& AdditionalData, const bool bExposeThroughNetworkStack)
	{
		if (!Owner)
		{
			return nullptr;
		}

		// Do not instantiate or persist self-reference entries on class default objects; they should only exist at runtime.
		if (Owner->HasAnyFlags(RF_ClassDefaultObject))
		{
			SanitizeCDOAdditionalData(Owner, AdditionalData);
			return nullptr;
		}
		
		// Do no instantiate or persist self-reference entries on in editor objects
		if (Owner->HasAnyFlags(RF_ArchetypeObject) && Owner->GetOutermost()->HasAnyPackageFlags(PKG_EditorOnly))
		{
			return nullptr;
		}


		// One SelfReference OV per GameInstance: all splitscreen players share the same OV
		// and each accumulates itself into the OV's object array rather than owning a
		// separate variable. This makes ResolveSelfReference() unambiguous regardless of
		// how many local players are active.
		UGameInstance* OwnerGameInstance = nullptr;
		if (const UWorld* World = Owner->GetWorld())
		{
			OwnerGameInstance = World->GetGameInstance();
		}
		if (!OwnerGameInstance)
		{
			OwnerGameInstance = Cast<UGameInstance>(Owner);
		}
		UObject* const SelfRefOuter = OwnerGameInstance ? static_cast<UObject*>(OwnerGameInstance) : Owner;

		// Hoist PreferredRoot so the registry search below and the config assignment below both use it.
		const FName PreferredRoot = ResolvePreferredRootName(bExposeThroughNetworkStack);

		FGorgeousObjectVariableEntry& Entry = AdditionalData.FindOrAdd("SelfReference");
		UObject_AOTOV* SelfVariable = Cast<UObject_AOTOV>(Entry.DefaultValue);
		if (!SelfVariable || SelfVariable->GetOuter() != SelfRefOuter)
		{
			// Before allocating a fresh OV, check whether another local-player already
			// created a shared one for this GameInstance (splitscreen second player path).
			if (OwnerGameInstance)
			{
				if (UGorgeousRootObjectVariable* Root = UGorgeousRootObjectVariable::GetRootObjectVariable(PreferredRoot))
				{
					for (const auto& [Key, OVPtr] : Root->VariableRegistry)
					{
						if (UObject_AOTOV* Candidate = Cast<UObject_AOTOV>(OVPtr.Get()))
						{
							if (Candidate->GetOuter() == SelfRefOuter)
							{
								SelfVariable = Candidate;
								Entry.DefaultValue = SelfVariable;
								break;
							}
						}
					}
				}
			}

			if (!SelfVariable)
			{
				constexpr EObjectFlags SelfReferenceFlags = RF_Transactional;
				SelfVariable = NewObject<UObject_AOTOV>(SelfRefOuter, MakeSubobjectName(SelfRefOuter), SelfReferenceFlags);
				Entry.DefaultValue = SelfVariable;
			}
		}

		if (SelfVariable && !SelfVariable->UniqueIdentifier.IsValid())
		{
			SelfVariable->UniqueIdentifier = FGuid::NewGuid();
		}
		// Do NOT call AddToRoot() on SelfVariable.
		// It is kept alive by:
		//   1. The root OV's UPROPERTY VariableRegistry (TObjectPtr<> chain up to the rooted root OV).
		//   2. The Outer chain: Outer = Owner (PlayerController / GameInstance) keeps it reachable.
		// Adding RF_RootSet here causes EndPlayMap's ForEachObjectWithOuter(GameInstance, MarkAsGarbage)
		// to assert !IsRooted() when it descends the outer chain and encounters this object.
		if (!SelfVariable)
		{
			return nullptr;
		}

		const bool bIsClassDefaultObject = Owner->HasAnyFlags(RF_ClassDefaultObject);

		// Do NOT call Owner->AddToRoot() for UGameInstance owners.
		// The GameInstance lifetime is managed by the engine; AddToRoot prevents proper PIE teardown.

		SelfVariable->RootConfiguration.PreferredRootName = PreferredRoot;
		SelfVariable->RootNetworkConfig.bExposeThroughRootNetworkStack = bExposeThroughNetworkStack;
		SelfVariable->RootNetworkConfig.AccessPolicy = EGorgeousObjectVariableAccessPolicy::Everyone;
		SelfVariable->RootNetworkConfig.ReplicationChannel = "SelfReference";
		SelfVariable->SetNetworkingEnabled(bExposeThroughNetworkStack);

		if (!bIsClassDefaultObject && !SelfVariable->GetParent())
		{
			if (UGorgeousRootObjectVariable* Root = UGorgeousRootObjectVariable::GetRootObjectVariable(PreferredRoot))
			{
#if GORGEOUS_EXPERIMENTAL_MEMORY_FIXES
				SelfVariable->SetParentRefOnly(Root);
				// We add to VariableRegistry + TrackRegisteredVariable so the SelfVariable is discoverable
				// and properly cleaned up. We do NOT call SetParent() because that would Rename the Outer
				// to the Root, making the SelfVariable immortal.
				// The strong TObjectPtr in VariableRegistry is safe because PurgeWorldOwnedRegistryEntries
				// (registered on FWorldDelegates::OnWorldCleanup) removes these entries BEFORE
				// CheckForWorldGCLeaks runs, preventing the "2 leaked objects" fatal.
				Root->VariableRegistry.Add(SelfVariable->GetFName(), SelfVariable);
				UGorgeousRootObjectVariable::TrackRegisteredVariable(SelfVariable);
#else
				// @TODO: Cannot add SelfVariable to Root->VariableRegistry here.
				// The Root is immortal (AddToRoot) and VariableRegistry uses TObjectPtr (strong ref).
				// This creates a strong reference chain: Root → SelfVariable → GameInstance/World.
				// Unreal's CheckForWorldGCLeaks runs BEFORE BeginDestroy/GC, so the cleanup in
				// EnsureRemovedFromRegistry never gets a chance to fire, causing "2 leaked objects" fatal.
				// Needs a proper pre-world-destroy cleanup hook (e.g. FWorldDelegates::OnPreWorldFinishDestroy)
				// to remove the SelfVariable from the registry before the leak check runs.
				// SelfVariable->SetParent(Root);
#endif
			}
		}

		if (!bIsClassDefaultObject)
		{
			FString ContextName = FString::Printf(TEXT("SelfReference_%s"), *SelfRefOuter->GetClass()->GetName());
			if (int32 UnderscoreIndex; ContextName.FindLastChar('_', UnderscoreIndex)) { ContextName = ContextName.Left(UnderscoreIndex); }
			SelfVariable->SetDisplayName(ContextName);
		}
#if GORGEOUS_EXPERIMENTAL_MEMORY_FIXES
		// CRITICAL: A shared SelfReference OV is owned by the GameInstance.
		// It MUST NEVER be registered as a replicated subobject on the Actor (Entry.bReplicate = true),
		// because replicating a subobject whose Outer is not the Actor completely breaks Unreal's 
		// replication layout and causes fatal Realloc memory corruption crashes in PIE with multiple clients.
		Entry.bReplicate = false;
#else
		Entry.bReplicate = bExposeThroughNetworkStack;
#endif

		if (bIsClassDefaultObject || !Entry.bOverrideStreamConfig)
		{
			Entry.StreamConfigOverride = UGorgeousCoreRuntimeGlobals::GetDefaultAutoReplicationStreamConfig();
			Entry.StreamConfigOverride.bRespectAccessPolicy = Entry.StreamConfigOverride.bRespectAccessPolicy || bExposeThroughNetworkStack;
			Entry.bOverrideStreamConfig = true;
		}

		if (!bIsClassDefaultObject)
		{
			UObject* OwnerReference = Owner;

			TArray<UObject*> OwnerArray = IGorgeousArrayObjectVariablesGetter_I::Execute_GetObjectObjectArrayObjectVariable(SelfVariable, NAME_None);

			if (OwnerArray.Contains(OwnerReference))
			{
				OwnerArray[OwnerArray.IndexOfByKey(OwnerReference)] = OwnerReference;
			}
			else
			{
				OwnerArray.Add(OwnerReference);
			}
			
			IGorgeousArrayObjectVariablesSetter_I::Execute_SetObjectObjectArrayObjectVariable(SelfVariable, NAME_None, OwnerArray);
		}

		// Auto-register the PlayerController in the stable-ID registry so
		// GetQualityOfLifeReferences(StablePlayerId) can address it immediately.
		if (APlayerController* PC = Cast<APlayerController>(Owner))
		{
			AutoRegisterLocalPlayerStableId(PC);
		}

		return SelfVariable;
	}

	void ClearSelfReference(const UObject* Owner, TMap<FName, FGorgeousObjectVariableEntry>& AdditionalData)
	{
		if (!Owner)
		{
			return;
		}
		
		if (const FGorgeousObjectVariableEntry* Entry = AdditionalData.Find("SelfReference"))
		{
			if (Entry->DefaultValue)
			{
				UGorgeousRootObjectVariable::RemoveVariableFromRegistry(Entry->DefaultValue);
			}
			AdditionalData.Remove("SelfReference");
		}
	}

	void RemoveOwnerFromSelfReference(const UObject* Owner, TMap<FName, FGorgeousObjectVariableEntry>& AdditionalData)
	{
		if (!Owner)
		{
			return;
		}

		const FGorgeousObjectVariableEntry* Entry = AdditionalData.Find("SelfReference");
		if (!Entry)
		{
			return;
		}

		UObject_AOTOV* SelfVariable = Cast<UObject_AOTOV>(Entry->DefaultValue);
		if (!SelfVariable)
		{
			AdditionalData.Remove("SelfReference");
			return;
		}

		TArray<UObject*> CurrentArray = IGorgeousArrayObjectVariablesGetter_I::Execute_GetObjectObjectArrayObjectVariable(SelfVariable, NAME_None);

		// Remove this owner and opportunistically prune any other stale (already-destroyed) entries.
		const int32 Removed = CurrentArray.RemoveAll([Owner](const UObject* Obj)
		{
			return Obj == Owner || !IsValid(Obj);
		});

		if (Removed > 0)
		{
			IGorgeousArrayObjectVariablesSetter_I::Execute_SetObjectObjectArrayObjectVariable(SelfVariable, NAME_None, CurrentArray);
		}

		// If the array is now empty the shared OV has no more live references, remove it entirely.
		if (CurrentArray.IsEmpty())
		{
			UGorgeousRootObjectVariable::RemoveVariableFromRegistry(SelfVariable);
			AdditionalData.Remove("SelfReference");
		}
	}

	UObject* ResolveSelfReference(const TSubclassOf<UObject> QualityOfLifeClass)
	{
		return UGorgeousCoreRuntimeGlobals::GetNamedObjectReference(FName(ResolveSelfReferenceName(QualityOfLifeClass)), false);
	}

	FString ResolveSelfReferenceName(const TSubclassOf<UObject> QualityOfLifeClass)
	{
		// Since EnsureSelfReference now creates exactly one OV per GameInstance outer,
		// we can find the right entry by checking whether any registered UObject_AOTOV
		// contains an object of the requested class, no display-name pattern matching needed.
		UGorgeousRootObjectVariable* Roots[] = {
			UGorgeousRootObjectVariable::GetRootObjectVariable(ResolvePreferredRootName(false)),
			UGorgeousRootObjectVariable::GetRootObjectVariable(ResolvePreferredRootName(true))
		};

		for (UGorgeousRootObjectVariable* Root : Roots)
		{
			if (!Root) { continue; }
			for (const auto& [EntryKey, EntryPtr] : Root->VariableRegistry)
			{
				// Match purely on type and array contents, avoids fragile display-name
				// substring checks that break when names have _X collision suffixes.
				if (const UObject_AOTOV* SelfRefOV = Cast<UObject_AOTOV>(EntryPtr.Get()))
				{
					for (const TArray<UObject*> ReferencedObject = IGorgeousArrayObjectVariablesGetter_I::Execute_GetObjectObjectArrayObjectVariable(SelfRefOV, NAME_None);
						const auto Object : ReferencedObject)
					{
						if (!Object || !Object->IsA(*QualityOfLifeClass)) { continue; }

						return SelfRefOV->GetDisplayName();
					}
				}
			}
		}

		return LexToString(NAME_None);
	}

	TArray<UObject*> ResolveSelfReferences(const UObject* WorldContextObject, const TSubclassOf<UObject> QualityOfLifeClass, const FString& StablePlayerId)
	{
		TArray<UObject*> Result;
		if (!*QualityOfLifeClass)
		{
			return Result;
		}

		// Resolve the caller's GameInstance to filter out entries from other PIE worlds
		// sharing the same process. The OV outer is always a UGameInstance (set in
		// EnsureSelfReference), so comparing GI pointers directly is the reliable approach —
		// UGameInstance::GetWorld() is not stable cross-PIE-instance.
		const UGameInstance* CallerGI = WorldContextObject && WorldContextObject->GetWorld()
			? WorldContextObject->GetWorld()->GetGameInstance()
			: nullptr;

		UGorgeousRootObjectVariable* Roots[] = {
			UGorgeousRootObjectVariable::GetRootObjectVariable(ResolvePreferredRootName(false)),
			UGorgeousRootObjectVariable::GetRootObjectVariable(ResolvePreferredRootName(true))
		};

		for (UGorgeousRootObjectVariable* Root : Roots)
		{
			if (!Root) { continue; }
			for (const auto& [EntryKey, EntryPtr] : Root->VariableRegistry)
			{
				const UObject_AOTOV* SelfRefOV = Cast<UObject_AOTOV>(EntryPtr.Get());
				if (!SelfRefOV) { continue; }

				// Skip OVs that belong to a different GameInstance (PIE cross-world bleed guard).
				if (CallerGI)
				{
					const UGameInstance* OVGI = Cast<UGameInstance>(SelfRefOV->GetOuter());
					if (OVGI && OVGI != CallerGI)
					{
						continue;
					}
				}

				const TArray<UObject*> ReferencedObjects =
					IGorgeousArrayObjectVariablesGetter_I::Execute_GetObjectObjectArrayObjectVariable(SelfRefOV, NAME_None);

				for (UObject* Object : ReferencedObjects)
				{
					if (!IsValid(Object) || !Object->IsA(*QualityOfLifeClass))
					{
						continue;
					}

					// If a stable ID filter was provided, verify that this object belongs
					// to the requested player.  We prefer the IGorgeousPlayerConnectionInfo_I
					// path because it uses ReplicatedGorgeousStableId on PlayerState, which
					// is replicated to ALL machines, fixing the remote-client cross-
					// assignment bug where GetOwningController() returns null for remote
					// PS on clients, causing every other player's object to be skipped.
					if (!StablePlayerId.IsEmpty())
					{
						FString ObjectStableId;
						if (Object->Implements<UGorgeousPlayerConnectionInfo_I>())
						{
							// Interface path: works on all machines including remote PS
							// on clients, because GetGorgeousStablePlayerId() prefers the
							// replicated ReplicatedGorgeousStableId field.
							ObjectStableId = IGorgeousPlayerConnectionInfo_I::Execute_GetGorgeousStablePlayerId(Object);
						}
						else
						{
							// Fallback for QoL objects that don't implement the interface:
							// walk the outer / relationship chain to reach the PC and look
							// up the local stable ID from the GIS registry.
							const APlayerController* OwningPC = Cast<APlayerController>(Object);
							if (!OwningPC)
							{
								if (const APlayerState* PS = Cast<APlayerState>(Object))
								{
									OwningPC = Cast<APlayerController>(PS->GetOwningController());
								}
							}
							if (!OwningPC)
							{
								OwningPC = Object->GetTypedOuter<APlayerController>();
							}
							if (!OwningPC)
							{
								continue;
							}
							ObjectStableId = GetLocalPlayerStableId(OwningPC);
						}
						if (ObjectStableId != StablePlayerId)
						{
							continue;
						}
					}

					Result.AddUnique(Object);
				}
			}
		}
		return Result;
	}

	// ── Local Player Stable ID helpers ────────────────────────────────────

	void AutoRegisterLocalPlayerStableId(APlayerController* PC)
	{
		if (!PC) { return; }
		if (UGorgeousLocalPlayerRegistry_GIS* Registry = UGorgeousLocalPlayerRegistry_GIS::GetFromPC(PC))
		{
			// RegisterPC is a no-op when the PC already has an entry.
			if (Registry->GetStableId(PC).IsEmpty())
			{
				const int32 NextIdx = Registry->GetNextAutoIndex();
				Registry->RegisterPC(PC, FString::Printf(TEXT("LocalPlayer_%d"), NextIdx));
			}
		}
	}

	bool RegisterLocalPlayerStableId(APlayerController* PC, const FString& StableId)
	{
		if (UGorgeousLocalPlayerRegistry_GIS* Registry = UGorgeousLocalPlayerRegistry_GIS::GetFromPC(PC))
		{
			return Registry->RegisterPC(PC, StableId);
		}
		return false;
	}

	void UnregisterLocalPlayerStableId(APlayerController* PC)
	{
		if (UGorgeousLocalPlayerRegistry_GIS* Registry = UGorgeousLocalPlayerRegistry_GIS::GetFromPC(PC))
		{
			Registry->UnregisterPC(PC);
		}
	}

	FString GetLocalPlayerStableId(const APlayerController* PC)
	{
		if (UGorgeousLocalPlayerRegistry_GIS* Registry = UGorgeousLocalPlayerRegistry_GIS::GetFromPC(PC))
		{
			return Registry->GetStableId(PC);
		}
		return FString();
	}

	APlayerController* GetPlayerControllerForStableId(const UObject* WorldContextObject, const FString& StableId)
	{
		if (UGorgeousLocalPlayerRegistry_GIS* Registry = UGorgeousLocalPlayerRegistry_GIS::Get(WorldContextObject))
		{
			return Registry->GetPCForStableId(StableId);
		}
		return nullptr;
	}

	bool RenameLocalPlayerStableId(APlayerController* PC, const FString& NewStableId)
	{
		if (UGorgeousLocalPlayerRegistry_GIS* Registry = UGorgeousLocalPlayerRegistry_GIS::GetFromPC(PC))
		{
			return Registry->RenameLocalPlayer(PC, NewStableId);
		}
		return false;
	}

	void GetAllRegisteredLocalPlayers(const UObject* WorldContextObject, TArray<FString>& OutStableIds, TArray<int32>& OutPlayerIndices)
	{
		OutStableIds.Reset();
		OutPlayerIndices.Reset();
		if (UGorgeousLocalPlayerRegistry_GIS* Registry = UGorgeousLocalPlayerRegistry_GIS::Get(WorldContextObject))
		{
			Registry->GetAllEntries(OutStableIds, OutPlayerIndices);
		}
	}
}
