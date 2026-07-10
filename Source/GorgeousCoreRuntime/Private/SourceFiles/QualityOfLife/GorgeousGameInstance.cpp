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
#include "QualityOfLife/GorgeousGameInstance.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousRootObjectVariable.h"

// Set to 1 to enable experimental memory leak fixes for PIE teardown and replication crashes.
#ifndef GORGEOUS_EXPERIMENTAL_MEMORY_FIXES
#define GORGEOUS_EXPERIMENTAL_MEMORY_FIXES 0
#endif

#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "QualityOfLife/GorgeousQualityOfLifeHelperMacros.h"
#include "Containers/Set.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/GameEngine.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/UObjectIterator.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousGameInstance Implementation
//=============================================================================

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousGameInstance, Log, All);

namespace GorgeousGameInstance_Private
{
	static FString BuildOuterChain(const UObject* Start)
	{
		if (!Start)
		{
			return TEXT("<null>");
		}

		FString Chain;
		const UObject* Current = Start;
		while (Current)
		{
			if (!Chain.IsEmpty())
			{
				Chain += TEXT(" -> ");
			}

			Chain += FString::Printf(TEXT("%s[%s]"), *GetNameSafe(Current), *GetNameSafe(Current->GetClass()));
			Current = Current->GetOuter();
		}

		return Chain;
	}

	static bool IsInOuterChain(const UObject* Candidate, const UObject* TargetOuter)
	{
		const UObject* Current = Candidate;
		while (Current)
		{
			if (Current == TargetOuter)
			{
				return true;
			}
			Current = Current->GetOuter();
		}

		return false;
	}

	static void LogNamedRootInstances(const UGorgeousGameInstance* GameInstance)
	{
		UE_LOG(LogGorgeousGameInstance, VeryVerbose, TEXT("Inspecting NamedRootInstances (%d entries) before purge:"), UGorgeousRootObjectVariable::NamedRootInstances.Num());

		for (const TPair<FName, TObjectPtr<UGorgeousRootObjectVariable>>& Pair : UGorgeousRootObjectVariable::NamedRootInstances)
		{
			const UGorgeousRootObjectVariable* Root = Pair.Value.Get();
			UE_LOG(
				LogGorgeousGameInstance,
				VeryVerbose,
				TEXT("  RootKey='%s' Root='%s' Class='%s' Rooted=%s Outer='%s' InGIChain=%s OuterChain=%s"),
				*Pair.Key.ToString(),
				*GetNameSafe(Root),
				*GetNameSafe(Root ? Root->GetClass() : nullptr),
				(Root && Root->IsRooted()) ? TEXT("true") : TEXT("false"),
				*GetNameSafe(Root ? Root->GetOuter() : nullptr),
				(Root && IsInOuterChain(Root, GameInstance)) ? TEXT("true") : TEXT("false"),
				*BuildOuterChain(Root));
		}
	}

	static int32 DumpRootedObjectsInGameInstanceOuterChain(const UGorgeousGameInstance* GameInstance)
	{
		int32 RootedInChainCount = 0;
		for (TObjectIterator<UObject> It; It; ++It)
		{
			UObject* Object = *It;
			if (!Object || !Object->IsRooted())
			{
				continue;
			}

			if (!IsInOuterChain(Object, GameInstance))
			{
				continue;
			}

			++RootedInChainCount;
			UE_LOG(
				LogGorgeousGameInstance,
				VeryVerbose,
				TEXT("Rooted object in GameInstance outer chain: Obj='%s' Class='%s' Flags=0x%08x Outer='%s' OuterChain=%s"),
				*GetNameSafe(Object),
				*GetNameSafe(Object->GetClass()),
				static_cast<uint32>(Object->GetFlags()),
				*GetNameSafe(Object->GetOuter()),
				*BuildOuterChain(Object));
		}

		return RootedInChainCount;
	}
}

UGorgeousGameInstance::UGorgeousGameInstance()
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FGorgeousQualityOfLifeStatics::SanitizeCDOAdditionalData(this, AdditionalGorgeousData);
	}
	FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, false);
}

