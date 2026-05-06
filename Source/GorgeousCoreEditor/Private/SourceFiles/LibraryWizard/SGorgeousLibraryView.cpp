// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|             Gorgeous Core - Library Wizard View Widget Implementation      |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
<=========================================================================*/

#include "LibraryWizard/SGorgeousLibraryView.h"

//<=====--- Includes ---=====>
//<----- Engine Includes ----->
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STileView.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Modules/ModuleManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SWrapBox.h"
//<----- Module Includes ----->
#include "LibraryWizard/IGorgeousLibraryParticipant.h"
#include "LibraryWizard/SGorgeousSetupWizard.h"
#include "LibraryWizard/GorgeousSystemTemplate_DA.h"
#include "LibraryWizard/GorgeousAssetFilters.h"
#include "Helpers/GorgeousPluginHelper.h"
#include "IGorgeousThingsModuleInterface.h"
//<-------------------------->

//=============================================================================
// SGorgeousLibraryView Helper Widgets
//=============================================================================

/**
 * A premium filter pill widget mimicking the Unreal Engine's content browser filters.
 */
class SGorgeousLibraryFilterPill : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGorgeousLibraryFilterPill) {}
		SLATE_ARGUMENT(FGorgeousLibraryFilter, Filter)
		SLATE_EVENT(SGorgeousLibraryView::FOnFilterTagToggled, OnFilterToggled)
		SLATE_ATTRIBUTE(ECheckBoxState, IsChecked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		Filter = InArgs._Filter;
		OnFilterToggled = InArgs._OnFilterToggled;

		ChildSlot
		[
			SNew(SCheckBox)
			.Style(FAppStyle::Get(), "FilterBar.FilterButton")
			.ToolTipText(Filter.DisplayName)
			.IsChecked(InArgs._IsChecked)
			.OnCheckStateChanged(this, &SGorgeousLibraryFilterPill::HandleCheckStateChanged)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("FilterBar.FilterImage"))
					.ColorAndOpacity(this, &SGorgeousLibraryFilterPill::GetFilterImageColor)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(Filter.DisplayName)
					.TextStyle(FAppStyle::Get(), "SmallText")
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			]
		];
	}

	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override
	{
		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			// Double-click to enable ONLY this filter
			OnFilterToggled.ExecuteIfBound(true, Filter.FilterId, true);
			return FReply::Handled();
		}
		return SCompoundWidget::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
	}

private:

	void HandleCheckStateChanged(ECheckBoxState NewState)
	{
		OnFilterToggled.ExecuteIfBound(NewState == ECheckBoxState::Checked, Filter.FilterId, false);
	}

	FSlateColor GetFilterImageColor() const
	{
		return Filter.Color;
	}

	FGorgeousLibraryFilter Filter;
	SGorgeousLibraryView::FOnFilterTagToggled OnFilterToggled;
};

/**
 * A helper widget that manages the lifetime of dynamic asset icons.
 * This prevents crashes caused by local TSharedPtr<FSlateBrush> going out of scope.
 */
class SGorgeousAssetIcon : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGorgeousAssetIcon)
		: _IconSize(FVector2D(48.0f, 48.0f))
	{}
		SLATE_ARGUMENT(TSharedPtr<FAssetData>, Asset)
		SLATE_ARGUMENT(TSharedPtr<IGorgeousLibraryParticipant>, Participant)
		SLATE_ARGUMENT(FVector2D, IconSize)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		TSharedPtr<FAssetData> Asset = InArgs._Asset;
		TSharedPtr<IGorgeousLibraryParticipant> Participant = InArgs._Participant;
		const FVector2D IconSize = InArgs._IconSize;

		if (Participant.IsValid() && Asset.IsValid())
		{
			IconBrush = Participant->GetAssetIcon(*Asset);
		}

		const FSlateBrush* FinalBrush = IconBrush.IsValid() ? IconBrush.Get() : FAppStyle::GetBrush("ContentBrowser.DefaultAssetIcon");

		ChildSlot
		[
			SNew(SImage)
			.Image(FinalBrush)
			.DesiredSizeOverride(IconSize)
		];
	}

private:
	TSharedPtr<FSlateBrush> IconBrush;
};


//=============================================================================
// SGorgeousLibraryView Implementation
//=============================================================================

