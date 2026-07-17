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
#include "CoreMinimal.h"
#include "Engine/AssetUserData.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousExposedAssetUserData.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Exposed Asset User Data
| Functional Name: UGorgeousExposedAssetUserData
| Parent Class: UAssetUserData
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| whi the hell wrote this class?
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName = "Gorgeous Exposed Asset User Data",
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/QualityOfLife/Miscellaneous/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/QualityOfLife/Miscellaneous/GorgeousExposedAssetUserData",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/QualityOfLife/Miscellaneous/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousExposedAssetUserData : public UAssetUserData
{
	GENERATED_BODY()
};