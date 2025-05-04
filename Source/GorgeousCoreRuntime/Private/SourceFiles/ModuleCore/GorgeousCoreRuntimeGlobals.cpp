// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/
#include "GorgeousCoreRuntimeGlobals.h"

//=============================================================================
// UGorgeousCoreRuntimeGlobals Implementation
//=============================================================================

UGorgeousGameInstance* UGorgeousCoreRuntimeGlobals::GetGorgeousGameInstance(const UObject* WorldContextObject)
{
	return Cast<UGorgeousGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject));
}

AGorgeousGameMode* UGorgeousCoreRuntimeGlobals::GetGorgeousGameMode(const UObject* WorldContextObject)
{
	return Cast<AGorgeousGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
}

AGorgeousGameState* UGorgeousCoreRuntimeGlobals::GetGorgeousGameState(const UObject* WorldContextObject)
{
	return Cast<AGorgeousGameState>(UGameplayStatics::GetGameState(WorldContextObject));
}

AGorgeousPlayerController* UGorgeousCoreRuntimeGlobals::GetGorgeousPlayerController(const UObject* WorldContextObject, const int32 PlayerIndex)
{
	return Cast<AGorgeousPlayerController>(UGameplayStatics::GetPlayerController(WorldContextObject, PlayerIndex));
}

AGorgeousPlayerState* UGorgeousCoreRuntimeGlobals::GetGorgeousPlayerState(const UObject* WorldContextObject, const int32 PlayerStateIndex)
{
	return Cast<AGorgeousPlayerState>(UGameplayStatics::GetPlayerState(WorldContextObject, PlayerStateIndex));
}

AGorgeousWorldSettings* UGorgeousCoreRuntimeGlobals::GetGorgeousWorldSettings(const UObject* WorldContextObject)
{
	return Cast<AGorgeousWorldSettings>(WorldContextObject->GetWorld()->GetWorldSettings());
}
