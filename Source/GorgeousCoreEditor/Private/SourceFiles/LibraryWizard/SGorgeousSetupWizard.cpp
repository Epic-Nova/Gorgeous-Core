// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|             Gorgeous Core - Setup Wizard Widget Implementation             |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
<=========================================================================*/

#include "LibraryWizard/SGorgeousSetupWizard.h"

//<=====--- Includes ---=====>
//<----- Engine Includes ----->
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Editor.h"
#include "Factories/Factory.h"
#include "UObject/SavePackage.h"
#include "Engine/DataAsset.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
//<----- Module Includes ----->
#include "LibraryWizard/GorgeousSystemTemplate_DA.h"
#include "LibraryWizard/GorgeousSetupWizardPayload.h"
#include "FunctionalStructures/GorgeousFunctionalStructure.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "UObject/UnrealType.h"
#include "Widgets/Layout/SSpacer.h"
#include "Misc/MessageDialog.h"
//<-------------------------->

//=============================================================================
// SGorgeousSetupWizard
//=============================================================================

SGorgeousSetupWizard::~SGorgeousSetupWizard()
{
	for (UObject* Instance : PayloadInstances)
	{
		if (Instance)
		{
			Instance->RemoveFromRoot();
		}
	}
	PayloadInstances.Empty();
}

/** Helper to initialize functional structures within a payload object or struct. */
static void InitializePayloadStructsRecursive(void* DataPtr, UStruct* StructType, UObject* OwnerObject)
{
	if (!DataPtr || !StructType)
	{
		return;
	}

	for (TFieldIterator<FProperty> PropIt(StructType); PropIt; ++PropIt)
	{
		FProperty* Prop = *PropIt;

		// 1. Direct FGorgeousFunctionalStructure_S (or child)
		if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
		{
			if (StructProp->Struct->IsChildOf(FGorgeousFunctionalStructure_S::StaticStruct()))
			{
				void* NestedDataPtr = StructProp->ContainerPtrToValuePtr<void>(DataPtr);
				FGorgeousFunctionalStructure_S* FS = static_cast<FGorgeousFunctionalStructure_S*>(NestedDataPtr);
				if (FS)
				{
					//FS->OwnerObject = OwnerObject;
					if (!FS->Identifier.IsValid())
					{
						FS->Identifier = FGuid::NewGuid();
					}
					FS->AllocateDefaultValues(OwnerObject);
					GT_S_LOG("GT.Wizard.InitStruct", TEXT("Initialized functional structure '%s' in %s"), *FS->Identifier.ToString(), *OwnerObject->GetName());
				}
			}
			else
			{
				// Recursive search into other structs
				void* NestedDataPtr = StructProp->ContainerPtrToValuePtr<void>(DataPtr);
				InitializePayloadStructsRecursive(NestedDataPtr, StructProp->Struct, OwnerObject);
			}
		}
		// 2. Arrays of structs
		else if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(Prop))
		{
			if (FStructProperty* InnerStructProp = CastField<FStructProperty>(ArrayProp->Inner))
			{
				FScriptArrayHelper ArrayHelper(ArrayProp, ArrayProp->ContainerPtrToValuePtr<void>(DataPtr));
				for (int32 i = 0; i < ArrayHelper.Num(); ++i)
				{
					InitializePayloadStructsRecursive(ArrayHelper.GetRawPtr(i), InnerStructProp->Struct, OwnerObject);
				}
			}
		}
	}
}

static void InitializePayloadStructs(UObject* InObject)
{
	if (!InObject)
	{
		return;
	}

	InitializePayloadStructsRecursive(InObject, InObject->GetClass(), InObject);
}

