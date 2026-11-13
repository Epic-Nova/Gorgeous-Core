#include "QualityOfLife/GorgeousPlayerController.h"
#include "GeneralSystems/GorgeousPermissionManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "GeneralSystems/GorgeousAdminDebugWidget.h"
#include "GeneralSystems/GorgeousPermissionProvider.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

bool UGorgeousAdminDebugWidget::HasPermission(const FString& PermissionKey, AActor* TargetActor) const
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return false;
	}

	APawn* LocalPawn = PC->GetPawn();
	AActor* Interactor = LocalPawn ? Cast<AActor>(LocalPawn) : Cast<AActor>(PC);

    if (PC->GetWorld() && PC->GetWorld()->GetNetMode()==NM_Client)
    {
        auto* Manager=PC->GetGameInstance() ? PC->GetGameInstance()->GetSubsystem<UGorgeousPermissionManagerSubsystem>() : nullptr;
        bool Handled=false;
        return Manager && Manager->EvaluatePlayerPermission(Interactor,PermissionKey,Handled) && Handled;
    }
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

void UGorgeousAdminDebugWidget::RequestPermissionChange(APlayerState* TargetPlayer,EGorgeousPermissionMutation Operation,const FString& Key,bool bAllow)
{
    if (auto* Controller=Cast<AGorgeousPlayerController>(GetOwningPlayer())) Controller->Server_RequestGorgeousPermissionMutation(TargetPlayer,Operation,Key,bAllow);
}
