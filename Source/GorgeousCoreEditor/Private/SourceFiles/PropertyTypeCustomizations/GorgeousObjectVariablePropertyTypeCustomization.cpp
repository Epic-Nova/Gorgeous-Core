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

#if 0
#include "GorgeousObjectVariablePropertyTypeCustomization.h"

#include "Algo/Sort.h"
#include "DetailWidgetRow.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "ISinglePropertyView.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "Templates/UniquePtr.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousObjectVariableTrunk.h"
#include "Misc/SecureHash.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UnrealType.h"

namespace
{
static constexpr float HeaderRowSpacing = 4.0f;
static const FName DefaultTrunkPropertyName(TEXT("DefaultObjectVariableTrunk"));
static const FName DefaultValuePropertyName(TEXT("Value"));
static const FName ValuePropertyOverrideMetaKey(TEXT("GorgeousValuePropertyNames"));
static const FName TrunkPropertyOverrideMetaKey(TEXT("GorgeousTrunkProperty"));

/**
 * Checks if the class is a reinstancing skeleton or otherwise invalid for iteration.
 * This can occur during Blueprint hot-reload when classes are being reinstanced.
 */
bool IsClassBeingReinstanced(const UClass* Class)
{
	if (!Class)
	{
		return true;
	}

	// Check for REINST_ or SKEL_ prefix which indicates reinstancing
	const FString ClassName = Class->GetName();
	if (ClassName.StartsWith(TEXT("REINST_")) || ClassName.StartsWith(TEXT("SKEL_")))
	{
		return true;
	}

	// For BlueprintGeneratedClass, verify ClassGeneratedBy is valid
	if (const UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(Class))
	{
		if (!BPClass->ClassGeneratedBy)
		{
			return true;
		}
	}

	return false;
}

// Helper that inspects the class default object to discover which container type it actually exposes.
bool ResolveSupportedContainerType(
	const TSharedPtr<FGorgeousObjectVariablePropertyTypeCustomization::FVariableClassOption>& Option,
	EObjectVariableContainerType_E& OutType)
{
	if (!Option.IsValid())
	{
		return false;
	}

	if (const UClass* Class = Option->Class.Get())
	{
		if (const UGorgeousObjectVariable* CDO = Class->GetDefaultObject<UGorgeousObjectVariable>())
		{
			OutType = CDO->GetObjectVariablePinConfiguration().ContainerType;
			return true;
		}
	}

	return false;
}

bool IsContainerTypeSupported(
	const TSharedPtr<FGorgeousObjectVariablePropertyTypeCustomization::FVariableClassOption>& Option,
	EObjectVariableContainerType_E DesiredType)
{
	EObjectVariableContainerType_E SupportedType;
	return ResolveSupportedContainerType(Option, SupportedType) && SupportedType == DesiredType;
}

FText GetContainerDisplayName(const EObjectVariableContainerType_E Type)
{
	if (const UEnum* ContainerEnum = StaticEnum<EObjectVariableContainerType_E>())
	{
		return ContainerEnum->GetDisplayNameTextByValue(static_cast<int64>(Type));
	}

	return FText::FromString(TEXT("Unknown"));
}
}

#define LOCTEXT_NAMESPACE "GorgeousObjectVariableDetailCustomization"

TSharedRef<IPropertyTypeCustomization> FGorgeousObjectVariablePropertyTypeCustomization::MakeInstance()
{
	return MakeShareable(new FGorgeousObjectVariablePropertyTypeCustomization());
}

void FGorgeousObjectVariablePropertyTypeCustomization::CustomizeHeader(
	const TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	ObjectPropertyHandle = PropertyHandle;
	PropertyUtilities = CustomizationUtils.GetPropertyUtilities();
	TrunkPropertyName = ResolveTrunkPropertyName(PropertyHandle);

	InitializeState(CustomizationUtils);

	HeaderRow.NameContent()[PropertyHandle->CreatePropertyNameWidget()];
	HeaderRow.ValueContent()
		.MinDesiredWidth(400.0f)
		[
			BuildHeaderContent()
		];
}

void FGorgeousObjectVariablePropertyTypeCustomization::CustomizeChildren(
	const TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	ChildBuilder.AddCustomRow(LOCTEXT("GorgeousOVValueFilter", "Gorgeous Object Variable"))
	[
		BuildValueWidget()
	];

	ChildBuilder.AddCustomRow(LOCTEXT("GorgeousOVValueWarningFilter", "Value Warning"))
	[
		BuildValueWarning()
	];

	ChildBuilder.AddCustomRow(LOCTEXT("GorgeousManualOverrideFilter", "Manual Value Overrides"))
	[
		SNew(SBox)
		.Visibility(this, &FGorgeousObjectVariablePropertyTypeCustomization::GetManualOverrideVisibility)
		[
			BuildManualOverrideEditor()
		]
	];
}

void FGorgeousObjectVariablePropertyTypeCustomization::InitializeState(
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	CacheOwnerContexts();
	SynchronizePayloadsFromExistingValues();
	RefreshPreviewFromPayload();
	BuildContainerOptions();
	RefreshClassOptions();
	RefreshSelectedClass();
	RefreshContainerSelection();
	RebuildValuePropertyTargets();
	RebuildManualOverrideState();
	RegisterChangeHandlers();
}

void FGorgeousObjectVariablePropertyTypeCustomization::CacheOwnerContexts()
{
	OwnerContexts.Reset();

	TSharedPtr<IPropertyHandle> Handle = ObjectPropertyHandle.Pin();
	if (!Handle.IsValid())
	{
		return;
	}

	TArray<UObject*> OuterObjects;
	Handle->GetOuterObjects(OuterObjects);

	TArray<void*> RawData;
	Handle->AccessRawData(RawData);

	const FName PreferredTrunkName = TrunkPropertyName.IsNone() ? DefaultTrunkPropertyName : TrunkPropertyName;
	FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Handle->GetProperty());

	for (int32 Index = 0; Index < OuterObjects.Num(); ++Index)
	{
		UObject* Outer = OuterObjects[Index];
		if (!Outer)
		{
			continue;
		}

		FGorgeousObjectVariableTrunk* Trunk = GorgeousObjectVariableTrunkUtils::ResolveTrunk(Outer, PreferredTrunkName);
		if (!Trunk && PreferredTrunkName != DefaultTrunkPropertyName)
		{
			Trunk = GorgeousObjectVariableTrunkUtils::ResolveTrunk(Outer, DefaultTrunkPropertyName);
		}

		FOwnerContext Context;
		Context.Owner = Outer;
		Context.Trunk = Trunk;

		if (ObjectProperty && RawData.IsValidIndex(Index) && RawData[Index])
		{
			if (UGorgeousObjectVariable* Value = Cast<UGorgeousObjectVariable>(ObjectProperty->GetObjectPropertyValue(RawData[Index])))
			{
				Context.Value = Value;
				Context.ResolvedContainerType = Value->GetObjectVariablePinConfiguration().ContainerType;
			}
		}

		Context.Identifier = EnsureIdentifierForContext(Context, Index);

		if (Context.Trunk && Context.Identifier.IsValid())
		{
			Context.Payload = Context.Trunk->FindPayload(Context.Identifier);
			if (!Context.Payload)
			{
				Context.Payload = &Context.Trunk->UpsertPayload(Context.Identifier);
			}

			if (Context.Payload)
			{
				Context.Payload->VariableIdentifier = Context.Identifier;
				Context.ResolvedContainerType = Context.Payload->SelectedContainerType;
			}
		}

		OwnerContexts.Add(Context);
	}

	bHasValidTrunk = OwnerContexts.ContainsByPredicate([](const FOwnerContext& Context)
	{
		return Context.Trunk != nullptr;
	});
}

