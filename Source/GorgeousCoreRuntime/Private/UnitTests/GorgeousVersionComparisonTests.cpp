#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Helpers/GorgeousVersionComparison.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousVersionComparisonRegression, "GorgeousCore.Limitations.VersionComparison", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousVersionComparisonRegression::RunTest(const FString&)
{
    TestFalse(TEXT("Numeric minor versions, not lexicographic order"), GorgeousVersionComparison::IsOlder(TEXT("1.10"), TEXT("1.9")));
    TestFalse(TEXT("Descriptor prefix and suffix preserve equality"), GorgeousVersionComparison::IsOlder(TEXT("GT-C-1.0-General"), TEXT("1.0")));
    TestTrue(TEXT("Numeric patch minimum"), GorgeousVersionComparison::IsOlder(TEXT("GT-C-1.0.2-General"), TEXT("1.0.10")));
    TestFalse(TEXT("Empty requirement does not request repair"), GorgeousVersionComparison::IsOlder(TEXT("1.0"), TEXT("")));
    return true;
}
#endif
