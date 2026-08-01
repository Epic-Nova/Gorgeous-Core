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
#include "DataSchemaMapping/GorgeousDataSchemaMappingEditorPanel.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "DataSchemaMapping/GorgeousDataSchemaMapping_DA.h"
#include "DataSchemaMapping/GorgeousDataSchemaMappingTypes.h"
#include "Helpers/GorgeousPropertyPathHelper.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "ContentBrowserModule.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "EdGraphSchema_K2.h"
#include "Engine/DataTable.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Application/SlateApplication.h"
#include "IContentBrowserSingleton.h"
#include "IDetailsView.h"
#include "Math/UnrealMathUtility.h"
#include "Modules/ModuleManager.h"
#include "Styling/AppStyle.h"
#include "UObject/UnrealType.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SNullWidget.h"
//<-------------------------------------------------------------------------->

#define LOCTEXT_NAMESPACE "GorgeousDataSchemaMappingEditorPanel"

namespace
{
	constexpr float SchemaSettingsMinExpandedHeightPx = 220.0f;
	constexpr float SchemaSettingsCollapsedHeightPx = 34.0f;
	constexpr float SchemaSettingsMaxExpandedHeightPx = 520.0f;
	constexpr float SchemaSettingsMaximumExpandedSize = 0.75f;

	FString GetLeafPathSegment(const FString& PropertyPath)
	{
		FString LeafSegment = PropertyPath;
		int32 LastDotIndex = INDEX_NONE;
		if (PropertyPath.FindLastChar(TEXT('.'), LastDotIndex))
		{
			LeafSegment = PropertyPath.Mid(LastDotIndex + 1);
		}

		return GorgeousPropertyPath::StripGeneratedStructSuffix(LeafSegment);
	}

	bool IsNumericProperty(const FProperty* Property)
	{
		return CastField<FNumericProperty>(Property) != nullptr;
	}

	bool IsStringFamilyProperty(const FProperty* Property)
	{
		return CastField<FStrProperty>(Property) != nullptr
			|| CastField<FNameProperty>(Property) != nullptr
			|| CastField<FTextProperty>(Property) != nullptr;
	}

	bool IsEnumProperty(const FProperty* Property)
	{
		if (CastField<FEnumProperty>(Property) != nullptr)
		{
			return true;
		}

		if (const FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
		{
			return ByteProperty->Enum != nullptr;
		}

		return false;
	}

	bool IsObjectLikeProperty(const FProperty* Property)
	{
		return CastField<FObjectPropertyBase>(Property) != nullptr
			|| CastField<FSoftObjectProperty>(Property) != nullptr
			|| CastField<FClassProperty>(Property) != nullptr
			|| CastField<FSoftClassProperty>(Property) != nullptr;
	}

	FLinearColor GetPropertyTypeTint(const FProperty* Property)
	{
		constexpr FLinearColor NullPropertyFallbackColor(0.48f, 0.48f, 0.48f, 1.0f);
		constexpr FLinearColor UnsupportedTypeFallbackColor(0.52f, 0.52f, 0.52f, 1.0f);

		if (!Property)
		{
			return NullPropertyFallbackColor;
		}

		if (const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>())
		{
			FEdGraphPinType PinType;
			if (Schema->ConvertPropertyToPinType(Property, PinType))
			{
				return Schema->GetPinTypeColor(PinType);
			}
		}

		return UnsupportedTypeFallbackColor;
	}

	FLinearColor MakeTypeBackgroundTint(const FLinearColor& TypeColor, const float Opacity)
	{
		return FLinearColor(TypeColor.R, TypeColor.G, TypeColor.B, Opacity);
	}

	FLinearColor MakeReadableTypeColor(const FLinearColor& TypeColor, const float BrightenFactor)
	{
		const float ClampedBrightenFactor = FMath::Clamp(BrightenFactor, 0.0f, 1.0f);
		return FLinearColor(
			FMath::Lerp(TypeColor.R, 1.0f, ClampedBrightenFactor),
			FMath::Lerp(TypeColor.G, 1.0f, ClampedBrightenFactor),
			FMath::Lerp(TypeColor.B, 1.0f, ClampedBrightenFactor),
			TypeColor.A);
	}

	FSlateColor MakeReadableTextColorForBackground(const FLinearColor& BackgroundColor)
	{
		const float RelativeLuminance = (0.2126f * BackgroundColor.R) + (0.7152f * BackgroundColor.G) + (0.0722f * BackgroundColor.B);
		return RelativeLuminance >= 0.56f
			? FSlateColor(FLinearColor(0.06f, 0.06f, 0.06f, 1.0f))
			: FSlateColor(FLinearColor(0.95f, 0.95f, 0.95f, 1.0f));
	}

	const FSlateBrush* GetTypePillBrush()
	{
		static const FSlateRoundedBoxBrush TypePillBrush(FLinearColor::White, 8.0f);
		return &TypePillBrush;
	}

	const FSlateBrush* GetTypeStripBrush()
	{
		static const FSlateRoundedBoxBrush TypeStripBrush(FLinearColor::White, 2.0f);
		return &TypeStripBrush;
	}

	bool UsesTransformParameterA(const EGorgeousDataSchemaTransformKind_E TransformKind)
	{
		switch (TransformKind)
		{
		case EGorgeousDataSchemaTransformKind_E::Transform_SetLiteral:
		case EGorgeousDataSchemaTransformKind_E::Transform_UseFallbackLiteralIfEmpty:
		case EGorgeousDataSchemaTransformKind_E::Transform_AddPrefix:
		case EGorgeousDataSchemaTransformKind_E::Transform_AddSuffix:
		case EGorgeousDataSchemaTransformKind_E::Transform_ReplaceText:
		case EGorgeousDataSchemaTransformKind_E::Transform_MultiplyNumeric:
		case EGorgeousDataSchemaTransformKind_E::Transform_AddNumericOffset:
			return true;
		default:
			return false;
		}
	}

	bool UsesTransformParameterB(const EGorgeousDataSchemaTransformKind_E TransformKind)
	{
		return TransformKind == EGorgeousDataSchemaTransformKind_E::Transform_ReplaceText;
	}
}

void SGorgeousDataSchemaMappingEditorPanel::Construct(const FArguments& InArgs)
{
	SchemaMapAsset = InArgs._SchemaMapAsset;
	DetailsView = InArgs._DetailsView;

	if (DetailsView.IsValid())
	{
		DetailsView->OnFinishedChangingProperties().AddSP(this, &SGorgeousDataSchemaMappingEditorPanel::HandleDetailsPropertiesChanged);
	}

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(6.0f, 6.0f, 6.0f, 4.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(6.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("TargetRootPathLabel", "Target Root Path"))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(SEditableTextBox)
					.Text(this, &SGorgeousDataSchemaMappingEditorPanel::GetTargetRootPathText)
					.OnTextCommitted(this, &SGorgeousDataSchemaMappingEditorPanel::HandleTargetRootPathCommitted)
					.HintText(LOCTEXT("TargetRootPathHint", "/Game/MySystem/Migrated"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SAssignNew(BrowseTargetRootPathButton, SButton)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.ToolTipText(LOCTEXT("BrowsePathTooltip", "Pick a content path from your project."))
					.OnClicked(this, &SGorgeousDataSchemaMappingEditorPanel::HandleBrowseTargetRootPathClicked)
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("Icons.FolderOpen"))
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("ReloadCatalogButton", "Reload Paths"))
					.ToolTipText(LOCTEXT("ReloadCatalogTooltip", "Rebuild source and target path catalogs from current schema settings."))
					.OnClicked_Lambda([this]()
					{
						RefreshPathCatalogs();
						return FReply::Handled();
					})
				]
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(6.0f, 0.0f)
		[
			SAssignNew(MainVerticalSplitter, SSplitter)
			.Orientation(Orient_Vertical)
			+ SSplitter::Slot()
			.Value(0.75f)
			[
				SNew(SSplitter)
				+ SSplitter::Slot()
				.Value(0.26f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(6.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("SourceCatalogTitle", "Source Fields"))
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.Padding(0.0f, 4.0f, 0.0f, 0.0f)
						[
							SAssignNew(SourceCatalogContainer, SBox)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("SourceSchemaUnavailable", "Source schema is unavailable."))
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 6.0f, 0.0f, 0.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SGorgeousDataSchemaMappingEditorPanel::GetSourceRowsDiagnosticVisibility)
							.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
							.BorderBackgroundColor(FLinearColor(0.05f, 0.20f, 0.24f, 1.0f))
							.Padding(8.0f)
							[
								SNew(STextBlock)
								.Text(this, &SGorgeousDataSchemaMappingEditorPanel::GetSourceRowsDiagnosticText)
								.AutoWrapText(true)
							]
						]
					]
				]
				+ SSplitter::Slot()
				.Value(0.48f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(6.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("MappingsTitle", "Field Mappings"))
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0.0f, 0.0f, 4.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("AutoMatchButton", "Auto Match Fields"))
								.OnClicked(this, &SGorgeousDataSchemaMappingEditorPanel::HandleAddSelectedMappingRow)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
								.Text(LOCTEXT("AddEmptyButton", "Add Empty Row"))
								.OnClicked(this, &SGorgeousDataSchemaMappingEditorPanel::HandleAddEmptyMappingRow)
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("MappingEditorHint", "Each row defines Source → Target. Transform settings are edited directly below each row."))
							.ColorAndOpacity(FLinearColor(0.62f, 0.62f, 0.62f, 1.0f))
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.Padding(0.0f, 6.0f, 0.0f, 0.0f)
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
							.Padding(6.0f)
							[
								SNew(SScrollBox)
								+ SScrollBox::Slot()
								[
									SAssignNew(MappingRowsContainer, SVerticalBox)
								]
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 6.0f, 0.0f, 0.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SGorgeousDataSchemaMappingEditorPanel::GetMappingDiagnosticsVisibility)
							.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
							.BorderBackgroundColor(FLinearColor(0.24f, 0.20f, 0.05f, 1.0f))
							.Padding(8.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Top)
								.Padding(0.0f, 0.0f, 6.0f, 0.0f)
								[
									SNew(SImage)
									.Image(FAppStyle::GetBrush("Icons.Warning"))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(this, &SGorgeousDataSchemaMappingEditorPanel::GetMappingDiagnosticsText)
									.AutoWrapText(true)
								]
							]
						]
					]
				]
				+ SSplitter::Slot()
				.Value(0.26f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(6.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("TargetCatalogTitle", "Target Fields"))
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.Padding(0.0f, 4.0f, 0.0f, 0.0f)
						[
							SAssignNew(TargetCatalogContainer, SBox)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("TargetSchemaUnavailable", "Target schema is unavailable."))
							]
						]
					]
				]
			]
			+ SSplitter::Slot()
			.Value(this, &SGorgeousDataSchemaMappingEditorPanel::GetSchemaSettingsSlotSize)
			.OnSlotResized(this, &SGorgeousDataSchemaMappingEditorPanel::HandleSchemaSettingsSlotResized)
			[
				SNew(SBorder)
				.Padding(0.0f, 4.0f, 0.0f, 0.0f)
				[
					SAssignNew(SchemaSettingsArea, SExpandableArea)
					.InitiallyCollapsed(true)
					.MaxHeight(SchemaSettingsMaxExpandedHeightPx)
					.OnAreaExpansionChanged(this, &SGorgeousDataSchemaMappingEditorPanel::HandleSchemaSettingsExpansionChanged)
					.HeaderContent()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("AdvancedSettingsTitle", "Schema Settings"))
					]
					.BodyContent()
					[
						DetailsView.IsValid()
							? StaticCastSharedRef<SWidget>(DetailsView.ToSharedRef())
							: SNullWidget::NullWidget
					]
				]
			]
		]
	];

	RefreshPathCatalogs();
	RebuildMappingRows();
}

