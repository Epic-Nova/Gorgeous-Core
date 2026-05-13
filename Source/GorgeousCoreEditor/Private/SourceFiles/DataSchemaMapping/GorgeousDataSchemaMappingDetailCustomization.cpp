// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#include "DataSchemaMapping/GorgeousDataSchemaMappingDetailCustomization.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "DataSchemaMapping/GorgeousDataSchemaMapping_DA.h"
#include "DataSchemaMapping/GorgeousDataSchemaMappingTypes.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "AssetRegistry/AssetData.h"
#include "ContentBrowserModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/DataTable.h"
#include "Framework/Application/SlateApplication.h"
#include "IContentBrowserSingleton.h"
#include "IDetailGroup.h"
#include "PropertyHandle.h"
#include "PropertyCustomizationHelpers.h"
#include "Styling/AppStyle.h"
#include "UObject/UnrealType.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
//<-------------------------------------------------------------------------->

#define LOCTEXT_NAMESPACE "GorgeousDataSchemaMappingDetailCustomization"

TSharedRef<IDetailCustomization> FGorgeousDataSchemaMappingDetailCustomization::MakeInstance()
{
	return MakeShared<FGorgeousDataSchemaMappingDetailCustomization>();
}

void FGorgeousDataSchemaMappingDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);
	EditedSchemaAsset.Reset();
	for (const TWeakObjectPtr<UObject>& CustomizedObject : CustomizedObjects)
	{
		if (UGorgeousDataSchemaMapping_DA* SchemaAsset = Cast<UGorgeousDataSchemaMapping_DA>(CustomizedObject.Get()))
		{
			EditedSchemaAsset = SchemaAsset;
			break;
		}
	}

	SourceKindHandle.Reset();
	RequiredDataTableRowStructHandle.Reset();
	SourceDataTableHandle.Reset();
	TargetPathHandle.Reset();

	IDetailCategoryBuilder& SchemaCategory = DetailBuilder.EditCategory(TEXT("Schema"));
	DetailBuilder.HideCategory(TEXT("Source"));
	DetailBuilder.HideCategory(TEXT("Target"));
	DetailBuilder.HideCategory(TEXT("Migration"));
	DetailBuilder.HideCategory(TEXT("Schema|Source"));
	DetailBuilder.HideCategory(TEXT("Schema|Target"));
	DetailBuilder.HideCategory(TEXT("Schema|Migration"));

	const TSharedPtr<IPropertyHandle> SourceDefinitionHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGorgeousDataSchemaMapping_DA, SourceDefinition));
	if (SourceDefinitionHandle.IsValid())
	{
		SourceKindHandle = SourceDefinitionHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGorgeousDataSchemaSourceDefinition_S, SourceKind));
		RequiredDataTableRowStructHandle = SourceDefinitionHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGorgeousDataSchemaSourceDefinition_S, RequiredDataTableRowStruct));
		SourceDataTableHandle = SourceDefinitionHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGorgeousDataSchemaSourceDefinition_S, SourceDataTable));

		DetailBuilder.HideProperty(SourceDefinitionHandle);

		IDetailGroup& SourceGroup = SchemaCategory.AddGroup(TEXT("SchemaSourceDefinitionGroup"), LOCTEXT("SourceDefinitionGroupLabel", "Source Definition"));
		uint32 SourceChildCount = 0;
		SourceDefinitionHandle->GetNumChildren(SourceChildCount);
		for (uint32 ChildIndex = 0; ChildIndex < SourceChildCount; ++ChildIndex)
		{
			const TSharedPtr<IPropertyHandle> ChildHandle = SourceDefinitionHandle->GetChildHandle(ChildIndex);
			if (!ChildHandle.IsValid())
			{
				continue;
			}

			const FProperty* ChildProperty = ChildHandle->GetProperty();
			if (!ChildProperty)
			{
				continue;
			}

			const FName ChildPropertyName = ChildProperty->GetFName();

			DetailBuilder.HideProperty(ChildHandle);

			if (ChildProperty->GetFName() == GET_MEMBER_NAME_CHECKED(FGorgeousDataSchemaSourceDefinition_S, SourceDataTable)
				&& SourceDataTableHandle.IsValid())
			{
				SourceGroup.AddWidgetRow()
				.Visibility(TAttribute<EVisibility>::CreateSP(this, &FGorgeousDataSchemaMappingDetailCustomization::GetSourceDefinitionPropertyVisibility, ChildPropertyName))
				.NameContent()
				[
					SourceDataTableHandle->CreatePropertyNameWidget()
				]
				.ValueContent()
				.MinDesiredWidth(450.0f)
				[
					SNew(SObjectPropertyEntryBox)
					.PropertyHandle(SourceDataTableHandle)
					.AllowedClass(UDataTable::StaticClass())
					.OnShouldFilterAsset(FOnShouldFilterAsset::CreateSP(this, &FGorgeousDataSchemaMappingDetailCustomization::ShouldFilterSourceDataTableAsset))
					.IsEnabled(this, &FGorgeousDataSchemaMappingDetailCustomization::IsSourceDataTablePickerEnabled)
				];

				continue;
			}

			SourceGroup.AddWidgetRow()
			.Visibility(TAttribute<EVisibility>::CreateSP(this, &FGorgeousDataSchemaMappingDetailCustomization::GetSourceDefinitionPropertyVisibility, ChildPropertyName))
			.NameContent()
			[
				ChildHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			.MinDesiredWidth(450.0f)
			[
				ChildHandle->CreatePropertyValueWidget()
			];
		}
	}

	const TSharedPtr<IPropertyHandle> TargetDefinitionHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGorgeousDataSchemaMapping_DA, TargetDefinition));
	if (TargetDefinitionHandle.IsValid())
	{
		TargetPathHandle = TargetDefinitionHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGorgeousDataSchemaTargetDefinition_S, TargetContentRootPath));

		DetailBuilder.HideProperty(TargetDefinitionHandle);

		IDetailGroup& TargetGroup = SchemaCategory.AddGroup(TEXT("SchemaTargetDefinitionGroup"), LOCTEXT("TargetDefinitionGroupLabel", "Target Definition"));
		uint32 TargetChildCount = 0;
		TargetDefinitionHandle->GetNumChildren(TargetChildCount);
		for (uint32 ChildIndex = 0; ChildIndex < TargetChildCount; ++ChildIndex)
		{
			const TSharedPtr<IPropertyHandle> ChildHandle = TargetDefinitionHandle->GetChildHandle(ChildIndex);
			if (!ChildHandle.IsValid())
			{
				continue;
			}

			const FProperty* ChildProperty = ChildHandle->GetProperty();
			if (!ChildProperty)
			{
				continue;
			}

			DetailBuilder.HideProperty(ChildHandle);

			if (ChildProperty->GetFName() == GET_MEMBER_NAME_CHECKED(FGorgeousDataSchemaTargetDefinition_S, TargetContentRootPath)
				&& TargetPathHandle.IsValid())
			{
				TargetGroup.AddWidgetRow()
				.NameContent()
				[
					TargetPathHandle->CreatePropertyNameWidget()
				]
				.ValueContent()
				.MinDesiredWidth(450.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(0.0f, 0.0f, 4.0f, 0.0f)
					[
						TargetPathHandle->CreatePropertyValueWidget()
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SAssignNew(BrowseButton, SButton)
						.ButtonStyle(FAppStyle::Get(), "SimpleButton")
						.ToolTipText(LOCTEXT("BrowsePathTooltip", "Pick a content path from your project."))
						.OnClicked(this, &FGorgeousDataSchemaMappingDetailCustomization::HandleBrowseTargetPathClicked)
						[
							SNew(SImage)
							.Image(FAppStyle::GetBrush("Icons.FolderOpen"))
						]
					]
				];

				continue;
			}

			TargetGroup.AddWidgetRow()
			.NameContent()
			[
				ChildHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			.MinDesiredWidth(450.0f)
			[
				ChildHandle->CreatePropertyValueWidget()
			];
		}
	}

	const TSharedPtr<IPropertyHandle> MigrationDefinitionHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGorgeousDataSchemaMapping_DA, MigrationDefinition));
	if (MigrationDefinitionHandle.IsValid())
	{
		DetailBuilder.HideProperty(MigrationDefinitionHandle);

		IDetailGroup& MigrationGroup = SchemaCategory.AddGroup(TEXT("SchemaMigrationDefinitionGroup"), LOCTEXT("MigrationDefinitionGroupLabel", "Migration Definition"));
		uint32 MigrationChildCount = 0;
		MigrationDefinitionHandle->GetNumChildren(MigrationChildCount);
		for (uint32 ChildIndex = 0; ChildIndex < MigrationChildCount; ++ChildIndex)
		{
			const TSharedPtr<IPropertyHandle> ChildHandle = MigrationDefinitionHandle->GetChildHandle(ChildIndex);
			if (ChildHandle.IsValid())
			{
				DetailBuilder.HideProperty(ChildHandle);
				MigrationGroup.AddWidgetRow()
				.NameContent()
				[
					ChildHandle->CreatePropertyNameWidget()
				]
				.ValueContent()
				.MinDesiredWidth(450.0f)
				[
					ChildHandle->CreatePropertyValueWidget()
				];
			}
		}

	}
}

