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
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Overview/#functional-tests",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Overview/#adding-your-own-scenario"
		)
)
class GORGEOUSCOREEDITOR_API AGorgeousInsightTestVolume : public AVolume
{
	GENERATED_BODY()

public:
	AGorgeousInsightTestVolume(const FObjectInitializer& ObjectInitializer);
    // Test actors stay usable in PIE while their instances/classes are excluded from game cooks.
    virtual bool IsEditorOnly() const override { return true; }
    virtual bool IsEditorOnlyLoadedInPIE() const override { return true; }


	/** Trigger execution of all contained functional tests */
	UFUNCTION(BlueprintCallable, Category="Insight Matrix")
	void RunAllTests();

	/** Find all tests physically inside this volume */
	UFUNCTION(BlueprintCallable, Category="Insight Matrix")
	TArray<AGorgeousInsightFunctionalTest*> GetContainedTests() const;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    UFUNCTION() void HandleContainedTestFinished();
    void RunNextTest();
    void PollContainedTest();
    TArray<TWeakObjectPtr<AGorgeousInsightFunctionalTest>> PendingTests;
    TWeakObjectPtr<AGorgeousInsightFunctionalTest> ActiveTest;
    FTimerHandle QueueWatchdog;
    bool bIsRunningTests;
};