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
#include "QualityOfLife/GorgeousGameState.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "QualityOfLife/GorgeousQualityOfLifeHelperMacros.h"
#include "AutoReplication/Helpers/GorgeousAutoReplicationHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Net/UnrealNetwork.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// AGorgeousGameState Implementation
//=============================================================================

UE_QOL_DEFINE_CONSTRUCTOR(AGorgeousGameState, true)

UE_QOL_DEFINE_HANDLE_AUTOREPLICATION_RPC(AGorgeousGameState)

UE_QOL_DEFINE_BEGIN_PLAY(AGorgeousGameState)

UE_QOL_DEFINE_REGISTER_AUTOREPLICATION_ENTRY(AGorgeousGameState)

UE_QOL_DEFINE_POST_INIT_AND_LOAD(AGorgeousGameState)

UE_DECLARE_AUTOREPLICATION_CLASS_GET_LIFETIME_REPLICATED_PROPS(AGorgeousGameState)

UE_DECLARE_AUTOREPLICATION_CLASS_ON_REP_VARIABLES(AGorgeousGameState)

UE_QOL_DEFINE_POST_EDIT_CHANGE_PROPERTY(AGorgeousGameState)