void FGorgeousObjectVariablePropertyTypeCustomization::SynchronizePayloadsFromExistingValues()
{
	for (FOwnerContext& Context : OwnerContexts)
	{
		UGorgeousObjectVariable* ExistingValue = Context.Value.Get();
		if (!ExistingValue || !Context.Trunk)
		{
			continue;
		}

		if (!Context.Payload)
		{
			Context.Payload = &Context.Trunk->UpsertPayload(Context.Identifier);
		}

		if (!Context.Payload || Context.Payload->SerializedBytes.Num() > 0)
		{
			continue;
		}

		FGorgeousObjectVariableSerialization::WriteObjectToPayload(*ExistingValue, *Context.Payload);
		Context.Trunk->SetPayload(*Context.Payload);
	}
}

void FGorgeousObjectVariablePropertyTypeCustomization::BuildContainerOptions()
{
	EObjectVariableContainerType_E PreferredType = SelectedContainerOption.IsValid()
		? SelectedContainerOption->ContainerType
		: (OwnerContexts.Num() > 0 ? OwnerContexts[0].ResolvedContainerType : EObjectVariableContainerType_E::ESingle);

	ContainerOptions.Reset();

	if (const UEnum* ContainerEnum = StaticEnum<EObjectVariableContainerType_E>())
	{
		const int32 EnumCount = ContainerEnum->NumEnums() - 1; // skip _MAX sentinel
		for (int32 Index = 0; Index < EnumCount; ++Index)
		{
			const EObjectVariableContainerType_E Value = static_cast<EObjectVariableContainerType_E>(ContainerEnum->GetValueByIndex(Index));
			ContainerOptions.Add(MakeShared<FContainerTypeOption>(FContainerTypeOption{Value, ContainerEnum->GetDisplayNameTextByIndex(Index)}));
		}
	}

	SelectedContainerOption = FindContainerOption(PreferredType);
	if (!SelectedContainerOption.IsValid() && ContainerOptions.Num() > 0)
	{
		SelectedContainerOption = ContainerOptions[0];
	}

	if (ContainerComboBox.IsValid())
	{
		ContainerComboBox->RefreshOptions();
		ContainerComboBox->SetSelectedItem(SelectedContainerOption);
	}
}

void FGorgeousObjectVariablePropertyTypeCustomization::RefreshClassOptions()
{
	ClassOptions.Reset();
	UClass* const BaseClass = UGorgeousObjectVariable::StaticClass();

	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Candidate = *It;
		if (!Candidate->IsChildOf(BaseClass))
		{
			continue;
		}

		const bool bIsBlueprint = Candidate->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
		const bool bIsAbstract = Candidate->HasAnyClassFlags(CLASS_Abstract);
		const bool bIsBaseClass = (Candidate == BaseClass);

		if (Candidate->HasAnyClassFlags(CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_Hidden))
		{
			continue;
		}

		// Skip classes that are being reinstanced (hot-reload skeleton classes)
		if (IsClassBeingReinstanced(Candidate))
		{
			continue;
		}

		if (bIsAbstract && !bIsBaseClass)
		{
			continue;
		}

		TSharedPtr<FVariableClassOption> Option = MakeShared<FVariableClassOption>();
		Option->Class = Candidate;
		Option->DisplayName = Candidate->GetDisplayNameText();
		Option->bIsBlueprint = bIsBlueprint;
		Option->bIsAbstractBase = bIsBaseClass;
		ClassOptions.Add(Option);
	}

	ClassOptions.Sort([](const TSharedPtr<FVariableClassOption>& Lhs, const TSharedPtr<FVariableClassOption>& Rhs)
	{
		return Lhs->DisplayName.ToString() < Rhs->DisplayName.ToString();
	});

	FilteredClassOptions = ClassOptions;

	if (ClassListView.IsValid())
	{
		ClassListView->RequestListRefresh();
	}
}

void FGorgeousObjectVariablePropertyTypeCustomization::RefreshSelectedClass()
{
	SelectedClassOption.Reset();

	if (UGorgeousObjectVariable* Value = GetPreviewValue())
	{
		SelectedClassOption = FindOptionByClass(Value->GetClass());
	}

	UpdateContainerCompatibilityWarning();
	RefreshValueWidgetDisplay();

	if (ClassListView.IsValid())
	{
		ClassListView->RequestListRefresh();
	}
}

void FGorgeousObjectVariablePropertyTypeCustomization::RefreshContainerSelection()
{
	SelectedContainerOption.Reset();

	EObjectVariableContainerType_E ResolvedType = EObjectVariableContainerType_E::ESingle;
	bool bHasResolvedType = false;

	if (OwnerContexts.Num() > 0)
	{
		if (OwnerContexts[0].Payload)
		{
			ResolvedType = OwnerContexts[0].Payload->SelectedContainerType;
			bHasResolvedType = true;
		}
		else if (OwnerContexts[0].ResolvedContainerType != EObjectVariableContainerType_E::ESingle
			|| SelectedContainerOption.IsValid())
		{
			ResolvedType = OwnerContexts[0].ResolvedContainerType;
			bHasResolvedType = true;
		}
	}

	if (!bHasResolvedType)
	{
		if (UGorgeousObjectVariable* Preview = GetPreviewValue())
		{
			ResolvedType = Preview->GetObjectVariablePinConfiguration().ContainerType;
			bHasResolvedType = true;
		}
	}

	if (bHasResolvedType)
	{
		SelectedContainerOption = FindContainerOption(ResolvedType);
	}

	if (!SelectedContainerOption.IsValid() && ContainerOptions.Num() > 0)
	{
		SelectedContainerOption = ContainerOptions[0];
	}

	if (ContainerComboBox.IsValid())
	{
		ContainerComboBox->SetSelectedItem(SelectedContainerOption);
	}

	UpdateContainerCompatibilityWarning();
	RefreshValueWidgetDisplay();

	if (ClassListView.IsValid())
	{
		ClassListView->RequestListRefresh();
	}
}

