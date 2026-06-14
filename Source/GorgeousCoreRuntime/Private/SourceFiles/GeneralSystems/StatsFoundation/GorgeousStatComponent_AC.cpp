// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/StatsFoundation/GorgeousStatComponent_AC.h"
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

UGorgeousStatComponent_AC::UGorgeousStatComponent_AC()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UGorgeousStatComponent_AC::BeginPlay()
{
	Super::BeginPlay();

	InitializeStatStorage();
	
	if (GetOwner()->HasAuthority())
	{
		RegisterSignalListeners();
	}
}

void UGorgeousStatComponent_AC::InitializeStatStorage()
{
	if (StatStorage) return;

	UWorld* World = GetWorld();
	AGorgeousGameState* GS = World ? World->GetGameState<AGorgeousGameState>() : nullptr;
	if (!GS) return;

	static const FName StatStorageKey = TEXT("StatStorage");

	// On the server, ensure the global storage exists.
	if (GetOwner()->HasAuthority())
	{
		GS->RegisterAutoReplicationEntry(StatStorageKey, UGorgeousStatFoundationStorage_OV::StaticClass(), true, false, FGorgeousAutoReplicationStreamConfig());
		UGorgeousCoreRuntimeGlobals::RefreshQualityOfLifeReplication(World, AGorgeousGameState::StaticClass());
	}

	// Resolve the global storage
	UGorgeousObjectVariable* ExistingVar = nullptr;
	if (UGorgeousCoreRuntimeGlobals::GetNetGorgeousAutoReplicationValue(World, StatStorageKey, ExistingVar))
	{
		StatStorage = Cast<UGorgeousStatFoundationStorage_OV>(ExistingVar);
		if (StatStorage)
		{
			StatStorage->OnStatChanged.AddDynamic(this, &UGorgeousStatComponent_AC::HandleStatChanged);
			
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
	DOREPLIFETIME(UGorgeousStatComponent_AC, StatStorage);
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

void UGorgeousStatComponent_AC::SetStat(FGameplayTag Tag, float Value, AGorgeousPlayerController* Requester)
{
	if (StatStorage)
	{
		StatStorage->SetStatValue(GetOwner(), Tag, Value, Requester);
	}
}

void UGorgeousStatComponent_AC::ModifyStat(FGameplayTag Tag, float Delta, AGorgeousPlayerController* Requester)
{
	if (StatStorage)
	{
		float CurrentValue = StatStorage->GetStatValue(GetOwner(), Tag);
		StatStorage->SetStatValue(GetOwner(), Tag, CurrentValue + Delta, Requester);
	}
}

float UGorgeousStatComponent_AC::GetStat(FGameplayTag Tag) const
{
	return StatStorage ? StatStorage->GetStatValue(GetOwner(), Tag) : 0.0f;
}

void UGorgeousStatComponent_AC::HandleStatChanged(FGameplayTag Tag, float NewValue)
{
	const UGorgeousStatFoundationSettings* Settings = GetDefault<UGorgeousStatFoundationSettings>();
	if (Settings->bAutoDispatchChangeSignals)
	{
		FGameplayTag ChangeSignal = FGameplayTag::RequestGameplayTag(
			FName(*FString::Printf(TEXT("%s.%s"), *Settings->ChangeSignalRoot.ToString(), *Tag.ToString()))
		);

		FGorgeousStatModificationPayload_S Payload;
		Payload.Value = NewValue;
		Payload.Source = GetOwner();

		// Use local dispatch as the state itself is already replicated
		USignalBridgeBlueprintFunctionLibrary::DispatchLocal(this, ChangeSignal, FInstancedStruct::Make(Payload));
	}
}

void UGorgeousStatComponent_AC::OnSignalReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	if (!GetOwner()->HasAuthority()) return;

	// Find which stat this signal belongs to
	const UGorgeousStatFoundationSettings* Settings = GetDefault<UGorgeousStatFoundationSettings>();
	for (const auto& Pair : Settings->StatRegistry)
	{
		if (Pair.Value.ModificationSignal == SignalTag)
		{
			if (const FGorgeousStatModificationPayload_S* StatPayload = Payload.GetPtr<FGorgeousStatModificationPayload_S>())
			{
				if (Pair.Value.bSignalIsDelta)
				{
					ModifyStat(Pair.Key, StatPayload->Value);
				}
				else
				{
					SetStat(Pair.Key, StatPayload->Value);
				}
			}
			break;
		}
	}
}