void SGorgeousLibraryView::Construct(const FArguments& InArgs)
{
	// Collect all participants from registered modules (silently skip non-participants).
	if (UGorgeousPluginHelper* Helper = UGorgeousPluginHelper::GetSingleton())
	{
		for (IGorgeousThingsModuleInterface* Module : Helper->GetAllRegisteredModules())
		{
			if (IGorgeousLibraryParticipant* Participant = Module->GetLibraryParticipant())
			{
				AllParticipants.Add(MakeShareable(Participant, [](IGorgeousLibraryParticipant*){}));
			}
		}
	}

	// Auto-select first participant if available.
	if (AllParticipants.Num() > 0)
	{
		SelectedParticipant = AllParticipants[0];
		RefreshAssetsForParticipant(SelectedParticipant);
	}

	ChildSlot
	[
		BuildLayout()
	];
}

TSharedRef<SWidget> SGorgeousLibraryView::BuildLayout()
{
	return SNew(SSplitter)
		.Orientation(Orient_Horizontal)

		// Left panel: Participant list
		+ SSplitter::Slot()
		.Value(0.22f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(FMargin(0.0f))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(10.0f, 10.0f, 10.0f, 6.0f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("GorgeousCore", "LibraryPlugins", "Gorgeous Plugins"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(6.0f, 0.0f, 6.0f, 6.0f)
				[
					SNew(SSeparator)
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					BuildParticipantList()
				]
			]
		]

		// Right panel: Category sections
		+ SSplitter::Slot()
		.Value(0.78f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			.Padding(FMargin(0.0f))
			[
				SAssignNew(RightPanelContainer, SBox)
				[
					BuildDetailPanel()
				]
			]
		];
}

TSharedRef<SWidget> SGorgeousLibraryView::BuildParticipantList()
{
	return SAssignNew(ParticipantListView, SListView<TSharedPtr<IGorgeousLibraryParticipant>>)
		.ListItemsSource(&AllParticipants)
		.SelectionMode(ESelectionMode::Single)
		.OnGenerateRow(this, &SGorgeousLibraryView::OnGenerateParticipantRow)
		.OnSelectionChanged_Lambda([this](TSharedPtr<IGorgeousLibraryParticipant> Item, ESelectInfo::Type)
		{
			if (Item.IsValid())
			{
				OnParticipantSelected(Item);
			}
		});
}

TSharedRef<SWidget> SGorgeousLibraryView::BuildDetailPanel()
{
	if (!SelectedParticipant)
	{
		return SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("GorgeousCore", "LibraryNoPlugin", "Select a plugin from the left panel."))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			];
	}

	TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox);

	// Header
	ScrollBox->AddSlot()
		.Padding(16.0f, 14.0f, 16.0f, 4.0f)
		[
			SNew(STextBlock)
			.Text(SelectedParticipant->GetParticipantDisplayName())
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
		];

	ScrollBox->AddSlot()
		.Padding(16.0f, 0.0f, 16.0f, 10.0f)
		[
			SNew(SSeparator)
		];

	// Add one section per category
	for (const FGorgeousLibraryCategoryDescriptor& Category : SelectedParticipant->GetLibraryCategories())
	{
		const TArray<TSharedPtr<FAssetData>>* Assets = FilteredCategoryAssets.Find(Category.CategoryId);
		TArray<TSharedPtr<FAssetData>> EmptyList;
		const TArray<TSharedPtr<FAssetData>>& AssetList = Assets ? *Assets : EmptyList;

		ScrollBox->AddSlot()
			.Padding(0.0f)
			[
				BuildCategorySection(Category, AssetList)
			];
	}

	return ScrollBox;
}