void FGorgeousObjectVariablePropertyTypeCustomization::RebuildValuePropertyTargets()
{
	ValuePropertyNames.Reset();
	bValuePropertyWarningActive = false;
	ValuePropertyWarningText = FText::GetEmpty();

	UClass* TargetClass = nullptr;
	if (SelectedClassOption.IsValid() && SelectedClassOption->Class.IsValid())
	{
		TargetClass = SelectedClassOption->Class.Get();
	}

	if (!TargetClass)
	{
		if (UGorgeousObjectVariable* Value = GetPreviewValue())
		{
			TargetClass = Value->GetClass();
		}
	}

	if (TargetClass)
	{
		if (FProperty* DefaultValueProperty = TargetClass->FindPropertyByName(DefaultValuePropertyName))
		{
			ValuePropertyNames.Add(DefaultValueProperty->GetFName());
		}
		else
		{
			const TArray<FName> ExplicitNames = ResolveExplicitValuePropertyNames();
			if (ExplicitNames.Num() > 0)
			{
				bool bAnyValid = false;
				for (const FName PropertyName : ExplicitNames)
				{
					if (TargetClass->FindPropertyByName(PropertyName))
					{
						ValuePropertyNames.Add(PropertyName);
						bAnyValid = true;
					}
					else
					{
						bValuePropertyWarningActive = true;
						ValuePropertyWarningText = FText::Format(
							LOCTEXT("GorgeousValuePropertyMissing", "'{0}' is not a valid property on {1}."),
							FText::FromName(PropertyName),
							TargetClass->GetDisplayNameText());
						break;
					}
				}

				if (!bAnyValid && !bValuePropertyWarningActive)
				{
					bValuePropertyWarningActive = true;
					ValuePropertyWarningText = FText::Format(
						LOCTEXT("GorgeousValuePropertyOverridesInvalid", "Configured value property overrides do not exist on {0}."),
						TargetClass->GetDisplayNameText());
				}
			}
			else
			{
				bValuePropertyWarningActive = true;
				ValuePropertyWarningText = FText::Format(
					LOCTEXT("GorgeousValuePropertyDefaultMissing", "{0} does not declare a property named 'Value'. Add GorgeousValuePropertyNames metadata to the owning property to specify the fields to expose."),
					TargetClass->GetDisplayNameText());
			}
		}
	}

	RefreshValueWidgetDisplay();
	RebuildValuePropertyEditors();
}

void FGorgeousObjectVariablePropertyTypeCustomization::RefreshValueWidgetDisplay()
{
	if (TSharedPtr<STextBlock> TitleLabel = ValueSummaryTitleText.Pin())
	{
		TitleLabel->SetText(GetValueSummaryTitleText());
	}

	if (TSharedPtr<STextBlock> DescriptionLabel = ValueSummaryDescriptionText.Pin())
	{
		DescriptionLabel->SetText(GetValueSummaryDescriptionText());
	}

	if (TSharedPtr<STextBlock> FieldListLabel = ValueSummaryFieldsText.Pin())
	{
		FieldListLabel->SetText(GetValueFieldListText());
	}
}

void FGorgeousObjectVariablePropertyTypeCustomization::RebuildValuePropertyEditors()
{
	ActiveValuePropertyEditors.Reset();

	TSharedPtr<SVerticalBox> EditorContainer = ValuePropertyEditorContainer.Pin();
	if (!EditorContainer.IsValid())
	{
		return;
	}

	EditorContainer->ClearChildren();

	if (!ValuePropertyChangedDelegate.IsBound())
	{
		ValuePropertyChangedDelegate = FSimpleDelegate::CreateSP(this, &FGorgeousObjectVariablePropertyTypeCustomization::HandleValueWidgetPropertyChanged);
	}

	UGorgeousObjectVariable* Preview = GetPreviewValue();
	if (!Preview)
	{
		EditorContainer->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GorgeousOVNoPreview", "Preview object is not available."))
				.AutoWrapText(true)
			];
		return;
	}

	if (ValuePropertyNames.Num() == 0)
	{
		EditorContainer->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GorgeousOVNoValueProperties", "No value fields are configured for this class."))
				.AutoWrapText(true)
			];
		return;
	}

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	const bool bHideNames = ValuePropertyNames.Num() == 1;
	FSinglePropertyParams PropertyParams;
	PropertyParams.NamePlacement = bHideNames ? EPropertyNamePlacement::Hidden : EPropertyNamePlacement::Left;

	bool bCreatedAnyEditors = false;
	for (const FName PropertyName : ValuePropertyNames)
	{
		if (!Preview->GetClass()->FindPropertyByName(PropertyName))
		{
			continue;
		}

		TSharedPtr<ISinglePropertyView> SinglePropertyView = PropertyEditorModule.CreateSingleProperty(Preview, PropertyName, PropertyParams);
		if (!SinglePropertyView.IsValid() || !SinglePropertyView->HasValidProperty())
		{
			continue;
		}

		SinglePropertyView->SetOnPropertyValueChanged(ValuePropertyChangedDelegate);
		ActiveValuePropertyEditors.Add(SinglePropertyView);
		bCreatedAnyEditors = true;

		EditorContainer->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SinglePropertyView->AsShared()
			];
	}

	if (!bCreatedAnyEditors)
	{
		EditorContainer->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GorgeousOVNoEditorAvailable", "The configured value fields cannot be edited in this view."))
				.AutoWrapText(true)
			];
	}
}

TArray<FName> FGorgeousObjectVariablePropertyTypeCustomization::ResolveExplicitValuePropertyNames() const
{
	TArray<FName> Result;
	TSharedPtr<IPropertyHandle> Handle = ObjectPropertyHandle.Pin();
	if (!Handle.IsValid())
	{
		return Result;
	}

	FString OverrideList = Handle->GetMetaData(ValuePropertyOverrideMetaKey);
	if (OverrideList.IsEmpty())
	{
		return Result;
	}

	OverrideList.ReplaceInline(TEXT(";"), TEXT(","));
	OverrideList.ReplaceInline(TEXT("|"), TEXT(","));

	TArray<FString> Tokens;
	OverrideList.ParseIntoArray(Tokens, TEXT(","), true);
	for (FString& Token : Tokens)
	{
		Token.TrimStartAndEndInline();
		if (!Token.IsEmpty())
		{
			Result.Add(*Token);
		}
	}

	return Result;
}

void FGorgeousObjectVariablePropertyTypeCustomization::RegisterChangeHandlers()
{
	if (TSharedPtr<IPropertyHandle> Handle = ObjectPropertyHandle.Pin())
	{
		Handle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(
			this, &FGorgeousObjectVariablePropertyTypeCustomization::HandlePropertyValueChanged));
	}
}

