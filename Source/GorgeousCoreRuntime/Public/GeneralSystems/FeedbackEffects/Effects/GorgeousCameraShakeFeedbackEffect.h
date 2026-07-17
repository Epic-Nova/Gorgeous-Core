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
#include "GorgeousCameraShakeFeedbackEffect.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Camera Shake Feedback Effect
| Functional Name: UGorgeousCameraShakeFeedbackEffect
| Parent Class: UGorgeousFeedbackEffect
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Feedback effect that plays a camera shake on the player's camera.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Effects/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Effects/GorgeousCameraShakeFeedbackEffect",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Effects/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousCameraShakeFeedbackEffect : public UGorgeousFeedbackEffect
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	/**
	 * Starts the configured camera shake on the resolved player controller's camera.
	 *
	 * @param Context The runtime situation used to resolve the player controller.
	 */
	virtual void PerformExecute(const FGorgeousFeedbackContext& Context) override;

	/**
	 * Determines whether this effect is enabled and has a camera shake class configured.
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

	// The camera shake class to start on the player's camera.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UCameraShakeBase> CameraShake;

	// Intensity multiplier applied to the camera shake.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Scale = 1.f;

	// Coordinate space in which the camera shake is applied.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECameraShakePlaySpace PlaySpace = ECameraShakePlaySpace::CameraLocal;

	// Custom rotation used when PlaySpace is set to UserDefined.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator UserPlaySpaceRotation;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};