TSharedRef<SWidget> SGorgeousLibraryView::BuildCategorySection(
	const FGorgeousLibraryCategoryDescriptor& Category,
	const TArray<TSharedPtr<FAssetData>>& Assets)
{
	const FName CatId = Category.CategoryId;

	// Gather filtered asset list for this category
	TArray<TSharedPtr<FAssetData>>* FilteredPtr = FilteredCategoryAssets.Find(CatId);
	TArray<TSharedPtr<FAssetData>> EmptyList;
	TArray<TSharedPtr<FAssetData>>& Source = FilteredPtr ? *FilteredPtr : EmptyList;

	TSharedPtr<SWidget> ViewWidget;

	if (Category.ViewMode == EGorgeousLibraryViewMode::List)
	{
		TSharedPtr<SListView<TSharedPtr<FAssetData>>> ListView;
		SAssignNew(ListView, SListView<TSharedPtr<FAssetData>>)
			.ListItemsSource(&Source)
			.OnGenerateRow(this, &SGorgeousLibraryView::OnGenerateAssetRow)
			.OnMouseButtonDoubleClick_Lambda([this, CatId](TSharedPtr<FAssetData> Item)
			{
				OnAssetDoubleClicked(Item, CatId);
			})
			.OnContextMenuOpening(FOnContextMenuOpening::CreateSP(this, &SGorgeousLibraryView::OnContextMenuOpening, CatId));

		CategoryListViews.Add(CatId, ListView);
		ViewWidget = ListView;
	}
	else
	{
		TSharedPtr<STileView<TSharedPtr<FAssetData>>> TileView;
		SAssignNew(TileView, STileView<TSharedPtr<FAssetData>>)
			.ListItemsSource(&Source)
			.ItemWidth(140.0f)
			.ItemHeight(120.0f)
			.OnGenerateTile(this, &SGorgeousLibraryView::OnGenerateAssetTile)
			.OnMouseButtonDoubleClick_Lambda([this, CatId](TSharedPtr<FAssetData> Item)
			{
				OnAssetDoubleClicked(Item, CatId);
			})
			.OnContextMenuOpening(FOnContextMenuOpening::CreateSP(this, &SGorgeousLibraryView::OnContextMenuOpening, CatId));

		CategoryTileViews.Add(CatId, TileView);
		ViewWidget = TileView;
	}

	TSharedRef<SVerticalBox> Section = SNew(SVerticalBox)

		// ── Category label
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(16.0f, 10.0f, 16.0f, 4.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(Category.DisplayName)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SSpacer)
			]
		];

	TSharedPtr<TFilterCollection<TSharedPtr<FAssetData>>> FilterCollection = CategoryFilterCollections.FindOrAdd(CatId);
	if (!FilterCollection.IsValid())
	{
		FilterCollection = MakeShared<TFilterCollection<TSharedPtr<FAssetData>>>();
		CategoryFilterCollections.Add(CatId, FilterCollection);
	}

	// ── Custom Category Actions
	for (const FGorgeousLibraryCategoryAction& Action : Category.CategoryActions)
	{
		const FSlateBrush* IconBrush = Action.IconName.IsNone() ? nullptr : FAppStyle::GetBrush(Action.IconName);

		TSharedPtr<SHorizontalBox> HeaderBox = StaticCastSharedRef<SHorizontalBox>(Section->GetSlot(0).GetWidget());
		HeaderBox->AddSlot()
			.AutoWidth()
			.Padding(FMargin(5.0f, 0.0f))
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(Action.Tooltip)
				.OnClicked(this, &SGorgeousLibraryView::OnCategoryActionClicked, Category.CategoryId, Action.ActionId)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(IconBrush ? IconBrush : FAppStyle::GetBrush("Icons.Plus"))
						.Visibility(IconBrush || Action.DisplayName.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed)
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(IconBrush ? 4.0f : 0.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(Action.DisplayName)
						.Visibility(Action.DisplayName.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
					]
				]
			];
	}

	TSharedPtr<SBasicFilterBar<TSharedPtr<FAssetData>>> FilterBar;

	// ── Filter Bar
	if (Category.bEnableAdvancedFiltering && SelectedParticipant.IsValid())
	{
		TArray<FGorgeousLibraryFilter> AvailableFilters = SelectedParticipant->GetFilters(CatId);
		if (AvailableFilters.Num() > 0)
		{
			TMap<FName, TSharedPtr<FFilterCategory>> FilterCategoryMap;
			TArray<TSharedRef<FFilterBase<TSharedPtr<FAssetData>>>> CustomFilters;

			for (const FGorgeousLibraryFilter& FilterDesc : AvailableFilters)
			{
				TSharedPtr<FFilterCategory>& CategoryPtr = FilterCategoryMap.FindOrAdd(FilterDesc.FilterCategory);
				if (!CategoryPtr.IsValid())
				{
					const FText CategoryTitle = FilterDesc.FilterCategory.IsNone()
						? NSLOCTEXT("GorgeousCore", "LibraryFilterCategoryDefault", "Filters")
						: FText::FromName(FilterDesc.FilterCategory);
					CategoryPtr = MakeShared<FFilterCategory>(CategoryTitle, CategoryTitle);
				}

				CustomFilters.Add(MakeShared<FGorgeousAssetFilter>(FilterDesc, CategoryPtr));
			}

			CategoryFilterBars.Add(CatId, nullptr);
			Section->AddSlot()
				.AutoHeight()
				.Padding(16.0f, 0.0f, 16.0f, 0.0f)
				[
					SAssignNew(FilterBar, SBasicFilterBar<TSharedPtr<FAssetData>>)
					.OnFilterChanged(this, &SGorgeousLibraryView::OnFilterChanged, CatId)
					.CustomFilters(CustomFilters)
					.UseSectionsForCategories(true)
					.FilterBarLayout(EFilterBarLayout::Vertical)
					.Visibility(EVisibility::Collapsed)
				];

			if (FilterBar.IsValid())
			{
				CategoryFilterBars[CatId] = FilterBar;
			}
		}
	}

	if (Category.bHasSearchFilter)
	{
		Section->AddSlot()
			.AutoHeight()
			.Padding(16.0f, 2.0f, 16.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SNew(SSearchBox)
						.HintText(NSLOCTEXT("GorgeousCore", "LibrarySearch", "Search assets…"))
						.OnTextChanged_Lambda([this, CatId](const FText& NewText)
						{
							OnSearchTextChanged(NewText, CatId);
						})
					]
				+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(8.0f, 0.0f, 0.0f, 0.0f)
					[
						FilterBar.IsValid() ? SBasicFilterBar<TSharedPtr<FAssetData>>::MakeAddFilterButton(FilterBar.ToSharedRef()) : SNullWidget::NullWidget
					]
			];
	}

	if (!Category.bHasSearchFilter && FilterBar.IsValid())
	{
		Section->AddSlot()
			.AutoHeight()
			.Padding(16.0f, 0.0f, 16.0f, 8.0f)
			[
				SBasicFilterBar<TSharedPtr<FAssetData>>::MakeAddFilterButton(FilterBar.ToSharedRef())
			];
	}

	// ── Content View
	Section->AddSlot()
		.FillHeight(1.0f)
		.Padding(8.0f, 0.0f, 8.0f, 12.0f)
		[
			ViewWidget.ToSharedRef()
		];

	return Section;
}