void SGorgeousDataSchemaMappingEditorPanel::RefreshPathCatalogs()
{
	TArray<FString> SourcePaths;
	TArray<FString> TargetPaths;
	const UStruct* SourceRootStruct = ResolveSourceRootStruct();
	const UStruct* TargetRootStruct = ResolveTargetRootStruct();

	if (SourceRootStruct)
	{
		TSet<const UStruct*> VisitedStructs;
		CollectStructPropertyPaths(SourceRootStruct, FString(), 0, VisitedStructs, SourcePaths);
	}

	if (TargetRootStruct)
	{
		TSet<const UStruct*> VisitedStructs;
		CollectStructPropertyPaths(TargetRootStruct, FString(), 0, VisitedStructs, TargetPaths);
	}

	SourcePaths.Sort();
	TargetPaths.Sort();

	CachedSourcePaths = MoveTemp(SourcePaths);
	CachedTargetPaths = MoveTemp(TargetPaths);

	RefreshSchemaDetailViews(SourceRootStruct, TargetRootStruct);
	RebuildMappingDiagnostics();
	RebuildMappingRows();
}

void SGorgeousDataSchemaMappingEditorPanel::RebuildMappingRows()
{
	if (!MappingRowsContainer.IsValid())
	{
		return;
	}

	MappingRowsContainer->ClearChildren();

	if (!SchemaMapAsset)
	{
		MappingRowsContainer->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("MappingRows_NoAsset", "Schema map asset is unavailable."))
		];
		return;
	}

	if (SchemaMapAsset->FieldMappings.IsEmpty())
	{
		MappingRowsContainer->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("MappingRows_Empty", "No mapping rows yet. Add a row or use Auto Match Fields."))
		];
		return;
	}

	for (int32 MappingIndex = 0; MappingIndex < SchemaMapAsset->FieldMappings.Num(); ++MappingIndex)
	{
		MappingRowsContainer->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 6.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(8.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(0.46f)
					.Padding(0.0f, 0.0f, 6.0f, 0.0f)
					[
						BuildVariableSelectorWidget(MappingIndex, true)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 6.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("MappingArrowText", "→"))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.46f)
					.Padding(0.0f, 0.0f, 6.0f, 0.0f)
					[
						BuildVariableSelectorWidget(MappingIndex, false)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("RemoveMappingRowButton", "Remove"))
						.OnClicked(this, &SGorgeousDataSchemaMappingEditorPanel::HandleRemoveMappingRow, MappingIndex)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 8.0f, 0.0f, 0.0f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(8.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(0.0f, 0.0f, 12.0f, 0.0f)
							[
								SNew(SCheckBox)
								.IsChecked_Lambda([this, MappingIndex]()
								{
									return GetMappingRequiredState(MappingIndex);
								})
								.OnCheckStateChanged_Lambda([this, MappingIndex](const ECheckBoxState NewState)
								{
									SetMappingRequiredState(MappingIndex, NewState);
								})
								[
									SNew(STextBlock)
									.Text(LOCTEXT("MappingRequiredLabel", "Required"))
								]
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(0.0f, 0.0f, 8.0f, 0.0f)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("TransformKindLabel", "Transform"))
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								SNew(SComboButton)
								.ContentPadding(FMargin(8.0f, 2.0f))
								.OnGetMenuContent(this, &SGorgeousDataSchemaMappingEditorPanel::BuildTransformKindMenu, MappingIndex)
								.ButtonContent()
								[
									SNew(STextBlock)
									.Text_Lambda([this, MappingIndex]()
									{
										return GetTransformKindText(MappingIndex);
									})
								]
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 6.0f, 0.0f, 0.0f)
						[
							SNew(SHorizontalBox)
							.Visibility_Lambda([this, MappingIndex]()
							{
								return GetTransformParameterSectionVisibility(MappingIndex);
							})
							+ SHorizontalBox::Slot()
							.FillWidth(0.5f)
							.Padding(0.0f, 0.0f, 4.0f, 0.0f)
							[
								SNew(SVerticalBox)
								.Visibility_Lambda([this, MappingIndex]()
								{
									return GetTransformParameterAVisibility(MappingIndex);
								})
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
									.Text_Lambda([this, MappingIndex]()
									{
										return GetTransformParameterALabel(MappingIndex);
									})
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f, 0.0f, 0.0f)
								[
									SNew(SEditableTextBox)
									.Text_Lambda([this, MappingIndex]()
									{
										return GetTransformParameterAText(MappingIndex);
									})
									.OnTextCommitted_Lambda([this, MappingIndex](const FText& NewText, const ETextCommit::Type CommitType)
									{
										HandleTransformParameterACommitted(MappingIndex, NewText, CommitType);
									})
								]
							]
							+ SHorizontalBox::Slot()
							.FillWidth(0.5f)
							.Padding(4.0f, 0.0f, 0.0f, 0.0f)
							[
								SNew(SVerticalBox)
								.Visibility_Lambda([this, MappingIndex]()
								{
									return GetTransformParameterBVisibility(MappingIndex);
								})
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
									.Text_Lambda([this, MappingIndex]()
									{
										return GetTransformParameterBLabel(MappingIndex);
									})
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f, 0.0f, 0.0f)
								[
									SNew(SEditableTextBox)
									.Text_Lambda([this, MappingIndex]()
									{
										return GetTransformParameterBText(MappingIndex);
									})
									.OnTextCommitted_Lambda([this, MappingIndex](const FText& NewText, const ETextCommit::Type CommitType)
									{
										HandleTransformParameterBCommitted(MappingIndex, NewText, CommitType);
									})
								]
							]
						]
					]
				]
			]
		];
	}
}

