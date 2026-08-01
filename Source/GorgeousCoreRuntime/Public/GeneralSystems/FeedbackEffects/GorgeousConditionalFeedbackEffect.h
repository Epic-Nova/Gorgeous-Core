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
#include "GorgeousFeedbackEffect.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousConditionalFeedbackEffect.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Conditional Feedback Effect
| Functional Name: UGorgeousConditionalFeedbackEffect
| Parent Class: UGorgeousFeedbackEffect
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| A feedback effect that wraps another effect and only runs it when tag
| conditions are met. Use this to gate a single effect behind gameplay tags
| carried by the feedback context, for example playing an upgraded impact
| effect only when the source item is tagged as legendary.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/GorgeousConditionalFeedbackEffect",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousConditionalFeedbackEffect  : public UGorgeousFeedbackEffect
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	/**
	 * Determines whether the required tags are present and the wrapped effect can execute.
	 *
	 * @param Context The runtime situation to evaluate.
	 * @return True when the wrapped effect may execute, false otherwise.
	 */
	virtual bool CanExecute_Implementation(
		const FGorgeousFeedbackContext& Context
	) const override;

	/**
	 * Executes the wrapped effect when its conditions are satisfied.
	 *
	 * @param Context The runtime situation the wrapped effect executes against.
	 */
	virtual void Execute_Implementation(
		const FGorgeousFeedbackContext& Context
	) override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

	// The effect that is executed when the required tags are present.
	UPROPERTY(EditAnywhere, Instanced, Category="Condition")
	TObjectPtr<UGorgeousFeedbackEffect> Effect;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};