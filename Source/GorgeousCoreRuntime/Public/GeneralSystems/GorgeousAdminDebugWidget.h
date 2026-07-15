// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InstancedStruct.h"
#include "GorgeousAdminDebugWidget.generated.h"

/**
 * Base class for all Gorgeous runtime admin and debug widgets.
 * Integrates directly with the Gorgeous Permission System to check
 * access rights for the local player.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class GORGEOUSCORERUNTIME_API UGorgeousAdminDebugWidget : public UUserWidget
{
	GENERATED_BODY()

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
	virtual void ExecuteAdminAction_Implementation(FName ActionName, const FInstancedStruct& Payload);
};
