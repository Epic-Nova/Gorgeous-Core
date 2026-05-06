// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "Engine/Blueprint.h"
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousCoreBlueprintTypes.generated.h"
//<-------------------------------------------------------------------------->

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousObjectVariableBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousConditionalObjectChooserBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousConditionBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousBooleanConditionBlueprint : public UGorgeousConditionBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousGameplayTagConditionBlueprint : public UGorgeousConditionBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousIsValidConditionBlueprint : public UGorgeousConditionBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousGameInstanceBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousGameModeBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousGameStateBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousPlayerControllerBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousPlayerStateBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousWorldSettingsBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousHUDBlueprint : public UBlueprint
{
	GENERATED_BODY()
};