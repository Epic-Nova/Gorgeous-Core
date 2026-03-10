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
#include "QualityOfLife/GorgeousGameMode.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "QualityOfLife/GorgeousQualityOfLifeHelperMacros.h"
#include "QualityOfLife/GorgeousPlayerState.h"
#include "AutoReplication/GorgeousAutoReplicationHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Net/UnrealNetwork.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// AGorgeousGameMode Implementation
//=============================================================================

UE_QOL_DEFINE_CONSTRUCTOR(AGorgeousGameMode, true)

UE_QOL_DEFINE_HANDLE_AUTOREPLICATION_RPC(AGorgeousGameMode)

UE_QOL_DEFINE_BEGIN_PLAY(AGorgeousGameMode)

UE_QOL_DEFINE_REGISTER_AUTOREPLICATION_ENTRY(AGorgeousGameMode)

UE_QOL_DEFINE_POST_INIT_AND_LOAD(AGorgeousGameMode)

UE_DECLARE_AUTOREPLICATION_CLASS_GET_LIFETIME_REPLICATED_PROPS(AGorgeousGameMode)

UE_DECLARE_AUTOREPLICATION_CLASS_ON_REP_VARIABLES(AGorgeousGameMode)

UE_QOL_DEFINE_POST_EDIT_CHANGE_PROPERTY(AGorgeousGameMode)

void AGorgeousGameMode::Test()
{
	//ChangeName() IMPLEMENT THIS FOR CONTROLLER NAME CHAGE, SO WE DONT INVENT THE WHEEL AGAIN
}

UE_QOL_DEFINE_GAME_MODE_LOGIN_CALLBACKS(AGorgeousGameMode)
