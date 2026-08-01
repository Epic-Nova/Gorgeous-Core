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
#include "InsightMatrix/GorgeousInsightFunctionalTest.h"
#include "GeneralSystems/InteractionFoundation/InteractionFoundation_I.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousInteractionFunctionalTest.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UBoxComponent;
class AActor;
//<------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Interaction Functional Test
| Functional Name: AGorgeousInteractionFunctionalTest
| Parent Class: AGorgeousInsightFunctionalTest
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Functional Test Actor to physically test the Interaction Foundation using
| Sphere Traces. Requires a tester to physically interact with it within a
| timeframe to pass the test.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Blueprintable, BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/SharedTests/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/SharedTests/AGorgeousInteractionFunctionalTest",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/SharedTests/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API AGorgeousInteractionFunctionalTest : public AGorgeousInsightFunctionalTest, public IInteractionFoundation_I
{
	GENERATED_BODY()

public:
	AGorgeousInteractionFunctionalTest(const FObjectInitializer& ObjectInitializer);

	virtual void StartTest() override;
	virtual void Tick(float DeltaTime) override;

	//~ Begin IInteractionFoundation_I Interface
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor, const FHitResult& HitResult) override;
	//~ End IInteractionFoundation_I Interface

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
	TObjectPtr<UBoxComponent> InteractionBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Test Parameters")
	float InteractionTimeout = 10.0f;

	float CurrentTime = 0.0f;
	bool bTestActive = false;
};