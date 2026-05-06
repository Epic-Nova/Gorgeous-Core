// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|             Gorgeous Core - Setup Wizard Widget                            |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
<=========================================================================*/

#pragma once

//<=====--- Includes ---=====>
//<----- Engine Includes ----->
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
//<-------------------------->

//<=====--- Forward Declarations ---=====>
class UGorgeousSystemTemplate_DA;
class IDetailsView;
class SEditableTextBox;
class SButton;
class SImage;
//<--------------------------------------->

/**
 * A modal wizard dialog that walks the user through creating a new asset
 * from a UGorgeousSystemTemplate_DA.
 *
 * Wizard steps:
 *   1. Displays the template's title + description.
 *   2. Renders the ConfigurationPayloadClass UPROPERTIES via IDetailsView
 *      (or falls back to displaying the AssetClassToCreate object directly).
 *   3. The footer exposes a save path and asset name input.
 *   4. "Finish" creates the asset via IAssetTools, calls ExecuteTemplateGeneration()
 *      on the created asset, then opens it in the editor and closes the wizard.
 *   5. "Cancel" closes the wizard without creating anything.
 */
class GORGEOUSCOREEDITOR_API SGorgeousSetupWizard : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SGorgeousSetupWizard)
		: _SystemTemplate(nullptr)
		, _DefaultSavePath(TEXT("/Game/"))
	{}
		/** The template data asset driving this wizard's form and generation logic. */
		SLATE_ARGUMENT(UGorgeousSystemTemplate_DA*, SystemTemplate)

		/** Suggested save directory pre-filled in the footer path field. */
		SLATE_ARGUMENT(FString, DefaultSavePath)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SGorgeousSetupWizard();

private:

	//<=====--- Button Handlers ---=====>
	FReply OnFinishClicked();
	FReply OnNextClicked();
	FReply OnBackClicked();
	FReply OnCancelClicked();
	FReply OnBrowseFolderClicked();
	void HandleSavePathPicked(const FString& PickedPath);
	//<-------------------------->

	//<=====--- Validation ---=====>

	/** Returns an error message string if saved path / asset name are invalid, or empty string if valid. */
	FString ValidateSavePath() const;

	/** Refreshes the details view and buttons based on the current page index. */
	void UpdateWizardState();

	/** Returns true if we can proceed to the next page / finish, calling PreCreate on the payload. */
	bool CanProceedToNextPage(FString& OutFailureReason) const;

	/** Returns true if we are currently on the last page of the wizard. */
	bool IsLastPage() const;

	//<=====--- UI Components ---=====>

	/** The source template that drives generation. */
	UGorgeousSystemTemplate_DA* SourceTemplate = nullptr;

	/** IDetailsView rendering PayloadInstance's properties. */
	TSharedPtr<IDetailsView> DetailsView;

	/** User-editable save directory (e.g. "/Game/Inventory/"). */
	TSharedPtr<SEditableTextBox> SavePathTextBox;

	/** User-editable asset name (e.g. "MyInventoryConfig"). */
	TSharedPtr<SEditableTextBox> AssetNameTextBox;

	TSharedPtr<SButton> BrowseFolderButton;
	TSharedPtr<SButton> FinishButton;
	TSharedPtr<SButton> NextButton;
	TSharedPtr<SButton> BackButton;
	//<-------------------------->

	//<=====--- State ---=====>

	/** The instantiated payload objects whose UPROPERTIES are shown in the wizard form. */
	TArray<UObject*> PayloadInstances;

	/** The current page index being displayed (0-based). */
	int32 CurrentPageIndex = 0;
	//<-------------------------->
};
