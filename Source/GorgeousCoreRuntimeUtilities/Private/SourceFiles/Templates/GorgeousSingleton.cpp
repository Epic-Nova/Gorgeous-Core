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
#include "GorgeousSingleton.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "UObject/StrongObjectPtr.h"
#if WITH_EDITOR
#include "Editor.h"
#endif

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
	if (!SingletonClass)
	{
		return nullptr;
	}

	UObject* ContextObject = ResolveContextObject(WorldContextObject, Scope);
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
	if (!SingletonClass)
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
