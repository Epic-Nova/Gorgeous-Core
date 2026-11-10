#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "AssetRegistration/GorgeousAssetRegistrationHelpers.h"
#include "StyleRegistration/GorgeousStyleRegistrationHelperMacros.h"
#include "AssetTypeActions_Base.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Engine/DataAsset.h"
#include "Tests/GorgeousMigrationAuditTypes.h"
#include "Tests/GorgeousDetailExtensionAuditTypes.h"
#include "Misc/ScopeExit.h"
#include "InsightMatrix/GorgeousInsightFunctionalTest.h"
#include "InsightMatrix/GorgeousInsightTestVolume.h"
#include "SharedTests/GorgeousInteractionFunctionalTest.h"

void RegisterGorgeousAuditActionFromOtherTU(FName Owner, const TSharedRef<IAssetTypeActions>& Action);
const void* GetGorgeousAuditCategoryAddressFromOtherTU();
namespace
{
    class FGorgeousAuditAssetAction final : public FAssetTypeActions_Base
    {
    public:
        explicit FGorgeousAuditAssetAction(UClass* InSupportedClass) : SupportedClass(InSupportedClass) {}
        virtual FText GetName() const override { return FText::FromString(TEXT("Audit only")); }
        virtual FColor GetTypeColor() const override { return FColor::White; }
        virtual UClass* GetSupportedClass() const override { return SupportedClass; }
        virtual uint32 GetCategories() override { return EAssetTypeCategories::Misc; }
    private:
        UClass* SupportedClass;
    };
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousAssetRegistryCrossTUAudit,
    "GorgeousCore.Limitations.Editor.AssetRegistryCrossTranslationUnit",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousAssetRegistryCrossTUAudit::RunTest(const FString& Parameters)
{
    IAssetTools& Tools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
    const FName OwnerA(TEXT("AuditModuleA")), OwnerB(TEXT("AuditModuleB"));
    // AssetTools permits only one action per supported class; using UDataAsset
    // twice would replace A with B before testing our cross-TU ownership.
    UClass* ClassA = UGorgeousMigrationAuditAsset::StaticClass();
    UClass* ClassB = UGorgeousDetailAuditOwner::StaticClass();
    auto ActionA = MakeShared<FGorgeousAuditAssetAction>(ClassA);
    auto ActionB = MakeShared<FGorgeousAuditAssetAction>(ClassB);
    auto ReplacementA = MakeShared<FGorgeousAuditAssetAction>(ClassA);
    RegisterGorgeousAuditActionFromOtherTU(OwnerA, ActionA);
    Tools.RegisterAssetTypeActions(ActionB);
    GorgeousAssetRegistration::TrackAssetTypeAction(OwnerB, ActionB);
    ON_SCOPE_EXIT { GorgeousAssetRegistration::UnregisterAssetTypeActions(OwnerA); GorgeousAssetRegistration::UnregisterAssetTypeActions(OwnerB); };
    auto Contains = [&Tools](const TSharedPtr<IAssetTypeActions>& Action)
    {
        TArray<TWeakPtr<IAssetTypeActions>> Actions; Tools.GetAssetTypeActionsList(Actions);
        return Actions.ContainsByPredicate([&Action](const auto& Entry) { return Entry.Pin() == Action; });
    };
    TestTrue(TEXT("Both real engine actions registered"), Contains(ActionA) && Contains(ActionB));
    TestTrue(TEXT("Category address shared across translation units"), GetGorgeousAuditCategoryAddressFromOtherTU() == &GorgeousAssetRegistration::GGorgeousThingsCategory);
    GorgeousAssetRegistration::UnregisterAssetTypeActions(OwnerA);
    TestFalse(TEXT("Other-TU action removed through shared storage"), Contains(ActionA));
    TestTrue(TEXT("Unrelated module action retained"), Contains(ActionB));
    GorgeousAssetRegistration::UnregisterAssetTypeActions(OwnerA);
    TestTrue(TEXT("Repeated teardown is harmless"), Contains(ActionB));
    RegisterGorgeousAuditActionFromOtherTU(OwnerA, ActionA);
    Tools.RegisterAssetTypeActions(ReplacementA);
    TestFalse(TEXT("Engine replaces previous action for the same supported class"), Contains(ActionA));
    TestTrue(TEXT("Replacement action is actually registered"), Contains(ReplacementA));
    GorgeousAssetRegistration::UnregisterAssetTypeActions(OwnerA);
    TestTrue(TEXT("Stale owner teardown preserves another module's replacement"), Contains(ReplacementA));
    TestTrue(TEXT("Different supported class is still retained"), Contains(ActionB));
    Tools.UnregisterAssetTypeActions(ReplacementA);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousStyleMacroFlowAudit,
    "GorgeousCore.Limitations.Editor.StyleMacroCallerFlow",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousStyleMacroFlowAudit::RunTest(const FString& Parameters)
{
    const auto PriorHelperStyle = GorgeousStyleRegistration::GGorgeousStyleSet;
    ON_SCOPE_EXIT { GorgeousStyleRegistration::GGorgeousStyleSet = PriorHelperStyle; };
    TSharedPtr<FSlateStyleSet> Style;
    const FName StyleName(*(TEXT("Gorgeous.Audit.") + FGuid::NewGuid().ToString(EGuidFormats::Digits)));
    int32 Reached = 0;
    auto RegisterTwice = [&]()
    {
        GORGEOUS_REGISTER_STYLE_SET(Style, StyleName, TEXT("GorgeousCore"), {});
        ++Reached;
        GORGEOUS_REGISTER_STYLE_SET(Style, StyleName, TEXT("GorgeousCore"), {});
        ++Reached;
    };
    RegisterTwice();
    TestEqual(TEXT("Already-registered branch never returns from caller"), Reached, 2);
    auto UnregisterTwice = [&]()
    {
        GORGEOUS_UNREGISTER_STYLE_SET(Style);
        ++Reached;
        GORGEOUS_UNREGISTER_STYLE_SET(Style);
        ++Reached;
    };
    UnregisterTwice();
    TestEqual(TEXT("Missing-style teardown never returns from caller"), Reached, 4);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousFunctionalEditorOnlyAudit,
    "GorgeousCore.Limitations.Insight.EditorOnlyFunctionalActors",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousFunctionalEditorOnlyAudit::RunTest(const FString& Parameters)
{
    for (UClass* TestClass : { AGorgeousInsightFunctionalTest::StaticClass(), AGorgeousInsightTestVolume::StaticClass(), AGorgeousInteractionFunctionalTest::StaticClass() })
    {
        const AActor* Actor = TestClass->GetDefaultObject<AActor>();
        TestTrue(TEXT("Functional actor CDO is editor-only"), Actor->IsEditorOnly());
        TestTrue(TEXT("Functional actor explicitly remains usable in PIE"), Actor->IsEditorOnlyLoadedInPIE());
        TestTrue(TEXT("Native editor test class has editor-only package/class ancestry"), IsEditorOnlyObject(TestClass));
    }
    UClass* Beacon = LoadClass<AActor>(nullptr, TEXT("/GorgeousCore/InsightMatrix/BP_TestBeacon_A.BP_TestBeacon_A_C"));
    if (TestNotNull(TEXT("Redirected Blueprint test beacon loads"), Beacon))
    {
        TestTrue(TEXT("Beacon inherits editor-only cook classification"), IsEditorOnlyObject(Beacon));
        TestTrue(TEXT("Beacon preserves PIE loading"), Beacon->GetDefaultObject<AActor>()->IsEditorOnlyLoadedInPIE());
    }
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousParentBrushOwnershipAudit,
    "GorgeousCore.Limitations.Editor.ParentBrushOwnership",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousParentBrushOwnershipAudit::RunTest(const FString& Parameters)
{
    const FSlateBrush* Original = FAppStyle::Get().GetOptionalBrush(TEXT("ClassIcon.Object"), nullptr, nullptr);
    if (!TestNotNull(TEXT("Actual editor-owned parent brush"), Original)) return false;
    const FName Resource = Original->GetResourceName();
    const FVector2D Size(Original->GetImageSize());
    {
        auto DerivedStyle = MakeShared<FSlateStyleSet>(TEXT("Gorgeous.Audit.BrushOwnership"));
        GorgeousStyleRegistration::MapParentClassBrushes(DerivedStyle, TEXT("AuditChild"), TEXT("Object"));
        const FSlateBrush* Copy = DerivedStyle->GetOptionalBrush(TEXT("ClassIcon.AuditChild"), nullptr, nullptr);
        TestTrue(TEXT("Derived style owns an independent brush allocation"), Copy && Copy != Original);
        if (Copy) TestEqual(TEXT("Independent brush preserves resource"), Copy->GetResourceName(), Resource);
        GorgeousStyleRegistration::MapParentClassBrushes(DerivedStyle, TEXT("UnknownChild"), TEXT("ClassThatDoesNotExistForAudit"));
        TestNotNull(TEXT("Unknown parent uses safe optional-brush fallback"), DerivedStyle->GetOptionalBrush(TEXT("ClassIcon.UnknownChild"), nullptr, nullptr));
        // Scope exit destroys the actual style without touching AppStyle-owned memory.
    }
    const FSlateBrush* After = FAppStyle::Get().GetOptionalBrush(TEXT("ClassIcon.Object"), nullptr, nullptr);
    TestTrue(TEXT("Destroying child style preserves original pointer identity"), After == Original);
    if (After)
    {
        TestEqual(TEXT("Parent resource remains usable after child destruction"), After->GetResourceName(), Resource);
        TestTrue(TEXT("Parent dimensions remain intact"), FVector2D(After->GetImageSize()).Equals(Size));
    }
    return true;
}

#endif