void UGorgeousGameInstance::Init()
{
	UGorgeousRootObjectVariable::SetRootCreationAllowed(true);

	FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, false);

	if (!UGorgeousRootObjectVariable::GetRootObjectVariable())
	{
		UE_LOG(LogGorgeousGameInstance, Error, TEXT("RootObjectVariable is null during Init."));
		return;
	}

	EnsureRootVariablesFallbackToGameInstance();

	for (auto& GorgeousSetting : AdditionalGorgeousData)
	{
		if (UGorgeousObjectVariable* const DefaultVar = GorgeousSetting.Value.DefaultValue)
		{
			if (!DefaultVar->UniqueIdentifier.IsValid())
			{
				DefaultVar->UniqueIdentifier = FGuid::NewGuid();
			}
			DefaultVar->InvokeInstancedFunctionality(DefaultVar->UniqueIdentifier);
		}
	}

	if (!RootRegistryChangedHandle.IsValid())
	{
		RootRegistryChangedHandle = UGorgeousRootObjectVariable::OnRootRegistryChanged.AddUObject(this, &UGorgeousGameInstance::HandleRootRegistryChanged);
	}
	
	Super::Init();
}

void UGorgeousGameInstance::Shutdown()
{
	UGorgeousRootObjectVariable::SetRootCreationAllowed(false);

	GorgeousGameInstance_Private::LogNamedRootInstances(this);

	const int32 RootedBeforePurge = GorgeousGameInstance_Private::DumpRootedObjectsInGameInstanceOuterChain(this);
	UE_LOG(LogGorgeousGameInstance, VeryVerbose, TEXT("Found %d rooted object(s) in GameInstance outer chain before purge."), RootedBeforePurge);

	int32 PurgedWorldOwnedEntries = 0;
	const TArray<UGorgeousObjectVariable*> HierarchySnapshot = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry();
	for (UGorgeousObjectVariable* Variable : HierarchySnapshot)
	{
		if (!IsValid(Variable))
		{
			continue;
		}

		UObject* VariableOuter = Variable->GetOuter();
		bool bOwnedByThisGameInstance = VariableOuter == this;

		if (!bOwnedByThisGameInstance)
		{
			if (UWorld* OwningWorld = Variable->GetVariableWorld())
			{
				bOwnedByThisGameInstance = OwningWorld->GetGameInstance() == this;
			}
		}

		if (!bOwnedByThisGameInstance)
		{
			continue;
		}

		UE_LOG(
			LogGorgeousGameInstance,
			Verbose,
			TEXT("Purging world-owned OV during Shutdown: '%s' Outer='%s' Rooted=%s Chain=%s"),
			*GetNameSafe(Variable),
			*GetNameSafe(VariableOuter),
			Variable->IsRooted() ? TEXT("true") : TEXT("false"),
			*GorgeousGameInstance_Private::BuildOuterChain(Variable));

		UGorgeousRootObjectVariable::RemoveVariableFromRegistry(Variable);
		++PurgedWorldOwnedEntries;
	}

	UE_LOG(LogGorgeousGameInstance, VeryVerbose, TEXT("Purged %d world-owned object-variable registry entries during Shutdown."), PurgedWorldOwnedEntries);

	for (const TPair<FName, TObjectPtr<UGorgeousRootObjectVariable>>& Pair : UGorgeousRootObjectVariable::NamedRootInstances)
	{
		if (UGorgeousRootObjectVariable* RootInstance = Pair.Value.Get())
		{
			if (RootInstance->IsRooted() && RootInstance->GetOuter() == this)
			{
				UE_LOG(
					LogGorgeousGameInstance,
					VeryVerbose,
					TEXT("Unrooting rooted root OV outered to this GI during Shutdown: RootKey='%s' Root='%s' Chain=%s"),
					*Pair.Key.ToString(),
					*GetNameSafe(RootInstance),
					*GorgeousGameInstance_Private::BuildOuterChain(RootInstance));
				RootInstance->RemoveFromRoot();
			}
		}
	}

	const int32 RootedAfterPurge = GorgeousGameInstance_Private::DumpRootedObjectsInGameInstanceOuterChain(this);
	UE_LOG(LogGorgeousGameInstance, VeryVerbose, TEXT("Found %d rooted object(s) in GameInstance outer chain after purge/unroot pass."), RootedAfterPurge);

	if (RootRegistryChangedHandle.IsValid())
	{
		UGorgeousRootObjectVariable::OnRootRegistryChanged.Remove(RootRegistryChangedHandle);
		RootRegistryChangedHandle.Reset();
	}

	Super::Shutdown();
}

