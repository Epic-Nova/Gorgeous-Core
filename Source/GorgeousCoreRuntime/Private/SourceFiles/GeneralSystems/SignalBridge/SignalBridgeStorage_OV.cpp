// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/SignalBridge/SignalBridgeStorage_OV.h"
#include "GeneralSystems/SignalBridge/SignalBridgeListenerStructures.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "QualityOfLife/GorgeousWorldSettings.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

static const FName SignalBridgeEntryKey = TEXT("SignalBridge");

USignalBridgeStorage_OV::USignalBridgeStorage_OV()
{
	bSupportsNetworking = true;
	ReplicationMode = EGorgeousObjectVariableReplicationMode::EFullAutoReplication;
}

void USignalBridgeStorage_OV::OnReplicationActivated_Implementation(const FGorgeousAutoReplicationContext& Context)
{
	Super::OnReplicationActivated_Implementation(Context);
	RegisterReplicatedProperty(GET_MEMBER_NAME_CHECKED(USignalBridgeStorage_OV, AccessRules));
}

bool USignalBridgeStorage_OV::CanControllerAccessVariable_Implementation(AGorgeousPlayerController* Controller, FName PropertyName) const
{
	if (PropertyName.IsNone()) return true;

	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(PropertyName, false);
	if (Tag.IsValid())
	{
		return EvaluateTagAccess(Controller, Tag);
	}

	return true;
}

void USignalBridgeStorage_OV::RegisterSignal(FGameplayTag Tag, const FGorgeousSignalBridgeAccessRules_S& Rules, AGorgeousPlayerController* Requester)
{
	if (HasAuthority())
	{
		FGorgeousSignalBridgeAccessRules_S NormalizedRules = Rules;
		if (!NormalizedRules.RegisteredBy && Requester)
		{
			NormalizedRules.RegisteredBy = Requester;
		}

		// Only mark dirty if the rules are actually different
		if (!AccessRules.Contains(Tag) || !(AccessRules[Tag] == NormalizedRules))
		{
			AccessRules.Add(Tag, NormalizedRules);
			MarkPropertyDirty(GET_MEMBER_NAME_CHECKED(USignalBridgeStorage_OV, AccessRules));
		}
	}
}

bool USignalBridgeStorage_OV::Listen(FGameplayTag Tag, AGorgeousPlayerController* Controller, const FSignalBridgeEventDelegate& Delegate)
{
	// Bind local delegate - Always allowed locally
	LocalBindings.FindOrAdd(Tag).Add(Delegate);
	GT_I_LOG("GT.SignalBridge", TEXT("[%s] Registered local listener for tag %s."), *GetName(), *Tag.ToString());

	if (!Controller) return true; // Local only success

	// Register with server if we are a client
	if (!HasAuthority())
	{
		Server_RegisterListener(Tag, Controller);
	}
	else
	{
		FGorgeousSignalBridgeListener_S& Registry = GetOrCreateListenerRegistry(Tag);
		Registry.RegisteredListeners.AddUnique(Controller);
	}

	return true;
}

bool USignalBridgeStorage_OV::ListenToActor(FGameplayTag Tag, AActor* TargetActor, AGorgeousPlayerController* Controller, const FSignalBridgeEventDelegate& Delegate)
{
	if (!Controller || !TargetActor) return false;

	// For local bindings, we currently don't have per-actor filtering in the multicast delegate.
	// But we can wrap the delegate in a lambda that filters by source.
	// However, to stay consistent with the server-side optimization, we'll use a special local binding map or just filter in the delegate.
	
	// Actually, let's just use the standard local bindings but the server will only send us relevant ones.
	LocalBindings.FindOrAdd(Tag).Add(Delegate);

	if (!HasAuthority())
	{
		Server_RegisterActorListener(Tag, TargetActor, Controller);
	}
	else
	{
		FGorgeousSignalBridgeListener_S& Registry = GetOrCreateListenerRegistry(Tag);
		Registry.ActorScopedListeners.FindOrAdd(TargetActor).Controllers.AddUnique(Controller);
	}

	return true;
}

void USignalBridgeStorage_OV::Dispatch(FGameplayTag Tag, const FInstancedStruct& Payload)
{
	// 1. Fire local bindings first
	FireLocalSignal(Tag, Payload);

	// 2. Relay to server if we are a client
	if (!HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			if (AGorgeousPlayerController* PC = Cast<AGorgeousPlayerController>(World->GetFirstPlayerController()))
			{
				Server_RequestDispatch(Tag, Payload, PC);
			}
		}
	}
	else
	{
		// 3. Authority: Resolve listeners and RPC them
		if (FGorgeousSignalBridgeListener_S* Registry = DictionaryAssociations.Find(Tag))
		{
			TSet<AGorgeousPlayerController*> Targets;

			// Global listeners
			for (TObjectPtr<AGorgeousPlayerController> Listener : Registry->RegisteredListeners)
			{
				if (Listener && EvaluateTagAccess(Listener, Tag))
				{
					Targets.Add(Listener);
				}
			}

			// Actor-scoped listeners: Try to find Source in payload
			AActor* SourceActor = nullptr;
			if (const UScriptStruct* Struct = Payload.GetScriptStruct())
			{
				if (FObjectProperty* SourceProp = CastField<FObjectProperty>(Struct->FindPropertyByName(TEXT("Source"))))
				{
					if (UObject* Value = SourceProp->GetObjectPropertyValue_InContainer(Payload.GetMemory()))
					{
						SourceActor = Cast<AActor>(Value);
					}
				}
			}
			
			if (SourceActor)
			{
				if (FGorgeousControllerArray_S* Scoped = Registry->ActorScopedListeners.Find(SourceActor))
				{
					for (TObjectPtr<AGorgeousPlayerController> Listener : Scoped->Controllers)
					{
						if (Listener && EvaluateTagAccess(Listener, Tag))
						{
							Targets.Add(Listener);
						}
					}
				}
			}

			// SEND RPCs
			for (AGorgeousPlayerController* Target : Targets)
			{
				Target->Client_ReceiveSignal(Tag, Payload);
			}
		}
	}
}

