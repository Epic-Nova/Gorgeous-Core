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

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GeneralSystems/SignalBridge/GorgeousSignalBridgeCheatManagerExtension.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStorage_OV.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "QualityOfLife/GorgeousPlayerController.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "GameplayTagContainer.h"
//<-------------------------------------------------------------------------->

void UGorgeousSignalBridgeCheatManagerExtension::Cheat_ListSignalBridge()
{
	USignalBridgeStorage_OV* Storage = USignalBridgeBlueprintFunctionLibrary::GetSignalBridgeStorage(this, false);
	if (!Storage)
	{
		GT_W_LOG("GT.SignalBridge", TEXT("Signal Bridge storage is unavailable."));
		return;
	}

	GT_I_LOG("GT.SignalBridge", TEXT("--- Gorgeous Signal Bridge ---"));
	GT_I_LOG("GT.SignalBridge", TEXT("Active listener tags: %d"), Storage->GetTotalActiveListeners());
	GT_I_LOG("GT.SignalBridge", TEXT("Local signals fired: %lld"), USignalBridgeStorage_OV::GetTotalLocalSignalsFired());
	GT_I_LOG("GT.SignalBridge", TEXT("Signals without listeners: %lld"), USignalBridgeStorage_OV::GetTotalNoListenersFound());
}

void UGorgeousSignalBridgeCheatManagerExtension::Cheat_RegisterPublicSignal(FString SignalTag)
{
	const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(*SignalTag, false);
	if (!Tag.IsValid())
	{
		GT_E_LOG("GT.SignalBridge", TEXT("Invalid signal tag: %s"), *SignalTag);
		return;
	}

	USignalBridgeStorage_OV* Storage = USignalBridgeBlueprintFunctionLibrary::GetSignalBridgeStorage(this, false);
	if (!Storage)
	{
		GT_W_LOG("GT.SignalBridge", TEXT("Signal Bridge storage is unavailable."));
		return;
	}

	if (!Storage->HasAuthority())
	{
		GT_W_LOG("GT.SignalBridge", TEXT("Register public signal must be executed on the server."));
		return;
	}

	FGorgeousSignalBridgeAccessRules_S Rules;
	Rules.bNetworked = true;
	Rules.AccessPolicy = EGorgeousObjectVariableAccessPolicy::Everyone;
	Storage->RegisterSignal(Tag, Rules, Cast<AGorgeousPlayerController>(GetPlayerController()));
	GT_S_LOG("GT.SignalBridge", TEXT("Registered public Signal Bridge rule for %s."), *Tag.ToString());
}

void UGorgeousSignalBridgeCheatManagerExtension::Cheat_DispatchLocalSignal(FString SignalTag)
{
	const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(*SignalTag, false);
	if (!Tag.IsValid())
	{
		GT_E_LOG("GT.SignalBridge", TEXT("Invalid signal tag: %s"), *SignalTag);
		return;
	}

	if (USignalBridgeStorage_OV* Storage = USignalBridgeBlueprintFunctionLibrary::GetSignalBridgeStorage(this, false))
	{
		Storage->FireLocalSignal(Tag, FInstancedStruct());
		GT_S_LOG("GT.SignalBridge", TEXT("Dispatched local empty signal for %s."), *Tag.ToString());
	}
	else
	{
		GT_W_LOG("GT.SignalBridge", TEXT("Signal Bridge storage is unavailable."));
	}
}

void UGorgeousSignalBridgeCheatManagerExtension::Cheat_ClearSignalListeners(FString SignalTag)
{
	const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(*SignalTag, false);
	if (!Tag.IsValid())
	{
		GT_E_LOG("GT.SignalBridge", TEXT("Invalid signal tag: %s"), *SignalTag);
		return;
	}

	AGorgeousPlayerController* PlayerController = Cast<AGorgeousPlayerController>(GetPlayerController());
	if (!PlayerController)
	{
		GT_W_LOG("GT.SignalBridge", TEXT("A Gorgeous player controller is required to clear Signal Bridge listeners."));
		return;
	}

	if (USignalBridgeStorage_OV* Storage = USignalBridgeBlueprintFunctionLibrary::GetSignalBridgeStorage(this, false))
	{
		Storage->Clear(Tag, PlayerController);
		GT_S_LOG("GT.SignalBridge", TEXT("Cleared Signal Bridge listeners for %s."), *Tag.ToString());
	}
	else
	{
		GT_W_LOG("GT.SignalBridge", TEXT("Signal Bridge storage is unavailable."));
	}
}