void SGorgeousLibraryView::OnFilterChanged(FName CategoryId)
{
	if (TSharedPtr<SBasicFilterBar<TSharedPtr<FAssetData>>>* FilterBarPtr = CategoryFilterBars.Find(CategoryId))
	{
		CategoryFilterCollections.Add(CategoryId, (*FilterBarPtr)->GetAllActiveFilters());
	}
	OnSearchTextChanged(FText::FromString(CategorySearchText.FindRef(CategoryId)), CategoryId);
}

FReply SGorgeousLibraryView::OnToggleFilterBar(FName CategoryId)
{
	bool& bIsOpen = CategoryFilterBarOpen.FindOrAdd(CategoryId);
	bIsOpen = !bIsOpen;

	if (RightPanelContainer.IsValid())
	{
		RightPanelContainer->Invalidate(EInvalidateWidget::Layout);
	}

	return FReply::Handled();
}

EVisibility SGorgeousLibraryView::GetFilterBarVisibility(FName CategoryId) const
{
	const bool* bIsOpen = CategoryFilterBarOpen.Find(CategoryId);
	if (!bIsOpen)
	{
		return EVisibility::Visible;
	}
	return *bIsOpen ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply SGorgeousLibraryView::OnCategoryActionClicked(FName CategoryId, FName ActionId)
{
	if (SelectedParticipant.IsValid())
	{
		SelectedParticipant->OnCategoryActionExecuted(CategoryId, ActionId);
	}
	return FReply::Handled();
}

//-----------------------------------------------------------------------------
// Participant Callbacks
//-----------------------------------------------------------------------------

void SGorgeousLibraryView::OnParticipantSelected(TSharedPtr<IGorgeousLibraryParticipant> Participant)
{
	SelectedParticipant = Participant;

	// Clear per-category caches for the new participant
	CategoryAssets.Empty();
	FilteredCategoryAssets.Empty();
	CategorySearchText.Empty();
	CategoryFilterBars.Empty();
	CategoryFilterCollections.Empty();
	CategoryFilterBarOpen.Empty();
	CategoryTileViews.Empty();
	CategoryListViews.Empty();

	RefreshAssetsForParticipant(Participant);

	// Rebuild right panel
	if (RightPanelContainer.IsValid())
	{
		RightPanelContainer->SetContent(BuildDetailPanel());
	}
}

TSharedRef<ITableRow> SGorgeousLibraryView::OnGenerateParticipantRow(
	TSharedPtr<IGorgeousLibraryParticipant> Participant,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<IGorgeousLibraryParticipant>>, OwnerTable)
		.Padding(FMargin(10.0f, 6.0f))
		[
			SNew(STextBlock)
			.Text(Participant->GetParticipantDisplayName())
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
		];
}

