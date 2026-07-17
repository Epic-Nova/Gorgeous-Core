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
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousObjectVariableEnums.h"
#include "Templates/SharedPointer.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Views/SListView.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "PropertyHandle.h"
//<-------------------------------------------------------------------------->

class SComboButton;
class SVerticalBox;
class STextBlock;
class ISinglePropertyView;
class UGorgeousObjectVariable;
struct FGorgeousObjectVariableSerializedPayload;
struct FGorgeousObjectVariableTrunk;
enum class EObjectVariableContainerType_E : uint8;
struct FPropertyChangedEvent;

/**
 * Property customization that lets designers author `UGorgeousObjectVariable` defaults via
 * a trunk-backed workflow. The customization surfaces a searchable class picker, container
 * selector and a streamlined value summary that mirrors edits into the owning trunk payload.
 */
class FGorgeousObjectVariablePropertyTypeCustomization : public IPropertyTypeCustomization
{
public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance();

    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
        IPropertyTypeCustomizationUtils& CustomizationUtils) override;

    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
        IPropertyTypeCustomizationUtils& CustomizationUtils) override;

    struct FVariableClassOption
    {
        FText DisplayName;
        TWeakObjectPtr<UClass> Class;
        bool bIsBlueprint = false;
        bool bIsAbstractBase = false;

        bool operator==(const FVariableClassOption& Other) const
        {
            return Class == Other.Class;
        }
    };

private:
    struct FContainerTypeOption
    {
        EObjectVariableContainerType_E ContainerType;
        FText Label;
    };

    struct FOwnerContext
    {
        TWeakObjectPtr<UObject> Owner;
        FGorgeousObjectVariableTrunk* Trunk = nullptr;
        TWeakObjectPtr<UGorgeousObjectVariable> Value;
        FGorgeousObjectVariableSerializedPayload* Payload = nullptr;
        FGuid Identifier;
        EObjectVariableContainerType_E ResolvedContainerType = EObjectVariableContainerType_E::ESingle;
    };

    struct FManualValueEntry
    {
        FName FieldName;
        bool bIsValid = false;
    };

private:
    void InitializeState(IPropertyTypeCustomizationUtils& CustomizationUtils);
    void CacheOwnerContexts();
    void SynchronizePayloadsFromExistingValues();
    void RefreshClassOptions();
    void RefreshSelectedClass();
    void RefreshContainerSelection();
    void RebuildValuePropertyTargets();
    void RebuildManualOverrideState();
    void BuildContainerOptions();
    void RefreshValueWidgetDisplay();
    void RebuildValuePropertyEditors();

    void RegisterChangeHandlers();

    void HandlePropertyValueChanged();
    void HandleValueWidgetPropertyChanged();
    FGuid EnsureIdentifierForContext(FOwnerContext& Context, int32 ContextIndex);
    FGuid BuildDeterministicIdentifier(UObject* Owner, int32 OwnerIndex) const;
    TArray<FName> ResolveExplicitValuePropertyNames() const;

    FName ResolveTrunkPropertyName(const TSharedPtr<IPropertyHandle>& Handle) const;
    UGorgeousObjectVariable* GetPreviewValue() const;
    TSharedPtr<FVariableClassOption> FindOptionByClass(UClass* InClass) const;
    TSharedPtr<FContainerTypeOption> FindContainerOption(EObjectVariableContainerType_E InType) const;

    void CreateOrReplaceValue(UClass* DesiredClass);
    void UpdateContainerType(EObjectVariableContainerType_E InType);
    EObjectVariableContainerType_E GetActiveContainerType() const;
    void UpdateContainerCompatibilityWarning();
    TArray<FName> GatherConfiguredValuePropertyNames() const;
    bool ShouldDisplayManualOverrideEditor() const;
    void ApplyContainerTypeToPreview(EObjectVariableContainerType_E InType);

    void EnsurePreviewInstance(UClass* DesiredClass);
    void RefreshPreviewFromPayload();
    void CommitPreviewToPayload(const FText& TransactionContext);
    void SynchronizeManualOverridesToPayload(const FText& TransactionContext);
    void RefreshManualValueRows();

    TSharedRef<SWidget> BuildHeaderContent();
    TSharedRef<SWidget> BuildClassPicker();
    TSharedRef<SWidget> BuildContainerSelector();
    TSharedRef<SWidget> BuildContainerWarning();
    TSharedRef<SWidget> BuildValueWidget();
    TSharedRef<SWidget> BuildValueWarning();
    TSharedRef<SWidget> BuildTrunkWarning();
    TSharedRef<SWidget> BuildClassPickerMenu();
    TSharedRef<SWidget> BuildManualOverrideEditor();

    void HandleClassSelectionChanged(TSharedPtr<FVariableClassOption> Option, ESelectInfo::Type SelectInfo);
    TSharedRef<ITableRow> HandleGenerateClassRow(TSharedPtr<FVariableClassOption> Option, const TSharedRef<STableViewBase>& OwnerTable);
    void HandleClassSearchChanged(const FText& InSearchText);
    void HandleContainerComboOpening();
    void HandleContainerSelectionChanged(TSharedPtr<FContainerTypeOption> Option, ESelectInfo::Type SelectInfo);
    TSharedRef<SWidget> HandleGenerateContainerRow(TSharedPtr<FContainerTypeOption> Option);
    TSharedRef<ITableRow> HandleGenerateManualOverrideRow(TSharedPtr<FManualValueEntry> Entry, const TSharedRef<STableViewBase>& OwnerTable);
    void HandleManualOverrideCommitted(const FText& NewText, ETextCommit::Type CommitType, int32 EntryIndex);
    void HandleRemoveManualOverride(int32 EntryIndex);
    void HandleAddManualOverride();
    FText GetSelectedClassText() const;
    FText GetSelectedContainerText() const;
    FText GetContainerWarningText() const;
    EVisibility GetContainerWarningVisibility() const;
    EVisibility GetValueWarningVisibility() const;
    FText GetValueWarningText() const;
    EVisibility GetManualOverrideVisibility() const;
    FText GetValueSummaryTitleText() const;
    FText GetValueSummaryDescriptionText() const;
    FText GetValueFieldListText() const;

    void HandleShowInnerPropertiesChanged(ECheckBoxState NewState);
    ECheckBoxState GetShowInnerPropertiesCheckState() const;
    bool IsShowInnerPropertiesEnabled() const;
    void UpdateShowInnerPropertiesState(bool bNewValue);

