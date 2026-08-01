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
#include "GorgeousSoundFeedbackEffect.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Sound Feedback Effect
| Functional Name: UGorgeousSoundFeedbackEffect
| Parent Class: UGorgeousFeedbackEffect
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Feedback effect that plays a sound cue at the feedback location or
| attached to the target.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Effects/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Effects/GorgeousSoundFeedbackEffect",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Effects/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousSoundFeedbackEffect : public UGorgeousFeedbackEffect
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	/**
	 * Plays the configured sound at the feedback location or attached to the target.
	 *
	 * @param Context The runtime situation used to resolve the playback location.
	 */
	virtual void PerformExecute(const FGorgeousFeedbackContext& Context) override;

	/**
	 * Determines whether this effect is enabled and has a sound asset configured.
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

	// The sound asset to play.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	TObjectPtr<USoundBase> Sound;

	// Multiplier applied to the sound's volume.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	float VolumeMultiplier = 1.f;

	// Multiplier applied to the sound's pitch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	float PitchMultiplier = 1.f;

	// Time in seconds into the sound at which playback begins.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	float StartTime = 0.f;

	// Whether the sound should follow the target when an attach component is available.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	bool bFollowTarget = true;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables


	//<============================--- Callbacks ---============================>
	#pragma region Callbacks
protected:

#if WITH_EDITOR
	// Editor preview hook; intentionally a no-op because the properties fully describe this effect.
	virtual void Preview_Internal() override;
#endif
	//<------------------------------------------------------------------------->
	#pragma endregion Callbacks
};