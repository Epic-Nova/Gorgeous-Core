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
class FGorgeousPersistentData;
class FJsonObject;
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
	DocumentationOverview = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#updates-and-news",
	DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#updates-and-news",
	DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#updates-and-news"
	))
class GORGEOUSCOREEDITOR_API UGorgeousNudgeSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Initializes persistent nudge state and the periodic check ticker.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Cancels requests, stops the ticker and closes windows owned by this subsystem.
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

	/**
	 * Resizes the carousel for the currently selected nudge entry.
	 *
	 * @param Entry The entry whose preferred window dimensions are applied.
	 */
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

#if WITH_DEV_AUTOMATION_TESTS
	/**
	 * Overrides the feed endpoint only for loopback HTTP automation fixtures.
	 *
	 * @param Endpoint The loopback HTTP endpoint used by the isolated fixture.
	 */
	void SetFeedEndpointForTests(const FString& Endpoint);
#endif
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
	 * Separates opt-in toast announcements from the existing news carousel.
	 *
	 * @param Entry The nudge whose presentation is being selected.
	 * @return True for an independently delivered toast announcement.
	 */
	static bool UsesToast(const FGorgeousNudgeEntry& Entry);

	/**
	 * Applies current per-user tutorial, system and tip preferences.
	 *
	 * @param Entry The announcement whose preferences are evaluated.
	 * @return True when consent and its notification preference permit delivery.
	 */
	bool IsNotificationEnabled(const FGorgeousNudgeEntry& Entry) const;

	/**
	 * Returns the supported action caption, omitting a tip button without a valid link.
	 *
	 * @param Entry The announcement whose action is being presented.
	 * @return The action caption, or empty text when no action is available.
	 */
	static FText GetNotificationActionLabel(const FGorgeousNudgeEntry& Entry);

	/**
	 * Selects the primary article URL, preserving its separate action destination.
	 *
	 * @param Entry The announcement whose document is being displayed.
	 * @return The HTTPS document URL, or an empty string when unavailable.
	 */
	static FString GetDocumentUrl(const FGorgeousNudgeEntry& Entry);

	/**
	 * Resolves optional dimensions using the existing editor window bounds.
	 *
	 * @param Entry The announcement whose preferred dimensions are being applied.
	 * @return The bounded window dimensions used by carousel and tutorial windows.
	 */
	static FVector2D ResolveWindowSize(const FGorgeousNudgeEntry& Entry);

	/**
	 * Uses explicit offline precedence and the normal editor network consent gate.
	 *
	 * @return True when the current instance may perform network-backed work.
	 */
	bool IsNetworkAllowed() const;

#if WITH_DEV_AUTOMATION_TESTS
	/**
	 * Requires an isolated document and headless loopback endpoint for injected fixtures.
	 *
	 * @return True only for an isolated, unattended automation fixture.
	 */
	bool HasIsolatedTestContext() const;

	/**
	 * Accepts only an explicit loopback host, numeric port and HTTP path.
	 *
	 * @param Endpoint The candidate automation URL.
	 * @return True when the URL resolves to an explicit supported loopback authority.
	 */
	static bool IsLoopbackTestEndpoint(const FString& Endpoint);
