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
/*======================================================================================================================>
| --------------------------------------------------------------------------------------------------------------------- |
|								TO AVOID MISTAKES READ THIS BEFORE EDITING THIS FILE                                    |
| --------------------------------------------------------------------------------------------------------------------- |
|									USE ONLY FOR INCLUDES THAT ARE FOR THIS MODULE.                                     |
<======================================================================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

/*==============================>
| Engine Includes               |
<==============================*/
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/UnrealTypePrivate.h"
#include "Misc/MonolithicHeaderBoilerplate.h"
#include "TimerManager.h"
#include "GameplayTags.h"

#include "Interfaces/IPluginManager.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "Modules/ModuleInterface.h"

#include "Templates/SubclassOf.h"

#include "CoreUObject.h"
#include "UObject/Interface.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Info.h"

#include "Subsystems/GameInstanceSubsystem.h"

/*==============================>
| Module Includes               |
<==============================*/
#include "IGorgeousThingsModuleInterface.h"