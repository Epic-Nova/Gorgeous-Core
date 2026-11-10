// Copyright (c) 2026 Simsalabim Studios. All rights reserved.
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "GorgeousDevelopmentToolsTestScope.h"
#include "InsightMatrix/GorgeousInsightMatrixSubsystem.h"
#include "InsightMatrix/GorgeousInsightHarness.h"
#include "SharedTests/GorgeousMultiClientTestHarness.h"
#include "Misc/ScopeExit.h"
#include "Engine/Engine.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousDevelopmentSuiteDisabledExecutionAudit,
    "GorgeousCore.DevelopmentSuite.DisabledExecution",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousDevelopmentSuiteDisabledExecutionAudit::RunTest(const FString&)
{
    FGorgeousDevelopmentToolsTestScope ServiceScope;
    auto* Settings = GetMutableDefault<UGorgeousDevelopmentSuiteSettings>();
    Settings->bEnableInsightMatrix = false;
    TestFalse(TEXT("A runtime false setting overrides a registered service"), UGorgeousInsightMatrixSubsystem::IsDevelopmentToolsEnabled());
    class FCountingProvider final : public IGorgeousInsightMatrixProvider
    {
    public:
        int32 Executions = 0;
        virtual FName GetProviderName() const override { return TEXT("DisabledAudit"); }
        virtual FString GetPluginName() const override { return TEXT("GorgeousCore"); }
        virtual void ExecuteAction(FName, const FGorgeousInsightActionContext&) override { ++Executions; }
        virtual FGorgeousInsightTestResult RunTest(FName, const FGorgeousInsightTestContext&) override { ++Executions; return {}; }
    } Provider;
    auto* Matrix = NewObject<UGorgeousInsightMatrixSubsystem>(GEngine);
    Matrix->RegisterProvider(&Provider);
    ON_SCOPE_EXIT { Matrix->UnregisterProvider(&Provider); };
    TestFalse(TEXT("Direct action is rejected"), Matrix->ExecuteAction(Provider.GetProviderName(), TEXT("Action"), {}));
    const auto TestResult = Matrix->RunTest(Provider.GetProviderName(), TEXT("Test"), {});
    TestTrue(TEXT("Rejected test is a reported failure"), !TestResult.bSuccess && !TestResult.Errors.IsEmpty());
    TestFalse(TEXT("Queue refuses a test"), Matrix->EnqueueTest(Provider.GetProviderName(), TEXT("Test"), {}));
    TestFalse(TEXT("Queue refuses a scenario"), Matrix->EnqueueScenario(TEXT("Scenario")));
    TestFalse(TEXT("Public harness status reflects runtime opt-out"), FGorgeousInsightHarness::GetStatus().bHarnessEnabled);
    TestFalse(TEXT("Public shared harness startup has no blanket active-automation bypass"), FGorgeousInsightHarness::StartHarness(FGorgeousInsightMatrixRequest::FromParameters(TEXT(""))));
    FGorgeousMultiClientTestConfig ClientConfig;
    FGorgeousMultiClientTestReport ClientReport;
    TestFalse(TEXT("Public multi-client orchestration rejects a default-null runtime caller"), FGorgeousMultiClientTestOrchestrator::RunTests(ClientConfig, ClientReport));
    TestTrue(TEXT("Rejected orchestration records the actual tools policy failure"), ClientReport.FailureReason.Contains(TEXT("Development Suite")) && !ClientReport.bAllPassed && ClientReport.EndpointResults.IsEmpty());
    TestEqual(TEXT("No provider callback executed"), Provider.Executions, 0);
    int32 ScenarioExecutions = 0;
    FGorgeousInsightScenarioDescriptor Descriptor;
    Descriptor.ScenarioName = TEXT("DisabledRuntimeAudit");
    Descriptor.Runner = [&ScenarioExecutions](const FGorgeousInsightScenarioContext&) { ++ScenarioExecutions; return FGorgeousInsightScenarioResult(); };
    const auto Request = FGorgeousInsightMatrixRequest::FromParameters(TEXT(""));
    FGorgeousInsightScenarioContext RuntimeContext(Request, TEXT(""), 0, nullptr, Descriptor);
    const auto RuntimeResult = FGorgeousInsightTestMatrix::ExecuteScenario(Descriptor, RuntimeContext);
    TestTrue(TEXT("Direct runtime matrix entry reports disabled failure"), !RuntimeResult.bSuccess && !RuntimeResult.Errors.IsEmpty());
    TestEqual(TEXT("Runtime scenario did not run"), ScenarioExecutions, 0);
    FGorgeousInsightScenarioContext NativeContext(Request, TEXT(""), 0, this, Descriptor);
    const auto NativeResult = FGorgeousInsightTestMatrix::ExecuteScenario(Descriptor, NativeContext);
    TestEqual(TEXT("Actual native automation retains its scenario execution path"), ScenarioExecutions, 1);
    TestTrue(TEXT("Native scenario returned its actual result"), NativeResult.bSuccess);
    Matrix->ShowDebugPanel();
    Matrix->ShowInGamePanel();
    TestFalse(TEXT("No disabled panel was opened"), Matrix->IsDebugPanelOpen() || Matrix->IsInGamePanelOpen());
    TestTrue(TEXT("Draining a disabled queue cannot fabricate a successful empty run"), !Matrix->RunQueuedScenarios()[0].bSuccess);
    TestTrue(TEXT("Disabled matrix returns a concrete failed result"), !Matrix->RunMatrix()[0].Result.bSuccess);
    return true;
}
#endif