TSharedRef<SWidget> SGorgeousDataSchemaMappingEditorPanel::BuildVariableSelectorWidget(const int32 MappingIndex, const bool bSourcePath)
{
	return SNew(SComboButton)
		.ContentPadding(FMargin(0.0f))
		.OnGetMenuContent(this, &SGorgeousDataSchemaMappingEditorPanel::BuildVariablePickerMenu, MappingIndex, bSourcePath)
		.ButtonContent()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor_Lambda([this, MappingIndex, bSourcePath]()
			{
				const FLinearColor TypeTint = MakeReadableTypeColor(GetMappingVariableTypeTint(MappingIndex, bSourcePath), 0.20f);
				return MakeTypeBackgroundTint(TypeTint, 0.26f);
			})
			.Padding(6.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Fill)
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(4.0f)
					[
						SNew(SBorder)
						.BorderImage(GetTypeStripBrush())
						.BorderBackgroundColor_Lambda([this, MappingIndex, bSourcePath]()
						{
							const FLinearColor TypeTint = MakeReadableTypeColor(GetMappingVariableTypeTint(MappingIndex, bSourcePath), 0.14f);
							return MakeTypeBackgroundTint(TypeTint, 0.98f);
						})
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(this, &SGorgeousDataSchemaMappingEditorPanel::GetMappingVariablePathText, MappingIndex, bSourcePath)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBorder)
					.BorderImage(GetTypePillBrush())
					.BorderBackgroundColor_Lambda([this, MappingIndex, bSourcePath]()
					{
						const FLinearColor TypeTint = MakeReadableTypeColor(GetMappingVariableTypeTint(MappingIndex, bSourcePath), 0.26f);
						return MakeTypeBackgroundTint(TypeTint, 0.82f);
					})
					.Padding(FMargin(8.0f, 2.0f))
					[
						SNew(STextBlock)
						.Text(this, &SGorgeousDataSchemaMappingEditorPanel::GetMappingVariableTypeText, MappingIndex, bSourcePath)
						.ColorAndOpacity_Lambda([this, MappingIndex, bSourcePath]()
						{
							const FLinearColor TypeTint = MakeReadableTypeColor(GetMappingVariableTypeTint(MappingIndex, bSourcePath), 0.26f);
							const FLinearColor PillBackground = MakeTypeBackgroundTint(TypeTint, 0.82f);
							return MakeReadableTextColorForBackground(PillBackground);
						})
					]
				]
			]
		];
}

TSharedRef<SWidget> SGorgeousDataSchemaMappingEditorPanel::BuildVariablePickerMenu(const int32 MappingIndex, const bool bSourcePath)
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("ClearMappingVariableEntry", "Clear Selection"),
		LOCTEXT("ClearMappingVariableEntryTooltip", "Clear the selected variable path for this side of the mapping row."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this, MappingIndex, bSourcePath]()
		{
			SetMappingVariablePath(MappingIndex, bSourcePath, FString());
		})));

	const TArray<FString>& Paths = bSourcePath ? CachedSourcePaths : CachedTargetPaths;
	for (const FString& Path : Paths)
	{
		const UStruct* RootStruct = bSourcePath ? ResolveSourceRootStruct() : ResolveTargetRootStruct();
		const FProperty* Property = nullptr;
		FString ResolveError;
		FString TypeLabel = TEXT("Unknown");
		if (RootStruct && TryResolvePropertyPath(RootStruct, Path, Property, ResolveError))
		{
			TypeLabel = GetPropertyTypeLabel(Property);
		}

		MenuBuilder.AddMenuEntry(
			FText::FromString(Path),
			FText::FromString(FString::Printf(TEXT("Type: %s"), *TypeLabel)),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, MappingIndex, bSourcePath, Path]()
			{
				SetMappingVariablePath(MappingIndex, bSourcePath, Path);
			})));
	}

	return MenuBuilder.MakeWidget();
}

FText SGorgeousDataSchemaMappingEditorPanel::GetMappingVariablePathText(const int32 MappingIndex, const bool bSourcePath) const
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return FText::GetEmpty();
	}

	const FGorgeousDataSchemaFieldMapping_S& Mapping = SchemaMapAsset->FieldMappings[MappingIndex];
	const FString& Path = bSourcePath ? Mapping.SourcePath : Mapping.TargetPath;
	if (Path.IsEmpty())
	{
		return bSourcePath
			? LOCTEXT("SelectSourceVariablePrompt", "Select Source Variable")
			: LOCTEXT("SelectTargetVariablePrompt", "Select Target Variable");
	}

	return FText::FromString(Path);
}

FText SGorgeousDataSchemaMappingEditorPanel::GetMappingVariableTypeText(const int32 MappingIndex, const bool bSourcePath) const
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return LOCTEXT("UnknownTypeShort", "Unknown");
	}

	const FGorgeousDataSchemaFieldMapping_S& Mapping = SchemaMapAsset->FieldMappings[MappingIndex];
	const FString& Path = bSourcePath ? Mapping.SourcePath : Mapping.TargetPath;
	if (Path.IsEmpty())
	{
		return LOCTEXT("UnmappedTypeShort", "Unmapped");
	}

	const UStruct* RootStruct = bSourcePath ? ResolveSourceRootStruct() : ResolveTargetRootStruct();
	if (!RootStruct)
	{
		return LOCTEXT("UnknownTypeShort", "Unknown");
	}

	const FProperty* Property = nullptr;
	FString ResolveError;
	if (!TryResolvePropertyPath(RootStruct, Path, Property, ResolveError))
	{
		return LOCTEXT("InvalidTypeShort", "Invalid");
	}

	return FText::FromString(GetPropertyTypeLabel(Property));
}