void FGorgeousObjectVariablePropertyTypeCustomization::HandlePropertyValueChanged()
{
	CacheOwnerContexts();
	SynchronizePayloadsFromExistingValues();
	CacheOwnerContexts();
	RefreshPreviewFromPayload();
	RefreshSelectedClass();
	RefreshContainerSelection();
	RebuildManualOverrideState();
	RebuildValuePropertyTargets();
}

void FGorgeousObjectVariablePropertyTypeCustomization::HandleValueWidgetPropertyChanged()
{
	CommitPreviewToPayload(LOCTEXT("ModifyGorgeousObjectVariableValue", "Edit Object Variable Value"));
	CacheOwnerContexts();
	RefreshPreviewFromPayload();
	RefreshSelectedClass();
	RefreshContainerSelection();
	RebuildValuePropertyTargets();
}

FName FGorgeousObjectVariablePropertyTypeCustomization::ResolveTrunkPropertyName(const TSharedPtr<IPropertyHandle>& Handle) const
{
	if (!Handle.IsValid())
	{
		return NAME_None;
	}

	const FString OverrideName = Handle->GetMetaData(TrunkPropertyOverrideMetaKey);
	return OverrideName.IsEmpty() ? NAME_None : FName(*OverrideName);
}

UGorgeousObjectVariable* FGorgeousObjectVariablePropertyTypeCustomization::GetPreviewValue() const
{
	return PreviewVariable.Get();
}

FGuid FGorgeousObjectVariablePropertyTypeCustomization::EnsureIdentifierForContext(FOwnerContext& Context, int32 ContextIndex)
{
	if (Context.Identifier.IsValid())
	{
		return Context.Identifier;
	}

	if (UGorgeousObjectVariable* Value = Context.Value.Get())
	{
		if (Value->UniqueIdentifier.IsValid())
		{
			Context.Identifier = Value->UniqueIdentifier;
			return Context.Identifier;
		}
	}

	if (Context.Payload && Context.Payload->VariableIdentifier.IsValid())
	{
		Context.Identifier = Context.Payload->VariableIdentifier;
		return Context.Identifier;
	}

	Context.Identifier = BuildDeterministicIdentifier(Context.Owner.Get(), ContextIndex);
	return Context.Identifier;
}

FGuid FGorgeousObjectVariablePropertyTypeCustomization::BuildDeterministicIdentifier(UObject* Owner, int32 OwnerIndex) const
{
	if (!Owner)
	{
		return FGuid::NewGuid();
	}

	const FString OwnerPath = Owner->GetPathName();
	FSHA1 Hasher;
	Hasher.Update(reinterpret_cast<const uint8*>(*OwnerPath), OwnerPath.Len() * sizeof(TCHAR));
	Hasher.Update(reinterpret_cast<const uint8*>(&OwnerIndex), sizeof(OwnerIndex));
	Hasher.Final();

	uint32 Digest[5] = {};
	Hasher.GetHash(reinterpret_cast<uint8*>(Digest));
	return FGuid(Digest[0], Digest[1], Digest[2], Digest[3]);
}

void FGorgeousObjectVariablePropertyTypeCustomization::EnsurePreviewInstance(UClass* DesiredClass)
{
	UClass* TargetClass = DesiredClass;
	if (!TargetClass)
	{
		if (SelectedClassOption.IsValid() && SelectedClassOption->Class.IsValid())
		{
			TargetClass = SelectedClassOption->Class.Get();
		}
		else if (OwnerContexts.Num() > 0 && OwnerContexts[0].Payload && OwnerContexts[0].Payload->VariableClass)
		{
			TargetClass = OwnerContexts[0].Payload->VariableClass.Get();
		}
	}

	if (!TargetClass)
	{
		TargetClass = UGorgeousObjectVariable::StaticClass();
	}

	if (PreviewVariable.IsValid() && PreviewVariable->IsA(TargetClass))
	{
		if (!PreviewVariable->UniqueIdentifier.IsValid() && OwnerContexts.Num() > 0 && OwnerContexts[0].Identifier.IsValid())
		{
			PreviewVariable->UniqueIdentifier = OwnerContexts[0].Identifier;
		}
		return;
	}

	PreviewVariable.Reset();
	PreviewVariable = TStrongObjectPtr<UGorgeousObjectVariable>(NewObject<UGorgeousObjectVariable>(GetTransientPackage(), TargetClass, NAME_None, RF_Transactional));
	if (PreviewVariable.IsValid() && OwnerContexts.Num() > 0 && OwnerContexts[0].Identifier.IsValid())
	{
		PreviewVariable->UniqueIdentifier = OwnerContexts[0].Identifier;
	}
}

void FGorgeousObjectVariablePropertyTypeCustomization::RefreshPreviewFromPayload()
{
	FGorgeousObjectVariableSerializedPayload* SourcePayload = nullptr;
	for (FOwnerContext& Context : OwnerContexts)
	{
		if (Context.Payload && Context.Payload->SerializedBytes.Num() > 0)
		{
			SourcePayload = Context.Payload;
			break;
		}
	}

	UClass* DesiredClass = nullptr;
	if (SourcePayload && SourcePayload->VariableClass)
	{
		DesiredClass = SourcePayload->VariableClass.Get();
	}
	else if (SelectedClassOption.IsValid() && SelectedClassOption->Class.IsValid())
	{
		DesiredClass = SelectedClassOption->Class.Get();
	}

	EnsurePreviewInstance(DesiredClass);

	if (!PreviewVariable.IsValid())
	{
		return;
	}

	if (SourcePayload)
	{
		FGorgeousObjectVariableSerialization::LoadObjectFromPayload(*PreviewVariable, *SourcePayload);
		if (SourcePayload->VariableIdentifier.IsValid())
		{
			PreviewVariable->UniqueIdentifier = SourcePayload->VariableIdentifier;
		}
	}
	else if (OwnerContexts.Num() > 0 && OwnerContexts[0].Identifier.IsValid())
	{
		PreviewVariable->UniqueIdentifier = OwnerContexts[0].Identifier;
	}
}

void FGorgeousObjectVariablePropertyTypeCustomization::CommitPreviewToPayload(const FText& TransactionContext)
{
	UGorgeousObjectVariable* Preview = GetPreviewValue();
	if (!Preview)
	{
		return;
	}

	TUniquePtr<FScopedTransaction> Transaction;
	bool bUpdatedAnyPayload = false;
	for (FOwnerContext& Context : OwnerContexts)
	{
		if (!Context.Trunk)
		{
			continue;
		}

		if (!Context.Payload)
		{
			Context.Payload = &Context.Trunk->UpsertPayload(Context.Identifier);
		}

		if (!Context.Payload)
		{
			continue;
		}

		if (!Transaction)
		{
			Transaction = MakeUnique<FScopedTransaction>(TransactionContext);
		}

		FGorgeousObjectVariableSerializedPayload PayloadCopy = *Context.Payload;
		FGorgeousObjectVariableSerialization::WriteObjectToPayload(*Preview, PayloadCopy);
		if (Context.Identifier.IsValid())
		{
			PayloadCopy.VariableIdentifier = Context.Identifier;
		}
		else
		{
			Context.Identifier = PayloadCopy.VariableIdentifier;
		}

		Context.Trunk->SetPayload(PayloadCopy);
		Context.Payload = Context.Trunk->FindPayload(Context.Identifier);
		if (Context.Payload)
		{
			Context.ResolvedContainerType = Context.Payload->SelectedContainerType;
		}

		bUpdatedAnyPayload = true;
	}

	if (bUpdatedAnyPayload)
	{
		RefreshPreviewFromPayload();
	}
}

