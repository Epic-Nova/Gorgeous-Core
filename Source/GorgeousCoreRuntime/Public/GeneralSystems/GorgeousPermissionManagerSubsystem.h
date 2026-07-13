// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GorgeousPermissionManagerSubsystem.generated.h"

class AActor;

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousPermissionRole
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Permission")
	FName RoleName;

	/** Explicitly granted or denied permissions for this role. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Permission")
	TMap<FString, bool> Permissions;
};

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousPlayerPermissionData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Permission")
	TArray<FName> AssignedRoles;

	/** Player-specific permission overrides (overrides role permissions). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Permission")
	TMap<FString, bool> PlayerSpecificPermissions;
};

/**
 * Modern permission subsystem oriented on Minecraft-style Role and Permission management.
 * Manages roles, player-to-role assignments, and individual permission overrides.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousPermissionManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ── Role Management ────────────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Permissions")
	void CreateOrUpdateRole(FName RoleName, const TMap<FString, bool>& Permissions);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Permissions")
	void DeleteRole(FName RoleName);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Permissions")
	void GrantPermissionToRole(FName RoleName, const FString& PermissionName, bool bAllow);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Permissions")
	void RevokePermissionFromRole(FName RoleName, const FString& PermissionName);

	// ── Player Role/Permission Management ────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Permissions")
	void GrantRole(AActor* Player, FName RoleName);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Permissions")
	void RevokeRole(AActor* Player, FName RoleName);

	UFUNCTION(BlueprintPure, Category = "Gorgeous|Permissions")
	bool HasRole(AActor* Player, FName RoleName) const;

	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Permissions")
	void GrantPermission(AActor* Player, const FString& PermissionName, bool bAllow);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Permissions")
	void RevokePermission(AActor* Player, const FString& PermissionName);

	// ── Evaluation ───────────────────────────────────────────────────────────

	/** 
	 * Evaluates if a player has a specific permission. 
	 * Returns true if allowed, false if denied. 
	 * OutbHandled will be true if an explicit rule was found, false if it fell back to default.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Permissions")
	bool EvaluatePlayerPermission(AActor* Player, const FString& PermissionName, bool& out_bHandled) const;

protected:
	/** Global defined roles */
	UPROPERTY()
	TMap<FName, FGorgeousPermissionRole> Roles;

	/** Player permission data, keyed by a stable ID or Net ID string. 
	 * For simplicity right now, keyed by the Actor's name or stable ID string if available. 
	 * We'll use FString as the key to represent the Player's unique ID.
	 */
	UPROPERTY()
	TMap<FString, FGorgeousPlayerPermissionData> PlayerData;

	FString GetPlayerId(AActor* Player) const;
};