FLinearColor SGorgeousDataSchemaMappingEditorPanel::GetMappingVariableTypeTint(const int32 MappingIndex, const bool bSourcePath) const
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return FLinearColor(0.48f, 0.48f, 0.48f, 1.0f);
	}

	const FGorgeousDataSchemaFieldMapping_S& Mapping = SchemaMapAsset->FieldMappings[MappingIndex];
	const FString& Path = bSourcePath ? Mapping.SourcePath : Mapping.TargetPath;
	if (Path.IsEmpty())
	{
		return FLinearColor(0.40f, 0.40f, 0.40f, 1.0f);
	}

	const UStruct* RootStruct = bSourcePath ? ResolveSourceRootStruct() : ResolveTargetRootStruct();
	if (!RootStruct)
	{
		return FLinearColor(0.48f, 0.48f, 0.48f, 1.0f);
	}

	const FProperty* Property = nullptr;
	FString ResolveError;
	if (!TryResolvePropertyPath(RootStruct, Path, Property, ResolveError))
	{
		return FLinearColor(0.48f, 0.24f, 0.24f, 1.0f);
	}

	return GetPropertyTypeTint(Property);
}

FReply SGorgeousDataSchemaMappingEditorPanel::HandleRemoveMappingRow(const int32 MappingIndex)
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return FReply::Handled();
	}

	MarkSchemaMapModified();
	SchemaMapAsset->FieldMappings.RemoveAt(MappingIndex);
	RebuildMappingDiagnostics();
	RebuildMappingRows();
	return FReply::Handled();
}

void SGorgeousDataSchemaMappingEditorPanel::SetMappingVariablePath(const int32 MappingIndex, const bool bSourcePath, const FString& NewPath)
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return;
	}

	FGorgeousDataSchemaFieldMapping_S& Mapping = SchemaMapAsset->FieldMappings[MappingIndex];
	FString& PathToUpdate = bSourcePath ? Mapping.SourcePath : Mapping.TargetPath;
	if (PathToUpdate == NewPath)
	{
		return;
	}

	MarkSchemaMapModified();
	PathToUpdate = NewPath;
	RebuildMappingDiagnostics();
	RebuildMappingRows();
}

TSharedRef<SWidget> SGorgeousDataSchemaMappingEditorPanel::BuildTransformKindMenu(const int32 MappingIndex)
{
	FMenuBuilder MenuBuilder(true, nullptr);

	const UEnum* TransformEnum = StaticEnum<EGorgeousDataSchemaTransformKind_E>();
	if (!TransformEnum)
	{
		return MenuBuilder.MakeWidget();
	}

	for (int32 EnumIndex = 0; EnumIndex < TransformEnum->NumEnums(); ++EnumIndex)
	{
		if (TransformEnum->HasMetaData(TEXT("Hidden"), EnumIndex))
		{
			continue;
		}

		const FString EnumName = TransformEnum->GetNameStringByIndex(EnumIndex);
		if (EnumName.EndsWith(TEXT("_MAX")))
		{
			continue;
		}

		const int64 EnumValue = TransformEnum->GetValueByIndex(EnumIndex);
		if (EnumValue == INDEX_NONE)
		{
			continue;
		}

		const EGorgeousDataSchemaTransformKind_E TransformKind = static_cast<EGorgeousDataSchemaTransformKind_E>(EnumValue);
		MenuBuilder.AddMenuEntry(
			TransformEnum->GetDisplayNameTextByIndex(EnumIndex),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, MappingIndex, TransformKind]()
			{
				SetTransformKind(MappingIndex, TransformKind);
			})));
	}

	return MenuBuilder.MakeWidget();
}

FText SGorgeousDataSchemaMappingEditorPanel::GetTransformKindText(const int32 MappingIndex) const
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return LOCTEXT("TransformKind_None", "None");
	}

	const EGorgeousDataSchemaTransformKind_E TransformKind = SchemaMapAsset->FieldMappings[MappingIndex].TransformDefinition.TransformKind;
	if (const UEnum* TransformEnum = StaticEnum<EGorgeousDataSchemaTransformKind_E>())
	{
		return TransformEnum->GetDisplayNameTextByValue(static_cast<int64>(TransformKind));
	}

	return LOCTEXT("TransformKind_None", "None");
}

void SGorgeousDataSchemaMappingEditorPanel::SetTransformKind(const int32 MappingIndex, const EGorgeousDataSchemaTransformKind_E NewKind)
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return;
	}

	FGorgeousDataSchemaFieldMapping_S& Mapping = SchemaMapAsset->FieldMappings[MappingIndex];
	if (Mapping.TransformDefinition.TransformKind == NewKind)
	{
		return;
	}

	MarkSchemaMapModified();
	Mapping.TransformDefinition.TransformKind = NewKind;

	if (!UsesTransformParameterA(NewKind))
	{
		Mapping.TransformDefinition.ParameterA.Reset();
	}

	if (!UsesTransformParameterB(NewKind))
	{
		Mapping.TransformDefinition.ParameterB.Reset();
	}

	RebuildMappingRows();
}

ECheckBoxState SGorgeousDataSchemaMappingEditorPanel::GetMappingRequiredState(const int32 MappingIndex) const
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return ECheckBoxState::Unchecked;
	}

	return SchemaMapAsset->FieldMappings[MappingIndex].bRequired ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SGorgeousDataSchemaMappingEditorPanel::SetMappingRequiredState(const int32 MappingIndex, const ECheckBoxState NewState)
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return;
	}

	const bool bNewRequired = NewState == ECheckBoxState::Checked;
	if (SchemaMapAsset->FieldMappings[MappingIndex].bRequired == bNewRequired)
	{
		return;
	}

	MarkSchemaMapModified();
	SchemaMapAsset->FieldMappings[MappingIndex].bRequired = bNewRequired;
}

FText SGorgeousDataSchemaMappingEditorPanel::GetTransformParameterAText(const int32 MappingIndex) const
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return FText::GetEmpty();
	}

	return FText::FromString(SchemaMapAsset->FieldMappings[MappingIndex].TransformDefinition.ParameterA);
}

FText SGorgeousDataSchemaMappingEditorPanel::GetTransformParameterBText(const int32 MappingIndex) const
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return FText::GetEmpty();
	}

	return FText::FromString(SchemaMapAsset->FieldMappings[MappingIndex].TransformDefinition.ParameterB);
}

void SGorgeousDataSchemaMappingEditorPanel::HandleTransformParameterACommitted(const int32 MappingIndex, const FText& NewText, const ETextCommit::Type CommitType)
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return;
	}

	const FString NewValue = NewText.ToString();
	if (SchemaMapAsset->FieldMappings[MappingIndex].TransformDefinition.ParameterA == NewValue)
	{
		return;
	}

	MarkSchemaMapModified();
	SchemaMapAsset->FieldMappings[MappingIndex].TransformDefinition.ParameterA = NewValue;
}

void SGorgeousDataSchemaMappingEditorPanel::HandleTransformParameterBCommitted(const int32 MappingIndex, const FText& NewText, const ETextCommit::Type CommitType)
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return;
	}

	const FString NewValue = NewText.ToString();
	if (SchemaMapAsset->FieldMappings[MappingIndex].TransformDefinition.ParameterB == NewValue)
	{
		return;
	}

	MarkSchemaMapModified();
	SchemaMapAsset->FieldMappings[MappingIndex].TransformDefinition.ParameterB = NewValue;
}

