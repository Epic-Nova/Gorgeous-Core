// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/StatsFoundation/GorgeousStatComponent_AC.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundation.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationStorage_OV.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationSettings.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "QualityOfLife/GorgeousGameState.h"
#include "QualityOfLife/GorgeousPlayerState.h"
#include "QualityOfLife/GorgeousPlayerConnectionInfo_I.h"
#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
#include "Stats/Stats.h"

DECLARE_STATS_GROUP(TEXT("Gorgeous Stats Foundation"), STATGROUP_GorgeousStatsFoundation, STATCAT_Advanced);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Active Components"), STAT_GStats_ActiveComps, STATGROUP_GorgeousStatsFoundation);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Modifiers Applied"), STAT_GStats_ModifiersApplied, STATGROUP_GorgeousStatsFoundation);
DECLARE_CYCLE_STAT(TEXT("Process Stat Signal"), STAT_GStats_ProcessSignal, STATGROUP_GorgeousStatsFoundation);
static int32 GStatComponentActiveCount = 0;
static int32 GStatComponentTotalModifiersApplied = 0;

int32 UGorgeousStatComponent_AC::GetTotalActiveComponents()
{
	return GStatComponentActiveCount;
}

int32 UGorgeousStatComponent_AC::GetTotalModifiersApplied()
{
	return GStatComponentTotalModifiersApplied;
}

UGorgeousStatComponent_AC::UGorgeousStatComponent_AC()
{
	PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}

void UGorgeousStatComponent_AC::OnRegister()
{
	Super::OnRegister();
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		GStatComponentActiveCount++;
		INC_DWORD_STAT(STAT_GStats_ActiveComps);
	}
}

void UGorgeousStatComponent_AC::OnUnregister()
{
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		GStatComponentActiveCount--;
		DEC_DWORD_STAT(STAT_GStats_ActiveComps);
	}
	Super::OnUnregister();
}

void UGorgeousStatComponent_AC::BeginPlay()
{
	Super::BeginPlay();

	InitializeStatStorage();
    SetComponentTickEnabled(!StatStorage);
	
	if (GetOwner()->HasAuthority())
	{
		RegisterSignalListeners();
	}
}

void UGorgeousStatComponent_AC::InitializeStatStorage()
{
	if (StatStorage) return;

    StatStorage = UGorgeousStatFoundation::GetGorgeousStatStorage(this);
    {

		if (StatStorage)
		{
			StatStorage->OnActorStatChanged.AddDynamic(this, &UGorgeousStatComponent_AC::HandleActorStatChanged);
			
			// Dispatch a "Restored" signal locally so the HUD knows to do a full refresh.
			// This is especially important after respawn or re-connecting to a shared OV.
			const UGorgeousStatFoundationSettings* Settings = GetDefault<UGorgeousStatFoundationSettings>();
			USignalBridgeBlueprintFunctionLibrary::DispatchLocal(this, Settings->RestoredSignal, FInstancedStruct());
		}
	}
}

void UGorgeousStatComponent_AC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // Storage identity is resolved locally from the replicated WorldSettings payload slot.
}

void UGorgeousStatComponent_AC::RegisterSignalListeners()
{
	const UGorgeousStatFoundationSettings* Settings = GetDefault<UGorgeousStatFoundationSettings>();
	for (const auto& Pair : Settings->StatRegistry)
	{
		if (Pair.Value.ModificationSignal.IsValid())
		{
			// Listen to signals directed at this actor or global signals
			// For simplicity in this foundation, we'll assume the Signal Bridge is used.
				FSignalBridgeEventDelegate Delegate;
				Delegate.BindUFunction(this, TEXT("OnSignalReceived"));
				USignalBridgeBlueprintFunctionLibrary::Listen(
					this, 
					Pair.Value.ModificationSignal, 
					nullptr, 
					Delegate
				);
		}
	}
}

