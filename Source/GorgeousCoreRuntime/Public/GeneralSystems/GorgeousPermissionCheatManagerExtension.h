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
#include "GameFramework/CheatManager.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousPermissionCheatManagerExtension.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Permission Cheat Manager Extension
| Functional Name: UGorgeousPermissionCheatManagerExtension
| Parent Class: UCheatManagerExtension
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Cheat Manager Extension that inspects and changes runtime permission roles,
| player assignments, and registered provider overrides during development.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
//<=================--- Forward Declarations ---=================>
class AActor;
class UGorgeousPermissionManagerSubsystem;
//<------------------------------------------------------------->

UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/GorgeousPermissionCheatManagerExtension",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousPermissionCheatManagerExtension : public UCheatManagerExtension
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:
	// Lists every permission provider currently registered by the runtime.
	UFUNCTION(Exec)
	void Cheat_ListPermissionProviders();

	/**
	 * Enables a forced result for a registered permission provider.
	 *
	 * @param PermissionName The registered permission name to override.
	 * @param bAllow The result forced while the override is active.
	 */
	UFUNCTION(Exec)
	void Cheat_SetPermissionOverride(FString PermissionName, bool bAllow);

	/**
	 * Disables the forced result for a registered permission provider.
	 *
	 * @param PermissionName The registered permission name whose override is cleared.
	 */
	UFUNCTION(Exec)
	void Cheat_ClearPermissionOverride(FString PermissionName);

	/**
	 * Creates an empty permission role or preserves an existing role.
	 *
	 * @param RoleName The role name to create.
	 */
	UFUNCTION(Exec)
	void Cheat_CreatePermissionRole(FString RoleName);

	/**
	 * Deletes a permission role and removes it from all assigned players.
	 *
	 * @param RoleName The role name to delete.
	 */
	UFUNCTION(Exec)
	void Cheat_DeletePermissionRole(FString RoleName);

	/**
	 * Assigns a permission role to the local player's pawn.
	 *
	 * @param RoleName The role name to assign.
	 */
	UFUNCTION(Exec)
	void Cheat_GrantLocalPermissionRole(FString RoleName);

	/**
	 * Removes a permission role from the local player's pawn.
	 *
	 * @param RoleName The role name to remove.
	 */
	UFUNCTION(Exec)
	void Cheat_RevokeLocalPermissionRole(FString RoleName);

	/**
	 * Sets an explicit allow or deny rule on a permission role.
	 *
	 * @param RoleName The role that owns the permission rule.
	 * @param PermissionName The permission name to set.
	 * @param bAllow Whether the role grants the permission.
	 */
	UFUNCTION(Exec)
	void Cheat_SetRolePermission(FString RoleName, FString PermissionName, bool bAllow);

	/**
	 * Evaluates a permission for the local player's pawn and logs its source.
	 *
	 * @param PermissionName The permission name to evaluate.
	 */
	UFUNCTION(Exec)
	void Cheat_EvaluateLocalPermission(FString PermissionName);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
private:
	// Returns the permission subsystem for the local player's game instance.
	UGorgeousPermissionManagerSubsystem* GetPermissionSubsystem() const;

	// Returns the pawn controlled by this extension's local player controller.
	AActor* GetLocalPlayerActor() const;
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only
};