//-----------------------------------------------------------------------------
// Asset View Callbacks
//-----------------------------------------------------------------------------

TSharedRef<ITableRow> SGorgeousLibraryView::OnGenerateAssetTile(
	TSharedPtr<FAssetData> InAsset,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
		.Padding(6.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(8.0f)
			[
				SNew(SVerticalBox)
				// Asset icon
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SGorgeousAssetIcon)
					.Asset(InAsset)
					.Participant(SelectedParticipant)
					.IconSize(FVector2D(48.0f, 48.0f))
				]

				// Asset name

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 4.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(InAsset.IsValid() ? FText::FromName(InAsset->AssetName) : FText::GetEmpty())
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
					.Justification(ETextJustify::Center)
					.AutoWrapText(true)
				]
			]
		];
}

TSharedRef<ITableRow> SGorgeousLibraryView::OnGenerateAssetRow(
	TSharedPtr<FAssetData> InAsset,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	FText Description;
	if (SelectedParticipant.IsValid() && InAsset.IsValid())
	{
		Description = SelectedParticipant->GetAssetDescription(*InAsset);
	}

	return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
		.Padding(FMargin(16.0f, 4.0f))
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SGorgeousAssetIcon)
				.Asset(InAsset)
				.Participant(SelectedParticipant)
				.IconSize(FVector2D(24.0f, 24.0f))
			]


			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(12.0f, 0.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(InAsset.IsValid() ? FText::FromName(InAsset->AssetName) : FText::GetEmpty())
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(Description)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				]
			]
		];
}

void SGorgeousLibraryView::OnAssetDoubleClicked(TSharedPtr<FAssetData> InAsset, FName CategoryId)
{
	if (!InAsset.IsValid() || !SelectedParticipant.IsValid())
	{
		return;
	}

	// Find the category descriptor
	const TArray<FGorgeousLibraryCategoryDescriptor> Categories = SelectedParticipant->GetLibraryCategories();
	const FGorgeousLibraryCategoryDescriptor* Category = Categories.FindByPredicate([CategoryId](const FGorgeousLibraryCategoryDescriptor& Desc)
	{
		return Desc.CategoryId == CategoryId;
	});

	if (!Category)
	{
		return;
	}

	// Delegate to participant
	SelectedParticipant->OnAssetDoubleClicked(*InAsset, *Category);

	// Fallback to legacy behavior if requested
	if (Category->bOpensSetupWizard)
	{
		OnTemplateTileDoubleClicked(InAsset);
	}
}

TSharedPtr<SWidget> SGorgeousLibraryView::OnContextMenuOpening(FName CategoryId)
{
	if (!SelectedParticipant.IsValid())
	{
		return nullptr;
	}

	const TArray<FGorgeousLibraryCategoryDescriptor> Categories = SelectedParticipant->GetLibraryCategories();
	const FGorgeousLibraryCategoryDescriptor* Category = Categories.FindByPredicate([CategoryId](const FGorgeousLibraryCategoryDescriptor& Desc)
	{
		return Desc.CategoryId == CategoryId;
	});

	if (!Category)
	{
		return nullptr;
	}

	// Get selected assets from either tile or list view
	TArray<TSharedPtr<FAssetData>> SelectedAssets;
	if (Category->ViewMode == EGorgeousLibraryViewMode::List)
	{
		if (TSharedPtr<SListView<TSharedPtr<FAssetData>>>* ListViewPtr = CategoryListViews.Find(CategoryId))
		{
			(*ListViewPtr)->GetSelectedItems(SelectedAssets);
		}
	}
	else
	{
		if (TSharedPtr<STileView<TSharedPtr<FAssetData>>>* TileViewPtr = CategoryTileViews.Find(CategoryId))
		{
			(*TileViewPtr)->GetSelectedItems(SelectedAssets);
		}
	}

	if (SelectedAssets.Num() == 0)
	{
		return nullptr;
	}

	TArray<FAssetData> RawSelected;
	for (const TSharedPtr<FAssetData>& AssetPtr : SelectedAssets) { RawSelected.Add(*AssetPtr); }

	FMenuBuilder MenuBuilder(true, nullptr);
	SelectedParticipant->OnBuildContextMenu(MenuBuilder, RawSelected, *Category);

	return MenuBuilder.MakeWidget();
}