bool UGorgeousStatComponent_AC::SetStat(FGameplayTag Tag,float Value,AGorgeousPlayerController* Requester)
{
    InitializeStatStorage();
    return StatStorage && StatStorage->SetStatValue(GetOwner(),Tag,Value,Requester);
}
bool UGorgeousStatComponent_AC::ModifyStat(FGameplayTag Tag,float Delta,AGorgeousPlayerController* Requester)
{
    InitializeStatStorage();
    return StatStorage && StatStorage->SetStatValue(GetOwner(),Tag,StatStorage->GetStatValue(GetOwner(),Tag)+Delta,Requester);
}
void UGorgeousStatComponent_AC::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* Function)
{
    Super::TickComponent(DeltaTime,TickType,Function);
    InitializeStatStorage();
    if (StatStorage) SetComponentTickEnabled(false);
}
void UGorgeousStatComponent_AC::EndPlay(const EEndPlayReason::Type Reason)
{
    if (StatStorage) StatStorage->OnActorStatChanged.RemoveDynamic(this,&UGorgeousStatComponent_AC::HandleActorStatChanged);
    FSignalBridgeEventDelegate Delegate; Delegate.BindUFunction(this,TEXT("OnSignalReceived"));
    for (const auto& Pair:GetDefault<UGorgeousStatFoundationSettings>()->StatRegistry)
        if (Pair.Value.ModificationSignal.IsValid()) USignalBridgeBlueprintFunctionLibrary::Unlisten(this,Pair.Value.ModificationSignal,nullptr,Delegate);
    StatStorage=nullptr;
    Super::EndPlay(Reason);
}

float UGorgeousStatComponent_AC::GetStat(FGameplayTag Tag) const
{
	return StatStorage ? StatStorage->GetStatValue(GetOwner(), Tag) : 0.0f;
}

void UGorgeousStatComponent_AC::HandleActorStatChanged(AActor* Actor, FGameplayTag Tag, float NewValue)
{
    if (Actor == GetOwner()) HandleStatChanged(Tag, NewValue);
}

void UGorgeousStatComponent_AC::HandleStatChanged(FGameplayTag Tag, float NewValue)
{
	const UGorgeousStatFoundationSettings* Settings = GetDefault<UGorgeousStatFoundationSettings>();
	if (Settings->bAutoDispatchChangeSignals)
	{
        // Project stat tags cannot imply that every combined change tag exists.
        // A registered specialized tag retains compatibility; otherwise dispatch
        // the configured parent with the actual stat identity in the payload.
        FGameplayTag ChangeSignal = FGameplayTag::RequestGameplayTag(
            FName(*FString::Printf(TEXT("%s.%s"), *Settings->ChangeSignalRoot.ToString(), *Tag.ToString())), false);
        if (!ChangeSignal.IsValid()) ChangeSignal=Settings->ChangeSignalRoot;
        if (!ChangeSignal.IsValid()) return;

		FGorgeousStatModificationPayload_S Payload;
		Payload.Value = NewValue;
		Payload.Source = GetOwner();
        Payload.Target = GetOwner();
        Payload.Stat = Tag;

		// Use local dispatch as the state itself is already replicated
		USignalBridgeBlueprintFunctionLibrary::DispatchLocal(this, ChangeSignal, FInstancedStruct::Make(Payload));
	}
}

void UGorgeousStatComponent_AC::OnSignalReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	SCOPE_CYCLE_COUNTER(STAT_GStats_ProcessSignal);
	if (!GetOwner()->HasAuthority()) return;

	// Find which stat this signal belongs to
	const UGorgeousStatFoundationSettings* Settings = GetDefault<UGorgeousStatFoundationSettings>();
	for (const auto& Pair : Settings->StatRegistry)
	{
		if (Pair.Value.ModificationSignal == SignalTag)
		{
			if (const FGorgeousStatModificationPayload_S* StatPayload = Payload.GetPtr<FGorgeousStatModificationPayload_S>())
			{
                if (StatPayload->Target != GetOwner() || (StatPayload->Stat.IsValid() && StatPayload->Stat != Pair.Key)) continue;
				if (Pair.Value.bSignalIsDelta)
				{
					ModifyStat(Pair.Key, StatPayload->Value);
					GStatComponentTotalModifiersApplied++;
					INC_DWORD_STAT(STAT_GStats_ModifiersApplied);
				}
				else
				{
					SetStat(Pair.Key, StatPayload->Value);
					GStatComponentTotalModifiersApplied++;
					INC_DWORD_STAT(STAT_GStats_ModifiersApplied);
				}
			}
		}
	}
}
