#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "ConditionalObjectChoosers/GorgeousConditionalObjectChooser.h"
#include "ConditionalObjectChoosers/Conditions/GorgeousBooleanCondition.h"
#include "ConditionalObjectChoosers/Conditions/GorgeousIsValidCondition.h"
#include "ObjectVariables/NativeObjectVariableDefinitions.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousChooserLifecycleRegression, "GorgeousCore.Limitations.Chooser.StableEntriesAndPureLookup", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousChooserLifecycleRegression::RunTest(const FString&)
{
    auto* Chooser = NewObject<UGorgeousConditionalObjectChooser>();
    auto* Check = NewObject<UGorgeousBooleanCondition>(Chooser);
    Check->Mode = EConditionalChooserMode_E::A_ONLY;
    Check->A = true;
    Check->B = false;
    Chooser->ConditionCheck = Check;
    UGorgeousObjectVariable* Chosen = NewObject<UInteger_SOV>(Chooser);
    Chosen->UniqueIdentifier = FGuid::NewGuid();
    const FGuid OriginalId = Chosen->UniqueIdentifier;
    Chooser->Conditions = {nullptr, Chosen};
    Chooser->CleanupInvalidEntries();
    TestEqual(TEXT("Repair keeps the authored condition indices"), Chooser->Conditions.Num(), 2);
    TestEqual(TEXT("Index one still selects the intended variable"), Chooser->DecideCondition(), Chosen);
    TestEqual(TEXT("Repeated pure lookup selects the same object"), Chooser->DecideCondition(), Chosen);
    TestEqual(TEXT("Pure lookup never changes identity"), Chosen->UniqueIdentifier, OriginalId);

    auto* Validity = NewObject<UGorgeousIsValidCondition>();
    Validity->Mode = EConditionalChooserMode_E::A_ONLY;
    // UObject itself is abstract in this engine; use an ordinary concrete object.
    UObject* Dying = NewObject<UGorgeousBooleanCondition>();
    Validity->A = Dying;
    TestEqual(TEXT("Live object is valid"), Validity->CheckCondition(), uint8(1));
    Dying->MarkAsGarbage();
    TestEqual(TEXT("Non-null garbage-marked object is invalid"), Validity->CheckCondition(), uint8(0));
    Validity->A = nullptr;
    return true;
}
#endif
