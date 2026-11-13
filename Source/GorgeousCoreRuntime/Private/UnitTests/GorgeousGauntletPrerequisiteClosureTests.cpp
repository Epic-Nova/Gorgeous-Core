// Copyright (c) 2026 Simsalabim Studios. All rights reserved.
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "InsightMatrix/GorgeousInsightHarness.h"
#include "GorgeousDevelopmentToolsTestScope.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousGauntletPrerequisiteClosure,
    "GorgeousCore.LimitationClosure.Insight.GauntletPrerequisites",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousGauntletPrerequisiteClosure::RunTest(const FString&)
{
    FGorgeousDevelopmentToolsTestScope Tools;
    if(FGorgeousInsightHarness::IsGauntletActive())
    {AddError(TEXT("This negative control must execute without a Gauntlet host."));return false;}
    const auto Registered=FGorgeousInsightTestMatrix::GetRegisteredScenarios();
    for(FName Name:{FName(TEXT("AutoReplication.Networking.Iris")),FName(TEXT("AutoReplication.Networking.Graph")),
        FName(TEXT("AutoReplication.AccessPolicy.AllowDeny")),FName(TEXT("AutoReplication.AccessPolicy.Timing"))})
    {
        const auto* Descriptor=Registered.FindByPredicate([Name](const auto& Item){return Item.ScenarioName==Name;});
        if(!TestNotNull(*FString::Printf(TEXT("Actual registered scenario %s exists"),*Name.ToString()),Descriptor))continue;
        if(!TestTrue(TEXT("Actual registered runner exists"),bool(Descriptor->Runner)))continue;
        const auto Request=FGorgeousInsightMatrixRequest::FromParameters(FString::Printf(TEXT("Scenario=%s Harness=false"),*Name.ToString()));
        FGorgeousInsightScenarioContext Context(Request,Request.RawParameters,0,this,*Descriptor,nullptr);
        const auto Direct=Descriptor->Runner(Context);
        TestFalse(*FString::Printf(TEXT("Direct runner %s cannot pass without Gauntlet"),*Name.ToString()),Direct.bSuccess);
        TestFalse(TEXT("Missing Gauntlet is an actual error, not only a warning"),Direct.Errors.IsEmpty());
        const auto Routed=FGorgeousInsightTestMatrix::ExecuteScenario(*Descriptor,Context);
        TestFalse(*FString::Printf(TEXT("ExecuteScenario %s cannot pass without Gauntlet"),*Name.ToString()),Routed.bSuccess);
        TestFalse(TEXT("Matrix prerequisite failure carries explicit error"),Routed.Errors.IsEmpty());
    }
    return true;
}
#endif
