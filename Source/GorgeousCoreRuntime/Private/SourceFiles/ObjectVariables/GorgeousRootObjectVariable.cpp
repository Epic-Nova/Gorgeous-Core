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
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "ModuleCore/GorgeousObjectVariableRootSettings.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Templates/Function.h"
#include "Misc/CoreMisc.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousRootObjectVariable, Log, All);

//=============================================================================
// UGorgeousRootObjectVariable Implementation
//=============================================================================

TMap<FName, TObjectPtr<UGorgeousRootObjectVariable>> UGorgeousRootObjectVariable::NamedRootInstances;
FName UGorgeousRootObjectVariable::CachedDefaultRootName = NAME_None;
TMap<FGuid, TWeakObjectPtr<UGorgeousObjectVariable>> UGorgeousRootObjectVariable::IdentifierLookup;
TMap<FName, FGuid> UGorgeousRootObjectVariable::DisplayNameToIdentifier;
TMap<FGuid, FName> UGorgeousRootObjectVariable::IdentifierToDisplayName;
EGorgeousObjectVariableOrphanResolution UGorgeousRootObjectVariable::DefaultOrphanResolution = EGorgeousObjectVariableOrphanResolution::ReparentToRoot;
FSimpleMulticastDelegate UGorgeousRootObjectVariable::OnRootRegistryChanged;

namespace GorgeousRootObjectVariable_Private
{
	static void ForEachRootRegistry(TFunctionRef<void(TMap<FName, TObjectPtr<UGorgeousObjectVariable>>&, UGorgeousRootObjectVariable*)> Callback)
	{
			if (UGorgeousRootObjectVariable::NamedRootInstances.Num() == 0)
		{
				if (IsEngineExitRequested())
				{
					return;
				}

				if (UGorgeousRootObjectVariable* DefaultRoot = UGorgeousRootObjectVariable::GetRootObjectVariable(NAME_None))
				{
					Callback(DefaultRoot->VariableRegistry, DefaultRoot);
				}
			return;
		}

		for (TPair<FName, TObjectPtr<UGorgeousRootObjectVariable>>& Pair : UGorgeousRootObjectVariable::NamedRootInstances)
		{
			if (IsValid(Pair.Value))
			{
				Callback(Pair.Value->VariableRegistry, Pair.Value.Get());
			}
		}
	}

	struct FRootRegistryOwnerState
	{
		FGorgeousRootRegistryOwnerHandle Handle;
		TWeakObjectPtr<UObject> Owner;
	};

	static TMap<FName, FRootRegistryOwnerState> RootOwnerStates;

	static void ForEachVariableRecursive(UGorgeousObjectVariable* Variable, TFunctionRef<void(UGorgeousObjectVariable*)> Callback)
	{
		if (!IsValid(Variable))
		{
			return;
		}

		Callback(Variable);
		for (auto& [Key, Child] : Variable->VariableRegistry)
		{
			if (IsValid(Child))
			{
				ForEachVariableRecursive(Child.Get(), Callback);
			}
		}
	}

	static const FGorgeousObjectVariableRootEntry* LookupRootDescriptor(const FName RootName)
	{
		if (const UGorgeousObjectVariableRootSettings* Settings = UGorgeousObjectVariableRootSettings::Get())
		{
			const FName ResolvedName = UGorgeousRootObjectVariable::ResolveRootName(RootName);
			return Settings->FindRootEntry(ResolvedName);
		}
		return nullptr;
	}

	static bool SupportsSharedOwnership(const FName RootName)
	{
		if (const FGorgeousObjectVariableRootEntry* Descriptor = LookupRootDescriptor(RootName))
		{
			return Descriptor->bHandlesParentlessReplicatedVariables;
		}
		return false;
	}

	static bool EnforcesNetworking(const FName RootName)
	{
		if (const FGorgeousObjectVariableRootEntry* Descriptor = LookupRootDescriptor(RootName))
		{
			return Descriptor->bSupportsNetworking && Descriptor->bEnforceNetworking;
		}
		return false;
	}

