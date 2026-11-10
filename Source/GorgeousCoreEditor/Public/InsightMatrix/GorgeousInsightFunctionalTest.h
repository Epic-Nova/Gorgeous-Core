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
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "FunctionalTest.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousInsightFunctionalTest.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UStaticMeshComponent;
class UWidgetComponent;
//<------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Insight Functional Test
| Functional Name: AGorgeousInsightFunctionalTest
| Parent Class: AFunctionalTest
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| GameTest-style Functional Testing framework actor. Used to orchestrate
| Arrange, Act, and Assert steps in dedicated testing levels, reporting
| results directly to the Insight Matrix. Includes a visual beacon and
| interactive result log UI.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Blueprintable, BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Overview/#functional-tests",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/InsightMatrix/Overview/#adding-your-own-scenario"
		)
)
class GORGEOUSCOREEDITOR_API AGorgeousInsightFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

public:
	AGorgeousInsightFunctionalTest(const FObjectInitializer& ObjectInitializer);
    // Test actors stay usable in PIE while their instances/classes are excluded from game cooks.
    virtual bool IsEditorOnly() const override { return true; }
    virtual bool IsEditorOnlyLoadedInPIE() const override { return true; }


	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void StartTest() override;
	virtual void FinishTest(EFunctionalTestResult TestResult, const FString& Message) override;

	// Mesh used to visually indicate the state of the test (Beacon)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Insight Matrix")
	TObjectPtr<UStaticMeshComponent> BeaconMesh;

	// UI widget to show test results when interacted with
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Insight Matrix")
	TObjectPtr<UWidgetComponent> ResultWidget;

	/** Translates the standard Functional Test result into Insight Matrix telemetry */
	void ReportToInsightMatrix(EFunctionalTestResult TestResult, const FString& Message);

	/** Updates the Beacon material color (Grey=Unrun, Yellow=Running, Green=Passed, Red=Failed) */
	UFUNCTION(BlueprintCallable, Category="Insight Matrix")
	void UpdateBeaconColor(FLinearColor NewColor);

	/** Store the last scenario result for the interactive UI */
	FGorgeousInsightScenarioResult LastResult;
    double TestStartSeconds = 0.0;
};