FReply FGorgeousDataSchemaMappingDetailCustomization::HandleBrowseTargetPathClicked()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	FPathPickerConfig PickerConfig;

	FString CurrentPath;
	if (!TargetPathHandle.IsValid() || TargetPathHandle->GetValue(CurrentPath) != FPropertyAccess::Success || CurrentPath.IsEmpty())
	{
		CurrentPath = TEXT("/Game");
	}
	PickerConfig.DefaultPath = CurrentPath;
	PickerConfig.bAllowContextMenu = false;
	PickerConfig.bFocusSearchBoxWhenOpened = true;
	PickerConfig.OnPathSelected = FOnPathSelected::CreateSP(this, &FGorgeousDataSchemaMappingDetailCustomization::HandlePathPicked);

	TSharedRef<SWidget> PickerWidget = SNew(SBox)
		.WidthOverride(420.0f)
		.HeightOverride(480.0f)
		[
			ContentBrowserModule.Get().CreatePathPicker(PickerConfig)
		];

	const TSharedPtr<SButton> BrowseButtonPinned = BrowseButton.Pin();
	if (BrowseButtonPinned.IsValid())
	{
		FSlateApplication::Get().PushMenu(
			BrowseButtonPinned.ToSharedRef(),
			FWidgetPath(),
			PickerWidget,
			FSlateApplication::Get().GetCursorPos(),
			FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
	}

	return FReply::Handled();
}