FText SGorgeousDataSchemaMappingEditorPanel::GetTransformParameterALabel(const int32 MappingIndex) const
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return LOCTEXT("TransformParamADefault", "Value");
	}

	switch (SchemaMapAsset->FieldMappings[MappingIndex].TransformDefinition.TransformKind)
	{
	case EGorgeousDataSchemaTransformKind_E::Transform_SetLiteral:
		return LOCTEXT("TransformParamA_Literal", "Set To");
	case EGorgeousDataSchemaTransformKind_E::Transform_UseFallbackLiteralIfEmpty:
		return LOCTEXT("TransformParamA_FallbackLiteral", "Fallback If Empty");
	case EGorgeousDataSchemaTransformKind_E::Transform_AddPrefix:
		return LOCTEXT("TransformParamA_Prefix", "Prefix Text");
	case EGorgeousDataSchemaTransformKind_E::Transform_AddSuffix:
		return LOCTEXT("TransformParamA_Suffix", "Suffix Text");
	case EGorgeousDataSchemaTransformKind_E::Transform_ReplaceText:
		return LOCTEXT("TransformParamA_Search", "Search For");
	case EGorgeousDataSchemaTransformKind_E::Transform_MultiplyNumeric:
		return LOCTEXT("TransformParamA_Factor", "Multiply By");
	case EGorgeousDataSchemaTransformKind_E::Transform_AddNumericOffset:
		return LOCTEXT("TransformParamA_Offset", "Add Offset");
	default:
		return LOCTEXT("TransformParamADefault", "Value");
	}
}

FText SGorgeousDataSchemaMappingEditorPanel::GetTransformParameterBLabel(const int32 MappingIndex) const
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return LOCTEXT("TransformParamBDefault", "Secondary Value");
	}

	if (SchemaMapAsset->FieldMappings[MappingIndex].TransformDefinition.TransformKind == EGorgeousDataSchemaTransformKind_E::Transform_ReplaceText)
	{
		return LOCTEXT("TransformParamB_Replace", "Replace With");
	}

	return LOCTEXT("TransformParamBDefault", "Secondary Value");
}

bool SGorgeousDataSchemaMappingEditorPanel::IsTransformParameterAEnabled(const int32 MappingIndex) const
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return false;
	}

	return UsesTransformParameterA(SchemaMapAsset->FieldMappings[MappingIndex].TransformDefinition.TransformKind);
}

bool SGorgeousDataSchemaMappingEditorPanel::IsTransformParameterBEnabled(const int32 MappingIndex) const
{
	if (!SchemaMapAsset || !SchemaMapAsset->FieldMappings.IsValidIndex(MappingIndex))
	{
		return false;
	}

	return UsesTransformParameterB(SchemaMapAsset->FieldMappings[MappingIndex].TransformDefinition.TransformKind);
}

EVisibility SGorgeousDataSchemaMappingEditorPanel::GetTransformParameterAVisibility(const int32 MappingIndex) const
{
	return IsTransformParameterAEnabled(MappingIndex) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SGorgeousDataSchemaMappingEditorPanel::GetTransformParameterBVisibility(const int32 MappingIndex) const
{
	return IsTransformParameterBEnabled(MappingIndex) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SGorgeousDataSchemaMappingEditorPanel::GetTransformParameterSectionVisibility(const int32 MappingIndex) const
{
	return (IsTransformParameterAEnabled(MappingIndex) || IsTransformParameterBEnabled(MappingIndex))
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

void SGorgeousDataSchemaMappingEditorPanel::RebuildMappingDiagnostics()
{
	MappingDiagnostics.Reset();

	if (!SchemaMapAsset)
	{
		return;
	}

	const UStruct* SourceRootStruct = ResolveSourceRootStruct();
	const UStruct* TargetRootStruct = ResolveTargetRootStruct();
	if (!SourceRootStruct || !TargetRootStruct)
	{
		return;
	}

	TMap<FString, int32> TargetPathToFirstRow;

	for (int32 MappingIndex = 0; MappingIndex < SchemaMapAsset->FieldMappings.Num(); ++MappingIndex)
	{
		const FGorgeousDataSchemaFieldMapping_S& Mapping = SchemaMapAsset->FieldMappings[MappingIndex];
		if (Mapping.SourcePath.IsEmpty() || Mapping.TargetPath.IsEmpty())
		{
			continue;
		}

		const FProperty* SourceProperty = nullptr;
		const FProperty* TargetProperty = nullptr;
		FString ResolveError;

		if (!TryResolvePropertyPath(SourceRootStruct, Mapping.SourcePath, SourceProperty, ResolveError))
		{
			MappingDiagnostics.Add(FText::Format(
				LOCTEXT("MappingDiagInvalidSourceFmt", "Row {0}: Source path '{1}' is invalid ({2})."),
				FText::AsNumber(MappingIndex + 1),
				FText::FromString(Mapping.SourcePath),
				FText::FromString(ResolveError)));
			continue;
		}

		if (!TryResolvePropertyPath(TargetRootStruct, Mapping.TargetPath, TargetProperty, ResolveError))
		{
			MappingDiagnostics.Add(FText::Format(
				LOCTEXT("MappingDiagInvalidTargetFmt", "Row {0}: Target path '{1}' is invalid ({2})."),
				FText::AsNumber(MappingIndex + 1),
				FText::FromString(Mapping.TargetPath),
				FText::FromString(ResolveError)));
			continue;
		}

		if (!ArePropertiesCompatible(SourceProperty, TargetProperty))
		{
			MappingDiagnostics.Add(FText::Format(
				LOCTEXT("MappingDiagIncompatibleFmt", "Row {0}: Source '{1}' ({2}) is incompatible with target '{3}' ({4})."),
				FText::AsNumber(MappingIndex + 1),
				FText::FromString(Mapping.SourcePath),
				FText::FromString(GetPropertyTypeLabel(SourceProperty)),
				FText::FromString(Mapping.TargetPath),
				FText::FromString(GetPropertyTypeLabel(TargetProperty))));
		}

		const FString TargetPathKey = Mapping.TargetPath.ToLower();
		if (const int32* FirstRow = TargetPathToFirstRow.Find(TargetPathKey))
		{
			MappingDiagnostics.Add(FText::Format(
				LOCTEXT("MappingDiagDuplicateTargetFmt", "Rows {0} and {1} both map to target '{2}'."),
				FText::AsNumber(*FirstRow + 1),
				FText::AsNumber(MappingIndex + 1),
				FText::FromString(Mapping.TargetPath)));
		}
		else
		{
			TargetPathToFirstRow.Add(TargetPathKey, MappingIndex);
		}
	}
}

void SGorgeousDataSchemaMappingEditorPanel::RefreshSchemaDetailViews(const UStruct* SourceRootStruct, const UStruct* TargetRootStruct)
{
	if (SourceCatalogContainer.IsValid())
	{
		if (SourceRootStruct)
		{
			SourceCatalogContainer->SetContent(BuildCatalogListWidget(
				CachedSourcePaths,
				SourceRootStruct,
				LOCTEXT("SourceFieldsEmpty", "No source fields available for the selected source type.")));
		}
		else
		{
			SourceCatalogContainer->SetContent(
				SNew(STextBlock)
				.Text(LOCTEXT("SourceSchemaUnavailable", "Source schema is unavailable.")));
		}
	}

	if (TargetCatalogContainer.IsValid())
	{
		if (TargetRootStruct)
		{
			TargetCatalogContainer->SetContent(BuildCatalogListWidget(
				CachedTargetPaths,
				TargetRootStruct,
				LOCTEXT("TargetFieldsEmpty", "No target fields available for the selected target type.")));
		}
		else
		{
			TargetCatalogContainer->SetContent(
				SNew(STextBlock)
				.Text(LOCTEXT("TargetSchemaUnavailable", "Target schema is unavailable.")));
		}
	}
}

TSharedRef<SWidget> SGorgeousDataSchemaMappingEditorPanel::BuildCatalogListWidget(const TArray<FString>& Paths, const UStruct* RootStruct, const FText& EmptyText) const
{
	if (!RootStruct)
	{
		return SNew(STextBlock)
			.Text(LOCTEXT("CatalogRootMissing", "Schema root type is unavailable."));
	}

	if (Paths.IsEmpty())
	{
		return SNew(STextBlock)
			.Text(EmptyText);
	}

	TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);

	for (const FString& VariablePath : Paths)
	{
		Rows->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 4.0f)
		[
			BuildReadOnlyCatalogVariableRow(VariablePath, RootStruct)
		];
	}

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			Rows
		];
}