TArray<FName> FGorgeousObjectVariablePropertyTypeCustomization::GatherConfiguredValuePropertyNames() const
{
	return ResolveExplicitValuePropertyNames();
}

bool FGorgeousObjectVariablePropertyTypeCustomization::ShouldDisplayManualOverrideEditor() const
{
	const int32 ExplicitNameCount = ResolveExplicitValuePropertyNames().Num();
	return bValuePropertyWarningActive || ExplicitNameCount > 0;
}

void FGorgeousObjectVariablePropertyTypeCustomization::RebuildManualOverrideState()
{
	ManualValueEntries.Reset();
	const TArray<FName> ConfiguredNames = GatherConfiguredValuePropertyNames();
	for (const FName FieldName : ConfiguredNames)
	{
		TSharedPtr<FManualValueEntry> Entry = MakeShared<FManualValueEntry>();
		Entry->FieldName = FieldName;
		Entry->bIsValid = !FieldName.IsNone();
		ManualValueEntries.Add(Entry);
	}

	bDisplayManualOverrideEditor = ShouldDisplayManualOverrideEditor();
	RefreshManualValueRows();
}

void FGorgeousObjectVariablePropertyTypeCustomization::RefreshManualValueRows()
{
	if (ManualValueListView.IsValid())
	{
		ManualValueListView->RequestListRefresh();
	}
}

void FGorgeousObjectVariablePropertyTypeCustomization::SynchronizeManualOverridesToPayload(const FText& TransactionContext)
{
	FString OverrideBuffer;
	for (const TSharedPtr<FManualValueEntry>& Entry : ManualValueEntries)
	{
		if (!Entry.IsValid())
		{
			continue;
		}

		const bool bEntryValid = !Entry->FieldName.IsNone();
		Entry->bIsValid = bEntryValid;
		if (!bEntryValid)
		{
			continue;
		}

		if (!OverrideBuffer.IsEmpty())
		{
			OverrideBuffer.Append(TEXT(","));
		}

		OverrideBuffer.Append(Entry->FieldName.ToString());
	}

	TUniquePtr<FScopedTransaction> Transaction;
	if (!TransactionContext.IsEmpty())
	{
		Transaction = MakeUnique<FScopedTransaction>(TransactionContext);
	}

	if (TSharedPtr<IPropertyHandle> Handle = ObjectPropertyHandle.Pin())
	{
		Handle->SetInstanceMetaData(ValuePropertyOverrideMetaKey, OverrideBuffer);
		if (FProperty* Property = Handle->GetProperty())
		{
			Property->SetMetaData(ValuePropertyOverrideMetaKey, *OverrideBuffer);
		}
	}

	RebuildValuePropertyTargets();
	bDisplayManualOverrideEditor = ShouldDisplayManualOverrideEditor();
	RefreshManualValueRows();
}

TSharedRef<SWidget> FGorgeousObjectVariablePropertyTypeCustomization::BuildManualOverrideEditor()
{
	return SNew(SBorder)
		.Padding(8.0f)
		.BorderImage(FAppStyle::GetBrush("DetailsView.CategoryTop"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 6.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ManualOverrideDescription", "Specify the property names that should be treated as the value when this class does not expose a 'Value' member."))
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(ManualValueListView, SListView<TSharedPtr<FManualValueEntry>>)
				.ListItemsSource(&ManualValueEntries)
				.SelectionMode(ESelectionMode::None)
				.OnGenerateRow(this, &FGorgeousObjectVariablePropertyTypeCustomization::HandleGenerateManualOverrideRow)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton"))
				.ContentPadding(FMargin(6.0f, 2.0f))
				.OnClicked_Lambda([this]()
				{
					HandleAddManualOverride();
					return FReply::Handled();
				})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ManualOverrideAddButton", "Add Property"))
				]
			]
		];
}

TSharedRef<ITableRow> FGorgeousObjectVariablePropertyTypeCustomization::HandleGenerateManualOverrideRow(
	TSharedPtr<FManualValueEntry> Entry, const TSharedRef<STableViewBase>& OwnerTable)
{
	const bool bEntryValid = Entry.IsValid() && Entry->bIsValid;
	return SNew(STableRow<TSharedPtr<FManualValueEntry>>, OwnerTable)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SEditableTextBox)
			.Text(Entry.IsValid() && !Entry->FieldName.IsNone() ? FText::FromName(Entry->FieldName) : FText::GetEmpty())
			.SelectAllTextOnCommit(true)
			.RevertTextOnEscape(true)
			.ForegroundColor(bEntryValid ? FSlateColor::UseForeground() : FSlateColor(FLinearColor::Red))
			.OnTextCommitted_Lambda([this, Entry](const FText& NewText, ETextCommit::Type CommitType)
			{
				const int32 EntryIndex = ManualValueEntries.IndexOfByKey(Entry);
				HandleManualOverrideCommitted(NewText, CommitType, EntryIndex);
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(6.0f, 0.0f)
		[
			SNew(SButton)
			.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton"))
			.ContentPadding(FMargin(4.0f, 0.0f))
			.OnClicked_Lambda([this, Entry]()
			{
				const int32 EntryIndex = ManualValueEntries.IndexOfByKey(Entry);
				HandleRemoveManualOverride(EntryIndex);
				return FReply::Handled();
			})
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ManualOverrideRemoveButton", "Remove"))
			]
		]
	];
}

void FGorgeousObjectVariablePropertyTypeCustomization::HandleManualOverrideCommitted(
	const FText& NewText, ETextCommit::Type CommitType, int32 EntryIndex)
{
	if (!ManualValueEntries.IsValidIndex(EntryIndex))
	{
		return;
	}

	TSharedPtr<FManualValueEntry>& Entry = ManualValueEntries[EntryIndex];
	if (!Entry.IsValid())
	{
		return;
	}

	FString Sanitized = NewText.ToString();
	Sanitized.TrimStartAndEndInline();
	Entry->FieldName = Sanitized.IsEmpty() ? NAME_None : FName(*Sanitized);
	Entry->bIsValid = !Sanitized.IsEmpty();
	SynchronizeManualOverridesToPayload(LOCTEXT("ManualOverrideCommitTransaction", "Edit Manual Value Override"));
}

