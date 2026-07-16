// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "InsightMatrix/GorgeousInsightFunctionalTest.h"
#include "GeneralSystems/InteractionFoundation/InteractionFoundation_I.h"
#include "GorgeousInteractionFunctionalTest.generated.h"

class UBoxComponent;
class AActor;
/**
 * Functional Test Actor to physically test the Interaction Foundation using Sphere Traces.
 * Requires a tester to physically interact with it within a timeframe to pass the test.
 */
UCLASS(Blueprintable, BlueprintType)
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