	static FName ResolveRootNameForVariable(const UGorgeousObjectVariable* Variable)
	{
		const UGorgeousObjectVariable* Current = Variable;
		while (Current)
		{
			if (const UGorgeousRootObjectVariable* AsRoot = Cast<const UGorgeousRootObjectVariable>(Current))
			{
				return AsRoot->GetRegisteredRootName();
			}
			Current = Current->GetParent();
		}

		return UGorgeousRootObjectVariable::ResolveRootName(NAME_None);
	}
}

bool UGorgeousRootObjectVariable::IsSharedNetworkingRoot(const FName RootName)
{
	return GorgeousRootObjectVariable_Private::SupportsSharedOwnership(RootName);
}

bool UGorgeousRootObjectVariable::IsEnforcedNetworkingRoot(const FName RootName)
{
	return GorgeousRootObjectVariable_Private::EnforcesNetworking(RootName);
}

UGorgeousRootObjectVariable::UGorgeousRootObjectVariable()
{
	bPersistent = true;
	RegisteredRootName = NAME_None;
}

UGorgeousRootObjectVariable* UGorgeousRootObjectVariable::GetRootObjectVariable(const FName RootName)
{
	const FName ResolvedName = ResolveRootName(RootName);
	if (IsEngineExitRequested())
	{
		return TryGetExistingRoot(ResolvedName);
	}
	
	return GetOrCreateRootInternal(ResolvedName);
}

UGorgeousRootObjectVariable* UGorgeousRootObjectVariable::TryGetExistingRoot(const FName RootName)
{
	if (RootName.IsNone())
	{
		for (TPair<FName, TObjectPtr<UGorgeousRootObjectVariable>>& Pair : NamedRootInstances)
		{
			if (IsValid(Pair.Value))
			{
				return Pair.Value.Get();
			}
		}
		return nullptr;
	}

	if (TObjectPtr<UGorgeousRootObjectVariable>* Entry = NamedRootInstances.Find(RootName))
	{
		if (IsValid(Entry->Get()))
		{
			return Entry->Get();
		}

		NamedRootInstances.Remove(RootName);
	}

	return nullptr;
}

FName UGorgeousRootObjectVariable::GetDefaultRootName()
{
	return ResolveRootName(NAME_None);
}

TArray<FName> UGorgeousRootObjectVariable::GetRegisteredRootNames()
{
	if (const UGorgeousObjectVariableRootSettings* Settings = UGorgeousObjectVariableRootSettings::Get())
	{
		return Settings->GetRegisteredRootNames();
	}

	return { NAME_None };
}

UGorgeousObjectVariable* UGorgeousRootObjectVariable::FindVariableByIdentifier(const FGuid Identifier)
{
	if (!Identifier.IsValid())
	{
		return nullptr;
	}

	if (const TWeakObjectPtr<UGorgeousObjectVariable>* Entry = IdentifierLookup.Find(Identifier))
	{
		if (Entry->IsValid())
		{
			return Entry->Get();
		}

		IdentifierLookup.Remove(Identifier);
	}

	return nullptr;
}


void UGorgeousRootObjectVariable::SetDefaultOrphanResolution(const EGorgeousObjectVariableOrphanResolution InResolution)
{
	DefaultOrphanResolution = InResolution;
}

EGorgeousObjectVariableOrphanResolution UGorgeousRootObjectVariable::GetDefaultOrphanResolution()
{
	return DefaultOrphanResolution;
}

UGorgeousObjectVariable* UGorgeousRootObjectVariable::FindVariableByDisplayName(const FName InDisplayName)
{
	if (InDisplayName.IsNone())
	{
		return nullptr;
	}

	if (const FGuid* FoundIdentifier = DisplayNameToIdentifier.Find(InDisplayName))
	{
		if (UGorgeousObjectVariable* FoundVariable = FindVariableByIdentifier(*FoundIdentifier))
		{
			return FoundVariable;
		}

		DisplayNameToIdentifier.Remove(InDisplayName);
		IdentifierToDisplayName.Remove(*FoundIdentifier);
	}

	return nullptr;
}