TSharedRef<SWidget> SGorgeousDataSchemaMappingEditorPanel::BuildReadOnlyCatalogVariableRow(const FString& VariablePath, const UStruct* RootStruct) const
{
	const FLinearColor TypeTint = GetCatalogVariableTypeTint(VariablePath, RootStruct);

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(MakeTypeBackgroundTint(TypeTint, 0.15f))
		.Padding(6.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(VariablePath))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(GetTypePillBrush())
				.BorderBackgroundColor(MakeTypeBackgroundTint(MakeReadableTypeColor(TypeTint, 0.24f), 0.78f))
				.Padding(FMargin(8.0f, 2.0f))
				[
					SNew(STextBlock)
					.Text(GetCatalogVariableTypeText(VariablePath, RootStruct))
					.ColorAndOpacity(MakeReadableTextColorForBackground(MakeTypeBackgroundTint(MakeReadableTypeColor(TypeTint, 0.24f), 0.78f)))
				]
			]
		];
}

FText SGorgeousDataSchemaMappingEditorPanel::GetCatalogVariableTypeText(const FString& VariablePath, const UStruct* RootStruct) const
{
	if (!RootStruct || VariablePath.IsEmpty())
	{
		return LOCTEXT("CatalogVariableTypeUnknown", "Unknown");
	}

	const FProperty* Property = nullptr;
	FString ResolveError;
	if (!TryResolvePropertyPath(RootStruct, VariablePath, Property, ResolveError))
	{
		return LOCTEXT("CatalogVariableTypeInvalid", "Invalid");
	}

	return FText::FromString(GetPropertyTypeLabel(Property));
}

FLinearColor SGorgeousDataSchemaMappingEditorPanel::GetCatalogVariableTypeTint(const FString& VariablePath, const UStruct* RootStruct) const
{
	if (!RootStruct || VariablePath.IsEmpty())
	{
		return FLinearColor(0.40f, 0.40f, 0.40f, 1.0f);
	}

	const FProperty* Property = nullptr;
	FString ResolveError;
	if (!TryResolvePropertyPath(RootStruct, VariablePath, Property, ResolveError))
	{
		return FLinearColor(0.48f, 0.24f, 0.24f, 1.0f);
	}

	return GetPropertyTypeTint(Property);
}

