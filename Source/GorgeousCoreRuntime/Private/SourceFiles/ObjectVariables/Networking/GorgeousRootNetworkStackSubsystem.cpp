#include "ObjectVariables/Networking/GorgeousRootNetworkStackSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "QualityOfLife/GorgeousPlayerController.h"

namespace GorgeousRootNetworkStackSubsystem_Private
{
static const FName DefaultChannelName(TEXT("Default"));
}

UGorgeousRootNetworkStackSubsystem* UGorgeousRootNetworkStackSubsystem::Get(UWorld* World)
{
	return World ? World->GetSubsystem<UGorgeousRootNetworkStackSubsystem>() : nullptr;
}

FName UGorgeousRootNetworkStackSubsystem::GetDefaultChannelName()
{
	return GorgeousRootNetworkStackSubsystem_Private::DefaultChannelName;
}

void UGorgeousRootNetworkStackSubsystem::RegisterVariable(UGorgeousObjectVariable* Variable)
{
	if (!IsValid(Variable))
	{
		return;
	}

	RegisteredVariables.Add(Variable);

	if (AController* OwningController = Variable->ResolveOwningPlayerController())
	{
		TryAutoSubscribeController(Variable, OwningController);
	}
}

void UGorgeousRootNetworkStackSubsystem::UnregisterVariable(UGorgeousObjectVariable* Variable)
{
	if (!Variable)
	{
		return;
	}

	RegisteredVariables.Remove(Variable);
}

bool UGorgeousRootNetworkStackSubsystem::CanControllerAccess(const UGorgeousObjectVariable* Variable, AController* Controller) const
{
	if (!Variable)
	{
		return false;
	}

	if (!RegisteredVariables.Contains(Variable))
	{
		return true;
	}

	const FName ChannelName = Variable->GetEffectiveNetworkChannel();
	TryAutoSubscribeController(Variable, Controller);
	if (!IsControllerSubscribedToChannel(Controller, ChannelName))
	{
		return false;
	}

	return Variable->EvaluateAccessPolicyForController(Controller);
}

bool UGorgeousRootNetworkStackSubsystem::HasChannelSubscribers(FName ChannelName) const
{
	if (ChannelName.IsNone())
	{
		return false;
	}

	if (const TSet<FObjectKey>* Membership = ChannelMembership.Find(ChannelName))
	{
		TArray<FObjectKey> ControllerKeys;
		ControllerKeys.Reserve(Membership->Num());
		for (const FObjectKey& ControllerKey : *Membership)
		{
			ControllerKeys.Add(ControllerKey);
		}

		for (const FObjectKey& ControllerKey : ControllerKeys)
		{
			CleanupControllerEntry(ControllerKey);
			if (ControllerStates.Contains(ControllerKey))
			{
				return true;
			}
		}
	}

	return false;
}

