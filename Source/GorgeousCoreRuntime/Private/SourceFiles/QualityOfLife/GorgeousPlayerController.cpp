#include "HAL/PlatformTime.h"
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
#include "QualityOfLife/GorgeousPlayerController.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "QualityOfLife/GorgeousQualityOfLifeHelperMacros.h"
#include "AutoReplication//GorgeousAutoReplicationHelperMacros.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStorage_OV.h"
#include "GeneralSystems/SignalBridge/GorgeousSignalBridgeCheatManagerExtension.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationCheatManagerExtension.h"
#include "GeneralSystems/InteractionFoundation/GorgeousInteractionFoundationCheatManagerExtension.h"
#include "GeneralSystems/GorgeousPermissionCheatManagerExtension.h"
#include "ObjectVariables/GorgeousRootNetworkStackSubsystem.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Net/UnrealNetwork.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/CheatManager.h"
#include "Framework/Application/SlateApplication.h"
//<-------------------------------------------------------------------------->

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousPlayerController, Log, All);

//=============================================================================
// AGorgeousPlayerController Implementation
//=============================================================================

AGorgeousPlayerController::AGorgeousPlayerController()
	: bActivateNetworkingCapabilities(true)
{
	UE_QOL_INITIALIZE_ADDITIONAL_DATA();
	AutoReplicationRPCRelay = CreateDefaultSubobject<UGorgeousAutoReplicationRPCRelayComponent>(TEXT("AutoReplicationRPCRelay"));
}

UE_QOL_DEFINE_HANDLE_AUTOREPLICATION_RPC(AGorgeousPlayerController)

UE_QOL_DEFINE_BEGIN_PLAY_WITH_RELAY_AND_EXTRA(AGorgeousPlayerController,
    if (HasAuthority() && bActivateNetworkingCapabilities)
    {
        // The public Default channel is joined once for this controller's lifetime.
        // Named channels require explicit membership; no send may undo an unsubscribe.
        if (auto* Stack=UGorgeousRootNetworkStackSubsystem::Get(GetWorld()))
            Stack->SubscribeControllerToChannel(this,UGorgeousRootNetworkStackSubsystem::GetDefaultChannelName());
    }
    if (IsLocalController() && !HasAuthority()) Server_RequestGorgeousPermissionSnapshot();
	if (CheatManager)
	{
		CheatManager->AddCheatManagerExtension(NewObject<UGorgeousStatFoundationCheatManagerExtension>(CheatManager));
		CheatManager->AddCheatManagerExtension(NewObject<UGorgeousSignalBridgeCheatManagerExtension>(CheatManager));
		CheatManager->AddCheatManagerExtension(NewObject<UGorgeousInteractionFoundationCheatManagerExtension>(CheatManager));
		CheatManager->AddCheatManagerExtension(NewObject<UGorgeousPermissionCheatManagerExtension>(CheatManager));
	}
	// temp for debugging so that the mouse pointer gets not stuck (linux issue)
	// Delay input mode setup and reset any early suspension to frame 1.
	// This ensures the Slate viewport is fully initialized and can capture keyboard/mouse focus correctly,
	// and clears any desynchronized ignore-input flags caused by startup widget transitions.
	/*if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (IsValid(this))
			{
				ResetIgnoreInputFlags();
				
				FInputModeGameAndUI InputMode;
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				InputMode.SetHideCursorDuringCapture(false);
				SetInputMode(InputMode);
				bShowMouseCursor = true;
				// Ensure game captures focus without needing a click
				FSlateApplication::Get().SetAllUserFocusToGameViewport(EFocusCause::SetDirectly);
				
				UE_LOG(LogGorgeousPlayerController, Log, TEXT("PlayerController: Next-tick input initialization complete. Viewport focused and input ignore flags reset."));
			}
		}));
	}*/
)

UE_QOL_DEFINE_REGISTER_AUTOREPLICATION_ENTRY(AGorgeousPlayerController)

UE_QOL_DEFINE_POST_INIT_AND_LOAD(AGorgeousPlayerController)

UE_DECLARE_AUTOREPLICATION_CLASS_GET_LIFETIME_REPLICATED_PROPS(AGorgeousPlayerController)

UE_DECLARE_AUTOREPLICATION_CLASS_ON_REP_VARIABLES(AGorgeousPlayerController)

UE_QOL_DEFINE_POST_EDIT_CHANGE_PROPERTY(AGorgeousPlayerController)