FGorgeousRootRegistryOwnerHandle UGorgeousRootObjectVariable::ClaimRootRegistryOwnership(const FName RootName, const FString& StableIdentifier, UObject* OwningContext)
{
	const FName ResolvedRootName = ResolveRootName(RootName);
	FGorgeousRootRegistryOwnerHandle Result;
	if (!GorgeousRootObjectVariable_Private::SupportsSharedOwnership(ResolvedRootName))
	{
		UE_LOG(LogGorgeousRootObjectVariable, VeryVerbose, TEXT("Root '%s' does not support shared ownership; skipping ClaimRootRegistryOwnership."), *ResolvedRootName.ToString());
		return Result;
	}
	GorgeousRootObjectVariable_Private::FRootRegistryOwnerState& State = GorgeousRootObjectVariable_Private::RootOwnerStates.FindOrAdd(ResolvedRootName);
	if (!State.Handle.OwnerToken.IsValid())
	{
		State.Handle.OwnerToken = FGuid::NewGuid();
	}
	State.Handle.RootName = ResolvedRootName;
	if (!StableIdentifier.IsEmpty())
	{
		State.Handle.StableIdentifier = StableIdentifier;
	}
	State.Owner = OwningContext;
	Result = State.Handle;
	PromoteRootRegistryOwner(ResolvedRootName, OwningContext);
	OnRootRegistryChanged.Broadcast();
	return Result;
}

bool UGorgeousRootObjectVariable::RestoreRootRegistryOwnership(const FString& StableIdentifier, UObject* ReturningOwner, FGorgeousRootRegistryOwnerHandle& OutHandle)
{
	if (StableIdentifier.IsEmpty() || !ReturningOwner)
	{
		return false;
	}

	for (TPair<FName, GorgeousRootObjectVariable_Private::FRootRegistryOwnerState>& Pair : GorgeousRootObjectVariable_Private::RootOwnerStates)
	{
		if (!GorgeousRootObjectVariable_Private::SupportsSharedOwnership(Pair.Key))
		{
			continue;
		}
		if (Pair.Value.Handle.StableIdentifier == StableIdentifier)
		{
			Pair.Value.Owner = ReturningOwner;
			if (!Pair.Value.Handle.OwnerToken.IsValid())
			{
				Pair.Value.Handle.OwnerToken = FGuid::NewGuid();
			}
			Pair.Value.Handle.RootName = ResolveRootName(Pair.Key);
			OutHandle = Pair.Value.Handle;
			PromoteRootRegistryOwner(Pair.Key, ReturningOwner);
			OnRootRegistryChanged.Broadcast();
			return true;
		}
	}

	return false;
}

void UGorgeousRootObjectVariable::ReleaseRootRegistryOwnership(const FGorgeousRootRegistryOwnerHandle& Handle, UObject* CachedOwner)
{
	if (!Handle.IsValid())
	{
		return;
	}

	const FName ResolvedRootName = ResolveRootName(Handle.RootName);
	if (!GorgeousRootObjectVariable_Private::SupportsSharedOwnership(ResolvedRootName))
	{
		return;
	}
	if (GorgeousRootObjectVariable_Private::FRootRegistryOwnerState* State = GorgeousRootObjectVariable_Private::RootOwnerStates.Find(ResolvedRootName))
	{
		if (State->Handle.OwnerToken == Handle.OwnerToken)
		{
			State->Owner.Reset();
			if (CachedOwner)
			{
				PromoteRootRegistryOwner(ResolvedRootName, CachedOwner);
			}
			OnRootRegistryChanged.Broadcast();
		}
	}
}

