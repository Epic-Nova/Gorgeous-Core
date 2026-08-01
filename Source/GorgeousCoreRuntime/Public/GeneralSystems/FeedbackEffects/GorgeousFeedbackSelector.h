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
#include "GorgeousFeedbackEffectTypes.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousFeedbackSelector.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Feedback Selector
| Functional Name: UGorgeousFeedbackSelector
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Policy object that fully owns selection when a feedback definition uses
| EGorgeousFeedbackSelectionMode::ECustom. The dispatcher hands the selector
| the already-filter-passed candidate effects plus the feedback context, and
| uses whatever ordered list it returns as the final set to play. This gives
| absolute freedom: a selector may keep, drop, reorder, duplicate or even
| synthesize brand-new effects (e.g. spawn an instanced effect at runtime) —
| there is no constraint on the result. Subclass and override SelectEffects
| (BlueprintNativeEvent, so it can be implemented in either C++ or
| Blueprint). The default implementation returns the candidates unchanged.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
//<=================--- Forward Declarations ---=================>
class UGorgeousFeedbackEffect;
//<------------------------------------------------------------->
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, DisplayName="Feedback Selector",
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/GorgeousFeedbackSelector",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousFeedbackSelector : public UObject
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Decides which effects from the candidate set should actually play.
	 *
	 * @param Candidates The effects that survived the definition's FilterTags (already gated).
	 * @param Context    The feedback context the definition is executing against.
	 * @param OutSelected The ordered list of effects to play. May be any subset, in any order,
	 *                     and may include effects not present in Candidates (full creative freedom).
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Gorgeous|Feedback")
	void SelectEffects(
		const TArray<UGorgeousFeedbackEffect*>& Candidates,
		const FGorgeousFeedbackContext& Context,
		TArray<UGorgeousFeedbackEffect*>& OutSelected) const;
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions
};