UE_QOL_DEFINE_END_PLAY(AGorgeousPlayerController)

// ── IGorgeousPlayerConnectionInfo_I ──────────────────────────────────────

bool AGorgeousPlayerController::IsRemoteNetConnection_Implementation() const
{
	// A PC that has no local ULocalPlayer is a server-side proxy for a remote client.
	return !IsLocalController();
}

FUniqueNetIdRepl AGorgeousPlayerController::GetPlayerNetId_Implementation() const
{
	if (const APlayerState* PS = GetPlayerState<APlayerState>())
	{
		return PS->GetUniqueId();
	}
	return FUniqueNetIdRepl();
}

FString AGorgeousPlayerController::GetGorgeousStablePlayerId_Implementation() const
{
	return FGorgeousQualityOfLifeStatics::GetLocalPlayerStableId(this);
}

int32 AGorgeousPlayerController::GetPlayerConnectionIndex_Implementation() const
{
	if (const ULocalPlayer* LP = GetLocalPlayer())
	{
		if (const UGameInstance* GI = LP->GetGameInstance())
		{
			return GI->GetLocalPlayers().IndexOfByKey(LP);
		}
	}
	return -1;
}

#if WITH_DEV_AUTOMATION_TESTS
void AGorgeousPlayerController::ResetAutomationRPCWitnessEntries()
{
	AutomationRPCWitnessEntries.Reset();
}

void AGorgeousPlayerController::RecordAutomationRPCWitness(const FName HandlerName, const int32 Sequence, const FString& Stamp)
{
	FGorgeousAutomationRPCWitnessEntry& Entry = AutomationRPCWitnessEntries.AddDefaulted_GetRef();
	Entry.HandlerName = HandlerName;
	Entry.Sequence = Sequence;
	Entry.Stamp = Stamp;
	Entry.NetRole = GetLocalRole();
	Entry.NetMode = GetNetMode();
	Entry.TimestampSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
}

#endif

void AGorgeousPlayerController::Automation_ServerToClient(int32 Sequence, const FString& Stamp)
{
	UE_LOG(LogGorgeousPlayerController, Log, TEXT("[AutomationRPC] Server->Client handled on %s seq=%d stamp=%s"), *GetName(), Sequence, *Stamp);
#if WITH_DEV_AUTOMATION_TESTS
	RecordAutomationRPCWitness(TEXT("Automation_ServerToClient"), Sequence, Stamp);
#endif
}

void AGorgeousPlayerController::Automation_ClientToServer(int32 Sequence, const FString& Stamp)
{
	UE_LOG(LogGorgeousPlayerController, Log, TEXT("[AutomationRPC] Client->Server handled on %s seq=%d stamp=%s"), *GetName(), Sequence, *Stamp);
#if WITH_DEV_AUTOMATION_TESTS
	RecordAutomationRPCWitness(TEXT("Automation_ClientToServer"), Sequence, Stamp);
#endif
}

void AGorgeousPlayerController::Automation_Multicast(int32 Sequence, const FString& Stamp)
{
	UE_LOG(LogGorgeousPlayerController, Log, TEXT("[AutomationRPC] Multicast handled on %s seq=%d stamp=%s"), *GetName(), Sequence, *Stamp);
#if WITH_DEV_AUTOMATION_TESTS
	RecordAutomationRPCWitness(TEXT("Automation_Multicast"), Sequence, Stamp);
#endif
}

void AGorgeousPlayerController::Automation_HandleRPC_WithReturnOV(UInteger_SOV* ReturnValue, int32 TestInputInt, const FString& TestInputString, int32 Sequence, const FString& Origin, const FString& Timestamp)
{
	constexpr int32 RPC_TRANSFORM_MULTIPLY = 7;
	constexpr int32 RPC_TRANSFORM_ADD = 42;

	if (ReturnValue)
	{
		ReturnValue->Value = TestInputInt * RPC_TRANSFORM_MULTIPLY + RPC_TRANSFORM_ADD;
	}
	UE_LOG(LogGorgeousPlayerController, Log,
		TEXT("[AutomationRPC] Owner handler on %s: TestInputInt=%d -> ReturnOV.Value=%d seq=%d from=%s"),
		*GetName(), TestInputInt, ReturnValue ? ReturnValue->Value : -1, Sequence, *Origin);
#if WITH_DEV_AUTOMATION_TESTS
	RecordAutomationRPCWitness(TEXT("Automation_HandleRPC_WithReturnOV"), Sequence,
		FString::Printf(TEXT("Int=%d->OV=%d Str=%s From=%s"),
			TestInputInt, ReturnValue ? ReturnValue->Value : -1, *TestInputString, *Origin));
#endif
}

