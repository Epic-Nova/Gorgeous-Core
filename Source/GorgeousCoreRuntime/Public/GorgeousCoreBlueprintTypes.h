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
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "Engine/Blueprint.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousCoreBlueprintTypes.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Object Variable Blueprint
| Functional Name: UGorgeousObjectVariableBlueprint
| Parent Class: UBlueprint
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Provides runtime functionality for Gorgeous Object Variable Blueprint.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime//Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime//GorgeousObjectVariableBlueprint",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime//Examples/"
		)
)
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