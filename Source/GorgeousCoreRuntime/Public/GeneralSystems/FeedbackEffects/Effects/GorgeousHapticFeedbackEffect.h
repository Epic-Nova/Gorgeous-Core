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
#include "GorgeousHapticFeedbackEffect.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Haptic Feedback Effect
| Functional Name: UGorgeousHapticFeedbackEffect
| Parent Class: UGorgeousFeedbackEffect
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Feedback effect that triggers haptic feedback on a controller or mobile
| device. Uses Unreal's built-in haptics orchestration; no low-level
| platform API is accessed.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Effects/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Effects/GorgeousHapticFeedbackEffect",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Effects/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousHapticFeedbackEffect : public UGorgeousFeedbackEffect
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	/**
	 * Plays the configured haptic effect on the resolved player controller.
	 *
	 * @param Context The runtime situation used to resolve the player controller.
	 */
	virtual void PerformExecute(const FGorgeousFeedbackContext& Context) override;

	/**
	 * Determines whether this effect is enabled and has a haptic asset configured.
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

	// The haptic curve asset describing the vibration pattern.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UHapticFeedbackEffect_Base> HapticEffect;

	// Which hand/controller the haptic effect is played on.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EControllerHand Hand = EControllerHand::Left;

	// Multiplier applied to the intensity of the haptic effect.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Scale = 1.f;

	// Whether the haptic effect should follow the player's hand.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFollowTarget = false;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};