// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundation.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatComponent_AC.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationStorage_OV.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "QualityOfLife/GorgeousWorldSettings.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

#include "GameFramework/PlayerState.h"
#include "GameFramework/CheatManager.h"
#include "Components/ActorComponent.h"
static AGorgeousPlayerController* ResolveStatRequester(UObject* Context)
{
    if (!Context) return nullptr;
    if (auto* PC=Cast<AGorgeousPlayerController>(Context)) return PC;
    if (auto* Pawn=Cast<APawn>(Context)) return Cast<AGorgeousPlayerController>(Pawn->GetController());
    if (auto* PS=Cast<APlayerState>(Context)) return Cast<AGorgeousPlayerController>(PS->GetOwningController());
    if (auto* Component=Cast<UActorComponent>(Context)) return ResolveStatRequester(Component->GetOwner());
    if (auto* Manager=Cast<UCheatManager>(Context)) return Cast<AGorgeousPlayerController>(Manager->GetOuterAPlayerController());
    return Context->GetTypedOuter<AGorgeousPlayerController>();
}
static const FName StatStorageEntryKey = TEXT("StatStorage");

UGorgeousStatFoundationStorage_OV* UGorgeousStatFoundation::GetGorgeousStatStorage(UObject* WorldContextObject, bool bRequireNetworking)
{
	if (!WorldContextObject) return nullptr;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	UGorgeousObjectVariable* ExistingVar = nullptr;
	if (UGorgeousCoreRuntimeGlobals::GetNetGorgeousAutoReplicationValue(World->GetWorldSettings(), StatStorageEntryKey, ExistingVar))
	{
		if (UGorgeousStatFoundationStorage_OV* Storage = Cast<UGorgeousStatFoundationStorage_OV>(ExistingVar))
		{
			return Storage;
		}
	}

	if (bRequireNetworking && World->GetNetMode() != NM_Client)
	{
		AGorgeousWorldSettings* WorldSettings = Cast<AGorgeousWorldSettings>(World->GetWorldSettings());
		if (WorldSettings)
		{
			if (!WorldSettings->AdditionalGorgeousData.Contains(StatStorageEntryKey))
			{
				if (WorldSettings->RegisterAutoReplicationEntry(StatStorageEntryKey, UGorgeousStatFoundationStorage_OV::StaticClass(), true, false, FGorgeousAutoReplicationStreamConfig()))
				{
					UGorgeousCoreRuntimeGlobals::RefreshQualityOfLifeReplication(WorldContextObject, AGorgeousWorldSettings::StaticClass());

					if (UGorgeousCoreRuntimeGlobals::GetNetGorgeousAutoReplicationValue(World->GetWorldSettings(), StatStorageEntryKey, ExistingVar))
					{
						return Cast<UGorgeousStatFoundationStorage_OV>(ExistingVar);
					}
				}
			}
		}
	}

	return nullptr;
}

UGorgeousStatComponent_AC* UGorgeousStatFoundation::GetStatComponent(UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

    if (auto* Component=Cast<UGorgeousStatComponent_AC>(WorldContextObject)) return Component;
    if (auto* Actor=Cast<AActor>(WorldContextObject))
        if (auto* Component=Actor->FindComponentByClass<UGorgeousStatComponent_AC>()) return Component;
    if (auto* PC=ResolveStatRequester(WorldContextObject))
        if (PC->GetPawn()) return PC->GetPawn()->FindComponentByClass<UGorgeousStatComponent_AC>();
	return nullptr;
}

float UGorgeousStatFoundation::GetStatValue(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag)
{
	if (UGorgeousStatComponent_AC* Component = Actor ? Actor->FindComponentByClass<UGorgeousStatComponent_AC>() : GetStatComponent(WorldContextObject))
	{
		return Component->GetStat(StatTag);
	}
	return 0.0f;
}

bool UGorgeousStatFoundation::SetStatValue(UObject* Context,AActor* Actor,FGameplayTag Tag,float Value)
{
    if (auto* Component=Actor ? Actor->FindComponentByClass<UGorgeousStatComponent_AC>() : GetStatComponent(Context)) return Component->SetStat(Tag,Value,ResolveStatRequester(Context));
    return false;
}
bool UGorgeousStatFoundation::ModifyStatValue(UObject* Context,AActor* Actor,FGameplayTag Tag,float Delta)
{
    if (auto* Component=Actor ? Actor->FindComponentByClass<UGorgeousStatComponent_AC>() : GetStatComponent(Context)) return Component->ModifyStat(Tag,Delta,ResolveStatRequester(Context));
    return false;
}

void UGorgeousStatFoundation::AddAllowedController(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag, AGorgeousPlayerController* Controller)
{
	if (UGorgeousStatFoundationStorage_OV* Storage = GetGorgeousStatStorage(WorldContextObject))
	{
		Storage->AddAllowedController(Actor, StatTag, Controller);
	}
}

void UGorgeousStatFoundation::RemoveAllowedController(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag, AGorgeousPlayerController* Controller)
{
	if (UGorgeousStatFoundationStorage_OV* Storage = GetGorgeousStatStorage(WorldContextObject))
	{
		Storage->RemoveAllowedController(Actor, StatTag, Controller);
	}
}
