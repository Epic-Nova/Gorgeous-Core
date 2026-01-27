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
#include "QualityOfLife/GorgeousPlayerState.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "QualityOfLife/GorgeousQualityOfLifeHelperMacros.h"
#include "AutoReplication/Helpers/GorgeousAutoReplicationHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Net/UnrealNetwork.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// AGorgeousPlayerState Implementation
//=============================================================================

UE_QOL_DEFINE_CONSTRUCTOR(AGorgeousPlayerState, true)

UE_QOL_DEFINE_HANDLE_AUTOREPLICATION_RPC(AGorgeousPlayerState)

UE_QOL_DEFINE_BEGIN_PLAY(AGorgeousPlayerState)

UE_QOL_DEFINE_POST_INIT_AND_LOAD(AGorgeousPlayerState)

UE_DECLARE_AUTOREPLICATION_CLASS_GET_LIFETIME_REPLICATED_PROPS(AGorgeousPlayerState)

UE_DECLARE_AUTOREPLICATION_CLASS_ON_REP_VARIABLES(AGorgeousPlayerState)

UE_QOL_DEFINE_POST_EDIT_CHANGE_PROPERTY(AGorgeousPlayerState)


