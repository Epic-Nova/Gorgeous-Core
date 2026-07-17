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
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousFeedbackProvider.generated.h"
//<-------------------------------------------------------------------------->

// @TODO: providers could theoretically also draw from the internet (if added then it would come in 2.0 of this system)

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Feedback Provider
| Functional Name: UGorgeousFeedbackProvider
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Abstract base class for objects that provide feedback definitions. A
| feedback provider resolves a situation (described by an
| FGorgeousFeedbackContext) into a concrete FGorgeousFeedbackDefinition.
| This allows feedback to be sourced dynamically, for example from an item
| definition, a surface type or a gameplay tag, instead of being hardcoded.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Abstract, Blueprintable,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/GorgeousFeedbackProvider",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousFeedbackProvider : public UObject
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Resolves the feedback that should play for the given context.
	 *
	 * @param Context    The runtime situation the feedback is requested for.
	 * @param OutFeedback The feedback definition to populate and return to the caller.
	 * @return True if a feedback definition was provided, false otherwise.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Feedback")
	bool ProvideFeedback(const FGorgeousFeedbackContext& Context, FGorgeousFeedbackDefinition& OutFeedback) const;
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions
};