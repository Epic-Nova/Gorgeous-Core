#include "QualityOfLife/GorgeousPlayerController.h"
#include "Misc/ScopeExit.h"
// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "GeneralSystems/GorgeousPermissionManagerSubsystem.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

FString UGorgeousPermissionManagerSubsystem::GetPlayerId(AActor* Player) const
{
	if (!Player)
	{
		return TEXT("System");
	}

    if (APlayerController* PC = Cast<APlayerController>(Player))
    {
        if (const FString* SnapshotKey=OwnerSnapshotKeys.Find(PC)) return *SnapshotKey;
        if (PC->PlayerState) return FString::Printf(TEXT("%d"), PC->PlayerState->GetPlayerId());
    }
	if (APawn* Pawn = Cast<APawn>(Player))
	{
		if (APlayerState* PS = Pawn->GetPlayerState())
		{
			// In a real network implementation, you would use UniqueId. @TODO: Use the Stable Player ID for this
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
    if (GetWorld() && GetWorld()->GetNetMode()==NM_Client) return;
    ON_SCOPE_EXIT { SendOwnerSnapshots(); };
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
    if (GetWorld() && GetWorld()->GetNetMode()==NM_Client) return;
    ON_SCOPE_EXIT { SendOwnerSnapshots(); };
	Roles.Remove(RoleName);
	
	// Remove from all players
	for (auto& Pair : PlayerData)
	{
		Pair.Value.AssignedRoles.Remove(RoleName);
	}
}

void UGorgeousPermissionManagerSubsystem::GrantPermissionToRole(FName RoleName, const FString& PermissionName, bool bAllow)
{
    if (GetWorld() && GetWorld()->GetNetMode()==NM_Client) return;
    ON_SCOPE_EXIT { SendOwnerSnapshots(); };
	if (RoleName.IsNone() || PermissionName.IsEmpty()) return;

	FGorgeousPermissionRole& Role = Roles.FindOrAdd(RoleName);
	Role.RoleName = RoleName;
	Role.Permissions.Add(PermissionName, bAllow);
}

void UGorgeousPermissionManagerSubsystem::RevokePermissionFromRole(FName RoleName, const FString& PermissionName)
{
    if (GetWorld() && GetWorld()->GetNetMode()==NM_Client) return;
    ON_SCOPE_EXIT { SendOwnerSnapshots(); };
	if (FGorgeousPermissionRole* Role = Roles.Find(RoleName))
	{
		Role->Permissions.Remove(PermissionName);
	}
}

void UGorgeousPermissionManagerSubsystem::GrantRole(AActor* Player, FName RoleName)
{
    if (GetWorld() && GetWorld()->GetNetMode()==NM_Client) return;
    ON_SCOPE_EXIT { SendOwnerSnapshots(); };
	if (!Player || RoleName.IsNone()) return;

	FString PlayerId = GetPlayerId(Player);
	FGorgeousPlayerPermissionData& Data = PlayerData.FindOrAdd(PlayerId);
	Data.AssignedRoles.AddUnique(RoleName);
}

void UGorgeousPermissionManagerSubsystem::RevokeRole(AActor* Player, FName RoleName)
{
    if (GetWorld() && GetWorld()->GetNetMode()==NM_Client) return;
    ON_SCOPE_EXIT { SendOwnerSnapshots(); };
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
    if (GetWorld() && GetWorld()->GetNetMode()==NM_Client) return;
    ON_SCOPE_EXIT { SendOwnerSnapshots(); };
	if (!Player || PermissionName.IsEmpty()) return;

	FString PlayerId = GetPlayerId(Player);
	FGorgeousPlayerPermissionData& Data = PlayerData.FindOrAdd(PlayerId);
	Data.PlayerSpecificPermissions.Add(PermissionName, bAllow);
}

void UGorgeousPermissionManagerSubsystem::RevokePermission(AActor* Player, const FString& PermissionName)
{
    if (GetWorld() && GetWorld()->GetNetMode()==NM_Client) return;
    ON_SCOPE_EXIT { SendOwnerSnapshots(); };
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

FGorgeousOwnerPermissionSnapshot UGorgeousPermissionManagerSubsystem::BuildOwnerSnapshot(APlayerController* Controller) const
{
    FGorgeousOwnerPermissionSnapshot Snapshot;
    if (!Controller || !Controller->HasAuthority()) return Snapshot;
    Snapshot.PlayerKey=GetPlayerId(Controller);
    auto AppendRules=[](const TMap<FString,bool>& Source,TArray<FGorgeousPermissionRuleWire>& Out)
    {
        for (const auto& Pair:Source)
        {
            auto& Rule=Out.AddDefaulted_GetRef(); Rule.Permission=Pair.Key; Rule.bAllow=Pair.Value;
        }
    };
    if (const auto* Data=PlayerData.Find(Snapshot.PlayerKey))
    {
        Snapshot.AssignedRoles=Data->AssignedRoles;
        AppendRules(Data->PlayerSpecificPermissions,Snapshot.PlayerOverrides);
    }
    for (FName Name:Snapshot.AssignedRoles)
        if (const auto* Role=Roles.Find(Name))
        {
            auto& WireRole=Snapshot.AssignedRoleRules.AddDefaulted_GetRef();
            WireRole.RoleName=Name; AppendRules(Role->Permissions,WireRole.Rules);
        }
    return Snapshot;
}
void UGorgeousPermissionManagerSubsystem::ApplyOwnerSnapshot(const FGorgeousOwnerPermissionSnapshot& Snapshot,APlayerController* ReceivingOwner)
{
    if (!GetWorld() || GetWorld()->GetNetMode()!=NM_Client || Snapshot.PlayerKey.IsEmpty() || !ReceivingOwner || !ReceivingOwner->IsLocalController()) return;
    OwnerSnapshotKeys.Add(ReceivingOwner,Snapshot.PlayerKey);
    FGorgeousPlayerPermissionData Player;
    Player.AssignedRoles=Snapshot.AssignedRoles;
    for (const auto& Rule:Snapshot.PlayerOverrides) Player.PlayerSpecificPermissions.Add(Rule.Permission,Rule.bAllow);
    PlayerData.Add(Snapshot.PlayerKey,MoveTemp(Player));
    for (const auto& WireRole:Snapshot.AssignedRoleRules)
    {
        FGorgeousPermissionRole Role; Role.RoleName=WireRole.RoleName;
        for (const auto& Rule:WireRole.Rules) Role.Permissions.Add(Rule.Permission,Rule.bAllow);
        Roles.Add(Role.RoleName,MoveTemp(Role));
    }
    OnPermissionsUpdated.Broadcast();
}
void UGorgeousPermissionManagerSubsystem::SendOwnerSnapshots()
{
    UWorld* World=GetWorld(); if (!World || World->GetNetMode()==NM_Client) return;
    OnPermissionsUpdated.Broadcast();
    for (FConstPlayerControllerIterator It=World->GetPlayerControllerIterator();It;++It)
        if (auto* Controller=Cast<AGorgeousPlayerController>(It->Get()))
            if (!Controller->IsLocalController()) Controller->Client_ReceiveGorgeousPermissionSnapshot(BuildOwnerSnapshot(Controller));
}