void SGorgeousDataSchemaMappingEditorPanel::CollectStructPropertyPaths(const UStruct* InStructType, const FString& Prefix, int32 Depth, TSet<const UStruct*>& VisitedStructs, TArray<FString>& OutPaths) const
{
	if (!InStructType || Depth > MaxPropertyTraversalDepth)
	{
		return;
	}

	if (VisitedStructs.Contains(InStructType))
	{
		return;
	}

	VisitedStructs.Add(InStructType);

	for (TFieldIterator<FProperty> PropertyIt(InStructType, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
	{
		const FProperty* Property = *PropertyIt;
		if (!Property || Property->HasAnyPropertyFlags(CPF_Transient))
		{
			continue;
		}

		const FString Segment = GorgeousPropertyPath::GetPreferredPropertySegment(Property);
		const FString Path = Prefix.IsEmpty() ? Segment : FString::Printf(TEXT("%s.%s"), *Prefix, *Segment);
		OutPaths.Add(Path);

		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			CollectStructPropertyPaths(StructProperty->Struct, Path, Depth + 1, VisitedStructs, OutPaths);
		}
	}

	VisitedStructs.Remove(InStructType);
}

const UStruct* SGorgeousDataSchemaMappingEditorPanel::ResolveSourceRootStruct() const
{
	if (!SchemaMapAsset)
	{
		return nullptr;
	}

	switch (SchemaMapAsset->SourceDefinition.SourceKind)
	{
	case EGorgeousDataSchemaSourceKind_E::Source_DataTableRows:
		if (UScriptStruct* RequiredRowStruct = SchemaMapAsset->SourceDefinition.RequiredDataTableRowStruct.LoadSynchronous())
		{
			return RequiredRowStruct;
		}

		if (UDataTable* SourceDataTable = SchemaMapAsset->SourceDefinition.SourceDataTable.LoadSynchronous())
		{
			return SourceDataTable->GetRowStruct();
		}

		return nullptr;

	case EGorgeousDataSchemaSourceKind_E::Source_DataAsset:
		return SchemaMapAsset->SourceDefinition.RequiredSourceDataAssetClass.LoadSynchronous();

	case EGorgeousDataSchemaSourceKind_E::Source_BlueprintClassDefaults:
	case EGorgeousDataSchemaSourceKind_E::Source_AnyUObject:
	default:
		return SchemaMapAsset->SourceDefinition.RequiredSourceClass.LoadSynchronous();
	}
}

const UStruct* SGorgeousDataSchemaMappingEditorPanel::ResolveTargetRootStruct() const
{
	if (!SchemaMapAsset)
	{
		return nullptr;
	}

	return SchemaMapAsset->TargetDefinition.TargetClass.LoadSynchronous();
}

bool SGorgeousDataSchemaMappingEditorPanel::TryResolvePropertyPath(const UStruct* RootStruct, const FString& PropertyPath, const FProperty*& OutProperty, FString& OutError) const
{
	OutProperty = nullptr;
	OutError.Reset();

	if (!RootStruct)
	{
		OutError = TEXT("Root schema type is missing.");
		return false;
	}

	TArray<FString> Segments;
	PropertyPath.ParseIntoArray(Segments, TEXT("."), true);
	if (Segments.IsEmpty())
	{
		OutError = TEXT("Property path is empty.");
		return false;
	}

	const UStruct* CurrentStruct = RootStruct;
	for (int32 SegmentIndex = 0; SegmentIndex < Segments.Num(); ++SegmentIndex)
	{
		const FString& Segment = Segments[SegmentIndex];
		const FProperty* ResolvedProperty = CurrentStruct->FindPropertyByName(*Segment);
		if (!ResolvedProperty)
		{
			for (TFieldIterator<FProperty> PropertyIt(CurrentStruct, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
			{
				if (GorgeousPropertyPath::DoesPropertySegmentMatch(*PropertyIt, Segment))
				{
					ResolvedProperty = *PropertyIt;
					break;
				}
			}
		}

		if (!ResolvedProperty)
		{
			OutError = FString::Printf(TEXT("Segment '%s' was not found on '%s'"), *Segment, *CurrentStruct->GetName());
			return false;
		}

		if (SegmentIndex == Segments.Num() - 1)
		{
			OutProperty = ResolvedProperty;
			return true;
		}

		if (const FStructProperty* StructProperty = CastField<FStructProperty>(ResolvedProperty))
		{
			CurrentStruct = StructProperty->Struct;
			continue;
		}

		if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(ResolvedProperty))
		{
			CurrentStruct = ObjectProperty->PropertyClass;
			continue;
		}

		if (const FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(ResolvedProperty))
		{
			CurrentStruct = SoftObjectProperty->PropertyClass;
			continue;
		}

		if (const FClassProperty* ClassProperty = CastField<FClassProperty>(ResolvedProperty))
		{
			CurrentStruct = ClassProperty->MetaClass;
			continue;
		}

		if (const FSoftClassProperty* SoftClassProperty = CastField<FSoftClassProperty>(ResolvedProperty))
		{
			CurrentStruct = SoftClassProperty->MetaClass;
			continue;
		}

		OutError = FString::Printf(TEXT("Segment '%s' cannot be traversed further."), *Segment);
		return false;
	}

	OutError = TEXT("Unexpected resolver termination.");
	return false;
}

bool SGorgeousDataSchemaMappingEditorPanel::ArePropertiesCompatible(const FProperty* SourceProperty, const FProperty* TargetProperty) const
{
	if (!SourceProperty || !TargetProperty)
	{
		return false;
	}

	if (const FArrayProperty* SourceArray = CastField<FArrayProperty>(SourceProperty))
	{
		const FArrayProperty* TargetArray = CastField<FArrayProperty>(TargetProperty);
		return TargetArray && ArePropertiesCompatible(SourceArray->Inner, TargetArray->Inner);
	}

	if (CastField<FArrayProperty>(TargetProperty))
	{
		return false;
	}

	if (const FSetProperty* SourceSet = CastField<FSetProperty>(SourceProperty))
	{
		const FSetProperty* TargetSet = CastField<FSetProperty>(TargetProperty);
		return TargetSet && ArePropertiesCompatible(SourceSet->ElementProp, TargetSet->ElementProp);
	}

	if (CastField<FSetProperty>(TargetProperty))
	{
		return false;
	}

	if (const FMapProperty* SourceMap = CastField<FMapProperty>(SourceProperty))
	{
		const FMapProperty* TargetMap = CastField<FMapProperty>(TargetProperty);
		return TargetMap
			&& ArePropertiesCompatible(SourceMap->KeyProp, TargetMap->KeyProp)
			&& ArePropertiesCompatible(SourceMap->ValueProp, TargetMap->ValueProp);
	}

	if (CastField<FMapProperty>(TargetProperty))
	{
		return false;
	}

	if (SourceProperty->SameType(TargetProperty))
	{
		return true;
	}

	if (IsNumericProperty(SourceProperty) && IsNumericProperty(TargetProperty))
	{
		return true;
	}

	if (IsStringFamilyProperty(SourceProperty) && IsStringFamilyProperty(TargetProperty))
	{
		return true;
	}

	if (IsEnumProperty(SourceProperty) && (IsEnumProperty(TargetProperty) || IsNumericProperty(TargetProperty)))
	{
		return true;
	}

	if (IsEnumProperty(TargetProperty) && IsNumericProperty(SourceProperty))
	{
		return true;
	}

	if (IsObjectLikeProperty(SourceProperty) && IsObjectLikeProperty(TargetProperty))
	{
		return true;
	}

	return false;
}

FString SGorgeousDataSchemaMappingEditorPanel::GetPropertyTypeLabel(const FProperty* Property) const
{
	if (!Property)
	{
		return TEXT("Unknown");
	}

	FString ExtendedType;
	const FString CppType = Property->GetCPPType(&ExtendedType);
	return ExtendedType.IsEmpty() ? CppType : FString::Printf(TEXT("%s%s"), *CppType, *ExtendedType);
}

bool SGorgeousDataSchemaMappingEditorPanel::TryFindAutoMatchedTargetPath(const FString& SourcePath, FString& OutTargetPath) const
{
	OutTargetPath.Reset();

	const UStruct* SourceRootStruct = ResolveSourceRootStruct();
	const UStruct* TargetRootStruct = ResolveTargetRootStruct();
	if (!SourceRootStruct || !TargetRootStruct)
	{
		return false;
	}

	const FProperty* SourceProperty = nullptr;
	FString ResolveError;
	if (!TryResolvePropertyPath(SourceRootStruct, SourcePath, SourceProperty, ResolveError))
	{
		return false;
	}

	const FString SourceLeafSegment = GetLeafPathSegment(SourcePath);
	TArray<FString> Candidates;

	for (const FString& TargetPath : CachedTargetPaths)
	{
		if (!GetLeafPathSegment(TargetPath).Equals(SourceLeafSegment, ESearchCase::IgnoreCase))
		{
			continue;
		}

		const FProperty* TargetProperty = nullptr;
		if (!TryResolvePropertyPath(TargetRootStruct, TargetPath, TargetProperty, ResolveError))
		{
			continue;
		}

		if (ArePropertiesCompatible(SourceProperty, TargetProperty))
		{
			Candidates.Add(TargetPath);
		}
	}

	if (Candidates.IsEmpty())
	{
		return false;
	}

	for (const FString& Candidate : Candidates)
	{
		if (Candidate.Equals(SourcePath, ESearchCase::IgnoreCase))
		{
			OutTargetPath = Candidate;
			return true;
		}
	}

	Candidates.Sort([](const FString& Left, const FString& Right)
	{
		return Left.Len() < Right.Len();
	});

	OutTargetPath = Candidates[0];
	return true;
}

FText SGorgeousDataSchemaMappingEditorPanel::GetMappingDiagnosticsText() const
{
	if (MappingDiagnostics.IsEmpty())
	{
		return FText::GetEmpty();
	}

	FString WarningText;
	for (int32 MessageIndex = 0; MessageIndex < MappingDiagnostics.Num(); ++MessageIndex)
	{
		if (MessageIndex > 0)
		{
			WarningText += TEXT("\n");
		}

		WarningText += FString::Printf(TEXT("- %s"), *MappingDiagnostics[MessageIndex].ToString());
	}

	return FText::FromString(WarningText);
}

EVisibility SGorgeousDataSchemaMappingEditorPanel::GetMappingDiagnosticsVisibility() const
{
	return MappingDiagnostics.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible;
}

void SGorgeousDataSchemaMappingEditorPanel::MarkSchemaMapModified() const
{
	if (!SchemaMapAsset)
	{
		return;
	}

	SchemaMapAsset->Modify();
	SchemaMapAsset->MarkPackageDirty();
}

void SGorgeousDataSchemaMappingEditorPanel::HandleDetailsPropertiesChanged(const FPropertyChangedEvent& PropertyChangedEvent)
{
	RefreshPathCatalogs();
}

FText SGorgeousDataSchemaMappingEditorPanel::GetTargetRootPathText() const
{
	if (!SchemaMapAsset)
	{
		return FText::GetEmpty();
	}

	return FText::FromString(SchemaMapAsset->TargetDefinition.TargetContentRootPath);
}

void SGorgeousDataSchemaMappingEditorPanel::HandleTargetRootPathCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	SetTargetRootPath(NewText.ToString());
}

void SGorgeousDataSchemaMappingEditorPanel::SetTargetRootPath(const FString& NewPath)
{
	if (!SchemaMapAsset)
	{
		return;
	}

	if (SchemaMapAsset->TargetDefinition.TargetContentRootPath == NewPath)
	{
		return;
	}

	MarkSchemaMapModified();
	SchemaMapAsset->TargetDefinition.TargetContentRootPath = NewPath;
}

FReply SGorgeousDataSchemaMappingEditorPanel::HandleBrowseTargetRootPathClicked()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	FPathPickerConfig PickerConfig;

	PickerConfig.DefaultPath = SchemaMapAsset && !SchemaMapAsset->TargetDefinition.TargetContentRootPath.IsEmpty()
		? SchemaMapAsset->TargetDefinition.TargetContentRootPath
		: FString(TEXT("/Game"));
	PickerConfig.bAllowContextMenu = false;
	PickerConfig.bFocusSearchBoxWhenOpened = true;
	PickerConfig.OnPathSelected = FOnPathSelected::CreateSP(this, &SGorgeousDataSchemaMappingEditorPanel::HandleTargetRootPathPicked);

	const TSharedRef<SWidget> PickerWidget = SNew(SBox)
		.WidthOverride(420.0f)
		.HeightOverride(480.0f)
		[
			ContentBrowserModule.Get().CreatePathPicker(PickerConfig)
		];

	if (BrowseTargetRootPathButton.IsValid())
	{
		FSlateApplication::Get().PushMenu(
			BrowseTargetRootPathButton.ToSharedRef(),
			FWidgetPath(),
			PickerWidget,
			FSlateApplication::Get().GetCursorPos(),
			FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
	}

	return FReply::Handled();
}

void SGorgeousDataSchemaMappingEditorPanel::HandleTargetRootPathPicked(const FString& PickedPath)
{
	SetTargetRootPath(PickedPath);
	FSlateApplication::Get().DismissAllMenus();
}

void SGorgeousDataSchemaMappingEditorPanel::HandleSchemaSettingsExpansionChanged(const bool bExpanded)
{
	bSchemaSettingsExpanded = bExpanded;

	if (bSchemaSettingsExpanded)
	{
		const float MinimumExpandedSize = GetSchemaSettingsMinExpandedSlotSize();
		SchemaSettingsExpandedSize = FMath::Clamp(SchemaSettingsExpandedSize, MinimumExpandedSize, SchemaSettingsMaximumExpandedSize);
	}

	if (MainVerticalSplitter.IsValid())
	{
		MainVerticalSplitter->Invalidate(EInvalidateWidgetReason::Layout);
	}
}

void SGorgeousDataSchemaMappingEditorPanel::HandleSchemaSettingsSlotResized(const float NewSize)
{
	if (!bSchemaSettingsExpanded)
	{
		return;
	}

	const float MinimumExpandedSize = GetSchemaSettingsMinExpandedSlotSize();
	SchemaSettingsExpandedSize = FMath::Clamp(NewSize, MinimumExpandedSize, SchemaSettingsMaximumExpandedSize);
}

float SGorgeousDataSchemaMappingEditorPanel::GetSchemaSettingsSlotSize() const
{
	if (!bSchemaSettingsExpanded)
	{
		return GetSchemaSettingsCollapsedSlotSize();
	}

	const float MinimumExpandedSize = GetSchemaSettingsMinExpandedSlotSize();
	return FMath::Clamp(SchemaSettingsExpandedSize, MinimumExpandedSize, SchemaSettingsMaximumExpandedSize);
}

float SGorgeousDataSchemaMappingEditorPanel::GetSchemaSettingsMinExpandedSlotSize() const
{
	float SplitterHeight = GetCachedGeometry().GetLocalSize().Y;
	if (MainVerticalSplitter.IsValid())
	{
		const float SplitterCachedHeight = MainVerticalSplitter->GetCachedGeometry().GetLocalSize().Y;
		if (SplitterCachedHeight > KINDA_SMALL_NUMBER)
		{
			SplitterHeight = SplitterCachedHeight;
		}
	}

	if (SplitterHeight <= KINDA_SMALL_NUMBER)
	{
		return 0.18f;
	}

	return FMath::Clamp(SchemaSettingsMinExpandedHeightPx / SplitterHeight, 0.12f, SchemaSettingsMaximumExpandedSize);
}

float SGorgeousDataSchemaMappingEditorPanel::GetSchemaSettingsCollapsedSlotSize() const
{
	float SplitterHeight = GetCachedGeometry().GetLocalSize().Y;
	if (MainVerticalSplitter.IsValid())
	{
		const float SplitterCachedHeight = MainVerticalSplitter->GetCachedGeometry().GetLocalSize().Y;
		if (SplitterCachedHeight > KINDA_SMALL_NUMBER)
		{
			SplitterHeight = SplitterCachedHeight;
		}
	}

	if (SplitterHeight <= KINDA_SMALL_NUMBER)
	{
		return 0.03f;
	}

	return FMath::Clamp(SchemaSettingsCollapsedHeightPx / SplitterHeight, 0.015f, 0.08f);
}

FText SGorgeousDataSchemaMappingEditorPanel::GetSourceRowsDiagnosticText() const
{
	if (!SchemaMapAsset)
	{
		return FText::GetEmpty();
	}

	const UDataTable* SourceDataTable = SchemaMapAsset->SourceDefinition.SourceDataTable.LoadSynchronous();
	if (!SourceDataTable)
	{
		return LOCTEXT("SourceRowsDiagnosticNoDataTable", "No Source DataTable is selected.");
	}

	const int32 SourceRowCount = SourceDataTable->GetRowMap().Num();
	return FText::Format(
		LOCTEXT("SourceRowsDiagnosticRowCount", "Rows to migrate from selected SourceDataTable: {0}"),
		FText::AsNumber(SourceRowCount));
}

EVisibility SGorgeousDataSchemaMappingEditorPanel::GetSourceRowsDiagnosticVisibility() const
{
	if (!SchemaMapAsset)
	{
		return EVisibility::Collapsed;
	}

	if (SchemaMapAsset->SourceDefinition.SourceKind != EGorgeousDataSchemaSourceKind_E::Source_DataTableRows)
	{
		return EVisibility::Collapsed;
	}

	return SchemaMapAsset->SourceDefinition.SourceDataTable.IsNull()
		? EVisibility::Collapsed
		: EVisibility::Visible;
}

FReply SGorgeousDataSchemaMappingEditorPanel::HandleAddEmptyMappingRow()
{
	if (!SchemaMapAsset)
	{
		return FReply::Handled();
	}

	MarkSchemaMapModified();
	SchemaMapAsset->FieldMappings.AddDefaulted();
	RebuildMappingRows();
	return FReply::Handled();
}

FReply SGorgeousDataSchemaMappingEditorPanel::HandleAddSelectedMappingRow()
{
	if (!SchemaMapAsset)
	{
		return FReply::Handled();
	}

	if (CachedSourcePaths.IsEmpty() || CachedTargetPaths.IsEmpty())
	{
		return FReply::Handled();
	}

	TArray<TPair<FString, FString>> RowsToAdd;
	RowsToAdd.Reserve(CachedSourcePaths.Num());

	for (const FString& SourcePath : CachedSourcePaths)
	{
		if (SourcePath.IsEmpty())
		{
			continue;
		}

		bool bSourceAlreadyMapped = false;
		for (const FGorgeousDataSchemaFieldMapping_S& ExistingMapping : SchemaMapAsset->FieldMappings)
		{
			if (ExistingMapping.SourcePath.Equals(SourcePath, ESearchCase::IgnoreCase))
			{
				bSourceAlreadyMapped = true;
				break;
			}
		}

		if (bSourceAlreadyMapped)
		{
			continue;
		}

		FString MatchedTargetPath;
		if (!TryFindAutoMatchedTargetPath(SourcePath, MatchedTargetPath))
		{
			continue;
		}

		bool bPairExists = false;
		for (const FGorgeousDataSchemaFieldMapping_S& ExistingMapping : SchemaMapAsset->FieldMappings)
		{
			if (ExistingMapping.SourcePath.Equals(SourcePath, ESearchCase::IgnoreCase)
				&& ExistingMapping.TargetPath.Equals(MatchedTargetPath, ESearchCase::IgnoreCase))
			{
				bPairExists = true;
				break;
			}
		}

		if (!bPairExists)
		{
			RowsToAdd.Emplace(SourcePath, MatchedTargetPath);
		}
	}

	if (RowsToAdd.IsEmpty())
	{
		return FReply::Handled();
	}

	MarkSchemaMapModified();
	for (const TPair<FString, FString>& RowToAdd : RowsToAdd)
	{
		FGorgeousDataSchemaFieldMapping_S& NewRow = SchemaMapAsset->FieldMappings.AddDefaulted_GetRef();
		NewRow.SourcePath = RowToAdd.Key;
		NewRow.TargetPath = RowToAdd.Value;
	}

	RebuildMappingRows();
	RefreshPathCatalogs();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
