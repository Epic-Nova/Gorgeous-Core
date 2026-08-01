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

static const FName StatStorageEntryKey = TEXT("StatStorage");

UGorgeousStatFoundationStorage_OV* UGorgeousStatFoundation::GetGorgeousStatStorage(UObject* WorldContextObject, bool bRequireNetworking)
{
	if (!WorldContextObject) return nullptr;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	UGorgeousObjectVariable* ExistingVar = nullptr;
	if (UGorgeousCoreRuntimeGlobals::GetNetGorgeousAutoReplicationValue(WorldContextObject, StatStorageEntryKey, ExistingVar))
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

					if (UGorgeousCoreRuntimeGlobals::GetNetGorgeousAutoReplicationValue(WorldContextObject, StatStorageEntryKey, ExistingVar))
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

	APlayerController* PC = World->GetFirstPlayerController();
	if (PC && PC->GetPawn())
	{
		return PC->GetPawn()->FindComponentByClass<UGorgeousStatComponent_AC>();
	}
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

void UGorgeousStatFoundation::SetStatValue(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag, float Value)
{
	APlayerController* PC = WorldContextObject ? WorldContextObject->GetWorld()->GetFirstPlayerController() : nullptr;
	AGorgeousPlayerController* GPC = PC ? Cast<AGorgeousPlayerController>(PC) : nullptr;

	if (UGorgeousStatComponent_AC* Component = Actor ? Actor->FindComponentByClass<UGorgeousStatComponent_AC>() : GetStatComponent(WorldContextObject))
	{
		Component->SetStat(StatTag, Value, GPC);
	}
}

void UGorgeousStatFoundation::ModifyStatValue(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag, float Delta)
{
	APlayerController* PC = WorldContextObject ? WorldContextObject->GetWorld()->GetFirstPlayerController() : nullptr;
	AGorgeousPlayerController* GPC = PC ? Cast<AGorgeousPlayerController>(PC) : nullptr;

	if (UGorgeousStatComponent_AC* Component = Actor ? Actor->FindComponentByClass<UGorgeousStatComponent_AC>() : GetStatComponent(WorldContextObject))
	{
		Component->ModifyStat(StatTag, Delta, GPC);
	}
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
