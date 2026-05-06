// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/StatsFoundation/GorgeousStatBlueprintFunctionLibrary.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatComponent_AC.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatStorage_OV.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "QualityOfLife/GorgeousWorldSettings.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

static const FName StatStorageEntryKey = TEXT("StatStorage");

UGorgeousStatStorage_OV* UGorgeousStatBlueprintFunctionLibrary::GetGorgeousStatStorage(UObject* WorldContextObject, bool bRequireNetworking)
{
	if (!WorldContextObject) return nullptr;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	UGorgeousObjectVariable* ExistingVar = nullptr;
	if (UGorgeousCoreRuntimeGlobals::GetNetGorgeousAutoReplicationValue(WorldContextObject, StatStorageEntryKey, ExistingVar))
	{
		if (UGorgeousStatStorage_OV* Storage = Cast<UGorgeousStatStorage_OV>(ExistingVar))
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
				if (WorldSettings->RegisterAutoReplicationEntry(StatStorageEntryKey, UGorgeousStatStorage_OV::StaticClass(), true, false, FGorgeousAutoReplicationStreamConfig()))
				{
					UGorgeousCoreRuntimeGlobals::RefreshQualityOfLifeReplication(WorldContextObject, AGorgeousWorldSettings::StaticClass());

					if (UGorgeousCoreRuntimeGlobals::GetNetGorgeousAutoReplicationValue(WorldContextObject, StatStorageEntryKey, ExistingVar))
					{
						return Cast<UGorgeousStatStorage_OV>(ExistingVar);
					}
				}
			}
		}
	}

	return nullptr;
}

UGorgeousStatComponent_AC* UGorgeousStatBlueprintFunctionLibrary::GetStatComponent(UObject* WorldContextObject)
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

float UGorgeousStatBlueprintFunctionLibrary::GetStatValue(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag)
{
	if (UGorgeousStatComponent_AC* Component = Actor ? Actor->FindComponentByClass<UGorgeousStatComponent_AC>() : GetStatComponent(WorldContextObject))
	{
		return Component->GetStat(StatTag);
	}
	return 0.0f;
}

void UGorgeousStatBlueprintFunctionLibrary::SetStatValue(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag, float Value)
{
	APlayerController* PC = WorldContextObject ? WorldContextObject->GetWorld()->GetFirstPlayerController() : nullptr;
	AGorgeousPlayerController* GPC = PC ? Cast<AGorgeousPlayerController>(PC) : nullptr;

	if (UGorgeousStatComponent_AC* Component = Actor ? Actor->FindComponentByClass<UGorgeousStatComponent_AC>() : GetStatComponent(WorldContextObject))
	{
		Component->SetStat(StatTag, Value, GPC);
	}
}

void UGorgeousStatBlueprintFunctionLibrary::ModifyStatValue(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag, float Delta)
{
	APlayerController* PC = WorldContextObject ? WorldContextObject->GetWorld()->GetFirstPlayerController() : nullptr;
	AGorgeousPlayerController* GPC = PC ? Cast<AGorgeousPlayerController>(PC) : nullptr;

	if (UGorgeousStatComponent_AC* Component = Actor ? Actor->FindComponentByClass<UGorgeousStatComponent_AC>() : GetStatComponent(WorldContextObject))
	{
		Component->ModifyStat(StatTag, Delta, GPC);
	}
}

void UGorgeousStatBlueprintFunctionLibrary::AddAllowedController(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag, AGorgeousPlayerController* Controller)
{
	if (UGorgeousStatStorage_OV* Storage = GetGorgeousStatStorage(WorldContextObject))
	{
		Storage->AddAllowedController(Actor, StatTag, Controller);
	}
}

void UGorgeousStatBlueprintFunctionLibrary::RemoveAllowedController(UObject* WorldContextObject, AActor* Actor, FGameplayTag StatTag, AGorgeousPlayerController* Controller)
{
	if (UGorgeousStatStorage_OV* Storage = GetGorgeousStatStorage(WorldContextObject))
	{
		Storage->RemoveAllowedController(Actor, StatTag, Controller);
	}
}