void UGorgeousGameInstance::EnsureRootVariablesFallbackToGameInstance()
{
#if GORGEOUS_EXPERIMENTAL_MEMORY_FIXES
	if (bIsEnsuringRootFallback || IsEngineExitRequested() || HasAnyFlags(RF_BeginDestroyed))
	{
		return;
	}
#else
	if (bIsEnsuringRootFallback)
	{
		return;
	}
#endif

	TGuardValue<bool> ReentrancyGuard(bIsEnsuringRootFallback, true);

	TSet<FName> RootsToInitialize;
	const FName DefaultRootName = UGorgeousRootObjectVariable::GetDefaultRootName();
	RootsToInitialize.Add(DefaultRootName.IsNone() ? NAME_None : DefaultRootName);

#if GORGEOUS_EXPERIMENTAL_MEMORY_FIXES
	const TArray<FName> RegisteredRootNames = UGorgeousRootObjectVariable::GetRegisteredRootNames();
	for (const FName& RootName : RegisteredRootNames)
	{
		if (!RootName.IsNone())
		{
			RootsToInitialize.Add(RootName);
		}
	}
#else
	// @TODO
	//const TArray<FName> RegisteredRootNames = UGorgeousRootObjectVariable::GetRegisteredRootNames();
	//for (const FName& RootName : RegisteredRootNames)
	//{
	//	if (!RootName.IsNone())
	//	{
	//		RootsToInitialize.Add(RootName);
	//	}
	//}
#endif

	// This loop sets up FallbackOwner and shared network stack ownership for all roots.
	// Safe because PurgeWorldOwnedRegistryEntries (registered on FWorldDelegates::OnWorldCleanup)
	// breaks the strong reference chains BEFORE CheckForWorldGCLeaks runs.
	for (const FName& RootName : RootsToInitialize)
	{
		const bool bSupportsSharedOwnership = UGorgeousRootObjectVariable::IsSharedNetworkingRoot(RootName);
		if (UGorgeousRootObjectVariable* Root = UGorgeousRootObjectVariable::GetRootObjectVariable(RootName))
		{
			Root->SetFallbackOwner(this);

			if (bSupportsSharedOwnership)
			{
				Root->EnsureSharedNetworkStackOwner(this);
				UGorgeousRootObjectVariable::PromoteRootRegistryOwner(RootName, this);
			}
		}
		else
		{
			const FString Label = RootName.IsNone() ? TEXT("<DefaultRoot>") : RootName.ToString();
			UE_LOG(LogGorgeousGameInstance, Warning, TEXT("Failed to assign cached owner for root object variable '%s'."), *Label);
		}
	}

	for (const TPair<FName, TObjectPtr<UGorgeousRootObjectVariable>>& Pair : UGorgeousRootObjectVariable::NamedRootInstances)
	{
		if (UGorgeousRootObjectVariable* RootInstance = Pair.Value)
		{
			RootInstance->SetFallbackOwner(this);

			if (RootInstance->GetOuter() == GetTransientPackage())
			{
				if (RootInstance->IsRooted())
				{
					UE_LOG(
						LogGorgeousGameInstance,
						VeryVerbose,
						TEXT("Skipping Rename for rooted root OV '%s' to avoid entering GameInstance outer chain during teardown."),
						*GetNameSafe(RootInstance));
				}
				else
				{
					RootInstance->Rename(nullptr, this);
				}
			}
		}
	}
}

void UGorgeousGameInstance::PostInitProperties()
{
	Super::PostInitProperties();
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FGorgeousQualityOfLifeStatics::SanitizeCDOAdditionalData(this, AdditionalGorgeousData);
		return;
	}
	FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, false);
}

void UGorgeousGameInstance::PostLoad()
{
	Super::PostLoad();
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FGorgeousQualityOfLifeStatics::SanitizeCDOAdditionalData(this, AdditionalGorgeousData);
		return;
	}
	FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, false);
}

void UGorgeousGameInstance::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UGorgeousGameInstance, AdditionalGorgeousData))
	{
		FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, false);
		for (auto& GorgeousSetting : AdditionalGorgeousData)
		{
			if (UGorgeousObjectVariable* const DefaultVar = GorgeousSetting.Value.DefaultValue)
			{
				DefaultVar->UniqueIdentifier = FGuid::NewGuid();
			}
		}
	}
}

void UGorgeousGameInstance::HandleRootRegistryChanged()
{
	bIsEnsuringRootFallback = false;
	EnsureRootVariablesFallbackToGameInstance();
}
