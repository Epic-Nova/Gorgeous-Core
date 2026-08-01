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
#include "Subsystems/GameInstanceSubsystem.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousFeedbackDispatcher.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Feedback Dispatcher
| Functional Name: UGorgeousFeedbackDispatcher
| Parent Class: UGameInstanceSubsystem
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Runtime dispatcher for the Gorgeous Feedback Effects system. This is a
| Game Instance Subsystem: there is exactly one instance for the whole game,
| so the provider set is not instance-editable. Instead, providers are
| registered and unregistered at runtime (e.g. during subsystem
| initialization or by other systems). When feedback is requested the
| dispatcher asks each registered provider (in registration order) for a
| feedback definition and, once one resolves a non-empty definition,
| executes every effect it contains. The dispatcher is the missing link that
| actually triggers effects: providers resolve "what" feedback to play for a
| situation, this subsystem plays it.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
//<=================--- Forward Declarations ---=================>
class UGorgeousFeedbackEffect;
class UGorgeousFeedbackProvider;
struct FGorgeousFeedbackContext;
struct FGorgeousFeedbackDefinition;
//<------------------------------------------------------------->
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/GorgeousFeedbackDispatcher",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/FeedbackEffects/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousFeedbackDispatcher : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Registers a feedback provider. Providers are queried in registration order.
	 *
	 * @param Provider The provider instance to add. Ignored if null or already registered.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Feedback")
	void RegisterProvider(UGorgeousFeedbackProvider* Provider);

	/**
	 * Unregisters a previously registered feedback provider.
	 *
	 * @param Provider The provider instance to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Feedback")
	void UnregisterProvider(UGorgeousFeedbackProvider* Provider);

	/**
	 * Returns the number of feedback providers currently registered with the dispatcher.
	 *
	 * @return The number of registered feedback providers.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Feedback")
	int32 GetRegisteredProviderCount() const;

	/**
	 * Returns the number of feedback requests made since this subsystem was created.
	 *
	 * @return The total number of feedback requests.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Feedback")
	int32 GetTotalTriggers() const { return TotalTriggers; }

	/**
	 * Returns the number of feedback requests resolved by a registered provider.
	 *
	 * @return The number of successfully resolved feedback requests.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Feedback")
	int32 GetTotalTriggersResolved() const { return TotalTriggersResolved; }

	/**
	 * Returns the number of feedback requests that no registered provider resolved.
	 *
	 * @return The number of unresolved feedback requests.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Feedback")
	int32 GetTotalTriggersUnresolved() const { return TotalTriggersUnresolved; }

	/**
	 * Returns the total number of individual feedback effects executed by this dispatcher.
	 *
	 * @return The total number of executed feedback effects.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Feedback")
	int32 GetTotalEffectsExecuted() const { return TotalEffectsExecuted; }

	/**
	 * Returns the rolling log of recently executed effects, ordered oldest to newest.
	 *
	 * @return The effect display names and descriptions retained for the Insight Matrix.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Feedback")
	const TArray<FString>& GetRecentEffects() const { return RecentEffects; }

	/**
	 * Requests feedback for the given context.
	 *
	 * Iterates registered providers in order and uses the first one that resolves a non-empty
	 * feedback definition, then executes every effect in that definition. If no provider resolves
	 * a definition, nothing plays.
	 *
	 * @param Context The situation feedback is requested for.
	 * @return True if a provider resolved a definition and at least one effect ran, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Feedback")
	bool TriggerFeedback(const FGorgeousFeedbackContext& Context);

	/**
	 * Plays an explicit feedback definition directly, bypassing the provider registry.
	 *
	 * @param Definition The definition whose effects should be executed.
	 * @param Context    The situation the effects execute against.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Feedback")
	void PlayDefinition(const FGorgeousFeedbackDefinition& Definition, const FGorgeousFeedbackContext& Context);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
public:

	// Capacity of the recent-effects ring buffer.
	static constexpr int32 RecentEffectsCapacity = 8;

protected:

	/**
	 * Resolves a feedback definition for the context by querying registered providers in order.
	 *
	 * @param Context      The situation feedback is requested for.
	 * @param OutDefinition The first non-empty definition resolved by a provider.
	 * @return True if a provider resolved a non-empty definition, false otherwise.
	 */
	bool ResolveDefinition(const FGorgeousFeedbackContext& Context, FGorgeousFeedbackDefinition& OutDefinition) const;
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only


	//<============================--- Variables ---============================>
	#pragma region Variables
protected:

	// Ordered list of registered providers queried when feedback is requested.
	UPROPERTY()
	TArray<TObjectPtr<UGorgeousFeedbackProvider>> Providers;

	// Total TriggerFeedback calls since this subsystem was created.
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous|Feedback|Stats")
	int32 TotalTriggers = 0;

	// TriggerFeedback calls that resolved a non-empty definition via a provider.
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous|Feedback|Stats")
	int32 TotalTriggersResolved = 0;

	// TriggerFeedback calls that found no provider able to resolve a definition.
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous|Feedback|Stats")
	int32 TotalTriggersUnresolved = 0;

	// Total individual effects executed (across all definitions played).
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous|Feedback|Stats")
	int32 TotalEffectsExecuted = 0;

	// Rolling log of recently executed effects for the Insight Matrix.
	TArray<FString> RecentEffects;
	//<------------------------------------------------------------------------->
	#pragma endregion Variables
};