void UGorgeousRootObjectVariable::PromoteRootRegistryOwner(const FName RootName, UObject* CachedOwner)
{
	if (!CachedOwner)
	{
		return;
	}

	const FName ResolvedRootName = ResolveRootName(RootName);
	if (!GorgeousRootObjectVariable_Private::SupportsSharedOwnership(ResolvedRootName))
	{
		return;
	}
	UGorgeousRootObjectVariable* Root = GetRootObjectVariable(ResolvedRootName);
	if (!Root)
	{
		return;
	}

	GorgeousRootObjectVariable_Private::ForEachVariableRecursive(Root,
		[CachedOwner](UGorgeousObjectVariable* Variable)
		{
			if (Variable)
			{
				Variable->EnsureSharedNetworkStackOwner(CachedOwner);
			}
		});

	GorgeousRootObjectVariable_Private::FRootRegistryOwnerState& State = GorgeousRootObjectVariable_Private::RootOwnerStates.FindOrAdd(ResolvedRootName);
	State.Handle.RootName = ResolvedRootName;
	if (!State.Handle.OwnerToken.IsValid())
	{
		State.Handle.OwnerToken = FGuid::NewGuid();
	}
	if (State.Handle.StableIdentifier.IsEmpty())
	{
		State.Handle.StableIdentifier = CachedOwner->GetName();
	}

	OnRootRegistryChanged.Broadcast();
}

TArray<UGorgeousObjectVariable*> UGorgeousRootObjectVariable::GetVariableHierarchyRegistry(const FName RootName)
{
	TArray<UGorgeousObjectVariable*> Result;
	TArray<UGorgeousObjectVariable*> Visited;

	TFunction<void(UGorgeousObjectVariable*)> GatherEntries;
	GatherEntries = [&GatherEntries, &Visited, &Result](UGorgeousObjectVariable* Variable)
	{
		if (!IsValid(Variable) || Visited.Contains(Variable))
		{
			return;
		}

		Visited.Add(Variable);
		Result.Add(Variable);

		for (auto& [Key, Child] : Variable->VariableRegistry)
		{
			GatherEntries(Child.Get());
		}
	};

	if (UGorgeousRootObjectVariable* Root = GetRootObjectVariable(RootName))
	{
		for (auto& [Key, Entry] : Root->VariableRegistry)
		{
			GatherEntries(Entry.Get());
		}
	}

	return Result;
}

TArray<UGorgeousObjectVariable*> UGorgeousRootObjectVariable::GetRootVariableRegistry(const FName RootName)
{
	if (UGorgeousRootObjectVariable* Root = GetRootObjectVariable(RootName))
	{
		TArray<UGorgeousObjectVariable*> Copy;
		Copy.Reserve(Root->VariableRegistry.Num());
		for (auto& [Key, Entry] : Root->VariableRegistry)
		{
			Copy.Add(Entry.Get());
		}
		return Copy;
	}

	return {};
}

void UGorgeousRootObjectVariable::RemoveVariableFromRegistry(UGorgeousObjectVariable* VariableToRemove)
{
	if (!VariableToRemove)
	{
		return;
	}

	bool bRemoved = false;
	if (UGorgeousRootObjectVariable* AsRoot = Cast<UGorgeousRootObjectVariable>(VariableToRemove))
	{
		for (auto It = NamedRootInstances.CreateIterator(); It; ++It)
		{
			if (It->Value == AsRoot)
			{
				if (AsRoot->IsRooted())
				{
					AsRoot->RemoveFromRoot();
				}
				It.RemoveCurrent();
				bRemoved = true;
				break;
			}
		}
	}
	else
	{
		TFunction<bool(TMap<FName, TObjectPtr<UGorgeousObjectVariable>>&)> RemoveFromRegistry;
		RemoveFromRegistry = [&](TMap<FName, TObjectPtr<UGorgeousObjectVariable>>& Registry) -> bool
		{
			for (auto It = Registry.CreateIterator(); It; ++It)
			{
				// Prune null/invalid slots accumulated by GC or missed BeginDestroy paths
				if (!It->Value || !IsValid(It->Value.Get()))
				{
					It.RemoveCurrent();
					continue;
				}

				if (It->Value == VariableToRemove)
				{
					if (VariableToRemove->IsRooted())
					{
						VariableToRemove->RemoveFromRoot();
					}
					It.RemoveCurrent();
					return true;
				}

				if (RemoveFromRegistry(It->Value->VariableRegistry))
				{
					return true;
				}
			}

			return false;
		};

		GorgeousRootObjectVariable_Private::ForEachRootRegistry(
			[&](TMap<FName, TObjectPtr<UGorgeousObjectVariable>>& Registry, UGorgeousRootObjectVariable*)
			{
				if (!bRemoved)
				{
					bRemoved = RemoveFromRegistry(Registry);
				}
			});
	}

	if (!bRemoved)
	{
		return;
	}

	IdentifierLookup.Remove(VariableToRemove->UniqueIdentifier);
	ReleaseDisplayName(VariableToRemove);
	HandleOrphanedChildren(VariableToRemove, DefaultOrphanResolution);
}

