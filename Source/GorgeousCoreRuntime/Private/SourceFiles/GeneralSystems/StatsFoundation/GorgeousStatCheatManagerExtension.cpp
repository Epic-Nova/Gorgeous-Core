// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/StatsFoundation/GorgeousStatCheatManagerExtension.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatBlueprintFunctionLibrary.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatStorage_OV.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatSettings.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "GameplayTagContainer.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"

void UGorgeousStatCheatManagerExtension::Cheat_SetStat(FString StatTag, float Value)
{
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(*StatTag, false);
	if (Tag.IsValid())
	{
		UGorgeousStatBlueprintFunctionLibrary::SetStatValue(this, (AActor*)GetPlayerController()->GetPawn(), Tag, Value);
		GT_S_LOG(TEXT("Cheat"), TEXT("Set stat %s to %f"), *StatTag, Value);
	}
	else
	{
		GT_E_LOG(TEXT("Cheat"), TEXT("Invalid stat tag %s"), *StatTag);
	}
}

void UGorgeousStatCheatManagerExtension::Cheat_ModifyStat(FString StatTag, float Delta)
{
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(*StatTag, false);
	if (Tag.IsValid())
	{
		UGorgeousStatBlueprintFunctionLibrary::ModifyStatValue(this, (AActor*)GetPlayerController()->GetPawn(), Tag, Delta);
		GT_S_LOG(TEXT("Cheat"), TEXT("Modified stat %s by %f"), *StatTag, Delta);
	}
	else
	{
		GT_E_LOG(TEXT("Cheat"), TEXT("Invalid stat tag %s"), *StatTag);
	}
}

void UGorgeousStatCheatManagerExtension::Cheat_ListStats()
{
	UGorgeousStatStorage_OV* Storage = UGorgeousStatBlueprintFunctionLibrary::GetGorgeousStatStorage(this);
	if (!Storage)
	{
		GT_W_LOG(TEXT("Cheat"), TEXT("Stat Storage not found."));
		return;
	}

	AActor* Pawn = (AActor*)GetPlayerController()->GetPawn();
	if (!Pawn)
	{
		GT_W_LOG(TEXT("Cheat"), TEXT("No pawn found for player."));
		return;
	}

	GT_I_LOG(TEXT("Cheat"), TEXT("--- Gorgeous Stats for %s ---"), *Pawn->GetName());
	
	const UGorgeousStatSettings* Settings = GetDefault<UGorgeousStatSettings>();
	for (const auto& Pair : Settings->StatRegistry)
	{
		float Value = Storage->GetStatValue(Pawn, Pair.Key);
		GT_I_LOG(TEXT("Cheat"), TEXT("  [%s]: %f"), *Pair.Key.ToString(), Value);
	}
}

void UGorgeousStatCheatManagerExtension::Cheat_AddAllowedController(FString StatTag, FString ControllerClassName)
{
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(*StatTag, false);
	if (!Tag.IsValid())
	{
		GT_E_LOG(TEXT("Cheat"), TEXT("Invalid stat tag %s"), *StatTag);
		return;
	}

	UClass* ControllerClass = FindObject<UClass>(nullptr, *ControllerClassName);
	if (!ControllerClass || !ControllerClass->IsChildOf(AGorgeousPlayerController::StaticClass()))
	{
		GT_E_LOG(TEXT("Cheat"), TEXT("Invalid controller class %s. Must be a GorgeousPlayerController."), *ControllerClassName);
		return;
	}

	UGorgeousStatBlueprintFunctionLibrary::AddAllowedController(this, (AActor*)GetPlayerController()->GetPawn(), Tag, Cast<AGorgeousPlayerController>(GetPlayerController()));
	GT_S_LOG(TEXT("Cheat"), TEXT("Added local controller to whitelist for %s"), *StatTag);
}
