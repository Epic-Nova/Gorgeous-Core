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

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Blueprint/UserWidget.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "InstancedStruct.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousAdminDebugWidget.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Admin Debug Widget
| Functional Name: UGorgeousAdminDebugWidget
| Parent Class: UUserWidget
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Base class for all Gorgeous runtime admin and debug widgets. Integrates
| directly with the Gorgeous Permission System to check access rights for
| the local player.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Abstract, BlueprintType, Blueprintable,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/GorgeousAdminDebugWidget",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousAdminDebugWidget : public UUserWidget
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Checks if the local player (the one owning this widget) has the specified permission.
	 *
	 * @param PermissionKey The string identifier for the permission (e.g., "com.gorgeous.admin.godmode").
	 * @param TargetActor Optional target actor the permission might apply to.
	 * @return True if the local player is granted the permission.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Admin")
	bool HasPermission(const FString& PermissionKey, AActor* TargetActor = nullptr) const;

	/**
	 * Executes an abstract admin action with an arbitrary payload.
	 * This can be implemented in Blueprint to handle specific system admin tasks.
	 *
	 * @param ActionName The name of the action to execute.
	 * @param Payload The arbitrary data required for this action.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous|Admin")
	void ExecuteAdminAction(FName ActionName, const FInstancedStruct& Payload);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	/**
	 * Implements the abstract admin action event when no Blueprint implementation is supplied.
	 *
	 * @param ActionName The action to execute.
	 * @param Payload The payload supplied for the action.
	 */
	virtual void ExecuteAdminAction_Implementation(FName ActionName, const FInstancedStruct& Payload);
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides
};