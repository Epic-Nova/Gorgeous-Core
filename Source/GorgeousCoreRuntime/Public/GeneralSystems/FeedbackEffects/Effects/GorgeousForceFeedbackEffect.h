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
#include "FeedbackEffects/GorgeousFeedbackEffect.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousForceFeedbackEffect.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Force Feedback Effect
| Functional Name: UGorgeousForceFeedbackEffect
| Parent Class: UGorgeousFeedbackEffect
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Feedback effect that triggers controller force feedback using Unreal's
| built-in force feedback. The effect relies entirely on the engine's force
| feedback orchestration (player controller / world-spawned force feedback
| component) and does not touch any low-level platform API.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Effects/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Effects/GorgeousForceFeedbackEffect",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Effects/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousForceFeedbackEffect : public UGorgeousFeedbackEffect
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	/**
	 * Plays the configured force feedback effect on the resolved player controller.
	 *
	 * @param Context The runtime situation used to resolve the player controller.
	 */
	virtual void PerformExecute(const FGorgeousFeedbackContext& Context) override;

	/**
	 * Determines whether this effect is enabled and has a force feedback asset configured.
	 *
	 * @param Context The runtime situation to evaluate.
	 * @return True when the effect may execute, false otherwise.
	 */
	virtual bool CanExecute_Implementation(const FGorgeousFeedbackContext& Context) const override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

	// The force feedback curve asset that drives the controller motors.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UForceFeedbackEffect> ForceFeedback;

	// Whether the force feedback effect should loop until stopped.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLooping = false;

	// Whether time dilation should be ignored while the effect plays.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIgnoreTimeDilation = false;

	// Whether the effect should continue playing while the game is paused.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPlayWhilePaused = false;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};