bool UGorgeousRootObjectVariable::IsVariableRegistered(UGorgeousObjectVariable* Variable)
{
	if (!Variable)
	{
		return false;
	}

	if (const TWeakObjectPtr<UGorgeousObjectVariable>* Entry = IdentifierLookup.Find(Variable->UniqueIdentifier))
	{
		if (Entry->IsValid())
		{
			return true;
		}

		IdentifierLookup.Remove(Variable->UniqueIdentifier);
	}

	return false;
}


void UGorgeousRootObjectVariable::CleanupRegistry(const bool bFullCleanup)
{
	const EGorgeousObjectVariableOrphanResolution PreviousResolution = DefaultOrphanResolution;
	if (bFullCleanup)
	{
		DefaultOrphanResolution = EGorgeousObjectVariableOrphanResolution::DestroyOrphans;
	}

	int32 RemovedCount = 0;

	// -----------------------------------------------------------------------
	// Step 1: clean non-root OVs from every registry tree.
	//
	// bFullCleanup == false  (level switch): remove only dangling (invalid)
	//   entries — persistent OVs survive.
	// bFullCleanup == true   (session end):  remove dangling AND
	//   non-persistent entries.
	// -----------------------------------------------------------------------
	TFunction<void(TMap<FName, TObjectPtr<UGorgeousObjectVariable>>&)> PurgeRegistry;
	PurgeRegistry = [&](TMap<FName, TObjectPtr<UGorgeousObjectVariable>>& Registry)
	{
		// Collect removal candidates before touching the map to avoid iterator invalidation.
		TArray<FName> KeysToRemove;
		for (auto& [Key, OVPtr] : Registry)
		{
			UGorgeousObjectVariable* Entry = OVPtr.Get();
			const bool bEntryValid = IsValid(Entry);
			const bool bShouldRemove = !bEntryValid || (bFullCleanup && Entry && !Entry->bPersistent);
			if (bShouldRemove)
			{
				// Reset FallbackOwner (TStrongObjectPtr / FGCObject) BEFORE
				// removal so it no longer keeps the PIE world reachable.
				if (bFullCleanup && bEntryValid && Entry)
				{
					Entry->SetFallbackOwner(nullptr);
				}
				++RemovedCount;
				KeysToRemove.Add(Key);
			}
			else if (Entry)
			{
				PurgeRegistry(Entry->VariableRegistry);
			}
		}
		// RemoveVariableFromRegistry recurses through all root registries; call it outside
		// the range-for to avoid modifying the map while iterating.
		for (const FName& KeyToRemove : KeysToRemove)
		{
			if (const TObjectPtr<UGorgeousObjectVariable>* Found = Registry.Find(KeyToRemove))
			{
				RemoveVariableFromRegistry(Found->Get());
			}
		}
	};

	GorgeousRootObjectVariable_Private::ForEachRootRegistry(
		[&](TMap<FName, TObjectPtr<UGorgeousObjectVariable>>& Registry, UGorgeousRootObjectVariable*)
		{
			PurgeRegistry(Registry);
		});

	// -----------------------------------------------------------------------
	// Step 2  (session-end only): release root OVs and unroot everything.
	//
	// EndPlayMap calls ForEachObjectWithOuter(GameInstance, MarkAsGarbage) which asserts
	// !IsRooted() on every object in the GI's outer chain.
	// Root OVs carry RF_RootSet; child OVs do NOT (AddToRoot is intentionally NOT called on
	// child OVs — see NewObjectVariable).  Walk the full tree and clear FallbackOwner on each
	// descendant (releases the TStrongObjectPtr FGCObject that would otherwise keep the PIE world
	// reachable past GC), then RemoveFromRoot on the root OV itself.
	// -----------------------------------------------------------------------
	int32 RootsReleased = 0;

	if (bFullCleanup)
	{
		TFunction<void(UGorgeousObjectVariable*)> UnrootTree;
		UnrootTree = [&UnrootTree](UGorgeousObjectVariable* OV)
		{
			if (!OV) return;
			for (auto& [ChildKey, Child] : OV->VariableRegistry)
			{
				if (UGorgeousObjectVariable* C = Child.Get())
				{
					UnrootTree(C);
					// Clear FallbackOwner so the TStrongObjectPtr (FGCObject) no longer
					// independently keeps the PIE world reachable during GC.
					// Child OVs are NOT rooted (AddToRoot is only called on root OVs),
					// so no RemoveFromRoot() is needed here.
					C->SetFallbackOwner(nullptr);
				}
			}
		};

		TArray<FName> NamesToRelease;
		for (const TPair<FName, TObjectPtr<UGorgeousRootObjectVariable>>& Pair : NamedRootInstances)
		{
			NamesToRelease.Add(Pair.Key);
		}

		for (const FName& RootName : NamesToRelease)
		{
			if (TObjectPtr<UGorgeousRootObjectVariable>* Entry = NamedRootInstances.Find(RootName))
			{
				if (UGorgeousRootObjectVariable* Root = Entry->Get())
				{
					UnrootTree(Root);
					Root->SetFallbackOwner(nullptr);
					if (Root->IsRooted())
					{
						Root->RemoveFromRoot();
					}
					// Sever the UObject outer reference back to the PIE GameInstance.
					// If any descendent OV is still alive via a TStrongObjectPtr (e.g.
					// RuntimeContext in GorgeousEntertaining_GIS), its outer chain will
					// now lead to GetTransientPackage() rather than to GameInstance,
					// allowing GC to collect GameInstance when PIE ends.
					if (UObject* RootOuter = Root->GetOuter();
						RootOuter && RootOuter != GetTransientPackage())
					{
						Root->Rename(nullptr, GetTransientPackage(),
							REN_DontCreateRedirectors | REN_ForceNoResetLoaders);
					}
				}
				NamedRootInstances.Remove(RootName);
				++RootsReleased;
			}
		}
	}

	// -----------------------------------------------------------------------
	// Logging
	// -----------------------------------------------------------------------
	if (RootsReleased > 0 || RemovedCount > 0)
	{
		GT_S_LOG("GT.ObjectVariables.Registry.Cleaned",
			TEXT("Registry cleaned [%s]: removed %d OV(s), released %d root OV(s)."),
			bFullCleanup ? TEXT("SessionEnd") : TEXT("LevelSwitch"),
			RemovedCount, RootsReleased);
	}
	else
	{
		GT_S_LOG("GT.ObjectVariables.Registry.Cleaned",
			TEXT("Registry scan completed – no dangling entries detected."));
	}

	DefaultOrphanResolution = PreviousResolution;
}