void AGorgeousPlayerController::Client_ReceiveSignal_Implementation(FGameplayTag Tag, const FInstancedStruct& Payload)
{
	if (USignalBridgeStorage_OV* Storage = USignalBridgeBlueprintFunctionLibrary::GetSignalBridgeStorage(this))
	{
		Storage->FireLocalSignal(Tag, Payload);
	}
}

void AGorgeousPlayerController::Client_ReceiveGorgeousPermissionSnapshot_Implementation(const FGorgeousOwnerPermissionSnapshot& Snapshot)
{
    if (UGameInstance* Instance=GetGameInstance())
        if (auto* Manager=Instance->GetSubsystem<UGorgeousPermissionManagerSubsystem>()) Manager->ApplyOwnerSnapshot(Snapshot,this);
}
void AGorgeousPlayerController::Server_RequestGorgeousPermissionSnapshot_Implementation()
{
    const double Now=FPlatformTime::Seconds(); if (Now-LastPermissionSnapshotRequestTime<0.25) return;
    LastPermissionSnapshotRequestTime=Now;
    if (UGameInstance* Instance=GetGameInstance())
        if (auto* Manager=Instance->GetSubsystem<UGorgeousPermissionManagerSubsystem>()) Client_ReceiveGorgeousPermissionSnapshot(Manager->BuildOwnerSnapshot(this));
}
bool AGorgeousPlayerController::CanAdministerGorgeousPermissions_Implementation(APlayerState* TargetPlayer) const { return false; }
void AGorgeousPlayerController::Client_GorgeousPermissionMutationResult_Implementation(bool bAccepted)
{ bLastPermissionMutationAccepted=bAccepted; }
void AGorgeousPlayerController::Server_RequestGorgeousPermissionMutation_Implementation(APlayerState* TargetPlayer,EGorgeousPermissionMutation Operation,const FString& Key,bool bAllow)
{
    const double Now=FPlatformTime::Seconds();
    if (Now-LastPermissionMutationRequestTime<0.05) { Client_GorgeousPermissionMutationResult(false); return; }
    LastPermissionMutationRequestTime=Now;
    // Requester identity is this server-side owning controller, never a client argument.
    if (!TargetPlayer || TargetPlayer->GetWorld()!=GetWorld() || Key.IsEmpty() || Key.Len()>256 || !CanAdministerGorgeousPermissions(TargetPlayer))
    { Client_GorgeousPermissionMutationResult(false); return; }
    auto* Manager=GetGameInstance() ? GetGameInstance()->GetSubsystem<UGorgeousPermissionManagerSubsystem>() : nullptr;
    if (!Manager) { Client_GorgeousPermissionMutationResult(false); return; }
    switch(Operation)
    {
        case EGorgeousPermissionMutation::GrantRole: Manager->GrantRole(TargetPlayer,FName(*Key)); break;
        case EGorgeousPermissionMutation::RevokeRole: Manager->RevokeRole(TargetPlayer,FName(*Key)); break;
        case EGorgeousPermissionMutation::GrantPermission: Manager->GrantPermission(TargetPlayer,Key,bAllow); break;
        case EGorgeousPermissionMutation::RevokePermission: Manager->RevokePermission(TargetPlayer,Key); break;
        default: Client_GorgeousPermissionMutationResult(false); return;
    }
    Client_GorgeousPermissionMutationResult(true);
}

void AGorgeousPlayerController::SetVisualDataGatheringConsent(bool bAccepted)
{
    if (!HasAuthority() && !IsLocalController()) return;
    bVDGConsentDecisionMade=true;
    bVDGConsentGranted=bAccepted;
    if (!HasAuthority()) Server_SetVisualDataGatheringConsent(bAccepted);
}
void AGorgeousPlayerController::Server_SetVisualDataGatheringConsent_Implementation(bool bAccepted)
{
    // The actor's owning connection supplies the decision; no player identity is accepted.
    bVDGConsentDecisionMade=true;
    bVDGConsentGranted=bAccepted;
}
void AGorgeousPlayerController::ResetVisualDataGatheringConsent()
{
    SetVisualDataGatheringConsent(false);
    bVDGConsentDecisionMade=false;
}
