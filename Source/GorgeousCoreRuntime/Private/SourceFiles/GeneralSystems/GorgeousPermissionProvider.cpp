// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "GeneralSystems/GorgeousPermissionProvider.h"
#include "GeneralSystems/GorgeousPermissionManagerSubsystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

TMap<FString, TWeakObjectPtr<UGorgeousPermissionProvider>> UGorgeousPermissionProvider::RegisteredProviders;

UGorgeousPermissionProvider::UGorgeousPermissionProvider()
{
}

void UGorgeousPermissionProvider::SetCheatOverride(bool bInOverrideActive, bool bInOverrideValue)
{
	bCheatOverrideActive = bInOverrideActive;
	bCheatOverrideValue = bInOverrideValue;
}

bool UGorgeousPermissionProvider::EvaluatePermission_Implementation(AActor* Interactor, AActor* TargetObject) const
{
	if (bCheatOverrideActive)
	{
		return bCheatOverrideValue;
	}

	if (Interactor && Interactor->GetWorld())
	{
		if (UGameInstance* GI = Interactor->GetWorld()->GetGameInstance())
		{
			if (UGorgeousPermissionManagerSubsystem* Subsystem = GI->GetSubsystem<UGorgeousPermissionManagerSubsystem>())
			{
				bool bHandled = false;
				bool bResult = Subsystem->EvaluatePlayerPermission(Interactor, PermissionName, bHandled);
				if (bHandled)
				{
					return bResult;
				}
			}
		}
	}

	return bDefaultAllowed;
}

void UGorgeousPermissionProvider::RegisterProvider(UGorgeousPermissionProvider* Provider)
{
	if (Provider && !Provider->PermissionName.IsEmpty())
	{
		RegisteredProviders.Add(Provider->PermissionName, Provider);
	}
}

void UGorgeousPermissionProvider::UnregisterProvider(UGorgeousPermissionProvider* Provider)
{
	if (Provider && !Provider->PermissionName.IsEmpty())
	{
		RegisteredProviders.Remove(Provider->PermissionName);
	}
}

TArray<UGorgeousPermissionProvider*> UGorgeousPermissionProvider::GetRegisteredProviders()
{
	TArray<UGorgeousPermissionProvider*> OutProviders;
	for (auto& Pair : RegisteredProviders)
	{
		if (UGorgeousPermissionProvider* Provider = Pair.Value.Get())
		{
			OutProviders.Add(Provider);
		}
	}
	return OutProviders;
}

UGorgeousPermissionProvider* UGorgeousPermissionProvider::FindProvider(const FString& InPermissionName)
{
	if (TWeakObjectPtr<UGorgeousPermissionProvider>* Found = RegisteredProviders.Find(InPermissionName))
	{
		return Found->Get();
	}
	return nullptr;
}