void FGorgeousObjectVariablePropertyTypeCustomization::HandleRemoveManualOverride(int32 EntryIndex)
{
	if (!ManualValueEntries.IsValidIndex(EntryIndex))
	{
		return;
	}

	ManualValueEntries.RemoveAt(EntryIndex);
	RefreshManualValueRows();
	SynchronizeManualOverridesToPayload(LOCTEXT("ManualOverrideRemoveTransaction", "Remove Manual Value Override"));
}

void FGorgeousObjectVariablePropertyTypeCustomization::HandleAddManualOverride()
{
	TSharedPtr<FManualValueEntry> NewEntry = MakeShared<FManualValueEntry>();
	NewEntry->FieldName = NAME_None;
	NewEntry->bIsValid = false;
	ManualValueEntries.Add(NewEntry);
	RefreshManualValueRows();
}

TSharedPtr<FGorgeousObjectVariablePropertyTypeCustomization::FVariableClassOption>
FGorgeousObjectVariablePropertyTypeCustomization::FindOptionByClass(UClass* InClass) const
{
	if (!InClass)
	{
		return nullptr;
	}

	for (const TSharedPtr<FVariableClassOption>& Option : ClassOptions)
	{
		if (Option->Class.Get() == InClass)
		{
			return Option;
		}
	}

	return nullptr;
}

TSharedPtr<FGorgeousObjectVariablePropertyTypeCustomization::FContainerTypeOption>
FGorgeousObjectVariablePropertyTypeCustomization::FindContainerOption(EObjectVariableContainerType_E InType) const
{
	for (const TSharedPtr<FContainerTypeOption>& Option : ContainerOptions)
	{
		if (Option->ContainerType == InType)
		{
			return Option;
		}
	}

	return nullptr;
}

void FGorgeousObjectVariablePropertyTypeCustomization::CreateOrReplaceValue(UClass* DesiredClass)
{
	if (!DesiredClass)
	{
		return;
	}

	EnsurePreviewInstance(DesiredClass);
	if (!PreviewVariable.IsValid())
	{
		return;
	}

	SelectedClassOption = FindOptionByClass(DesiredClass);

	EObjectVariableContainerType_E SupportedType;
	if (SelectedClassOption.IsValid() && ResolveSupportedContainerType(SelectedClassOption, SupportedType))
	{
		ApplyContainerTypeToPreview(SupportedType);
		if (!SelectedContainerOption.IsValid() || SelectedContainerOption->ContainerType != SupportedType)
		{
			SelectedContainerOption = FindContainerOption(SupportedType);
			if (ContainerComboBox.IsValid())
			{
				ContainerComboBox->SetSelectedItem(SelectedContainerOption);
			}
		}
	}

	UpdateContainerCompatibilityWarning();
	RebuildValuePropertyTargets();
	RebuildManualOverrideState();

	CommitPreviewToPayload(LOCTEXT("SetGorgeousObjectVariableClass", "Set Object Variable Class"));
	CacheOwnerContexts();
	RefreshPreviewFromPayload();
	RefreshContainerSelection();
	RebuildValuePropertyTargets();
}

void FGorgeousObjectVariablePropertyTypeCustomization::UpdateContainerType(EObjectVariableContainerType_E InType)
{
	if (UGorgeousObjectVariable* Value = GetPreviewValue())
	{
		if (const FStructProperty* PinConfigProperty = FindFProperty<FStructProperty>(UGorgeousObjectVariable::StaticClass(), GET_MEMBER_NAME_CHECKED(UGorgeousObjectVariable, PinConfiguration)))
		{
			if (FObjectVariablePinConfiguration_S* PinConfiguration = PinConfigProperty->ContainerPtrToValuePtr<FObjectVariablePinConfiguration_S>(Value))
			{
				if (PinConfiguration->ContainerType != InType)
				{
					PinConfiguration->ContainerType = InType;
					CommitPreviewToPayload(LOCTEXT("SetGorgeousObjectVariableContainer", "Set Object Variable Container"));
					RefreshContainerSelection();
					RebuildValuePropertyTargets();
				}
			}
		}
	}
}

void FGorgeousObjectVariablePropertyTypeCustomization::ApplyContainerTypeToPreview(EObjectVariableContainerType_E InType)
{
	if (UGorgeousObjectVariable* Value = GetPreviewValue())
	{
		if (const FStructProperty* PinConfigProperty = FindFProperty<FStructProperty>(UGorgeousObjectVariable::StaticClass(), GET_MEMBER_NAME_CHECKED(UGorgeousObjectVariable, PinConfiguration)))
		{
			if (FObjectVariablePinConfiguration_S* PinConfiguration = PinConfigProperty->ContainerPtrToValuePtr<FObjectVariablePinConfiguration_S>(Value))
			{
				PinConfiguration->ContainerType = InType;
			}
		}
	}
}

TSharedRef<SWidget> FGorgeousObjectVariablePropertyTypeCustomization::BuildHeaderContent()
{
	TSharedRef<SVerticalBox> Container = SNew(SVerticalBox);

	Container->AddSlot()
		.AutoHeight()
		[
			BuildClassPicker()
		];

	Container->AddSlot()
		.AutoHeight()
		.Padding(0.0f, HeaderRowSpacing, 0.0f, 0.0f)
		[
			BuildContainerSelector()
		];

	Container->AddSlot()
		.AutoHeight()
		.Padding(0.0f, HeaderRowSpacing, 0.0f, 0.0f)
		[
			BuildContainerWarning()
		];

	if (!bHasValidTrunk)
	{
		Container->AddSlot()
			.AutoHeight()
			.Padding(0.0f, HeaderRowSpacing, 0.0f, 0.0f)
			[
				BuildTrunkWarning()
			];
	}

	return Container;
}