#endif

	/**
	 * Parses the versioned news protocol without modifying persistent state.
	 *
	 * @param FeedRoot The received versioned news envelope.
	 * @param OutEntries Receives the supported, well-formed entries.
	 * @return True when the envelope uses the supported schema.
	 */
	static bool ParseNudgeFeed(const TSharedPtr<FJsonObject>& FeedRoot, TArray<FGorgeousNudgeEntry>& OutEntries);

	/**
	 * Delivers one pending toast only after the interactive editor is ready.
	 *
	 * @return True only when a notification was actually presented and acknowledged.
	 */
	bool DeliverNextNotification();

	/**
	 * Opens a tutorial window, focuses a system in the Library, or follows a tip link.
	 *
	 * @param Entry The announcement whose action is requested.
	 * @param bContextualTip Whether the announcement requires Context Aware Tips.
	 */
	void ExecuteToastAction(FGorgeousNudgeEntry Entry, bool bContextualTip = false);

	/**
	 * Shows and acknowledges one nudge using the shared toast presentation.
	 *
	 * @param Entry The announcement to present.
	 * @param bContextualTip Whether the recommendation requires the contextual preference.
	 * @return Whether the toast was shown.
	 */
	bool PresentNotification(const FGorgeousNudgeEntry& Entry, bool bContextualTip);

	// Checks whether the current preferences and editor state permit a contextual request.
	bool CanUseContextTips() const;

	// Samples newly active asset editors and discards results from earlier assets.
	void ConsiderContextTip();

	/**
	 * Requests tips for one coarse category while the selected asset remains active.
	 *
	 * @param Asset The locally tracked editor asset. Only the category is sent.
	 * @param AssetType The allowlisted category to send.
	 */
	void FetchContextTip(UObject* Asset, const FString& AssetType);

	/**
	 * Uses the process service, or an explicitly isolated automation document.
	 *
	 * @return The persistent-data service belonging to this instance.
	 */
	FGorgeousPersistentData& GetPersistentData() const;

	/**
	 * Returns whether a nudge was displayed in an earlier editor session.
	 *
	 * @param NudgeId The nudge identifier to look up.
	 * @return True when the identifier was persisted.
	 */
	bool WasAcknowledged(const FString& NudgeId) const;

	// Loads persisted nudge identifiers.
	void LoadAcknowledgements();

	// Saves identifiers only after a carousel or toast was actually displayed.
	void SaveAcknowledgements() const;

	// Detects Gorgeous plugins installed since the previous editor session.
	void DetectNewGorgeousPlugins();
	void DetectNewGorgeousGeneralSystems();

	// Loads the previously fetched Everything Gorgeous news feed from persistent data.
	void LoadCachedNudgeFeed();

	// Saves a successful Everything Gorgeous news response to persistent data.
	void SaveNudgeFeedCache(const TSharedPtr<class FJsonObject>& FeedRoot) const;

	// Requests the Everything Gorgeous update and news feed.
	void FetchNudgeFeed();

	/**
	 * Downloads an applicable, enabled Markdown article before showing it.
	 *
	 * @param Entry The entry whose remote Markdown content is resolved.
	 */
	void FetchMarkdownNudge(FGorgeousNudgeEntry Entry);

	// Queues the automatic carousel until the editor has completed its Slate startup.
	void QueueAutomaticCarousel();

	// Delays general-system detection until startup has recorded the active systems.
	bool bGeneralSystemsDetectionCompleted = false;

	/**
	 * Clears the carousel reference and releases Slate pointer capture after its window closes.
	 *
	 * @param ClosedWindow The carousel window that Slate is closing.
	 */
	void HandleCarouselWindowClosed(const TSharedRef<SWindow>& ClosedWindow);

	/**
	 * Advances the periodic nudge timer.
	 *
	 * @param DeltaTime The time elapsed since the previous tick.
	 * @return True to keep the ticker registered.
	 */
	bool Tick(float DeltaTime);

	// Contains the entries currently shown by the carousel.
	TArray<FGorgeousNudgeEntry> VisibleNudges;

	// Keeps toast announcements out of the carousel and deduplicates them by entry ID.
	TArray<FGorgeousNudgeEntry> PendingNotifications;

	// Retains standalone tutorial windows until Slate closes them.
	TArray<TSharedPtr<SWindow>> TutorialWindows;

	// Cancels the feed request during subsystem shutdown and prevents overlapping checks.
	TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> FeedRequest;

	// Keeps context recommendations separate from the ordinary feed and its cache.
	TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> ContextTipRequest;

	// Tracks the asset locally while its recommendation is in flight.
	TWeakObjectPtr<UObject> ContextAsset;

	// Keeps the current asset's tip until the toast interval permits presentation.
	TOptional<FGorgeousNudgeEntry> PendingContextTip;

	// Limits contextual queries to one per cooldown, even across rapid tab switches.
	float NextContextTipRequest = 0.0f;

	// Records whether this foreground visit already received a random draw.
	bool bSampledContextAsset = false;

	// Prevents concurrent downloads for the same Markdown article.
	TMap<FString, TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe>> MarkdownRequests;

	// Spaces announcements so one feed cannot flood the editor with toasts.
	float SecondsSinceLastNotification = 12.0f;

	// Ignores callbacks after requests and UI have been released during shutdown.
	bool bStopping = false;

#if WITH_DEV_AUTOMATION_TESTS
	friend struct FGorgeousNudgeTestAccess;

	// Restricts private HTTP tests to an explicitly supplied loopback endpoint.
	FString TestFeedEndpoint;

	// Keeps commandlet validation from reading or writing the user's metadata document.
	FGorgeousPersistentData* TestPersistentData = nullptr;

	// Simulates consent only inside the isolated loopback automation fixture.
	TOptional<bool> TestNetworkConsent;

	// Replaces Slate presentation with a delivery receipt in headless automation tests.
	TFunction<bool(const FGorgeousNudgeEntry&, const FText&)> TestNotificationPresenter;

	// Records action dispatch without opening a browser or Library during automation tests.
	TFunction<void(const FGorgeousNudgeEntry&)> TestActionDispatcher;
#endif

	// Contains persisted identifiers for already displayed nudges.
	TSet<FString> AcknowledgedNudgeIds;

	// Identifies the periodic ticker registration.
	FTSTicker::FDelegateHandle TickerHandle;

	// Accumulates elapsed time between automatic checks.
	float SecondsSinceLastCheck = 0.0f;

	// Accumulates startup time before an automatic browser window may be created.
	float SecondsSinceInitialization = 0.0f;

	// Prevents startup and asynchronous feed work from creating a Slate window immediately.
	bool bAutomaticCarouselQueued = false;

	// Owns the active nudge carousel window.
	TSharedPtr<SWindow> CarouselWindow;
	//<------------------------------------------------------------------------->
	#pragma endregion Variables
};