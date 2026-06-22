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
#include "GameFramework/Volume.h"
#include "GorgeousInsightTestVolume.generated.h"

class AGorgeousInsightFunctionalTest;

/**
 * Volume used to group multiple Insight Functional Tests together.
 * Allows mass execution of all contained tests.
 */
UCLASS(Blueprintable)
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
