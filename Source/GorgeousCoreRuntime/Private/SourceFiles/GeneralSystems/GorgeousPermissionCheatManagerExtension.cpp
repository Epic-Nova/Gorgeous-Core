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
#include "GeneralSystems/GorgeousPermissionCheatManagerExtension.h"

//<=============================--- Includes ---============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GeneralSystems/GorgeousPermissionManagerSubsystem.h"
#include "GeneralSystems/GorgeousPermissionProvider.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Engine/GameInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
//<-------------------------------------------------------------------------->

UGorgeousPermissionManagerSubsystem* UGorgeousPermissionCheatManagerExtension::GetPermissionSubsystem() const
{
	if (const APlayerController* PlayerController = GetPlayerController())
	{
		if (UGameInstance* GameInstance = PlayerController->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UGorgeousPermissionManagerSubsystem>();
		}
	}

	return nullptr;
}

AActor* UGorgeousPermissionCheatManagerExtension::GetLocalPlayerActor() const
{
	if (const APlayerController* PlayerController = GetPlayerController())
	{
		return static_cast<AActor*>(PlayerController->GetPawn().Get());
	}

	return nullptr;
}

void UGorgeousPermissionCheatManagerExtension::Cheat_ListPermissionProviders()
{
	const TArray<UGorgeousPermissionProvider*> Providers = UGorgeousPermissionProvider::GetRegisteredProviders();
	GT_I_LOG(TEXT("Cheat"), TEXT("--- Gorgeous Permission Providers (%d) ---"), Providers.Num());

	for (const UGorgeousPermissionProvider* Provider : Providers)
	{
		GT_I_LOG(TEXT("Cheat"), TEXT("  [%s] Default=%s Override=%s%s"),
			*Provider->PermissionName,
			Provider->bDefaultAllowed ? TEXT("Allow") : TEXT("Deny"),
			Provider->bCheatOverrideActive ? TEXT("Active:") : TEXT("Inactive"),
			Provider->bCheatOverrideActive ? (Provider->bCheatOverrideValue ? TEXT("Allow") : TEXT("Deny")) : TEXT(""));
	}
}

void UGorgeousPermissionCheatManagerExtension::Cheat_SetPermissionOverride(FString PermissionName, bool bAllow)
{
	if (UGorgeousPermissionProvider* Provider = UGorgeousPermissionProvider::FindProvider(PermissionName))
	{
		Provider->SetCheatOverride(true, bAllow);
		GT_S_LOG(TEXT("Cheat"), TEXT("Forced permission %s to %s."), *PermissionName, bAllow ? TEXT("allow") : TEXT("deny"));
		return;
	}

	GT_W_LOG(TEXT("Cheat"), TEXT("Permission provider %s was not found."), *PermissionName);
}

void UGorgeousPermissionCheatManagerExtension::Cheat_ClearPermissionOverride(FString PermissionName)
{
	if (UGorgeousPermissionProvider* Provider = UGorgeousPermissionProvider::FindProvider(PermissionName))
	{
		Provider->SetCheatOverride(false, false);
		GT_S_LOG(TEXT("Cheat"), TEXT("Cleared the permission override for %s."), *PermissionName);
		return;
	}

	GT_W_LOG(TEXT("Cheat"), TEXT("Permission provider %s was not found."), *PermissionName);
}

void UGorgeousPermissionCheatManagerExtension::Cheat_CreatePermissionRole(FString RoleName)
{
	if (UGorgeousPermissionManagerSubsystem* Subsystem = GetPermissionSubsystem())
	{
		Subsystem->CreateOrUpdateRole(FName(*RoleName), TMap<FString, bool>());
		GT_S_LOG(TEXT("Cheat"), TEXT("Created or retained permission role %s."), *RoleName);
		return;
	}

	GT_W_LOG(TEXT("Cheat"), TEXT("Permission subsystem was not found."));
}

void UGorgeousPermissionCheatManagerExtension::Cheat_DeletePermissionRole(FString RoleName)
{
	if (UGorgeousPermissionManagerSubsystem* Subsystem = GetPermissionSubsystem())
	{
		Subsystem->DeleteRole(FName(*RoleName));
		GT_S_LOG(TEXT("Cheat"), TEXT("Deleted permission role %s."), *RoleName);
		return;
	}

	GT_W_LOG(TEXT("Cheat"), TEXT("Permission subsystem was not found."));
}

void UGorgeousPermissionCheatManagerExtension::Cheat_GrantLocalPermissionRole(FString RoleName)
{
	if (UGorgeousPermissionManagerSubsystem* Subsystem = GetPermissionSubsystem())
	{
		if (AActor* PlayerActor = GetLocalPlayerActor())
		{
			Subsystem->GrantRole(PlayerActor, FName(*RoleName));
			GT_S_LOG(TEXT("Cheat"), TEXT("Granted role %s to %s."), *RoleName, *PlayerActor->GetName());
			return;
		}
	}

	GT_W_LOG(TEXT("Cheat"), TEXT("Permission subsystem or local player pawn was not found."));
}

void UGorgeousPermissionCheatManagerExtension::Cheat_RevokeLocalPermissionRole(FString RoleName)
{
	if (UGorgeousPermissionManagerSubsystem* Subsystem = GetPermissionSubsystem())
	{
		if (AActor* PlayerActor = GetLocalPlayerActor())
		{
			Subsystem->RevokeRole(PlayerActor, FName(*RoleName));
			GT_S_LOG(TEXT("Cheat"), TEXT("Revoked role %s from %s."), *RoleName, *PlayerActor->GetName());
			return;
		}
	}

	GT_W_LOG(TEXT("Cheat"), TEXT("Permission subsystem or local player pawn was not found."));
}

void UGorgeousPermissionCheatManagerExtension::Cheat_SetRolePermission(FString RoleName, FString PermissionName, bool bAllow)
{
	if (UGorgeousPermissionManagerSubsystem* Subsystem = GetPermissionSubsystem())
	{
		Subsystem->GrantPermissionToRole(FName(*RoleName), PermissionName, bAllow);
		GT_S_LOG(TEXT("Cheat"), TEXT("Set %s on role %s to %s."), *PermissionName, *RoleName, bAllow ? TEXT("allow") : TEXT("deny"));
		return;
	}

	GT_W_LOG(TEXT("Cheat"), TEXT("Permission subsystem was not found."));
}

void UGorgeousPermissionCheatManagerExtension::Cheat_EvaluateLocalPermission(FString PermissionName)
{
	if (UGorgeousPermissionManagerSubsystem* Subsystem = GetPermissionSubsystem())
	{
		if (AActor* PlayerActor = GetLocalPlayerActor())
		{
			bool bHandled = false;
			const bool bAllowed = Subsystem->EvaluatePlayerPermission(PlayerActor, PermissionName, bHandled);
			GT_I_LOG(TEXT("Cheat"), TEXT("Permission %s for %s: %s (%s)."), *PermissionName, *PlayerActor->GetName(), bAllowed ? TEXT("allow") : TEXT("deny"), bHandled ? TEXT("explicit rule") : TEXT("no explicit rule"));
			return;
		}
	}

	GT_W_LOG(TEXT("Cheat"), TEXT("Permission subsystem or local player pawn was not found."));
}
