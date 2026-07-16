// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GorgeousFeedbackDispatcher.generated.h"

class UGorgeousFeedbackEffect;
class UGorgeousFeedbackProvider;
struct FGorgeousFeedbackContext;
struct FGorgeousFeedbackDefinition;

/**
 * Runtime dispatcher for the Gorgeous Feedback Effects system.
 *
 * This is a Game Instance Subsystem: there is exactly one instance for the whole game, so the
 * provider set is not instance-editable. Instead, providers are registered and unregistered at
 * runtime (e.g. during subsystem initialization or by other systems). When feedback is requested
 * the dispatcher asks each registered provider (in registration order) for a feedback definition
 * and, once one resolves a non-empty definition, executes every effect it contains.
 *
 * The dispatcher is the missing link that actually triggers effects: providers resolve "what"
 * feedback to play for a situation, this subsystem plays it.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousFeedbackDispatcher : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	// ── Provider registry ─────────────────────────────────────────────────────

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

	/** Returns the number of currently registered providers. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Feedback")
	int32 GetRegisteredProviderCount() const;

	/** Total TriggerFeedback calls since this subsystem was created. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Feedback")
	int32 GetTotalTriggers() const { return TotalTriggers; }

	/** TriggerFeedback calls that resolved a non-empty definition via a provider. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Feedback")
	int32 GetTotalTriggersResolved() const { return TotalTriggersResolved; }

	/** TriggerFeedback calls that found no provider able to resolve a definition. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Feedback")
	int32 GetTotalTriggersUnresolved() const { return TotalTriggersUnresolved; }

	/** Total individual effects executed (across all definitions played). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Feedback")
	int32 GetTotalEffectsExecuted() const { return TotalEffectsExecuted; }

	/** Most recent executed effects, newest last (capped at RecentEffectsCapacity). Exposes DisplayName/Description to the Insight Matrix. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous|Feedback")
	const TArray<FString>& GetRecentEffects() const { return RecentEffects; }

	/** Capacity of the recent-effects ring buffer. */
	static constexpr int32 RecentEffectsCapacity = 8;

	// ── Triggering feedback ───────────────────────────────────────────────────

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

protected:

	/** Ordered list of registered providers queried when feedback is requested. */
	UPROPERTY()
	TArray<TObjectPtr<UGorgeousFeedbackProvider>> Providers;

	/**
	 * Resolves a feedback definition for the context by querying registered providers in order.
	 *
	 * @param Context      The situation feedback is requested for.
	 * @param OutDefinition The first non-empty definition resolved by a provider.
	 * @return True if a provider resolved a non-empty definition, false otherwise.
	 */
	bool ResolveDefinition(const FGorgeousFeedbackContext& Context, FGorgeousFeedbackDefinition& OutDefinition) const;

	/** Total TriggerFeedback calls since this subsystem was created. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous|Feedback|Stats")
	int32 TotalTriggers = 0;

	/** TriggerFeedback calls that resolved a non-empty definition via a provider. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous|Feedback|Stats")
	int32 TotalTriggersResolved = 0;

	/** TriggerFeedback calls that found no provider able to resolve a definition. */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous|Feedback|Stats")
	int32 TotalTriggersUnresolved = 0;

	/** Total individual effects executed (across all definitions played). */
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous|Feedback|Stats")
	int32 TotalEffectsExecuted = 0;

	/** Rolling log of the most recently executed effects (DisplayName + Description) for the Insight Matrix. */
	TArray<FString> RecentEffects;
};