bool UGorgeousRootNetworkStackSubsystem::TryAutoSubscribeController(const UGorgeousObjectVariable* Variable, AController* Controller) const
{
	if (!Variable || !Controller)
	{
		return false;
	}

	if (!RegisteredVariables.Contains(Variable))
	{
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		if (World->GetNetMode() == NM_Client)
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	if (!Variable->IsRootNetworkStackEnabled())
	{
		return false;
	}

	const FName ChannelName = Variable->GetEffectiveNetworkChannel();
	if (ChannelName.IsNone())
	{
		return false;
	}

	if (IsControllerSubscribedToChannel(Controller, ChannelName))
	{
		return true;
	}

	if (!Variable->EvaluateAccessPolicyForController(Controller))
	{
		return false;
	}

	AddControllerSubscription(Controller, ChannelName);
	return true;
}

void UGorgeousRootNetworkStackSubsystem::SubscribeControllerToChannel(AController* Controller, FName ChannelName)
{
	if (!Controller || ChannelName.IsNone())
	{
		return;
	}

	AddControllerSubscription(Controller, ChannelName);
}

void UGorgeousRootNetworkStackSubsystem::UnsubscribeControllerFromChannel(AController* Controller, FName ChannelName)
{
	if (!Controller || ChannelName.IsNone())
	{
		return;
	}

	RemoveControllerSubscription(Controller, ChannelName);
}

void UGorgeousRootNetworkStackSubsystem::ClearControllerSubscriptions(AController* Controller)
{
	if (!Controller)
	{
		return;
	}

	const FObjectKey ControllerKey(Controller);
	CleanupControllerEntry(ControllerKey);
	RemoveControllerEntry(ControllerKey);
}

bool UGorgeousRootNetworkStackSubsystem::IsControllerSubscribedToChannel(AController* Controller, FName ChannelName) const
{
	if (ChannelName.IsNone() || !Controller)
	{
		return true;
	}

	const FObjectKey ControllerKey(Controller);
	CleanupControllerEntry(ControllerKey);
	if (const FRootNetworkChannelControllerState* State = ControllerStates.Find(ControllerKey))
	{
		return State->Subscriptions.Contains(ChannelName);
	}

	return false;
}

TArray<FName> UGorgeousRootNetworkStackSubsystem::GetControllerSubscriptions(AController* Controller) const
{
	TArray<FName> Result;
	if (!Controller)
	{
		return Result;
	}

	const FObjectKey ControllerKey(Controller);
	CleanupControllerEntry(ControllerKey);
	if (const FRootNetworkChannelControllerState* State = ControllerStates.Find(ControllerKey))
	{
		Result.Reserve(State->Subscriptions.Num());
		for (const FName Subscription : State->Subscriptions)
		{
			Result.Add(Subscription);
		}
	}
	return Result;
}

void UGorgeousRootNetworkStackSubsystem::CleanupControllerEntry(const FObjectKey& ControllerKey) const
{
	if (const FRootNetworkChannelControllerState* State = ControllerStates.Find(ControllerKey))
	{
		if (!State->Controller.IsValid())
		{
			RemoveControllerEntry(ControllerKey);
		}
	}
}

void UGorgeousRootNetworkStackSubsystem::RemoveControllerEntry(const FObjectKey& ControllerKey) const
{
	if (const FRootNetworkChannelControllerState* State = ControllerStates.Find(ControllerKey))
	{
		for (const FName Channel : State->Subscriptions)
		{
			if (TSet<FObjectKey>* Membership = ChannelMembership.Find(Channel))
			{
				Membership->Remove(ControllerKey);
				if (Membership->Num() == 0)
				{
					ChannelMembership.Remove(Channel);
				}
			}
		}
		ControllerStates.Remove(ControllerKey);
	}
}

void UGorgeousRootNetworkStackSubsystem::AddControllerSubscription(AController* Controller, FName ChannelName) const
{
	const FObjectKey ControllerKey(Controller);
	CleanupControllerEntry(ControllerKey);
	FRootNetworkChannelControllerState& State = ControllerStates.FindOrAdd(ControllerKey);
	State.Controller = Controller;
	if (State.Subscriptions.Contains(ChannelName))
	{
		return;
	}

	State.Subscriptions.Add(ChannelName);
	ChannelMembership.FindOrAdd(ChannelName).Add(ControllerKey);
}

void UGorgeousRootNetworkStackSubsystem::RemoveControllerSubscription(AController* Controller, FName ChannelName) const
{
	const FObjectKey ControllerKey(Controller);
	CleanupControllerEntry(ControllerKey);
	if (FRootNetworkChannelControllerState* State = ControllerStates.Find(ControllerKey))
	{
		if (!State->Subscriptions.Remove(ChannelName))
		{
			return;
		}

		if (State->Subscriptions.Num() == 0)
		{
			State->Controller = nullptr;
			ControllerStates.Remove(ControllerKey);
		}
	}

	if (TSet<FObjectKey>* Membership = ChannelMembership.Find(ChannelName))
	{
		Membership->Remove(ControllerKey);
		if (Membership->Num() == 0)
		{
			ChannelMembership.Remove(ChannelName);
		}
	}
}
