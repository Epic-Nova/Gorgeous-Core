// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "SharedTests/GorgeousInteractionFunctionalTest.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GeneralSystems/InteractionFoundation/GorgeousInteractionFoundation.h"

AGorgeousInteractionFunctionalTest::AGorgeousInteractionFunctionalTest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(RootComponent);
	InteractionBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	InteractionBox->SetCollisionProfileName(TEXT("BlockAll")); // Ensure it blocks traces for interaction
}

void AGorgeousInteractionFunctionalTest::StartTest()
{
	Super::StartTest();
	
	bTestActive = true;
	CurrentTime = 0.0f;
	
	// Set beacon to yellow indicating the test is actively waiting for an interaction
	UpdateBeaconColor(FLinearColor::Yellow);
	
	// Add an on-screen log message so the tester knows to interact with it
	GEngine->AddOnScreenDebugMessage(-1, InteractionTimeout, FColor::Yellow, FString::Printf(TEXT("Test %s Started! Interact with the Beacon within %f seconds!"), *GetName(), InteractionTimeout));
}

void AGorgeousInteractionFunctionalTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bTestActive)
	{
		CurrentTime += DeltaTime;
		
		if (CurrentTime >= InteractionTimeout)
		{
			bTestActive = false;
			UpdateBeaconColor(FLinearColor::Red);
			FinishTest(EFunctionalTestResult::Failed, TEXT("Tester failed to interact with the beacon within the timeout threshold."));
		}
	}
}

bool AGorgeousInteractionFunctionalTest::CanInteract_Implementation(AActor* Interactor) const
{
	// We only allow interaction if the test is active
	return bTestActive;
}

FGorgeousInteractionPayload_S AGorgeousInteractionFunctionalTest::GetInteractionPayload_Implementation(AActor* Interactor) const
{
	FGorgeousInteractionPayload_S Payload;
	Payload.ActionName = FText::FromString(TEXT("Complete Functional Test"));
	Payload.InteractionDuration = 0.0f; // Instant interaction
	return Payload;
}

void AGorgeousInteractionFunctionalTest::OnInteractionReceived_Implementation(AActor* Interactor, const FGorgeousInteractionPayload_S& Payload)
{
	if (bTestActive)
	{
		bTestActive = false;
		
		// Interaction received! The signal bridge payload routing worked and the sphere trace hit our component.
		UpdateBeaconColor(FLinearColor::Green);
		
		FinishTest(EFunctionalTestResult::Passed, TEXT("Interaction successfully routed! The tester triggered the interactive beacon."));
	}
}
