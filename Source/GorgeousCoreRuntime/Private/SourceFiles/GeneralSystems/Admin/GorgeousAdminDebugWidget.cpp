// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "GeneralSystems/Admin/GorgeousAdminDebugWidget.h"
#include "GeneralSystems/GorgeousPermissionProvider.h"
#include "GameFramework/PlayerController.h"

bool UGorgeousAdminDebugWidget::HasPermission(const FString& PermissionKey, AActor* TargetActor) const
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return false;
	}

	APawn* LocalPawn = PC->GetPawn();
	AActor* Interactor = LocalPawn ? Cast<AActor>(LocalPawn) : Cast<AActor>(PC);

	if (UGorgeousPermissionProvider* Provider = UGorgeousPermissionProvider::FindProvider(PermissionKey))
	{
		return Provider->EvaluatePermission(Interactor, TargetActor);
	}

	return false;
}

void UGorgeousAdminDebugWidget::ExecuteAdminAction_Implementation(FName ActionName, const FInstancedStruct& Payload)
{
	// Base implementation does nothing. Intended to be overridden in Blueprints.
}
