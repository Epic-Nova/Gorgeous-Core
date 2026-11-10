#include "ClosureCoreEditorTypes.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuardEnforcer.h"
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuard.h"
#include "ModuleCore/GorgeousUsefulSettings.h"
#include "DetailCustomizations/GorgeousGlobalDetailCustomization.h"
#include "DetailExtensions/GorgeousDetailExtensionSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "Layout/ArrangedChildren.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Misc/AutomationTest.h"
#include "Misc/OutputDevice.h"
#include "Misc/ScopeExit.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousGuardClosureWorkflowTest,
    "GorgeousClosure.CoreEditor.GuardSaveFailureCancelsExit",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousGuardClosureWorkflowTest::RunTest(const FString&)
{
    if (!TestNotNull(TEXT("Editor available"), GEditor)) return false;
    TStrongObjectPtr<UGorgeousExtensionResourceGuardEnforcer> Enforcer(NewObject<UGorgeousExtensionResourceGuardEnforcer>(GEditor));
    int32 Saves=0, Exits=0;
    Enforcer->ActiveEnforcements.Add(TEXT("Plugin:ClosureFailure"));
    Enforcer->TestSaveDirtyPackages=[&]() { ++Saves; return false; };
    Enforcer->TestRequestExit=[&]() { ++Exits; };
    AddExpectedError(TEXT("Saving assets failed. Automatic shutdown cancelled"), EAutomationExpectedErrorFlags::Contains, 1);
    Enforcer->SafeShutdownEditor();
    TestEqual(TEXT("Actual shutdown workflow attempts save exactly once"), Saves, 1);
    TestEqual(TEXT("Failed save never requests exit"), Exits, 0);
    TestFalse(TEXT("Failure cancels active shutdown attempts"), Enforcer->IsEnforcementActive());

    Enforcer->TestSaveDirtyPackages=[&]() { ++Saves; return true; };
    Enforcer->SafeShutdownEditor();
    TestEqual(TEXT("Success workflow attempts its save"), Saves, 2);
    TestEqual(TEXT("Successful save reaches exit boundary once"), Exits, 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousGuardClosureResourceLifecycleTest,
    "GorgeousClosure.CoreEditor.GuardOptionalAndPackLifecycle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousGuardClosureResourceLifecycleTest::RunTest(const FString&)
{
    if (!TestNotNull(TEXT("Editor available"), GEditor)) return false;
    auto* Settings=GetMutableDefault<UGorgeousUsefulSettings>();
    const bool PreviousDisabled=Settings->bDisableExtensionResourceProtection;
    Settings->bDisableExtensionResourceProtection=false;
    TStrongObjectPtr<UGorgeousExtensionResourceGuardEnforcer> Enforcer(NewObject<UGorgeousExtensionResourceGuardEnforcer>(GEditor));
    TStrongObjectPtr<UGorgeousExtensionResourceGuard> Guard(NewObject<UGorgeousExtensionResourceGuard>());
    Guard->bIsContentPackGuard=false;
    int32 Saves=0, Exits=0;
    Enforcer->TestSaveDirtyPackages=[&]() { ++Saves; return true; };
    Enforcer->TestRequestExit=[&]() { ++Exits; };
    ON_SCOPE_EXIT {
        Settings->bDisableExtensionResourceProtection=PreviousDisabled;
        if (Enforcer->ProtectionBanner.IsValid()) Enforcer->ProtectionBanner->ExpireAndFadeout();
    };
    const FName Missing(TEXT("ClosureMissingMandatoryResource"));
    Guard->RequiredPlugins.Add(Missing,true);
    Enforcer->BeginEnforcement(Missing.ToString(),TEXT("Closure"));
    TestFalse(TEXT("Optional-only plugin cannot start countdown"),Enforcer->IsEnforcementActive());
    Guard->RequiredPlugins[Missing]=false;
    Enforcer->BeginEnforcement(Missing.ToString(),TEXT("Closure"));
    TestTrue(TEXT("Mandatory missing plugin starts countdown"),Enforcer->IsEnforcementActive());
    Enforcer->ActiveEnforcements.FindChecked(TEXT("Plugin:")+Missing.ToString()).StartTimestamp=FPlatformTime::Seconds()-601.0;
    Guard->RequiredPlugins[Missing]=true;
    Enforcer->OnTick(0.f);
    TestFalse(TEXT("Making the last plugin requirement optional clears expired countdown"),Enforcer->IsEnforcementActive());
    TestEqual(TEXT("Optional requirement never reaches save boundary"),Saves,0);
    TestEqual(TEXT("Optional requirement never reaches exit boundary"),Exits,0);
    Guard->RequiredPlugins.Empty();
    Guard->RequiredBlueprintPacks.Add(Missing,true);
    Enforcer->BeginEnforcement(Missing.ToString(),TEXT("Closure"),true);
    TestFalse(TEXT("Optional-only pack cannot start countdown"),Enforcer->IsEnforcementActive());
    Guard->RequiredBlueprintPacks[Missing]=false;
    Enforcer->BeginEnforcement(Missing.ToString(),TEXT("Closure"),true);
    TestTrue(TEXT("Mandatory missing pack starts countdown"),Enforcer->IsEnforcementActive());
    Enforcer->ActiveEnforcements.FindChecked(TEXT("BlueprintPack:")+Missing.ToString()).StartTimestamp=FPlatformTime::Seconds()-601.0;
    Guard->RequiredBlueprintPacks[Missing]=true;
    Enforcer->OnTick(0.f);
    TestFalse(TEXT("Making the last pack requirement optional clears countdown"),Enforcer->IsEnforcementActive());
    Guard->RequiredBlueprintPacks.Empty();

    // Exercise the actual AssetRegistry startup workflow with one unsaved,
    // temporary in-memory asset. No content/config/descriptor file is written.
    IAssetRegistry& Registry=FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
    Registry.SearchAllAssets(true);
    UPackage* Package=CreatePackage(*FString::Printf(TEXT("/Temp/ClosureGuard_%s"),*FGuid::NewGuid().ToString(EGuidFormats::Digits)));
    TStrongObjectPtr<UGorgeousExtensionResourceGuard> Asset(NewObject<UGorgeousExtensionResourceGuard>(Package,TEXT("ClosureGuard"),RF_Public|RF_Standalone));
    Asset->bIsContentPackGuard=false;
    Asset->SystemIdentifier=TEXT("ClosurePresentPackMarker");
    Asset->SystemDisplayName=FText::FromString(TEXT("Closure startup fixture"));
    Asset->RequiredBlueprintPacks.Add(Missing,false);
    Asset->RequiredBlueprintPacks.Add(Asset->SystemIdentifier,true);
    FAssetRegistryModule::AssetCreated(Asset.Get());
    ON_SCOPE_EXIT { FAssetRegistryModule::AssetDeleted(Asset.Get()); Package->SetDirtyFlag(false); Asset->ClearFlags(RF_Public|RF_Standalone); };
    const FAssetData Registered=Registry.GetAssetByObjectPath(FSoftObjectPath(Asset.Get()));
    if (!TestTrue(TEXT("Unsaved fixture is visible to the actual AssetRegistry"),Registered.IsValid())) return false;
    class FPresentPackOutput final : public FOutputDevice {
    public:
        FThreadSafeCounter Count;
        void Serialize(const TCHAR* Value,ELogVerbosity::Type,const FName&) override {
            if (FCString::Stristr(Value,TEXT("Optional blueprint pack")) && FCString::Stristr(Value,TEXT("ClosurePresentPackMarker"))) Count.Increment();
        }
    } Capture;
    AddExpectedError(TEXT("Blueprint pack 'ClosureMissingMandatoryResource' is required"),EAutomationExpectedErrorFlags::Contains,1);
    GLog->AddOutputDevice(&Capture);
    Enforcer->PerformStartupValidation();
    GLog->RemoveOutputDevice(&Capture);
    TestTrue(TEXT("Startup immediately registers the missing mandatory pack"),Enforcer->ActiveEnforcements.Contains(TEXT("BlueprintPack:")+Missing.ToString()));
    TestEqual(TEXT("Present optional pack emits no missing/declared warning"),Capture.Count.GetValue(),0);
    Enforcer->StopEnforcement(Missing.ToString());
    TestFalse(TEXT("Stop clears blueprint-pack keys too"),Enforcer->IsEnforcementActive());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousDetailClosureHeterogeneousSelectionTest,
    "GorgeousClosure.CoreEditor.HeterogeneousDetailSelection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousDetailClosureHeterogeneousSelectionTest::RunTest(const FString&)
{
    auto* Settings=GetMutableDefault<UGorgeousDetailExtensionSettings>();
    const bool PreviousEnabled=Settings->bEnableDetailExtensions;
    const auto PreviousDisabled=Settings->DisabledExtensions;
    TStrongObjectPtr<UGorgeousDetailAuditExtension> Extension(NewObject<UGorgeousDetailAuditExtension>());
    TStrongObjectPtr<UClosureCoreEditorAdapterA> A(NewObject<UClosureCoreEditorAdapterA>());
    TStrongObjectPtr<UClosureCoreEditorAdapterB> B(NewObject<UClosureCoreEditorAdapterB>());
    FGorgeousDetailAuditAttachmentKey Key; Key.SocketName=TEXT("ClosureSocket");
    A->EquipOffsetsByAttachmentPoint.Add(Key,FVector(1,2,3));
    B->EquipOffsetsByAttachmentPoint.Add(Key,FVector(4,5,6));
    Settings->bEnableDetailExtensions=true;
    Settings->DisabledExtensions.Remove(Extension->GetExtensionName());
    FGorgeousGlobalDetailCustomization::RegisterExtension(Extension.Get());
    FDetailsViewArgs Args; Args.bUpdatesFromSelection=false; Args.bAllowSearch=false; Args.bHideSelectionTip=true;
    auto& PropertyEditor=FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
    TSharedPtr<IDetailsView> View=PropertyEditor.CreateDetailView(Args);
    ON_SCOPE_EXIT {
        View->SetObject(nullptr,true); View.Reset();
        FGorgeousGlobalDetailCustomization::UnregisterExtension(Extension->GetExtensionName());
        Settings->bEnableDetailExtensions=PreviousEnabled; Settings->DisabledExtensions=PreviousDisabled;
        PropertyEditor.NotifyCustomizationModuleChanged();
    };
    const TArray<UObject*> Selection{A.Get(),B.Get()};
    View->SetObjects(Selection,true);
    const FGeometry Geometry=FGeometry::MakeRoot(FVector2D(1200,2000),FSlateLayoutTransform());
    TFunction<void(const TSharedRef<SWidget>&,const FGeometry&)> Tick;
    Tick=[&Tick](const TSharedRef<SWidget>& Widget,const FGeometry& Area) {
        Widget->Tick(Area,FPlatformTime::Seconds(),1.f/60.f);
        FArrangedChildren Children(EVisibility::Visible); Widget->ArrangeChildren(Area,Children);
        for (int32 Index=0;Index<Children.Num();++Index) Tick(Children[Index].Widget,Children[Index].Geometry);
    };
    for (int32 Frame=0;Frame<4;++Frame) { View->SetRootExpansionStates(true,true); View->SlatePrepass(1.f); Tick(View.ToSharedRef(),Geometry); }
    TestTrue(TEXT("Actual heterogeneous selection invokes header"),Extension->HeaderCalls>0);
    TestTrue(TEXT("Actual heterogeneous selection invokes children"),Extension->ChildrenCalls>0);
    TestTrue(TEXT("Inherited map handle includes selected subclassA"),Extension->ChildOwners.Contains(A.Get()));
    TestTrue(TEXT("Inherited map handle includes selected subclassB"),Extension->ChildOwners.Contains(B.Get()));
    TestTrue(TEXT("All received handles identify the actual map"),Extension->bEveryHandleIsMap);
    TestEqual(TEXT("First selected object data preserved"),A->EquipOffsetsByAttachmentPoint.FindChecked(Key),FVector(1,2,3));
    TestEqual(TEXT("Second selected object data preserved"),B->EquipOffsetsByAttachmentPoint.FindChecked(Key),FVector(4,5,6));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousProtectionSettingsClosureTest,
    "GorgeousClosure.CoreEditor.ProtectionSettingsWarning",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousProtectionSettingsClosureTest::RunTest(const FString&)
{
    auto& PropertyEditor=FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
    FDetailsViewArgs Args; Args.bUpdatesFromSelection=false; Args.bAllowSearch=false; Args.bHideSelectionTip=true;
    TSharedPtr<IDetailsView> View=PropertyEditor.CreateDetailView(Args);
    View->SetObject(GetMutableDefault<UGorgeousUsefulSettings>(),true);
    ON_SCOPE_EXIT { View->SetObject(nullptr,true); View.Reset(); };
    const FGeometry Geometry=FGeometry::MakeRoot(FVector2D(1400,4000),FSlateLayoutTransform());
    TFunction<void(const TSharedRef<SWidget>&,const FGeometry&)> Tick;
    Tick=[&Tick](const TSharedRef<SWidget>& Widget,const FGeometry& Area) {
        Widget->Tick(Area,FPlatformTime::Seconds(),1.f/60.f);
        FArrangedChildren Children(EVisibility::Visible); Widget->ArrangeChildren(Area,Children);
        for (int32 Index=0;Index<Children.Num();++Index) Tick(Children[Index].Widget,Children[Index].Geometry);
    };
    for (int32 Frame=0;Frame<4;++Frame) { View->SetRootExpansionStates(true,true); View->SlatePrepass(1.f); Tick(View.ToSharedRef(),Geometry); }
    int32 Warnings=0,DocumentationLinks=0;
    TFunction<void(const TSharedRef<SWidget>&)> Inspect;
    Inspect=[&](const TSharedRef<SWidget>& Widget) {
        if (Widget->GetTag()==TEXT("Gorgeous.Protection.Warning")) ++Warnings;
        if (Widget->GetTag()==TEXT("Gorgeous.Protection.Documentation")) ++DocumentationLinks;
        FChildren* Children=Widget->GetChildren();
        for (int32 Index=0;Children && Index<Children->Num();++Index) Inspect(Children->GetChildAt(Index));
    };
    Inspect(View.ToSharedRef());
    TestEqual(TEXT("Actual settings details contain one visible risk banner"),Warnings,1);
    TestEqual(TEXT("Actual settings details contain one documentation hyperlink"),DocumentationLinks,1);
    return true;
}
#endif