void UGorgeousRootObjectVariable::RegisterWithRegistry(UGorgeousObjectVariable* NewObjectVariable, FName RegistryKey)
{
	if (!NewObjectVariable)
	{
		return;
	}

	if (!HandleUniqueRegistrationPolicy(NewObjectVariable))
	{
		return;
	}
	
	if (!IsVariableRegistered(NewObjectVariable))
	{
		FName Key = RegistryKey;
		if (Key == NAME_None)
		{
			Key = !NewObjectVariable->DisplayName.IsEmpty()
				? FName(*NewObjectVariable->DisplayName)
				: FName(*NewObjectVariable->UniqueIdentifier.ToString());
		}
		if (VariableRegistry.Contains(Key))
		{
			Key = FName(*FString::Printf(TEXT("%s_%s"), *Key.ToString(),
				*NewObjectVariable->UniqueIdentifier.ToString().Left(8)));
		}
		VariableRegistry.Add(Key, NewObjectVariable);
	}

	TrackRegisteredVariable(NewObjectVariable);

	OnRootRegistryChanged.Broadcast();
	OnVariableTreeChanged.Broadcast();
}

FString UGorgeousRootObjectVariable::ReserveDisplayName(UGorgeousObjectVariable* Variable, const FString& CandidateLabel)
{
	if (!Variable)
	{
		return CandidateLabel;
	}

	if (!Variable->UniqueIdentifier.IsValid())
	{
		return CandidateLabel;
	}

	FString BaseLabel = CandidateLabel.IsEmpty() ? Variable->GetName() : CandidateLabel;
	if (BaseLabel.IsEmpty())
	{
		BaseLabel = FString::Printf(TEXT("Variable_%s"), *Variable->UniqueIdentifier.ToString());
	}
	FName CandidateKey(*BaseLabel);
	int32 DisambiguationIndex = 1;
	while (const FGuid* ExistingGuid = DisplayNameToIdentifier.Find(CandidateKey))
	{
		if (*ExistingGuid == Variable->UniqueIdentifier)
		{
			break;
		}

		CandidateKey = FName(*FString::Printf(TEXT("%s_%d"), *BaseLabel, DisambiguationIndex++));
	}

	DisplayNameToIdentifier.FindOrAdd(CandidateKey) = Variable->UniqueIdentifier;
	IdentifierToDisplayName.FindOrAdd(Variable->UniqueIdentifier) = CandidateKey;
	return CandidateKey.ToString();
}

