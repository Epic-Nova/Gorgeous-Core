// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/SignalBridge/SignalBridgeStorage_OV.h"
#include "GeneralSystems/SignalBridge/SignalBridgeListener_O.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "Net/UnrealNetwork.h"

USignalBridgeStorage_OV::USignalBridgeStorage_OV()
{
	bSupportsNetworking = true;
	RootNetworkConfig.AccessPolicy = EGorgeousObjectVariableAccessPolicy::Custom;
}


void USignalBridgeStorage_OV::RegisterSignal(FGameplayTag Tag, const FGorgeousSignalBridgeAccessRules_S& Rules, AGorgeousPlayerController* Requester)
{
	if (HasAuthority())
	{
		FGorgeousSignalBridgeAccessRules_S& NewRules = AccessRules.FindOrAdd(Tag);
		NewRules = Rules;
		NewRules.RegisteredBy = Requester;
	}
}

bool USignalBridgeStorage_OV::Listen(FGameplayTag Tag, AGorgeousPlayerController* Controller, const FSignalBridgeEventDelegate& Delegate)
{
	if (!EvaluateTagAccess(Controller, Tag))
	{
		return false;
	}

	LocalBindings.FindOrAdd(Tag).Add(Delegate);

	if (HasAuthority())
	{
		if (USignalBridgeListener_O* Registry = GetOrCreateListenerRegistry(Tag))
		{
			Registry->RegisteredListeners.AddUnique(Controller);
		}
	}
	else
	{
		Server_RegisterListener(Tag, Controller);
	}

	return true;
}

void USignalBridgeStorage_OV::Dispatch(FGameplayTag Tag, const FInstancedStruct& Payload)
{
	if (HasAuthority())
	{
		// 1. Fire local bindings on server
		FireLocalSignal(Tag, Payload);

		// 2. Unicast to registered remote listeners via PlayerController relay
		if (USignalBridgeListener_O* Registry = DictionaryAssociations.FindRef(Tag))
		{
			const FGorgeousSignalBridgeAccessRules_S* Rules = AccessRules.Find(Tag);
			const bool bShouldNetwork = Rules ? Rules->bNetworked : true;

			if (bShouldNetwork)
			{
				for (TObjectPtr<AGorgeousPlayerController> Listener : Registry->RegisteredListeners)
				{
					if (Listener && !Listener->IsLocalController())
					{
						Listener->Client_ReceiveSignalBridgeSignal(Tag, Payload);
					}
				}
			}
		}
	}
	else
	{
		if (UWorld* World = GetWorld())
		{
			if (AGorgeousPlayerController* PC = Cast<AGorgeousPlayerController>(World->GetFirstPlayerController()))
			{
				Server_RequestDispatch(Tag, Payload, PC);
			}
		}
	}
}

void USignalBridgeStorage_OV::FireLocalSignal(FGameplayTag Tag, const FInstancedStruct& Payload)
{
	if (FSignalBridgeEventMulticastDelegate* LocalDelegate = LocalBindings.Find(Tag))
	{
		LocalDelegate->Broadcast(Payload);
	}
}

void USignalBridgeStorage_OV::Clear(FGameplayTag Tag, AGorgeousPlayerController* Controller)
{
	LocalBindings.Remove(Tag);

	if (HasAuthority())
	{
		if (USignalBridgeListener_O* Registry = DictionaryAssociations.FindRef(Tag))
		{
			Registry->RegisteredListeners.Remove(Controller);
		}
	}
	else
	{
		Server_UnregisterListener(Tag, Controller);
	}
}

void USignalBridgeStorage_OV::AddAllowedController(FGameplayTag Tag, AGorgeousPlayerController* Controller)
{
	if (HasAuthority() && Controller)
	{
		if (FGorgeousSignalBridgeAccessRules_S* Rules = AccessRules.Find(Tag))
		{
			Rules->AllowedControllers.AddUnique(Controller);
		}
	}
}

void USignalBridgeStorage_OV::RemoveAllowedController(FGameplayTag Tag, AGorgeousPlayerController* Controller)
{
	if (HasAuthority() && Controller)
	{
		if (FGorgeousSignalBridgeAccessRules_S* Rules = AccessRules.Find(Tag))
		{
			Rules->AllowedControllers.Remove(Controller);
		}
	}
}

void USignalBridgeStorage_OV::OnReplicationActivated_Implementation(const FGorgeousAutoReplicationContext& Context)
{
	Super::OnReplicationActivated_Implementation(Context);
	RegisterReplicatedProperty(TEXT("AccessRules"));
}

bool USignalBridgeStorage_OV::CanControllerAccessVariable_Implementation(AGorgeousPlayerController* Controller, FName PropertyName) const
{
	if (PropertyName.IsNone())
	{
		return true;
	}

	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(PropertyName, false);
	if (Tag.IsValid())
	{
		return EvaluateTagAccess(Controller, Tag);
	}

	return true;
}

void USignalBridgeStorage_OV::Server_RegisterListener_Implementation(FGameplayTag Tag, AGorgeousPlayerController* Controller)
{
	if (EvaluateTagAccess(Controller, Tag))
	{
		if (USignalBridgeListener_O* Registry = GetOrCreateListenerRegistry(Tag))
		{
			Registry->RegisteredListeners.AddUnique(Controller);
		}
	}
}

void USignalBridgeStorage_OV::Server_UnregisterListener_Implementation(FGameplayTag Tag, AGorgeousPlayerController* Controller)
{
	if (USignalBridgeListener_O* Registry = DictionaryAssociations.FindRef(Tag))
	{
		Registry->RegisteredListeners.Remove(Controller);
	}
}

void USignalBridgeStorage_OV::Server_RequestDispatch_Implementation(FGameplayTag Tag, FInstancedStruct Payload, AGorgeousPlayerController* Requester)
{
	if (EvaluateTagAccess(Requester, Tag))
	{
		Dispatch(Tag, Payload);
	}
}

USignalBridgeListener_O* USignalBridgeStorage_OV::GetOrCreateListenerRegistry(FGameplayTag Tag)
{
	TObjectPtr<USignalBridgeListener_O>& Registry = DictionaryAssociations.FindOrAdd(Tag);
	if (!Registry)
	{
		Registry = NewObject<USignalBridgeListener_O>(this);
		Registry->ForDispatchTag = Tag;
	}
	return Registry;
}

bool USignalBridgeStorage_OV::EvaluateTagAccess(AGorgeousPlayerController* Controller, FGameplayTag Tag) const
{
	const FGorgeousSignalBridgeAccessRules_S* Rules = AccessRules.Find(Tag);
	if (!Rules)
	{
		return true; 
	}

	switch (Rules->AccessPolicy)
	{
	case EGorgeousObjectVariableAccessPolicy::Everyone:
		return true;

	case EGorgeousObjectVariableAccessPolicy::OwningControllerOnly:
		return Controller && Controller == Rules->RegisteredBy;

	case EGorgeousObjectVariableAccessPolicy::Custom:
		{
			if (!Controller) return false;

			// 1. Check explicit controller instances
			if (Rules->AllowedControllers.Contains(Controller))
			{
				return true;
			}

			// 2. Check allowed classes
			if (Rules->AllowedClasses.Num() > 0)
			{
				bool bClassAllowed = false;
				for (auto& AllowedClass : Rules->AllowedClasses)
				{
					if (Controller->IsA(AllowedClass))
					{
						bClassAllowed = true;
						break;
					}
				}
				return bClassAllowed;
			}
			return false;
		}

	default:
		break;
	}

	return false;
}
