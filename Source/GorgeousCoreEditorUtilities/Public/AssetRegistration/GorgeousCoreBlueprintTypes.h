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

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "Engine/Blueprint.h"
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousCoreBlueprintTypes.generated.h"
//<-------------------------------------------------------------------------->

UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousObjectVariableBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousConditionalObjectChooserBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousConditionBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousBooleanConditionBlueprint : public UGorgeousConditionBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousGameplayTagConditionBlueprint : public UGorgeousConditionBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousIsValidConditionBlueprint : public UGorgeousConditionBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousGameInstanceBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousGameModeBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousGameStateBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousPlayerControllerBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousPlayerStateBlueprint : public UBlueprint
{
	GENERATED_BODY()
};

UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousWorldSettingsBlueprint : public UBlueprint
{
	GENERATED_BODY()
};
