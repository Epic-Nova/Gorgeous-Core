// Copyright (c) 2026 Simsalabim Studios. All rights reserved.
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/ScopeExit.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousInsightLateRegistrationClosure,
    "GorgeousCore.LimitationClosure.Insight.LateRegistration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousInsightLateRegistrationClosure::RunTest(const FString&)
{
    // Core has already completed its normal ActivateAll in StartupModule.
    // A later module's static registrar must participate without a second call.
    const FName Name(*FString::Printf(TEXT("AuditLateRegistration_%s"), *FGuid::NewGuid().ToString(EGuidFormats::Digits)));
    auto Contains = [&]()
    {
        return FGorgeousInsightTestMatrix::GetRegisteredScenarios().ContainsByPredicate(
            [&](const FGorgeousInsightScenarioDescriptor& Descriptor) { return Descriptor.ScenarioName == Name; });
    };
    TestFalse(TEXT("Unique fixture name does not exist before construction"), Contains());
    {
        FGorgeousInsightScenarioRegistrar Registrar([&]()
        {
            FGorgeousInsightScenarioDescriptor Descriptor;
            Descriptor.ScenarioName = Name;
            Descriptor.Runner = [](const FGorgeousInsightScenarioContext&) { return FGorgeousInsightScenarioResult(); };
            return Descriptor;
        });
        TestTrue(TEXT("Late registrar activates automatically after normal module startup"), Contains());
    }
    TestFalse(TEXT("Registrar destruction removes only its own scenario"), Contains());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousInsightAutomationParametersClosure,
    "GorgeousCore.LimitationClosure.Insight.AutomationParameters",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousInsightAutomationParametersClosure::RunTest(const FString&)
{
    const FString Previous = FCommandLine::Get();
    ON_SCOPE_EXIT { FCommandLine::Set(*Previous); };
    FCommandLine::Set(TEXT("-GorgeousInsightParameters=\"Preset=7 NumClients=4 Scenario=WrongGlobal HarnessTimeout=9\""));
    const FGorgeousInsightMatrixRequest Request = FGorgeousInsightMatrixRequest::FromParameters(TEXT("Scenario=ExactSelected NumClients=2"));
    TestEqual(TEXT("Explicit automation channel delivers nondefault Preset"), Request.GetParameterValue(TEXT("Preset")), FString(TEXT("7")));
    TestEqual(TEXT("Per-test parameters override global defaults"), Request.GetParameterValue(TEXT("NumClients")), FString(TEXT("2")));
    TestEqual(TEXT("Global harness timeout reaches actual parsed request"), Request.HarnessTimeoutSeconds, 9.0f);
    TestEqual(TEXT("Per-test scenario selection is preserved"), Request.ScenarioWhitelist.Num(), 1);
    if (Request.ScenarioWhitelist.Num() == 1)
        TestTrue(TEXT("Global Scenario cannot replace actual selected automation scenario"), Request.ScenarioWhitelist.Contains(FName(TEXT("ExactSelected"))));
    const TArray<int32> Variants = Request.ResolveVariantIndices({0});
    TestEqual(TEXT("Global Preset selects exactly one actual variant"), Variants.Num(), 1);
    if (Variants.Num() == 1) TestEqual(TEXT("Variant receives nondefault Preset"), Variants[0], 7);
    return true;
}
#endif
