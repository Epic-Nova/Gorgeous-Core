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
#pragma once

//<=============================--- Includes ---============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousPermissionManagerSubsystem.generated.h"
//<-------------------------------------------------------------------------->

class AActor;

/**
 * Defines the permissions granted or denied by a named role.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousPermissionRole
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Permission")
	FName RoleName;

	// Explicitly granted or denied permissions for this role.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Permission")
	TMap<FString, bool> Permissions;
};

/**
 * Defines roles and explicit overrides assigned to a player.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousPlayerPermissionData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Permission")
	TArray<FName> AssignedRoles;

	// Player-specific permission overrides (overrides role permissions).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Permission")
	TMap<FString, bool> PlayerSpecificPermissions;
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Permission Manager Subsystem
| Functional Name: UGorgeousPermissionManagerSubsystem
| Parent Class: UGameInstanceSubsystem
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Game instance subsystem that manages roles, player-role assignments, and
| explicit permission overrides for runtime permission evaluation.
<--------------------------------------------------------------------------->
<==========================================================================>
 */
UCLASS(meta = (
	DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/Overview",
	DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/GorgeousPermissionManagerSubsystem",
	DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/Examples/"
	))
class GORGEOUSCORERUNTIME_API UGorgeousPermissionManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Creates a role or replaces its explicit permission rules.
	 *
	 * @param RoleName The role to create or update.
	 * @param Permissions The explicit permission rules assigned to the role.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Permissions")
	void CreateOrUpdateRole(FName RoleName, const TMap<FString, bool>& Permissions);

	/**
	 * Deletes a role and its permission rules.
	 *
	 * @param RoleName The role to delete.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Permissions")
	void DeleteRole(FName RoleName);

	/**
	 * Grants or denies a permission through a role rule.
	 *
	 * @param RoleName The role whose rules are updated.
	 * @param PermissionName The permission to configure.
	 * @param bAllow True to grant the permission, false to deny it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Permissions")
	void GrantPermissionToRole(FName RoleName, const FString& PermissionName, bool bAllow);

	/**
	 * Removes a permission rule from a role.
	 *
	 * @param RoleName The role whose rules are updated.
	 * @param PermissionName The permission rule to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Permissions")
	void RevokePermissionFromRole(FName RoleName, const FString& PermissionName);

	/**
	 * Assigns a role to a player.
	 *
	 * @param Player The player receiving the role.
	 * @param RoleName The role to assign.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Permissions")
	void GrantRole(AActor* Player, FName RoleName);

	/**
	 * Removes a role from a player.
	 *
	 * @param Player The player losing the role.
	 * @param RoleName The role to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Permissions")
	void RevokeRole(AActor* Player, FName RoleName);

	/**
	 * Determines whether a player has an assigned role.
	 *
	 * @param Player The player to inspect.
	 * @param RoleName The role to find.
	 * @return True when the player has the role, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Permissions")
	bool HasRole(AActor* Player, FName RoleName) const;

	/**
	 * Grants or denies a player-specific permission override.
	 *
	 * @param Player The player whose override is updated.
	 * @param PermissionName The permission to configure.
	 * @param bAllow True to grant the permission, false to deny it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Permissions")
	void GrantPermission(AActor* Player, const FString& PermissionName, bool bAllow);

	/**
	 * Removes a player-specific permission override.
	 *
	 * @param Player The player whose override is removed.
	 * @param PermissionName The permission override to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Permissions")
	void RevokePermission(AActor* Player, const FString& PermissionName);

	/**
	 * Evaluates whether a player has a specific permission.
	 *
	 * @param Player The player whose permission is evaluated.
	 * @param PermissionName The permission to evaluate.
	 * @param out_bHandled Receives whether an explicit rule handled the result.
	 * @return True when the permission is granted, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Permissions")
	bool EvaluatePlayerPermission(AActor* Player, const FString& PermissionName, bool& out_bHandled) const;
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<============================--- Variables ---============================>
	#pragma region Variables
protected:
	// Global defined roles
	UPROPERTY()
	TMap<FName, FGorgeousPermissionRole> Roles;

	/** Player permission data, keyed by a stable ID or Net ID string.
	 * For simplicity right now, keyed by the Actor's name or stable ID string if available.
	 * We'll use FString as the key to represent the Player's unique ID.
	 */
	UPROPERTY()
	TMap<FString, FGorgeousPlayerPermissionData> PlayerData;
	//<------------------------------------------------------------------------->
	#pragma endregion Variables


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
protected:

	FString GetPlayerId(AActor* Player) const;
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only
};