void SGorgeousSetupWizard::Construct(const FArguments& InArgs)
{
	SourceTemplate = InArgs._SystemTemplate;
	const FString DefaultPath = InArgs._DefaultSavePath;

	// Instantiate the payload objects
	if (SourceTemplate)
	{
		if (!SourceTemplate->ConfigurationPayloadClasses.IsEmpty())
		{
			for (const TSubclassOf<UObject>& PayloadClass : SourceTemplate->ConfigurationPayloadClasses)
			{
				if (PayloadClass)
				{
					UObject* Instance = NewObject<UObject>(GetTransientPackage(), PayloadClass);
					Instance->AddToRoot();
					InitializePayloadStructs(Instance);
					PayloadInstances.Add(Instance);
				}
			}
		}
		else if (SourceTemplate->AssetClassToCreate)
		{
			// Fallback: use the asset class itself as the single payload
			UObject* Instance = NewObject<UObject>(GetTransientPackage(), SourceTemplate->AssetClassToCreate);
			Instance->AddToRoot();
			InitializePayloadStructs(Instance);
			PayloadInstances.Add(Instance);
		}
	}

	// Configure the property editor
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsArgs;
	DetailsArgs.bAllowSearch        = false;
	DetailsArgs.NameAreaSettings    = FDetailsViewArgs::HideNameArea;
	DetailsArgs.bHideSelectionTip   = true;
	DetailsArgs.bShowScrollBar      = false;

	DetailsView = PropertyEditorModule.CreateDetailView(DetailsArgs);

	// Listen for property changes to trigger real-time validation
	DetailsView->OnFinishedChangingProperties().AddLambda([this](const FPropertyChangedEvent& Event)
	{
		UpdateWizardState();
	});

	// Defer SetObject until AFTER Slate finishes constructing this widget.
	// Calling SetObject synchronously during Construct() triggers FGuidStructCustomization
	// which builds tooltips via SDocumentationToolTip, invalidating parent widgets mid-construction
	// and causing a recursive stack overflow.
	RegisterActiveTimer(0.0f, FWidgetActiveTimerDelegate::CreateLambda(
		[this](double, float) -> EActiveTimerReturnType
		{
			if (DetailsView.IsValid() && PayloadInstances.IsValidIndex(CurrentPageIndex))
			{
				DetailsView->SetObject(PayloadInstances[CurrentPageIndex]);
			}
			return EActiveTimerReturnType::Stop;
		}
	));

	// ── Build widget tree ───────────────────────────────────────────────────
	ChildSlot
	[
		SNew(SVerticalBox)

		// ── Header: template title
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(16.0f, 14.0f, 16.0f, 4.0f)
		[
			SNew(STextBlock)
			.Text(SourceTemplate
				? SourceTemplate->TemplateTitle
				: NSLOCTEXT("GorgeousCore", "WizardDefaultTitle", "Gorgeous Setup Wizard"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
		]

		// ── Header: template description
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(16.0f, 0.0f, 16.0f, 4.0f)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() {
				return SourceTemplate ? SourceTemplate->TemplateDescription : FText::GetEmpty();
			})
			.AutoWrapText(true)
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]

		// ── Header: Payload Title (e.g., "Grid & Identity")
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(16.0f, 10.0f, 16.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() {
				if (PayloadInstances.IsValidIndex(CurrentPageIndex))
				{
					if (UGorgeousSetupWizardPayload* Payload = Cast<UGorgeousSetupWizardPayload>(PayloadInstances[CurrentPageIndex]))
					{
						if (!Payload->PayloadTitle.IsEmpty())
						{
							return FText::Format(NSLOCTEXT("GorgeousCore", "WizardPageTitleFormat", "Step {0}: {1}"), 
								FText::AsNumber(CurrentPageIndex + 1), 
								FText::FromString(Payload->PayloadTitle));
						}
					}
				}
				return FText::Format(NSLOCTEXT("GorgeousCore", "WizardPageDefault", "Step {0}"), FText::AsNumber(CurrentPageIndex + 1));
			})
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			.Justification(ETextJustify::Center)
		]

		// ── Header: Payload Description
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(16.0f, 0.0f, 16.0f, 10.0f)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() {
				if (PayloadInstances.IsValidIndex(CurrentPageIndex))
				{
					if (UGorgeousSetupWizardPayload* Payload = Cast<UGorgeousSetupWizardPayload>(PayloadInstances[CurrentPageIndex]))
					{
						return FText::FromString(Payload->PayloadDescription);
					}
				}
				return FText::GetEmpty();
			})
			.AutoWrapText(true)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FSlateColor::UseForeground())
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(16.0f, 0.0f, 16.0f, 10.0f)
		[
			SNew(SSeparator)
		]

		// ── Main body: IDetailsView form
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(8.0f, 0.0f)
		[
			DetailsView.ToSharedRef()
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(16.0f, 6.0f, 16.0f, 0.0f)
		[
			SNew(SSeparator)
		]

		// ── Footer: save path + asset name
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(16.0f, 10.0f, 16.0f, 6.0f)
		[
			SNew(SVerticalBox)

			// Save path row
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 6.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("GorgeousCore", "WizardSavePath", "Save Path"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					.MinDesiredWidth(80.0f)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SAssignNew(SavePathTextBox, SEditableTextBox)
					.Text(FText::FromString(DefaultPath))
					.HintText(NSLOCTEXT("GorgeousCore", "WizardSavePathHint", "/Game/YourFolder/"))
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4.0f, 0.0f, 0.0f, 0.0f)
				[
					SAssignNew(BrowseFolderButton, SButton)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.ToolTipText(NSLOCTEXT("GorgeousCore", "WizardBrowseTooltip", "Browse for folder..."))
					.OnClicked(this, &SGorgeousSetupWizard::OnBrowseFolderClicked)
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("Icons.FolderClosed"))
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
				]
			]

			// Asset name row
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("GorgeousCore", "WizardAssetName", "Asset Name"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					.MinDesiredWidth(80.0f)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SAssignNew(AssetNameTextBox, SEditableTextBox)
					.HintText(NSLOCTEXT("GorgeousCore", "WizardAssetNameHint", "NewAsset"))
				]
			]
		]

		// ── Buttons row
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(16.0f, 10.0f, 16.0f, 16.0f))
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SSpacer)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))
			[
				SAssignNew(BackButton, SButton)
				.Text(NSLOCTEXT("GorgeousCore", "WizardBack", "Back"))
				.OnClicked(this, &SGorgeousSetupWizard::OnBackClicked)
				.Visibility(EVisibility::Collapsed)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))
			[
				SAssignNew(NextButton, SButton)
				.Text(NSLOCTEXT("GorgeousCore", "WizardNext", "Next"))
				.OnClicked(this, &SGorgeousSetupWizard::OnNextClicked)
				.Visibility(SourceTemplate && SourceTemplate->ConfigurationPayloadClasses.Num() > 1 ? EVisibility::Visible : EVisibility::Collapsed)
				.IsEnabled_Lambda([this]() { 
					FString Reason; 
					return CanProceedToNextPage(Reason); 
				})
				.ToolTipText_Lambda([this]() { 
					FString Reason; 
					CanProceedToNextPage(Reason); 
					return FText::FromString(Reason); 
				})
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))
			[
				SAssignNew(FinishButton, SButton)
				.Text(NSLOCTEXT("GorgeousCore", "WizardFinish", "Finish"))
				.OnClicked(this, &SGorgeousSetupWizard::OnFinishClicked)
				.Visibility(SourceTemplate && SourceTemplate->ConfigurationPayloadClasses.Num() <= 1 ? EVisibility::Visible : EVisibility::Collapsed)
				.IsEnabled_Lambda([this]() { 
					FString Reason; 
					return CanProceedToNextPage(Reason); 
				})
				.ToolTipText_Lambda([this]() { 
					FString Reason; 
					CanProceedToNextPage(Reason); 
					return FText::FromString(Reason); 
				})
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(NSLOCTEXT("GorgeousCore", "WizardCancel", "Cancel"))
				.OnClicked(this, &SGorgeousSetupWizard::OnCancelClicked)
			]
		]
	];
}