TSharedRef<SWidget> FGorgeousObjectVariablePropertyTypeCustomization::BuildValueWidget()
{
	TSharedPtr<STextBlock> TitleText;
	TSharedPtr<STextBlock> DescriptionText;
	TSharedPtr<STextBlock> FieldsText;
	TSharedPtr<SVerticalBox> EditorContainer;

	TSharedRef<SWidget> Widget =
		SNew(SBorder)
		.Padding(10.0f)
		.BorderImage(FAppStyle::GetBrush("DetailsView.CategoryTop"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GorgeousOVValueLabel", "Value"))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
				.Font(FAppStyle::Get().GetFontStyle("DetailsView.CategoryTextStyle"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SAssignNew(TitleText, STextBlock)
				.Text(GetValueSummaryTitleText())
				.Font(FAppStyle::Get().GetFontStyle("DetailsView.CategoryFontStyle"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 2.0f, 0.0f, 0.0f)
			[
				SAssignNew(DescriptionText, STextBlock)
				.Text(GetValueSummaryDescriptionText())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 2.0f, 0.0f, 0.0f)
			[
				SAssignNew(FieldsText, STextBlock)
				.Text(GetValueFieldListText())
				.AutoWrapText(true)
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 8.0f, 0.0f, 0.0f)
			[
				SAssignNew(EditorContainer, SVerticalBox)
			]
		];

	ValueSummaryTitleText = TitleText;
	ValueSummaryDescriptionText = DescriptionText;
	ValueSummaryFieldsText = FieldsText;
	ValuePropertyEditorContainer = EditorContainer;
	RefreshValueWidgetDisplay();
	RebuildValuePropertyEditors();

	return Widget;
}

TSharedRef<SWidget> FGorgeousObjectVariablePropertyTypeCustomization::BuildClassPicker()
{
	return SAssignNew(ClassPickerButton, SComboButton)
		.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("PropertyEditor.AssetComboStyle"))
		.ForegroundColor(FSlateColor::UseForeground())
		.OnGetMenuContent(this, &FGorgeousObjectVariablePropertyTypeCustomization::BuildClassPickerMenu)
		.ContentPadding(FMargin(6.0f, 2.0f))
		.ButtonContent()
		[
			SNew(STextBlock)
			.Text(this, &FGorgeousObjectVariablePropertyTypeCustomization::GetSelectedClassText)
		];
}

TSharedRef<SWidget> FGorgeousObjectVariablePropertyTypeCustomization::BuildContainerSelector()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ContainerTypeLabel", "Container"))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SAssignNew(ContainerComboBox, SComboBox<TSharedPtr<FContainerTypeOption>>)
			.OptionsSource(&ContainerOptions)
			.InitiallySelectedItem(SelectedContainerOption)
			.OnComboBoxOpening(this, &FGorgeousObjectVariablePropertyTypeCustomization::HandleContainerComboOpening)
			.OnSelectionChanged(this, &FGorgeousObjectVariablePropertyTypeCustomization::HandleContainerSelectionChanged)
			.OnGenerateWidget(this, &FGorgeousObjectVariablePropertyTypeCustomization::HandleGenerateContainerRow)
			[
				SNew(STextBlock)
				.Text(this, &FGorgeousObjectVariablePropertyTypeCustomization::GetSelectedContainerText)
			]
		];
}

TSharedRef<SWidget> FGorgeousObjectVariablePropertyTypeCustomization::BuildContainerWarning()
{
	return SNew(STextBlock)
		.Visibility(this, &FGorgeousObjectVariablePropertyTypeCustomization::GetContainerWarningVisibility)
		.Text(this, &FGorgeousObjectVariablePropertyTypeCustomization::GetContainerWarningText)
		.AutoWrapText(true)
		.Justification(ETextJustify::Left)
		.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.4f, 0.0f)));
}

TSharedRef<SWidget> FGorgeousObjectVariablePropertyTypeCustomization::BuildValueWarning()
{
	return SNew(STextBlock)
		.Visibility(this, &FGorgeousObjectVariablePropertyTypeCustomization::GetValueWarningVisibility)
		.Text(this, &FGorgeousObjectVariablePropertyTypeCustomization::GetValueWarningText)
		.AutoWrapText(true)
		.ColorAndOpacity(FLinearColor::Red);
}

TSharedRef<SWidget> FGorgeousObjectVariablePropertyTypeCustomization::BuildTrunkWarning()
{
	return SNew(STextBlock)
		.Text(LOCTEXT("MissingTrunkWarning", "No Gorgeous Object Variable trunk was found on this object. Defaults will not be persisted."))
		.Justification(ETextJustify::Left)
		.ColorAndOpacity(FLinearColor::Red);
}

TSharedRef<SWidget> FGorgeousObjectVariablePropertyTypeCustomization::BuildClassPickerMenu()
{
	RefreshClassOptions();
	ClassFilterText = FText::GetEmpty();
	FilteredClassOptions = ClassOptions;

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Menu.Background"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f)
			[
				SAssignNew(ClassSearchBox, SSearchBox)
				.HintText(LOCTEXT("ObjectVariableSearchHint", "Search classes"))
				.OnTextChanged(this, &FGorgeousObjectVariablePropertyTypeCustomization::HandleClassSearchChanged)
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.MaxHeight(360.0f)
			[
				SAssignNew(ClassListView, SListView<TSharedPtr<FVariableClassOption>>)
				.ListItemsSource(&FilteredClassOptions)
				.SelectionMode(ESelectionMode::Single)
				.OnGenerateRow(this, &FGorgeousObjectVariablePropertyTypeCustomization::HandleGenerateClassRow)
				.OnSelectionChanged(this, &FGorgeousObjectVariablePropertyTypeCustomization::HandleClassSelectionChanged)
			]
		];
}

void FGorgeousObjectVariablePropertyTypeCustomization::HandleClassSelectionChanged(
	TSharedPtr<FVariableClassOption> Option, ESelectInfo::Type SelectInfo)
{
	if (!Option.IsValid() || SelectInfo == ESelectInfo::Direct)
	{
		return;
	}

	if (TSharedPtr<SComboButton> ComboButton = ClassPickerButton.Pin())
	{
		ComboButton->SetIsOpen(false);
	}

	CreateOrReplaceValue(Option->Class.Get());
}

TSharedRef<ITableRow> FGorgeousObjectVariablePropertyTypeCustomization::HandleGenerateClassRow(
	TSharedPtr<FVariableClassOption> Option, const TSharedRef<STableViewBase>& OwnerTable)
{
	EObjectVariableContainerType_E SupportedType;
	const bool bHasSupportedType = ResolveSupportedContainerType(Option, SupportedType);
	const EObjectVariableContainerType_E ActiveContainer = GetActiveContainerType();
	const bool bSupportsActiveContainer = !bHasSupportedType || SupportedType == ActiveContainer;
	const FSlateColor NameColor = Option->bIsBlueprint
		? FSlateColor(FLinearColor(0.2f, 0.6f, 1.0f))
		: FSlateColor::UseForeground();

	return SNew(STableRow<TSharedPtr<FVariableClassOption>>, OwnerTable)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(Option->DisplayName)
				.ColorAndOpacity(NameColor)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 2.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Visibility(bSupportsActiveContainer ? EVisibility::Collapsed : EVisibility::Visible)
				.Text(bHasSupportedType
					? FText::Format(LOCTEXT("UnsupportedClassContainerRow", "Selecting will switch to {0}"), GetContainerDisplayName(SupportedType))
					: LOCTEXT("UnknownClassContainerRow", "Container requirements unknown"))
				.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.4f, 0.0f)))
				.AutoWrapText(true)
			]
		];
}

