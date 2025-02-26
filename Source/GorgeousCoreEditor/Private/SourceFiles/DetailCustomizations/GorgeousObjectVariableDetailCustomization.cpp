// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|      that is has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

#include "GorgeousObjectVariableDetailCustomization.h"

//<=============================--- Includes ---=============================>
//<-------------------------=== Engine Includes ===-------------------------->
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Blueprint/UserWidget.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// FGorgeousObjectVariableDetailCustomization Implementation
//=============================================================================


DECLARE_DELEGATE_OneParam(FOnPropertyValueChangedWithData, TSharedRef<IPropertyHandle>);

#define LOCTEXT_NAMESPACE "GorgeousObjectVariableDetailCustomization"

TSharedRef<IPropertyTypeCustomization> FGorgeousObjectVariableDetailCustomization::MakeInstance()
{
	return MakeShareable(new FGorgeousObjectVariableDetailCustomization);
}

void FGorgeousObjectVariableDetailCustomization::CustomizeHeader(const TSharedRef<IPropertyHandle> PropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Create a new graph object
	UEdGraph* CustomGraph = NewObject<UEdGraph>();
	CustomGraph->Schema = UEdGraphSchema_K2::StaticClass(); // Use Blueprint graph schema

	// Configure the Graph Editor Widget
	TSharedRef<SGraphEditor> GraphEditor = SNew(SGraphEditor)
		.AdditionalCommands(nullptr)
		.GraphToEdit(CustomGraph)
		.IsEditable(true)
		.TitleBar(SNew(STextBlock).Text(FText::FromString("Custom Event Graph")));
	
	HeaderRow.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	];

	//First iteration test for custom dropdowns
	//@TODO: When constructing these widgets the detail customizer should search every registered object variable and fill them here in aswell
	HeaderRow.ValueContent()
	[
		//Blueprint Graph
		SNew(SBox)
		.HeightOverride(400)
		.WidthOverride(600)
		[
			GraphEditor
		]

		//Webbrowser
		/*SNew(SBorder)
		.BorderBackgroundColor(FSlateColor(FLinearColor(1.0f, 0.2f, 0.2f, 1.0f))) // Custom Red Background
		.Padding(5)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Defenetly no Web Browser in the Details Panel"))
		]*/

		//Combo boxes: Original Opproach
		/*SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			GenerateEnumDropdown<EObjectVariableTypes_E>(
				LOCTEXT("ObjectVariableTypeLabel", "Variable Type"),
				StaticEnum<EObjectVariableTypes_E>(),
				[](EObjectVariableTypes_E NewSelection) {
					UE_LOG(LogTemp, Log, TEXT("Selected Variable Type: %s"), *StaticEnum<EObjectVariableTypes_E>()->GetNameStringByValue((int64)NewSelection));
				}
			)
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			GenerateEnumDropdown<EObjectVariableCount_E>(
				LOCTEXT("ObjectVariableCountLabel", "Variable Count"),
				StaticEnum<EObjectVariableCount_E>(),
				[](EObjectVariableCount_E NewSelection) {
					UE_LOG(LogTemp, Log, TEXT("Selected Variable Count: %s"), *StaticEnum<EObjectVariableCount_E>()->GetNameStringByValue((int64)NewSelection));
				}
			)
		]*/
	];
}

void FGorgeousObjectVariableDetailCustomization::CustomizeChildren(const TSharedRef<IPropertyHandle> PropertyHandle,
	IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;
	
	UClass* WidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/GorgeousCore/Editor/UserInterfaces/BP_GorgeousObjectVariableDetails_UW.BP_GorgeousObjectVariableDetails_UW_C"));
	if (!WidgetClass) return;

	UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(World, WidgetClass);
	if (!CreatedWidget) return;

	ChildBuilder.AddCustomRow(FText::FromString("Custom UI"))
	[
		SNew(SBox)
		.HeightOverride(500)
		.WidthOverride(300)
		[
			CreatedWidget->TakeWidget()
		]
	];
}

//Ready out the parsed enum entries and creates a entry for every enum inside a combo box dropdown: Doesn't work
template <typename TEnum>
TSharedRef<SWidget> FGorgeousObjectVariableDetailCustomization::GenerateEnumDropdown(FText LabelText, const UEnum* EnumType, TFunction<void(TEnum)> OnSelectionChangedCallback)
{
    // Ensure EnumType is valid
    if (!EnumType)
    {
        return SNew(STextBlock).Text(FText::FromString("Invalid Enum"));
    }

    // Populate options for the dropdown
    TArray<TSharedPtr<FString>> EnumOptions;
    for (int32 i = 0; i < EnumType->GetMaxEnumValue() - 1; ++i) // Skip _Max
    {
    	FString EnumName = EnumType->GetDisplayNameTextByIndex(i).ToString();
    	EnumOptions.Add(MakeShared<FString>(EnumName));
    }

    // Default selection (first item in the enum)
	TSharedPtr<FString> CurrentSelection = EnumOptions[0];

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock).Text(LabelText) // Label for the dropdown
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SComboBox<TSharedPtr<FString>>)
			.OptionsSource(&EnumOptions)
			.InitiallySelectedItem(CurrentSelection) // Ensure the dropdown has a default selected item
			.OnSelectionChanged_Lambda(
				[EnumType, OnSelectionChangedCallback](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
				{
					if (NewSelection.IsValid())
					{
						// Find enum value by name
						int64 EnumValue = EnumType->GetValueByNameString(*NewSelection);
						if (EnumValue != INDEX_NONE)
						{
							OnSelectionChangedCallback(static_cast<TEnum>(EnumValue));
						}
					}
				})
			.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
				{
					return SNew(STextBlock).Text(FText::FromString(*Item));
				})
			[
				SNew(STextBlock)
				.Text_Lambda([CurrentSelection]() -> FText
				{
					return CurrentSelection.IsValid() ? FText::FromString(*CurrentSelection) : FText::FromString("Select...");
				})
			]
		];
}

#undef LOCTEXT_NAMESPACE
