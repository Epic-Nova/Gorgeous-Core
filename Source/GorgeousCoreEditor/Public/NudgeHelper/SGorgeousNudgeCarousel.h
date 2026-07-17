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
//<--------------------------=== Engine Includes ===------------------------->
#include "Widgets/SCompoundWidget.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGorgeousNudgeSubsystem;
class SButton;
class STextBlock;
class SWebBrowser;
class SWidgetSwitcher;
//<------------------------------------------------------------->

/** Hosts the controls and content for a single nudge-carousel window. */
class SGorgeousNudgeCarousel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGorgeousNudgeCarousel) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UGorgeousNudgeSubsystem>, NudgeSubsystem)
	SLATE_END_ARGS()

	/**
	 * Constructs the carousel from the supplied Slate arguments.
	 *
	 * @param InArgs The arguments that provide the nudge subsystem.
	 */
	void Construct(const FArguments& InArgs);

private:
	/** Returns to the preceding visible nudge. */
	FReply OnPrevious();

	/** Returns to the preceding webpage within the active browser. */
	FReply OnBrowserBack();

	/** Advances to the next visible nudge. */
	FReply OnNext();

	/** Executes the action associated with the current nudge. */
	FReply OnAction();

	/**
	 * Selects the carousel page represented by an index.
	 *
	 * @param PageIndex The page index to select.
	 */
	void SelectPage(int32 PageIndex);

	// Updates the carousel controls from the current entry.
	void Refresh();

	// Provides the currently visible nudge entries.
	TWeakObjectPtr<UGorgeousNudgeSubsystem> NudgeSubsystem;

	// Identifies the entry displayed by the carousel.
	int32 CurrentIndex = 0;

	// Displays the current nudge title.
	TSharedPtr<STextBlock> TitleText;

	// Displays the current nudge description.
	TSharedPtr<STextBlock> DescriptionText;

	// Displays the current nudge title above the rendered page.
	TSharedPtr<STextBlock> HeaderTitleText;

	// Displays the current page position in the carousel header.
	TSharedPtr<STextBlock> PageCounterText;

	// Retains every preloaded HTML page and displays the selected page.
	TSharedPtr<SWidgetSwitcher> PageSwitcher;

	// Retains every preloaded preview browser instance.
	TArray<TSharedPtr<SWebBrowser>> BrowserPages;

	// Retains the destination browser for every carousel page.
	TArray<TSharedPtr<SWebBrowser>> DestinationBrowserPages;

	// Switches each carousel page between its retained preview and destination browser.
	TArray<TSharedPtr<SWidgetSwitcher>> BrowserContentSwitchers;

	// Tracks whether each browser currently displays its destination webpage.
	TArray<bool> bViewingDestinationPage;

	// Contains the dot buttons that indicate the active page.
	TArray<TSharedPtr<SButton>> ProgressDots;

	// Navigates to the preceding nudge.
	TSharedPtr<SButton> PreviousButton;

	// Returns to the preceding webpage in the active browser.
	TSharedPtr<SButton> BrowserBackButton;

	// Navigates to the following nudge.
	TSharedPtr<SButton> NextButton;

	// Executes the current nudge action.
	TSharedPtr<SButton> ActionButton;

	// Displays the current nudge action label.
	TSharedPtr<STextBlock> ActionButtonText;
};
