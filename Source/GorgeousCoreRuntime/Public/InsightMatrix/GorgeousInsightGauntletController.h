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
#include "GauntletTestController.h"
#include "GorgeousInsightGauntletController.generated.h"
//<-------------------------------------------------------------------------->

/** Runs an Insight suite through Gauntlet with -gauntlet=GorgeousInsightGauntlet. */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousInsightGauntletController : public UGauntletTestController
{
	GENERATED_BODY()
public:
	virtual void OnInit() override;
	virtual void OnPostMapChange(UWorld* World) override;
	virtual void OnTick(float TimeDelta) override;
private:
	TWeakObjectPtr<UWorld> TestWorld;
	FString MatrixParameters;
	float WarmupSeconds = 2.0f;
	float TimeoutSeconds = 120.0f;
	float ElapsedSeconds = 0.0f;
	float ReadySeconds = 0.0f;
	bool bFinished = false;
};
