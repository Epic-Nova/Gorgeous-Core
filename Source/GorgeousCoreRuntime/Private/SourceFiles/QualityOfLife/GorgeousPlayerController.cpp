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
#include "AutoReplication/Helpers/GorgeousAutoReplicationHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Net/UnrealNetwork.h"
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