void UGorgeousRootObjectVariable::ReleaseDisplayName(UGorgeousObjectVariable* Variable)
{
	if (!Variable || !Variable->UniqueIdentifier.IsValid())
	{
		return;
	}

	if (const FName* ExistingName = IdentifierToDisplayName.Find(Variable->UniqueIdentifier))
	{
		DisplayNameToIdentifier.Remove(*ExistingName);
		IdentifierToDisplayName.Remove(Variable->UniqueIdentifier);
	}
}

void UGorgeousRootObjectVariable::TrackRegisteredVariable(UGorgeousObjectVariable* Variable)
{
	if (!IsValid(Variable) || !Variable->UniqueIdentifier.IsValid())
	{
		return;
	}

	IdentifierLookup.FindOrAdd(Variable->UniqueIdentifier) = Variable;
}

FName UGorgeousRootObjectVariable::ResolveRootName(const FName RequestedRootName)
{
	const UGorgeousObjectVariableRootSettings* Settings = UGorgeousObjectVariableRootSettings::Get();
	if (Settings)
	{
		if (!RequestedRootName.IsNone() && Settings->FindRootEntry(RequestedRootName))
		{
			return RequestedRootName;
		}

		if (CachedDefaultRootName.IsNone() || !Settings->FindRootEntry(CachedDefaultRootName))
		{
			CachedDefaultRootName = Settings->GetDefaultRootName();
		}

		if (!CachedDefaultRootName.IsNone())
		{
			return CachedDefaultRootName;
		}

		const TArray<FName> RegisteredNames = Settings->GetRegisteredRootNames();
		if (RegisteredNames.Num() > 0)
		{
			return RegisteredNames[0];
		}
	}

	return RequestedRootName;
}