private:
    TWeakPtr<IPropertyHandle> ObjectPropertyHandle;
    TWeakPtr<IPropertyUtilities> PropertyUtilities;

    FName TrunkPropertyName;
    TArray<FOwnerContext> OwnerContexts;
    bool bHasValidTrunk = false;
    bool bShowInnerProperties = false;

    TArray<TSharedPtr<FVariableClassOption>> ClassOptions;
    TArray<TSharedPtr<FVariableClassOption>> FilteredClassOptions;
    TSharedPtr<FVariableClassOption> SelectedClassOption;
    TSharedPtr<SListView<TSharedPtr<FVariableClassOption>>> ClassListView;
    TSharedPtr<SSearchBox> ClassSearchBox;
    TWeakPtr<SComboButton> ClassPickerButton;
    FText ClassFilterText;

    TArray<TSharedPtr<FContainerTypeOption>> ContainerOptions;
    TSharedPtr<FContainerTypeOption> SelectedContainerOption;
    TSharedPtr<SComboBox<TSharedPtr<FContainerTypeOption>>> ContainerComboBox;
    bool bContainerWarningActive = false;
    FText ContainerWarningText;
    bool bValuePropertyWarningActive = false;
    FText ValuePropertyWarningText;
    TArray<FName> ValuePropertyNames;
    bool bDisplayManualOverrideEditor = false;
    TArray<TSharedPtr<FManualValueEntry>> ManualValueEntries;
    TSharedPtr<SListView<TSharedPtr<FManualValueEntry>>> ManualValueListView;
    TWeakPtr<STextBlock> ValueSummaryTitleText;
    TWeakPtr<STextBlock> ValueSummaryDescriptionText;
    TWeakPtr<STextBlock> ValueSummaryFieldsText;
    TWeakPtr<SVerticalBox> ValuePropertyEditorContainer;
    TArray<TSharedPtr<ISinglePropertyView>> ActiveValuePropertyEditors;
    FSimpleDelegate ValuePropertyChangedDelegate;
    TStrongObjectPtr<UGorgeousObjectVariable> PreviewVariable;
};

#endif