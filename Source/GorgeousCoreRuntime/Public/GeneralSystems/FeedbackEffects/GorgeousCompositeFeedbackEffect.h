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
#include "GorgeousCompositeFeedbackEffect.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Composite Feedback Effect
| Functional Name: UGorgeousCompositeFeedbackEffect
| Parent Class: UGorgeousFeedbackEffect
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| A feedback effect that groups several child effects and plays them
| together. Example: a sword hit could be expressed as a composite that
| fires a Niagara fire burst, a hit sound, a controller rumble and a camera
| shake all at once. The composite forwards the same feedback context to
| every child and only runs children whose own CanExecute passes.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/GorgeousCompositeFeedbackEffect",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousCompositeFeedbackEffect  : public UGorgeousFeedbackEffect
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	/**
	 * Executes every child effect whose individual conditions are satisfied.
	 *
	 * @param Context The runtime situation the child effects execute against.
	 */
	virtual void Execute_Implementation(
		const FGorgeousFeedbackContext& Context
	) override;

	/**
	 * Determines whether at least one child effect can execute in the supplied context.
	 *
	 * @param Context The runtime situation to evaluate.
	 * @return True when at least one child effect may execute, false otherwise.
	 */
	virtual bool CanExecute_Implementation(
		const FGorgeousFeedbackContext& Context
	) const override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

	// The child effects that are executed together when this composite runs.
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category="Composite")
	TArray<TObjectPtr<UGorgeousFeedbackEffect>> Children;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables


	//<============================--- Callbacks ---============================>
	#pragma region Callbacks
protected:

#if WITH_EDITOR

	// Previews every child effect in the editor.
	virtual void Preview_Internal() override;

	// Stops the preview of every child effect in the editor.
	virtual void StopPreview_Internal() override;

#endif
	//<------------------------------------------------------------------------->
	#pragma endregion Callbacks
};