void SGorgeousLibraryView::OnTemplateTileDoubleClicked(TSharedPtr<FAssetData> InAsset)
{
	if (!InAsset.IsValid())
	{
		return;
	}

	// Load the template data asset
	UGorgeousSystemTemplate_DA* Template = Cast<UGorgeousSystemTemplate_DA>(InAsset->GetAsset());
	if (!IsValid(Template))
	{
		return;
	}

	// Spawn the wizard in its own modal window
	const TSharedRef<SWindow> WizardWindow = SNew(SWindow)
		.Title(NSLOCTEXT("GorgeousCore", "SetupWizardTitle", "Gorgeous Setup Wizard"))
		.ClientSize(FVector2D(580.0f, 640.0f))
		.SizingRule(ESizingRule::FixedSize)
		.IsTopmostWindow(false)
		[
			SNew(SGorgeousSetupWizard)
			.SystemTemplate(Template)
		];

	FSlateApplication::Get().AddWindow(WizardWindow);
}

//-----------------------------------------------------------------------------
// Search & Filtering
//-----------------------------------------------------------------------------

void SGorgeousLibraryView::OnSearchTextChanged(const FText& NewText, FName CategoryId)
{
	CategorySearchText.Add(CategoryId, NewText.ToString().ToLower());

	const TArray<TSharedPtr<FAssetData>>* AllPtr = CategoryAssets.Find(CategoryId);
	if (!AllPtr || !SelectedParticipant.IsValid())
	{
		return;
	}

	TArray<TSharedPtr<FAssetData>>& Filtered = FilteredCategoryAssets.FindOrAdd(CategoryId);
	Filtered.Empty();

	const FString& SearchStr = CategorySearchText.FindRef(CategoryId);
	TSharedPtr<TFilterCollection<TSharedPtr<FAssetData>>> FilterCollection = CategoryFilterCollections.FindRef(CategoryId);

	for (const TSharedPtr<FAssetData>& Asset : *AllPtr)
	{
		// 1. Text Search Filter
		if (!SearchStr.IsEmpty() && !Asset->AssetName.ToString().ToLower().Contains(SearchStr))
		{
			continue;
		}

		// 2. Native Filter Bar
		if (FilterCollection.IsValid() && !FilterCollection->PassesAllFilters(Asset))
		{
			continue;
		}

		Filtered.Add(Asset);
	}

	// Refresh the relevant view
	if (TSharedPtr<STileView<TSharedPtr<FAssetData>>>* TileViewPtr = CategoryTileViews.Find(CategoryId))
	{
		if (TileViewPtr->IsValid()) { (*TileViewPtr)->RequestListRefresh(); }
	}
	if (TSharedPtr<SListView<TSharedPtr<FAssetData>>>* ListViewPtr = CategoryListViews.Find(CategoryId))
	{
		if (ListViewPtr->IsValid()) { (*ListViewPtr)->RequestListRefresh(); }
	}
}

void SGorgeousLibraryView::RefreshAssetsForParticipant(TSharedPtr<IGorgeousLibraryParticipant> Participant)
{
	if (!Participant)
	{
		return;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	for (const FGorgeousLibraryCategoryDescriptor& Category : Participant->GetLibraryCategories())
	{
		TArray<FAssetData> RawAssets;
		AssetRegistry.GetAssetsByClass(Category.AssetClassPath, RawAssets, /* bSearchSubClasses = */ true);

		TArray<TSharedPtr<FAssetData>>& CachedAll      = CategoryAssets.FindOrAdd(Category.CategoryId);
		TArray<TSharedPtr<FAssetData>>& CachedFiltered = FilteredCategoryAssets.FindOrAdd(Category.CategoryId);

		CachedAll.Reset(RawAssets.Num());
		for (FAssetData& RawAsset : RawAssets)
		{
			CachedAll.Add(MakeShared<FAssetData>(MoveTemp(RawAsset)));
		}

		// Apply initial filters
		OnSearchTextChanged(FText::GetEmpty(), Category.CategoryId);
	}
}
