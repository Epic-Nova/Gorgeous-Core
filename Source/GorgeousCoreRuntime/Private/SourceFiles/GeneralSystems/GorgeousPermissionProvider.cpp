// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "GeneralSystems/GorgeousPermissionProvider.h"
#include "GameFramework/Actor.h"

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