void FGorgeousObjectVariablePropertyTypeCustomization::HandleClassSearchChanged(const FText& InSearchText)
{
	ClassFilterText = InSearchText;
	const FString FilterString = InSearchText.ToString();

	FilteredClassOptions.Reset();
	for (const TSharedPtr<FVariableClassOption>& Option : ClassOptions)
	{
		if (FilterString.IsEmpty() || Option->DisplayName.ToString().Contains(FilterString, ESearchCase::IgnoreCase))
		{
			FilteredClassOptions.Add(Option);
		}
	}

	if (ClassListView.IsValid())
	{
		ClassListView->RequestListRefresh();
	}
}

void FGorgeousObjectVariablePropertyTypeCustomization::HandleContainerComboOpening()
{
	BuildContainerOptions();
}

void FGorgeousObjectVariablePropertyTypeCustomization::HandleContainerSelectionChanged(
	TSharedPtr<FContainerTypeOption> Option, ESelectInfo::Type SelectInfo)
{
	if (!Option.IsValid() || SelectInfo == ESelectInfo::Direct)
	{
		return;
	}

	SelectedContainerOption = Option;
	UpdateContainerType(Option->ContainerType);
	UpdateContainerCompatibilityWarning();

	if (ClassListView.IsValid())
	{
		ClassListView->RequestListRefresh();
	}
}

TSharedRef<SWidget> FGorgeousObjectVariablePropertyTypeCustomization::HandleGenerateContainerRow(
	TSharedPtr<FContainerTypeOption> Option)
{
	const bool bIsSupported = !SelectedClassOption.IsValid() || IsContainerTypeSupported(SelectedClassOption, Option->ContainerType);
	return SNew(STextBlock)
		.Text(Option->Label)
		.ToolTipText(bIsSupported
			? Option->Label
			: FText::Format(LOCTEXT("UnsupportedContainerTooltip", "Current type cannot use {0} containers."), Option->Label))
		.ColorAndOpacity(bIsSupported ? FSlateColor::UseForeground() : FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)));
}

EObjectVariableContainerType_E FGorgeousObjectVariablePropertyTypeCustomization::GetActiveContainerType() const
{
	if (SelectedContainerOption.IsValid())
	{
		return SelectedContainerOption->ContainerType;
	}

	if (OwnerContexts.Num() > 0 && OwnerContexts[0].Payload)
	{
		return OwnerContexts[0].Payload->SelectedContainerType;
	}

	if (const UGorgeousObjectVariable* Preview = GetPreviewValue())
	{
		return Preview->GetObjectVariablePinConfiguration().ContainerType;
	}

	if (OwnerContexts.Num() > 0)
	{
		return OwnerContexts[0].ResolvedContainerType;
	}

	return EObjectVariableContainerType_E::ESingle;
}

void FGorgeousObjectVariablePropertyTypeCustomization::UpdateContainerCompatibilityWarning()
{
	if (!SelectedClassOption.IsValid() || !SelectedContainerOption.IsValid())
	{
		bContainerWarningActive = false;
		ContainerWarningText = FText::GetEmpty();
		return;
	}

	EObjectVariableContainerType_E SupportedType;
	if (ResolveSupportedContainerType(SelectedClassOption, SupportedType)
		&& SupportedType != SelectedContainerOption->ContainerType)
	{
		bContainerWarningActive = true;
		ContainerWarningText = FText::Format(
			LOCTEXT("ContainerSelectionMismatch", "{0} supports {1} containers, but {2} is currently selected."),
			SelectedClassOption->DisplayName,
			GetContainerDisplayName(SupportedType),
			GetContainerDisplayName(SelectedContainerOption->ContainerType));
	}
	else
	{
		bContainerWarningActive = false;
		ContainerWarningText = FText::GetEmpty();
	}
}

FText FGorgeousObjectVariablePropertyTypeCustomization::GetSelectedClassText() const
{
	if (SelectedClassOption.IsValid())
	{
		return SelectedClassOption->DisplayName;
	}

	return LOCTEXT("NoGorgeousObjectVariableClass", "Select Object Variable Class");
}

FText FGorgeousObjectVariablePropertyTypeCustomization::GetSelectedContainerText() const
{
	if (SelectedContainerOption.IsValid())
	{
		return SelectedContainerOption->Label;
	}

	return LOCTEXT("NoGorgeousContainerType", "Select Container");
}

FText FGorgeousObjectVariablePropertyTypeCustomization::GetContainerWarningText() const
{
	return ContainerWarningText;
}

EVisibility FGorgeousObjectVariablePropertyTypeCustomization::GetContainerWarningVisibility() const
{
	return bContainerWarningActive ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility FGorgeousObjectVariablePropertyTypeCustomization::GetValueWarningVisibility() const
{
	return bValuePropertyWarningActive ? EVisibility::Visible : EVisibility::Collapsed;
}

FText FGorgeousObjectVariablePropertyTypeCustomization::GetValueWarningText() const
{
	return ValuePropertyWarningText;
}

EVisibility FGorgeousObjectVariablePropertyTypeCustomization::GetManualOverrideVisibility() const
{
	return bDisplayManualOverrideEditor ? EVisibility::Visible : EVisibility::Collapsed;
}

FText FGorgeousObjectVariablePropertyTypeCustomization::GetValueSummaryTitleText() const
{
	if (SelectedClassOption.IsValid())
	{
		return SelectedClassOption->DisplayName;
	}

	if (const UGorgeousObjectVariable* Value = GetPreviewValue())
	{
		if (const UClass* PreviewClass = Value->GetClass())
		{
			return PreviewClass->GetDisplayNameText();
		}
	}

	return LOCTEXT("GorgeousOVSummaryNoClass", "No class selected");
}

FText FGorgeousObjectVariablePropertyTypeCustomization::GetValueSummaryDescriptionText() const
{
	if (SelectedContainerOption.IsValid())
	{
		return FText::Format(
			LOCTEXT("GorgeousOVSummaryContainer", "Defaults stored as {0} container."),
			SelectedContainerOption->Label);
	}

	return LOCTEXT("GorgeousOVSummaryNoContainer", "Container type not resolved yet.");
}

FText FGorgeousObjectVariablePropertyTypeCustomization::GetValueFieldListText() const
{
	if (ValuePropertyNames.Num() > 0)
	{
		FString Combined;
		for (const FName PropertyName : ValuePropertyNames)
		{
			if (!Combined.IsEmpty())
			{
				Combined.Append(TEXT(", "));
			}

			Combined.Append(PropertyName.ToString());
		}

		return FText::Format(LOCTEXT("GorgeousOVSummaryFields", "Value fields: {0}"), FText::FromString(Combined));
	}

	if (bValuePropertyWarningActive && !ValuePropertyWarningText.IsEmpty())
	{
		return ValuePropertyWarningText;
	}

	return LOCTEXT("GorgeousOVSummaryNoFields", "No value fields configured.");
}

#undef LOCTEXT_NAMESPACE

#endif