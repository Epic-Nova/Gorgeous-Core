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
#include "GorgeousSingleton.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Misc/CoreDelegates.h"
#include "UObject/UObjectBase.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#if WITH_EDITOR
#include "Editor.h"
#endif
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousSingleton Implementation
//=============================================================================

namespace
{
	struct FGorgeousSingletonKey
	{
		UClass* Class = nullptr;
		TWeakObjectPtr<UObject> Context;
		EGorgeousSingletonScope Scope = EGorgeousSingletonScope::Global;

		bool operator==(const FGorgeousSingletonKey& Other) const
		{
			return Class == Other.Class && Context == Other.Context && Scope == Other.Scope;
		}
	};

	uint32 GetTypeHash(const FGorgeousSingletonKey& Key)
	{
		return HashCombine(GetTypeHash(Key.Class), HashCombine(GetTypeHash(Key.Context), ::GetTypeHash(static_cast<uint8>(Key.Scope))));
	}

	TMap<FGorgeousSingletonKey, TStrongObjectPtr<UObject>> GSingletonInstances;
	FDelegateHandle GSingletonCleanupHandle;
	FDelegateHandle GSingletonPreExitHandle;
	bool GSingletonExitStarted = false;

	void BindSingletonCleanup()
	{
		if (GSingletonCleanupHandle.IsValid() || GSingletonExitStarted) { return; }
		GSingletonPreExitHandle = FCoreDelegates::OnPreExit.AddLambda([]
		{
			GSingletonExitStarted = true;
			UGorgeousSingleton::DestroyAllSingletons();
		});
		GSingletonCleanupHandle = FWorldDelegates::OnWorldCleanup.AddLambda([](UWorld* World, bool bSessionEnded, bool)
		{
			for (auto It = GSingletonInstances.CreateIterator(); It; ++It)
			{
				const auto& Key = It.Key();
				const bool bWorldEnded = Key.Scope == EGorgeousSingletonScope::World && Key.Context.Get() == World;
				const bool bInstanceEnded = bSessionEnded && Key.Scope == EGorgeousSingletonScope::GameInstance
					&& Key.Context.Get() == World->GetGameInstance();
				if (bWorldEnded || bInstanceEnded) { It.RemoveCurrent(); }
			}
		});
	}

	UObject* ResolveContextObject(UObject* WorldContextObject, EGorgeousSingletonScope Scope)
	{
		if (Scope == EGorgeousSingletonScope::Global)
		{
			return nullptr;
		}

#if WITH_EDITOR
		if (Scope == EGorgeousSingletonScope::Editor)
		{
			if (GEditor)
			{
				if (UWorld* EditorWorld = GEditor->GetEditorWorldContext().World())
				{
					return EditorWorld;
				}
			}
			return nullptr;
		}
#endif

	if (!GEngine)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	if (Scope == EGorgeousSingletonScope::GameInstance)
	{
		return World->GetGameInstance();
	}

	return World;
	}
}

UObject* UGorgeousSingleton::GetOrCreateSingleton(UClass* SingletonClass, UObject* WorldContextObject, EGorgeousSingletonScope Scope)
{
	if (GSingletonExitStarted || !UObjectInitialized() || GExitPurge || !SingletonClass || SingletonClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return nullptr;
	}

	BindSingletonCleanup();
	UObject* ContextObject = ResolveContextObject(WorldContextObject, Scope);
	if (!ContextObject && Scope != EGorgeousSingletonScope::Global) { return nullptr; }
	FGorgeousSingletonKey Key{ SingletonClass, ContextObject, Scope };

	if (TStrongObjectPtr<UObject>* Existing = GSingletonInstances.Find(Key))
	{
		return Existing->Get();
	}

	UObject* Outer = ContextObject ? ContextObject : GetTransientPackage();
	UObject* NewInstance = NewObject<UObject>(Outer, SingletonClass);
	if (!NewInstance)
	{
		return nullptr;
	}

	if (Scope == EGorgeousSingletonScope::Global || Scope == EGorgeousSingletonScope::Editor)
	{
		NewInstance->AddToRoot();
	}

	GSingletonInstances.Add(Key, TStrongObjectPtr<UObject>(NewInstance));
	return NewInstance;
}

void UGorgeousSingleton::DestroySingletonByClass(UClass* SingletonClass, UObject* WorldContextObject, EGorgeousSingletonScope Scope)
{
	if (!UObjectInitialized() || GExitPurge || GSingletonExitStarted || !SingletonClass)
	{
		return;
	}

	UObject* ContextObject = ResolveContextObject(WorldContextObject, Scope);
	FGorgeousSingletonKey Key{ SingletonClass, ContextObject, Scope };

	if (TStrongObjectPtr<UObject>* Existing = GSingletonInstances.Find(Key))
	{
		if (UObject* Instance = Existing->Get())
		{
			if (Instance->IsRooted())
			{
				Instance->RemoveFromRoot();
			}
		}
		GSingletonInstances.Remove(Key);
	}
}

void UGorgeousSingleton::DestroyAllSingletons()
{
	FWorldDelegates::OnWorldCleanup.Remove(GSingletonCleanupHandle);
	GSingletonCleanupHandle.Reset();
	FCoreDelegates::OnPreExit.Remove(GSingletonPreExitHandle);
	GSingletonPreExitHandle.Reset();
	// Module shutdown can occur after UObjectBaseShutdown. Do not inspect a
	// stale object (even IsValid/IsRooted reads the already destroyed object array).
	if (!UObjectInitialized() || GExitPurge) { GSingletonInstances.Reset(); return; }
	for (auto& Pair : GSingletonInstances)
	{
		if (UObject* Instance = Pair.Value.Get())
		{
			if (Instance->IsRooted())
			{
				Instance->RemoveFromRoot();
			}
		}
	}
	GSingletonInstances.Reset();
}