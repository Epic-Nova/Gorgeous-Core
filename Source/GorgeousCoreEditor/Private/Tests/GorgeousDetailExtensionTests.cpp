// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GorgeousDetailExtensionAuditTypes.h"
#include "Widgets/Text/STextBlock.h"

void UGorgeousDetailAuditExtension::CustomizeHeader(TSharedRef<IPropertyHandle> Handle, FDetailWidgetRow& Header)
{
    ++HeaderCalls;
    bEveryHandleIsMap &= CastField<FMapProperty>(Handle->GetProperty()) != nullptr;
    Super::CustomizeHeader(Handle, Header);
}

void UGorgeousDetailAuditExtension::CustomizeChildren(TSharedRef<IPropertyHandle> Handle, IDetailChildrenBuilder& Children)
{
    ++ChildrenCalls;
    bEveryHandleIsMap &= CastField<FMapProperty>(Handle->GetProperty()) != nullptr;
    TArray<UObject*> Owners;
    Handle->GetOuterObjects(Owners);
    for (UObject* Owner : Owners) { ChildOwners.Add(Owner); }
    Children.AddCustomRow(FText::FromString(TEXT("Gorgeous nested map extension reached")))
        .WholeRowContent()[SNew(STextBlock).Text(FText::FromString(TEXT("Gorgeous nested map extension reached")))];
    Super::CustomizeChildren(Handle, Children);
}

