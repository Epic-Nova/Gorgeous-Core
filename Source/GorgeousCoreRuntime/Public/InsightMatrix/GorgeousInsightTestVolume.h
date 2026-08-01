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
#include "GameFramework/Volume.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousInsightTestVolume.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class AGorgeousInsightFunctionalTest;
//<------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Insight Test Volume
| Functional Name: AGorgeousInsightTestVolume
| Parent Class: AVolume
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Volume used to group multiple Insight Functional Tests together. Allows
| mass execution of all contained tests.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Blueprintable,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/AGorgeousInsightTestVolume",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API AGorgeousInsightTestVolume : public AVolume
{
	GENERATED_BODY()

public:
	AGorgeousInsightTestVolume(const FObjectInitializer& ObjectInitializer);

	/** Trigger execution of all contained functional tests */
	UFUNCTION(BlueprintCallable, Category="Insight Matrix")
	void RunAllTests();

	/** Find all tests physically inside this volume */
	UFUNCTION(BlueprintCallable, Category="Insight Matrix")
	TArray<AGorgeousInsightFunctionalTest*> GetContainedTests() const;

protected:
	bool bIsRunningTests;
};