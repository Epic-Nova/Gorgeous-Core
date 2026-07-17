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
//<--------------------------=== Engine Includes ===------------------------->
#include "NiagaraSystem.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousNiagaraFeedbackEffect.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Niagara Feedback Effect
| Functional Name: UGorgeousNiagaraFeedbackEffect
| Parent Class: UGorgeousFeedbackEffect
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Feedback effect that spawns a Niagara VFX system at the feedback location
| or attached to the target.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Effects/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Effects/GorgeousNiagaraFeedbackEffect",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Effects/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousNiagaraFeedbackEffect : public UGorgeousFeedbackEffect
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	/**
	 * Spawns the configured Niagara system at the feedback location or attached to the target.
	 *
	 * @param Context The runtime situation used to resolve the spawn transform.
	 */
	virtual void PerformExecute(const FGorgeousFeedbackContext& Context) override;

	/**
	 * Determines whether this effect is enabled and has a Niagara system configured.
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

	// The Niagara system template to spawn.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UNiagaraSystem> System;

	// World-space offset added to the spawn location.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Offset = FVector::ZeroVector;

	// Rotation offset added to the spawn rotation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RotationOffset = FRotator::ZeroRotator;

	// Scale applied to the spawned Niagara system.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale = FVector(1.f);

	// Whether the system follows the target when an attach component is available.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFollowTarget = true;

	// Whether the spawned system is automatically destroyed when it completes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoDestroy = true;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};