void USignalBridgeStorage_OV::Clear(FGameplayTag Tag, AGorgeousPlayerController* Controller)
{
	LocalBindings.Remove(Tag);

	if (!HasAuthority())
	{
		Server_UnregisterListener(Tag, Controller);
	}
	else
	{
		if (FGorgeousSignalBridgeListener_S* Registry = DictionaryAssociations.Find(Tag))
		{
			Registry->RegisteredListeners.Remove(Controller);
			// Also remove from actor scoped
			for (auto& Pair : Registry->ActorScopedListeners)
			{
				Pair.Value.Controllers.Remove(Controller);
			}
		}
	}
}

void USignalBridgeStorage_OV::FireLocalSignal(FGameplayTag Tag, const FInstancedStruct& Payload)
{
	if (FSignalBridgeEventMulticastDelegate* Delegate = LocalBindings.Find(Tag))
	{
		GT_I_LOG("GT.SignalBridge", TEXT("[%s] Firing local signal for tag %s."), *GetName(), *Tag.ToString());
		Delegate->Broadcast(Tag, Payload);
	}
	else
	{
		GT_I_LOG("GT.SignalBridge", TEXT("[%s] No local listeners found for tag %s."), *GetName(), *Tag.ToString());
	}
}

void USignalBridgeStorage_OV::AddAllowedController(FGameplayTag Tag, AGorgeousPlayerController* Controller)
{
	if (HasAuthority() && Controller)
	{
		FGorgeousSignalBridgeAccessRules_S& Rules = AccessRules.FindOrAdd(Tag);
		Rules.AccessPolicy = EGorgeousObjectVariableAccessPolicy::Custom;
		Rules.AllowedControllers.AddUnique(Controller);
		MarkPropertyDirty(GET_MEMBER_NAME_CHECKED(USignalBridgeStorage_OV, AccessRules));
	}
}

void USignalBridgeStorage_OV::RemoveAllowedController(FGameplayTag Tag, AGorgeousPlayerController* Controller)
{
	if (HasAuthority() && Controller)
	{
		if (FGorgeousSignalBridgeAccessRules_S* Rules = AccessRules.Find(Tag))
		{
			Rules->AllowedControllers.Remove(Controller);
			MarkPropertyDirty(GET_MEMBER_NAME_CHECKED(USignalBridgeStorage_OV, AccessRules));
		}
	}
}

void USignalBridgeStorage_OV::Server_RegisterListener_Implementation(FGameplayTag Tag, AGorgeousPlayerController* Controller)
{
	FGorgeousSignalBridgeListener_S& Registry = GetOrCreateListenerRegistry(Tag);
	Registry.RegisteredListeners.AddUnique(Controller);
}

void USignalBridgeStorage_OV::Server_RegisterActorListener_Implementation(FGameplayTag Tag, AActor* TargetActor, AGorgeousPlayerController* Controller)
{
	if (TargetActor && Controller)
	{
		FGorgeousSignalBridgeListener_S& Registry = GetOrCreateListenerRegistry(Tag);
		Registry.ActorScopedListeners.FindOrAdd(TargetActor).Controllers.AddUnique(Controller);
	}
}

void USignalBridgeStorage_OV::Server_UnregisterListener_Implementation(FGameplayTag Tag, AGorgeousPlayerController* Controller)
{
	if (FGorgeousSignalBridgeListener_S* Registry = DictionaryAssociations.Find(Tag))
	{
		Registry->RegisteredListeners.Remove(Controller);
		for (auto& Pair : Registry->ActorScopedListeners)
		{
			Pair.Value.Controllers.Remove(Controller);
		}
	}
}

void USignalBridgeStorage_OV::Server_RequestDispatch_Implementation(FGameplayTag Tag, FInstancedStruct Payload, AGorgeousPlayerController* Requester)
{
	// Validate access before dispatching from client
	if (EvaluateTagAccess(Requester, Tag))
	{
		Dispatch(Tag, Payload);
	}
}

FGorgeousSignalBridgeListener_S& USignalBridgeStorage_OV::GetOrCreateListenerRegistry(FGameplayTag Tag)
{
	if (FGorgeousSignalBridgeListener_S* Existing = DictionaryAssociations.Find(Tag))
	{
		return *Existing;
	}

	FGorgeousSignalBridgeListener_S& NewRegistry = DictionaryAssociations.Add(Tag);
	NewRegistry.ForDispatchTag = Tag;
	return NewRegistry;
}

bool USignalBridgeStorage_OV::EvaluateTagAccess(AGorgeousPlayerController* Controller, FGameplayTag Tag) const
{
	const FGorgeousSignalBridgeAccessRules_S* Rules = AccessRules.Find(Tag);
	if (!Rules) return true; // Default to public

	switch (Rules->AccessPolicy)
	{
	case EGorgeousObjectVariableAccessPolicy::Everyone:
		return true;
	case EGorgeousObjectVariableAccessPolicy::OwningControllerOnly:
		return Controller != nullptr; // Simplified
	case EGorgeousObjectVariableAccessPolicy::Custom:
		{
			if (!Controller) return false;
			if (Rules->AllowedControllers.Contains(Controller)) return true;
			for (const auto& AllowedClass : Rules->AllowedClasses)
			{
				if (Controller->IsA(AllowedClass)) return true;
			}
			return false;
		}
	default:
		break;
	}
	return false;
}
