// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Insight Matrix (Runtime)                   |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "GorgeousInsightFunctionalTest.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;

/**
 * GameTest-style Functional Testing framework actor.
 * Used to orchestrate Arrange, Act, and Assert steps in dedicated testing levels,
 * reporting results directly to the Insight Matrix. Includes a visual beacon 
 * and interactive result log UI.
 */
UCLASS(Blueprintable, BlueprintType)
class GORGEOUSCORERUNTIME_API AGorgeousInsightFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

public:
	AGorgeousInsightFunctionalTest(const FObjectInitializer& ObjectInitializer);

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void StartTest() override;
	virtual void FinishTest(EFunctionalTestResult TestResult, const FString& Message) override;

	/** Mesh used to visually indicate the state of the test (Beacon) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Insight Matrix")
	TObjectPtr<UStaticMeshComponent> BeaconMesh;

	/** UI widget to show test results when interacted with */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Insight Matrix")
	TObjectPtr<UWidgetComponent> ResultWidget;

	/** Translates the standard Functional Test result into Insight Matrix telemetry */
	void ReportToInsightMatrix(EFunctionalTestResult TestResult, const FString& Message);

	/** Updates the Beacon material color (Grey=Unrun, Yellow=Running, Green=Passed, Red=Failed) */
	UFUNCTION(BlueprintCallable, Category="Insight Matrix")
	void UpdateBeaconColor(FLinearColor NewColor);

	/** Store the last scenario result for the interactive UI */
	FGorgeousInsightScenarioResult LastResult;
};
