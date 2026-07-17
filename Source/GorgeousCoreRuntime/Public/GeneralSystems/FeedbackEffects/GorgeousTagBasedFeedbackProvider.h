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
#include "GorgeousFeedbackProvider.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousTagBasedFeedbackProvider.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Tag Based Feedback Provider
| Functional Name: UGorgeousTagBasedFeedbackProvider
| Parent Class: UGorgeousFeedbackProvider
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| A tag-driven feedback provider. Maps gameplay tags to feedback
| definitions. When queried it returns the definition whose tag is present
| in the feedback context, allowing designers to author feedback per
| situation (e.g. surface type, damage type, item rarity) entirely through
| data without writing code.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DisplayName="Tag Based Feedback Provider",
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/GorgeousTagBasedFeedbackProvider",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousTagBasedFeedbackProvider : public UGorgeousFeedbackProvider
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	/**
	 * Resolves the feedback for the given context.
	 *
	 * @param Context The runtime situation the feedback is requested for.
	 * @param OutFeedback Receives the resolved feedback definition.
	 * @return True if a non-empty feedback definition was resolved, false otherwise.
	 */
	virtual bool ProvideFeedback_Implementation(
		const FGorgeousFeedbackContext& Context,
		FGorgeousFeedbackDefinition& OutFeedback
	) const override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

	// Tag-to-feedback mapping. The first entry whose tag is present in the context is used.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feedback")
	TMap<FGameplayTag, FGorgeousFeedbackDefinition> TagToFeedback;

	// Optional fallback used when no mapped tag is present in the context.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feedback")
	FGorgeousFeedbackDefinition DefaultFeedback;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};