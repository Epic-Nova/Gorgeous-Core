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
//<--------------------------=== Module Includes ===------------------------->
#include "UObject/Interface.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousQualityOfLifeNodeTarget_I.generated.h"
//<-------------------------------------------------------------------------->

UINTERFACE(MinimalAPI, Blueprintable, BlueprintType)
class UGorgeousQualityOfLifeNodeTarget_I : public UInterface
{
	GENERATED_BODY()
};

class GORGEOUSCORERUNTIME_API IGorgeousQualityOfLifeNodeTarget_I
{
	GENERATED_BODY()

public:
	//virtual ~IGorgeousQualityOfLifeNodeTarget_I() = default;
};