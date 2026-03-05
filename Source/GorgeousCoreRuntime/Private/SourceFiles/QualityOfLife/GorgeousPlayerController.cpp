// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#include "QualityOfLife/GorgeousPlayerController.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "QualityOfLife/GorgeousQualityOfLifeHelperMacros.h"
#include "AutoReplication//GorgeousAutoReplicationHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Net/UnrealNetwork.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerState.h"
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

UE_QOL_DEFINE_BEGIN_PLAY_WITH_RELAY(AGorgeousPlayerController)

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