//-----------------------------------------------------------------------------
// Button Handlers
//-----------------------------------------------------------------------------

FReply SGorgeousSetupWizard::OnFinishClicked()
{
	if (!SourceTemplate)
	{
		return FReply::Handled();
	}

	// Validate inputs
	const FString ErrorMsg = ValidateSavePath();
	if (!ErrorMsg.IsEmpty())
	{
		FNotificationInfo Info(FText::FromString(ErrorMsg));
		Info.ExpireDuration = 5.0f;
		Info.bFireAndForget  = true;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Handled();
	}

	// Determine asset class to create
	const TSubclassOf<UPrimaryDataAsset> AssetClass = SourceTemplate->AssetClassToCreate;
	if (!AssetClass)
	{
		FNotificationInfo Info(NSLOCTEXT("GorgeousCore", "WizardNoClass", "Template does not specify an AssetClassToCreate!"));
		Info.ExpireDuration = 5.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Handled();
	}

	const FString AssetName    = AssetNameTextBox.IsValid() ? AssetNameTextBox->GetText().ToString().TrimStartAndEnd() : TEXT("NewAsset");
	FString PackagePath        = SavePathTextBox.IsValid()  ? SavePathTextBox->GetText().ToString().TrimEnd() : TEXT("/Game");
	PackagePath.RemoveFromEnd(TEXT("/"));

	// Create the asset using IAssetTools (this creates & saves the .uasset)
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UObject* CreatedAsset = AssetTools.CreateAsset(AssetName, PackagePath, AssetClass, nullptr);

	if (!IsValid(CreatedAsset))
	{
		FNotificationInfo Info(NSLOCTEXT("GorgeousCore", "WizardCreateFailed", "Failed to create asset. Check the save path and asset name."));
		Info.ExpireDuration = 6.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Handled();
	}

	// 1. Copy wizard payload properties into the newly created asset (Auto-Mapping)
	for (UObject* PayloadInstance : PayloadInstances)
	{
		if (IsValid(PayloadInstance))
		{
			// Check if this is a specialized payload and if we should run auto-mapping
			bool bRunAutoMapping = true;
			if (UGorgeousSetupWizardPayload* Payload = Cast<UGorgeousSetupWizardPayload>(PayloadInstance))
			{
				// If the user has overridden PostCreate, we skip auto-mapping and assume 
				// they want full manual control over the generation logic.
				static FName PostCreateName = FName(TEXT("PostCreate"));
				UFunction* PostCreateFunc = Payload->GetClass()->FindFunctionByName(PostCreateName);
				
				// If the function's outer is NOT the base class, it means it's been overridden 
				// in a Blueprint or a derived C++ class.
				if (PostCreateFunc && PostCreateFunc->GetOuter() != UGorgeousSetupWizardPayload::StaticClass())
				{
					bRunAutoMapping = false;
				}
			}

			if (bRunAutoMapping)
			{
				for (TFieldIterator<FProperty> PropIt(PayloadInstance->GetClass()); PropIt; ++PropIt)
				{
					FProperty* Prop = *PropIt;
					if (FProperty* DestProp = CreatedAsset->GetClass()->FindPropertyByName(Prop->GetFName()))
					{
						if (DestProp->SameType(Prop))
						{
							void* SourcePtr = Prop->ContainerPtrToValuePtr<void>(PayloadInstance);
							void* DestPtr = DestProp->ContainerPtrToValuePtr<void>(CreatedAsset);
							DestProp->CopyCompleteValue(DestPtr, SourcePtr);
						}
					}
				}
			}
		}
	}

	// 2. Call PostCreate on all payloads
	for (UObject* PayloadInstance : PayloadInstances)
	{
		if (UGorgeousSetupWizardPayload* Payload = Cast<UGorgeousSetupWizardPayload>(PayloadInstance))
		{
			FString FailureReason;
			if (!Payload->PostCreate(CreatedAsset, FailureReason))
			{
				// Show the error popup as requested
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FailureReason), 
					NSLOCTEXT("GorgeousCore", "WizardPostCreateError", "Generation Failed"));
				
				// We keep the wizard open
				return FReply::Handled();
			}
		}
	}

	// 3. Let the template run its generation logic
	const bool bSuccess = SourceTemplate->ExecuteTemplateGeneration(CreatedAsset, PayloadInstances);

	if (bSuccess)
	{
		// Mark the package as dirty so it's saved
		CreatedAsset->MarkPackageDirty();

		// Open the asset in the editor
		if (GEditor)
		{
			if (UAssetEditorSubsystem* EditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
			{
				EditorSubsystem->OpenEditorForAsset(CreatedAsset);
			}
		}

		// Show success toast
		FNotificationInfo Info(FText::Format(
			NSLOCTEXT("GorgeousCore", "WizardSuccess", "'{0}' created successfully!"),
			FText::FromString(AssetName)));
		Info.ExpireDuration = 4.0f;
		Info.bUseSuccessFailIcons = true;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
	else
	{
		FNotificationInfo Info(NSLOCTEXT("GorgeousCore", "WizardGenFailed", "Template generation failed. Check the output log for details."));
		Info.ExpireDuration = 6.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Handled();
	}

	// Close the wizard window
	TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	if (ParentWindow.IsValid())
	{
		ParentWindow->RequestDestroyWindow();
	}

	return FReply::Handled();
}

FReply SGorgeousSetupWizard::OnCancelClicked()
{
	TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	if (ParentWindow.IsValid())
	{
		ParentWindow->RequestDestroyWindow();
	}
	return FReply::Handled();
}

FReply SGorgeousSetupWizard::OnBrowseFolderClicked()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	FPathPickerConfig PickerConfig;

	PickerConfig.DefaultPath = SavePathTextBox.IsValid() ? SavePathTextBox->GetText().ToString() : TEXT("/Game");
	PickerConfig.bAllowContextMenu = false;
	PickerConfig.bFocusSearchBoxWhenOpened = true;
	PickerConfig.OnPathSelected = FOnPathSelected::CreateSP(this, &SGorgeousSetupWizard::HandleSavePathPicked);

	const TSharedRef<SWidget> PickerWidget = SNew(SBox)
		.WidthOverride(420.0f)
		.HeightOverride(480.0f)
		[
			ContentBrowserModule.Get().CreatePathPicker(PickerConfig)
		];

	if (BrowseFolderButton.IsValid())
	{
		FSlateApplication::Get().PushMenu(
			BrowseFolderButton.ToSharedRef(),
			FWidgetPath(),
			PickerWidget,
			FSlateApplication::Get().GetCursorPos(),
			FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
	}

	return FReply::Handled();
}

void SGorgeousSetupWizard::HandleSavePathPicked(const FString& PickedPath)
{
	if (SavePathTextBox.IsValid())
	{
		SavePathTextBox->SetText(FText::FromString(PickedPath));
	}

	FSlateApplication::Get().DismissAllMenus();
}

//-----------------------------------------------------------------------------
// Validation
//-----------------------------------------------------------------------------

FString SGorgeousSetupWizard::ValidateSavePath() const
{
	const FString SavePath  = SavePathTextBox.IsValid()  ? SavePathTextBox->GetText().ToString().TrimStartAndEnd()  : TEXT("");
	const FString AssetName = AssetNameTextBox.IsValid() ? AssetNameTextBox->GetText().ToString().TrimStartAndEnd() : TEXT("");

	if (SavePath.IsEmpty())
	{
		return TEXT("Save Path cannot be empty.");
	}

	if (!SavePath.StartsWith(TEXT("/Game")) && !SavePath.StartsWith(TEXT("/Plugin")))
	{
		return TEXT("Save Path must start with /Game or /Plugin.");
	}

	if (AssetName.IsEmpty())
	{
		return TEXT("Asset Name cannot be empty.");
	}

	// Basic name validation: no spaces, no special chars
	for (const TCHAR& Ch : AssetName)
	{
		if (!FChar::IsAlnum(Ch) && Ch != TEXT('_'))
		{
			return TEXT("Asset Name must only contain letters, numbers and underscores.");
		}
	}

	return TEXT("");
}

FReply SGorgeousSetupWizard::OnNextClicked()
{
	if (PayloadInstances.IsValidIndex(CurrentPageIndex + 1))
	{
		CurrentPageIndex++;
		UpdateWizardState();
	}
	return FReply::Handled();
}

FReply SGorgeousSetupWizard::OnBackClicked()
{
	if (PayloadInstances.IsValidIndex(CurrentPageIndex - 1))
	{
		CurrentPageIndex--;
		UpdateWizardState();
	}
	return FReply::Handled();
}

void SGorgeousSetupWizard::UpdateWizardState()
{
	if (DetailsView.IsValid() && PayloadInstances.IsValidIndex(CurrentPageIndex))
	{
		DetailsView->SetObject(PayloadInstances[CurrentPageIndex]);
	}

	if (BackButton.IsValid())
	{
		BackButton->SetVisibility(CurrentPageIndex > 0 ? EVisibility::Visible : EVisibility::Collapsed);
	}

	if (NextButton.IsValid())
	{
		NextButton->SetVisibility(IsLastPage() ? EVisibility::Collapsed : EVisibility::Visible);
	}

	if (FinishButton.IsValid())
	{
		FinishButton->SetVisibility(IsLastPage() ? EVisibility::Visible : EVisibility::Collapsed);
	}
}

bool SGorgeousSetupWizard::CanProceedToNextPage(FString& OutFailureReason) const
{
	if (PayloadInstances.IsValidIndex(CurrentPageIndex))
	{
		if (UGorgeousSetupWizardPayload* Payload = Cast<UGorgeousSetupWizardPayload>(PayloadInstances[CurrentPageIndex]))
		{
			return Payload->PreCreate(OutFailureReason);
		}
	}
	return true;
}

bool SGorgeousSetupWizard::IsLastPage() const
{
	return CurrentPageIndex >= PayloadInstances.Num() - 1;
}