#if WITH_DEV_AUTOMATION_TESTS
#include "DetailCustomizations/GorgeousGlobalDetailCustomization.h"
#include "DetailExtensions/GorgeousDetailExtensionSettings.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "PropertyPath.h"
#include "Layout/ArrangedChildren.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
    void TickDetailsTree(const TSharedRef<SWidget>& Widget, const FGeometry& Geometry)
    {
        Widget->Tick(Geometry, FPlatformTime::Seconds(), 1.f / 60.f);
        FArrangedChildren Children(EVisibility::Visible);
        Widget->ArrangeChildren(Geometry, Children);
        for (int32 Index = 0; Index < Children.Num(); ++Index)
        { TickDetailsTree(Children[Index].Widget, Children[Index].Geometry); }
    }

    void MaterializeDetails(const TSharedRef<IDetailsView>& View)
    {
        const FGeometry Geometry = FGeometry::MakeRoot(FVector2D(1200, 2000), FSlateLayoutTransform());
        View->SetRootExpansionStates(true, true);
        for (int32 Frame = 0; Frame < 3; ++Frame)
        {
            View->SlatePrepass(1.f);
            TickDetailsTree(View, Geometry);
            View->SetRootExpansionStates(true, true);
        }
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousNestedMapDetailExtensionTest,
    "GorgeousCore.Editor.DetailExtensions.InstancedNestedMap",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousNestedMapDetailExtensionTest::RunTest(const FString&)
{
    if (!FSlateApplication::IsInitialized())
    {
        AddError(TEXT("PropertyEditor integration requires a Slate-initialized editor session."));
        return false;
    }
    auto& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
    UGorgeousDetailExtensionSettings* Settings = GetMutableDefault<UGorgeousDetailExtensionSettings>();
    const bool bPreviousEnabled = Settings->bEnableDetailExtensions;
    const TSet<FName> PreviousDisabled = Settings->DisabledExtensions;
    TStrongObjectPtr<UGorgeousDetailAuditExtension> Extension(NewObject<UGorgeousDetailAuditExtension>());
    TStrongObjectPtr<UGorgeousDetailAuditOwner> Owner(NewObject<UGorgeousDetailAuditOwner>());
    UGorgeousDetailAuditWorldAdapter* ArrayAdapter = NewObject<UGorgeousDetailAuditWorldAdapter>(Owner.Get());
    UGorgeousDetailAuditWorldAdapter* NestedAdapter = NewObject<UGorgeousDetailAuditWorldAdapter>(Owner.Get());
    Owner->Adapters.Add(ArrayAdapter);
    Owner->Nested.Adapter = NestedAdapter;
    FGorgeousDetailAuditAttachmentKey Key;
    Key.SocketName = TEXT("RightHandSocket");
    ArrayAdapter->EquipOffsetsByAttachmentPoint.Add(Key, FVector(1, 2, 3));
    NestedAdapter->EquipOffsetsByAttachmentPoint.Add(Key, FVector(4, 5, 6));
    Settings->bEnableDetailExtensions = true;
    Settings->DisabledExtensions.Remove(Extension->GetExtensionName());
    FGorgeousGlobalDetailCustomization::RegisterExtension(Extension.Get());
    FDetailsViewArgs Args;
    Args.bUpdatesFromSelection = false;
    Args.bAllowSearch = false;
    Args.bHideSelectionTip = true;
    Args.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    TSharedPtr<IDetailsView> View = PropertyEditor.CreateDetailView(Args);
    ON_SCOPE_EXIT
    {
        View->SetObject(nullptr, true);
        View.Reset();
        FGorgeousGlobalDetailCustomization::UnregisterExtension(Extension->GetExtensionName());
        Settings->bEnableDetailExtensions = bPreviousEnabled;
        Settings->DisabledExtensions = PreviousDisabled;
        PropertyEditor.NotifyCustomizationModuleChanged();
    };

    // This intentionally uses the real PropertyEditor details tree; no direct helper/customization calls.
    View->SetObject(Owner.Get(), true);
    MaterializeDetails(View.ToSharedRef());
    TestTrue(TEXT("PropertyEditor invokes the extension header"), Extension->HeaderCalls > 0);
    TestTrue(TEXT("PropertyEditor invokes CustomizeChildren"), Extension->ChildrenCalls > 0);
    TestTrue(TEXT("Extension receives actual reflected Map handles"), Extension->bEveryHandleIsMap);
    TestTrue(TEXT("Polymorphic instanced array adapter reaches CustomizeChildren"), Extension->ChildOwners.Contains(ArrayAdapter));
    TestTrue(TEXT("Adapter nested inside a struct reaches CustomizeChildren"), Extension->ChildOwners.Contains(NestedAdapter));

    View->SetObject(nullptr, true);
    Extension->ResetObservations();
    Settings->DisabledExtensions.Add(Extension->GetExtensionName());
    View->SetObject(Owner.Get(), true);
    MaterializeDetails(View.ToSharedRef());
    TestEqual(TEXT("Per-extension opt-out prevents header customization"), Extension->HeaderCalls, 0);
    TestEqual(TEXT("Per-extension opt-out prevents child customization"), Extension->ChildrenCalls, 0);
    bool bNativeMapRowVisible = false;
    for (const auto& Row : View->GetPropertyRowNumbers())
    {
        if (!Row.Value.IsValid()) { continue; }
        const FProperty* Property = Row.Value.GetLeafMostProperty().Property.Get();
        bNativeMapRowVisible |= Property && Property->GetFName() == GET_MEMBER_NAME_CHECKED(UGorgeousDetailAuditWorldAdapter, EquipOffsetsByAttachmentPoint);
    }
    TestTrue(TEXT("Opt-out leaves the native Map property row visible in the real details tree"), bNativeMapRowVisible);
    TestEqual(TEXT("Opt-out preserves native map data"), ArrayAdapter->EquipOffsetsByAttachmentPoint.FindChecked(Key), FVector(1, 2, 3));

    View->SetObject(nullptr, true);
    Extension->ResetObservations();
    Settings->DisabledExtensions.Remove(Extension->GetExtensionName());
    Settings->bEnableDetailExtensions = false;
    View->SetObject(Owner.Get(), true);
    MaterializeDetails(View.ToSharedRef());
    TestEqual(TEXT("Global opt-out prevents extension customization"), Extension->HeaderCalls + Extension->ChildrenCalls, 0);
    return true;
}
#endif
