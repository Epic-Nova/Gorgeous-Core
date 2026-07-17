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
#include "NudgeHelper/GorgeousNudgeEntry.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "EditorSubsystem.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousNudgeSubsystem.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class SGorgeousNudgeCarousel;
class SWindow;
//<------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Nudge Subsystem
| Functional Name: UGorgeousNudgeSubsystem
| Parent Class: UEditorSubsystem
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Hosts applicable Gorgeous update, news, and tutorial notifications in the
| editor and persists entries that were already displayed.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(meta = (
	DocumentationOverview = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/NudgeHelper/Overview",
	DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/NudgeHelper/GorgeousNudgeSubsystem",
	DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/NudgeHelper/Examples"
	))
class GORGEOUSCOREEDITOR_API UGorgeousNudgeSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Initializes persistent nudge state and the periodic check ticker.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Stops the ticker and closes the active carousel window.
	virtual void Deinitialize() override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Registers nudge entries received from the Everything Gorgeous API.
	 *
	 * @param Entries The nudge entries to evaluate and display.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Nudges")
	void RegisterNudges(const TArray<FGorgeousNudgeEntry>& Entries);

	/** Opens the carousel for currently applicable nudge entries. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Nudges")
	void ShowNudgeCarousel();

	/** Requests an immediate update and nudge check. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Nudges")
	void CheckForNudgesNow();
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
public:

	/**
	 * Records that a nudge was handled by the carousel.
	 *
	 * @param NudgeId The identifier of the acknowledged nudge.
	 */
	void AcknowledgeNudge(const FString& NudgeId);

	/** Resizes the carousel for the currently selected nudge entry. */
	void ResizeCarouselForEntry(const FGorgeousNudgeEntry& Entry);

	/**
	 * Returns the nudge entries visible to the carousel.
	 *
	 * @return The currently visible nudge entries.
	 */
	const TArray<FGorgeousNudgeEntry>& GetVisibleNudges() const { return VisibleNudges; }

	/**
	 * Converts completed plugin update checks into nudge entries.
	 *
	 * @param bUpdatesAvailable Whether the update check found available updates.
	 */
	UFUNCTION()
	void HandlePluginUpdateCheckCompleted(bool bUpdatesAvailable);
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only


	//<============================--- Variables ---============================>
	#pragma region Variables
private:

	/**
	 * Returns whether an entry applies to currently installed Gorgeous content.
	 *
	 * @param Entry The nudge entry to evaluate.
	 * @return True when the entry applies to the current project.
	 */
	bool IsApplicable(const FGorgeousNudgeEntry& Entry) const;

	/**
	 * Returns whether a nudge was displayed in an earlier editor session.
	 *
	 * @param NudgeId The nudge identifier to look up.
	 * @return True when the identifier was persisted.
	 */
	bool WasAcknowledged(const FString& NudgeId) const;

	// Loads persisted nudge identifiers.
	void LoadAcknowledgements();

	// Saves displayed nudge identifiers.
	void SaveAcknowledgements() const;

	// Detects Gorgeous plugins installed since the previous editor session.
	void DetectNewGorgeousPlugins();

	// Loads the previously fetched Everything Gorgeous news feed from persistent data.
	void LoadCachedNudgeFeed();

	// Saves a successful Everything Gorgeous news response to persistent data.
	void SaveNudgeFeedCache(const TSharedPtr<class FJsonObject>& FeedRoot) const;

	// Requests the Everything Gorgeous update and news feed.
	void FetchNudgeFeed();

	// Downloads a Markdown document referenced by a feed entry before showing it.
	void FetchMarkdownNudge(FGorgeousNudgeEntry Entry);

	/**
	 * Advances the periodic nudge timer.
	 *
	 * @param DeltaTime The time elapsed since the previous tick.
	 * @return True to keep the ticker registered.
	 */
	bool Tick(float DeltaTime);

	// Contains the entries currently shown by the carousel.
	TArray<FGorgeousNudgeEntry> VisibleNudges;

	// Contains persisted identifiers for already displayed nudges.
	TSet<FString> AcknowledgedNudgeIds;

	// Identifies the periodic ticker registration.
	FTSTicker::FDelegateHandle TickerHandle;

	// Accumulates elapsed time between automatic checks.
	float SecondsSinceLastCheck = 0.0f;

	// Owns the active nudge carousel window.
	TSharedPtr<SWindow> CarouselWindow;
	//<------------------------------------------------------------------------->
	#pragma endregion Variables
};