UGorgeousRootObjectVariable* UGorgeousRootObjectVariable::GetOrCreateRootInternal(const FName RootName)
{
	if (IsEngineExitRequested())
	{
		return TryGetExistingRoot(RootName);
	}

	if (TObjectPtr<UGorgeousRootObjectVariable>* ExistingRoot = NamedRootInstances.Find(RootName))
	{
		if (IsValid(ExistingRoot->Get()))
		{
			return ExistingRoot->Get();
		}

		NamedRootInstances.Remove(RootName);
	}

	UClass* RootClass = UGorgeousRootObjectVariable::StaticClass();
	if (!RootName.IsNone())
	{
		if (const UGorgeousObjectVariableRootSettings* Settings = UGorgeousObjectVariableRootSettings::Get())
		{
			if (const FGorgeousObjectVariableRootEntry* Descriptor = Settings->FindRootEntry(RootName))
			{
				if (UClass* LoadedClass = Descriptor->RootClass.LoadSynchronous())
				{
					RootClass = LoadedClass;
				}
			}
		}
	}

	// Find a world-providing outer so UObject::GetWorld() works directly.
	// Prefer the GameInstance from an active PIE/Game world; fall back to
	// GetTransientPackage() only during very early startup before any world exists.
	UObject* RootOuter = GetTransientPackage();
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType != EWorldType::PIE && Context.WorldType != EWorldType::Game)
			{
				continue;
			}
			if (UWorld* World = Context.World())
			{
				if (UGameInstance* GI = World->GetGameInstance())
				{
					RootOuter = GI;
					break;
				}
			}
		}
	}

	UGorgeousRootObjectVariable* NewRoot = NewObject<UGorgeousRootObjectVariable>(RootOuter, RootClass);
	NewRoot->AddToRoot();
	NewRoot->RegisteredRootName = RootName;
	NewRoot->UniqueIdentifier = FGuid::NewGuid();
	NamedRootInstances.Add(RootName, NewRoot);
	TrackRegisteredVariable(NewRoot);

	// Notify listeners (e.g. UGorgeousGameInstance) so the new root gets its
	// FallbackOwner set and any roots still on GetTransientPackage() re-outered.
	OnRootRegistryChanged.Broadcast();

	return NewRoot;
}

void UGorgeousRootObjectVariable::HandleOrphanedChildren(UGorgeousObjectVariable* RemovedParent, const EGorgeousObjectVariableOrphanResolution Resolution)
{
	if (!IsValid(RemovedParent))
	{
		return;
	}

	const FName OwningRootName = GorgeousRootObjectVariable_Private::ResolveRootNameForVariable(RemovedParent);
	const bool bSharedRootAllowsReparent = GorgeousRootObjectVariable_Private::SupportsSharedOwnership(OwningRootName);
	UGorgeousRootObjectVariable* TargetRootForReparent = nullptr;
	if (bSharedRootAllowsReparent && Resolution == EGorgeousObjectVariableOrphanResolution::ReparentToRoot)
	{
		TargetRootForReparent = GetRootObjectVariable(OwningRootName);
	}

	TArray<TObjectPtr<UGorgeousObjectVariable>> OrphanedChildren;
	RemovedParent->VariableRegistry.GenerateValueArray(OrphanedChildren);
	RemovedParent->VariableRegistry.Reset();

	for (UGorgeousObjectVariable* Orphan : OrphanedChildren)
	{
		if (!IsValid(Orphan))
		{
			continue;
		}

		EGorgeousObjectVariableOrphanResolution EffectiveResolution = Resolution;
		if (EffectiveResolution == EGorgeousObjectVariableOrphanResolution::ReparentToRoot && (!bSharedRootAllowsReparent || !TargetRootForReparent))
		{
			UE_LOG(LogGorgeousRootObjectVariable, VeryVerbose, TEXT("Root '%s' does not allow reparenting for orphan '%s'; destroying instead."), *OwningRootName.ToString(), *Orphan->GetName());
			EffectiveResolution = EGorgeousObjectVariableOrphanResolution::DestroyOrphans;
		}

		switch (EffectiveResolution)
		{
		case EGorgeousObjectVariableOrphanResolution::DestroyOrphans:
		{
			const EGorgeousObjectVariableOrphanResolution Previous = DefaultOrphanResolution;
			DefaultOrphanResolution = EGorgeousObjectVariableOrphanResolution::DestroyOrphans;
			RemoveVariableFromRegistry(Orphan);
			DefaultOrphanResolution = Previous;
			Orphan->ConditionalBeginDestroy();
			break;
		}

		case EGorgeousObjectVariableOrphanResolution::ReparentToRoot:
		default:
			if (TargetRootForReparent)
			{
				Orphan->SetParent(TargetRootForReparent);
				TargetRootForReparent->RegisterWithRegistry(Orphan);
			}
			break;
		}
	}
}
