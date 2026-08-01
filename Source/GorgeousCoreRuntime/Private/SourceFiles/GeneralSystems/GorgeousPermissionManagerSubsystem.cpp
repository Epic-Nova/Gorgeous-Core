// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "GeneralSystems/GorgeousPermissionManagerSubsystem.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"

FString UGorgeousPermissionManagerSubsystem::GetPlayerId(AActor* Player) const
{
	if (!Player)
	{
		return TEXT("System");
	}

	if (APawn* Pawn = Cast<APawn>(Player))
	{
		if (APlayerState* PS = Pawn->GetPlayerState())
		{
			// In a real network implementation, you would use UniqueId.
			return FString::Printf(TEXT("%d"), PS->GetPlayerId());
		}
	}
	else if (APlayerState* PS = Cast<APlayerState>(Player))
	{
		return FString::Printf(TEXT("%d"), PS->GetPlayerId());
	}

	// Fallback to name
	return Player->GetName();
}

void UGorgeousPermissionManagerSubsystem::CreateOrUpdateRole(FName RoleName, const TMap<FString, bool>& Permissions)
{
	if (RoleName.IsNone()) return;

	FGorgeousPermissionRole& Role = Roles.FindOrAdd(RoleName);
	Role.RoleName = RoleName;
	
	// Merge permissions
	for (const auto& Pair : Permissions)
	{
		Role.Permissions.Add(Pair.Key, Pair.Value);
	}
}

void UGorgeousPermissionManagerSubsystem::DeleteRole(FName RoleName)
{
	Roles.Remove(RoleName);
	
	// Remove from all players
	for (auto& Pair : PlayerData)
	{
		Pair.Value.AssignedRoles.Remove(RoleName);
	}
}

void UGorgeousPermissionManagerSubsystem::GrantPermissionToRole(FName RoleName, const FString& PermissionName, bool bAllow)
{
	if (RoleName.IsNone() || PermissionName.IsEmpty()) return;

	FGorgeousPermissionRole& Role = Roles.FindOrAdd(RoleName);
	Role.RoleName = RoleName;
	Role.Permissions.Add(PermissionName, bAllow);
}

void UGorgeousPermissionManagerSubsystem::RevokePermissionFromRole(FName RoleName, const FString& PermissionName)
{
	if (FGorgeousPermissionRole* Role = Roles.Find(RoleName))
	{
		Role->Permissions.Remove(PermissionName);
	}
}

void UGorgeousPermissionManagerSubsystem::GrantRole(AActor* Player, FName RoleName)
{
	if (!Player || RoleName.IsNone()) return;

	FString PlayerId = GetPlayerId(Player);
	FGorgeousPlayerPermissionData& Data = PlayerData.FindOrAdd(PlayerId);
	Data.AssignedRoles.AddUnique(RoleName);
}

void UGorgeousPermissionManagerSubsystem::RevokeRole(AActor* Player, FName RoleName)
{
	if (!Player || RoleName.IsNone()) return;

	FString PlayerId = GetPlayerId(Player);
	if (FGorgeousPlayerPermissionData* Data = PlayerData.Find(PlayerId))
	{
		Data->AssignedRoles.Remove(RoleName);
	}
}

bool UGorgeousPermissionManagerSubsystem::HasRole(AActor* Player, FName RoleName) const
{
	if (!Player || RoleName.IsNone()) return false;

	FString PlayerId = GetPlayerId(Player);
	if (const FGorgeousPlayerPermissionData* Data = PlayerData.Find(PlayerId))
	{
		return Data->AssignedRoles.Contains(RoleName);
	}
	return false;
}

void UGorgeousPermissionManagerSubsystem::GrantPermission(AActor* Player, const FString& PermissionName, bool bAllow)
{
	if (!Player || PermissionName.IsEmpty()) return;

	FString PlayerId = GetPlayerId(Player);
	FGorgeousPlayerPermissionData& Data = PlayerData.FindOrAdd(PlayerId);
	Data.PlayerSpecificPermissions.Add(PermissionName, bAllow);
}

void UGorgeousPermissionManagerSubsystem::RevokePermission(AActor* Player, const FString& PermissionName)
{
	if (!Player || PermissionName.IsEmpty()) return;

	FString PlayerId = GetPlayerId(Player);
	if (FGorgeousPlayerPermissionData* Data = PlayerData.Find(PlayerId))
	{
		Data->PlayerSpecificPermissions.Remove(PermissionName);
	}
}

bool UGorgeousPermissionManagerSubsystem::EvaluatePlayerPermission(AActor* Player, const FString& PermissionName, bool& out_bHandled) const
{
	out_bHandled = false;
	if (!Player || PermissionName.IsEmpty()) return false;

	FString PlayerId = GetPlayerId(Player);
	
	// 1. Check Player-Specific Overrides First
	if (const FGorgeousPlayerPermissionData* Data = PlayerData.Find(PlayerId))
	{
		if (const bool* bOverrideValue = Data->PlayerSpecificPermissions.Find(PermissionName))
		{
			out_bHandled = true;
			return *bOverrideValue;
		}

		// 2. Check Roles (Highest precedence could be defined by array order or a priority, for now we just check all)
		for (FName RoleName : Data->AssignedRoles)
		{
			if (const FGorgeousPermissionRole* Role = Roles.Find(RoleName))
			{
				if (const bool* bRoleValue = Role->Permissions.Find(PermissionName))
				{
					out_bHandled = true;
					return *bRoleValue; // Return first found match (Could be refined with priority)
				}
			}
		}
	}

	return false;
}