void FGorgeousDataSchemaMappingDetailCustomization::HandlePathPicked(const FString& PickedPath)
{
	if (TargetPathHandle.IsValid())
	{
		TargetPathHandle->SetValue(PickedPath);
	}

	FSlateApplication::Get().DismissAllMenus();
}

EVisibility FGorgeousDataSchemaMappingDetailCustomization::GetSourceDefinitionPropertyVisibility(const FName SourcePropertyName) const
{
	const UGorgeousDataSchemaMapping_DA* SchemaAsset = EditedSchemaAsset.Get();
	if (!SchemaAsset)
	{
		return EVisibility::Collapsed;
	}

	const EGorgeousDataSchemaSourceKind_E SourceKind = SchemaAsset->SourceDefinition.SourceKind;

	if (SourcePropertyName == GET_MEMBER_NAME_CHECKED(FGorgeousDataSchemaSourceDefinition_S, SourceKind))
	{
		return EVisibility::Visible;
	}

	if (SourcePropertyName == GET_MEMBER_NAME_CHECKED(FGorgeousDataSchemaSourceDefinition_S, RequiredSourceClass))
	{
		return (SourceKind == EGorgeousDataSchemaSourceKind_E::Source_AnyUObject
			|| SourceKind == EGorgeousDataSchemaSourceKind_E::Source_BlueprintClassDefaults)
			? EVisibility::Visible
			: EVisibility::Collapsed;
	}

	if (SourcePropertyName == GET_MEMBER_NAME_CHECKED(FGorgeousDataSchemaSourceDefinition_S, RequiredSourceDataAssetClass))
	{
		return SourceKind == EGorgeousDataSchemaSourceKind_E::Source_DataAsset
			? EVisibility::Visible
			: EVisibility::Collapsed;
	}

	if (SourcePropertyName == GET_MEMBER_NAME_CHECKED(FGorgeousDataSchemaSourceDefinition_S, RequiredDataTableRowStruct)
		|| SourcePropertyName == GET_MEMBER_NAME_CHECKED(FGorgeousDataSchemaSourceDefinition_S, SourceDataTable))
	{
		return SourceKind == EGorgeousDataSchemaSourceKind_E::Source_DataTableRows
			? EVisibility::Visible
			: EVisibility::Collapsed;
	}

	return EVisibility::Visible;
}

bool FGorgeousDataSchemaMappingDetailCustomization::IsSourceDataTablePickerEnabled() const
{
	const UGorgeousDataSchemaMapping_DA* SchemaAsset = EditedSchemaAsset.Get();
	if (!SchemaAsset)
	{
		return false;
	}

	if (SchemaAsset->SourceDefinition.SourceKind != EGorgeousDataSchemaSourceKind_E::Source_DataTableRows)
	{
		return false;
	}

	return SchemaAsset->SourceDefinition.RequiredDataTableRowStruct.LoadSynchronous() != nullptr;
}

EVisibility FGorgeousDataSchemaMappingDetailCustomization::GetSourceDataTablePickerVisibility() const
{
	const UGorgeousDataSchemaMapping_DA* SchemaAsset = EditedSchemaAsset.Get();
	if (!SchemaAsset)
	{
		return EVisibility::Collapsed;
	}

	return SchemaAsset->SourceDefinition.SourceKind == EGorgeousDataSchemaSourceKind_E::Source_DataTableRows
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

bool FGorgeousDataSchemaMappingDetailCustomization::ShouldFilterSourceDataTableAsset(const FAssetData& AssetData) const
{
	const UGorgeousDataSchemaMapping_DA* SchemaAsset = EditedSchemaAsset.Get();
	if (!SchemaAsset)
	{
		return true;
	}

	const UScriptStruct* RequiredRowStruct = SchemaAsset->SourceDefinition.RequiredDataTableRowStruct.LoadSynchronous();
	if (!RequiredRowStruct)
	{
		return true;
	}

	const UDataTable* CandidateDataTable = Cast<UDataTable>(AssetData.GetAsset());
	if (!CandidateDataTable)
	{
		return true;
	}

	const UScriptStruct* CandidateRowStruct = CandidateDataTable->GetRowStruct();
	if (!CandidateRowStruct)
	{
		return true;
	}

	return !CandidateRowStruct->IsChildOf(RequiredRowStruct);
}

#undef LOCTEXT_NAMESPACE
