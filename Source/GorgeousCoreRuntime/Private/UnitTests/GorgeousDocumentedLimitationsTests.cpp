// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "ConditionalObjectChoosers/Conditions/GorgeousGameplayTagCondition.h"
#include "FeedbackEffects/Effects/GorgeousSoundFeedbackEffect.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousTextProcessor.h"
#include "Components/TextBlock.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "GeneralSystems/GorgeousGeneralSystemAction.h"
#include "GeneralSystems/GorgeousConfigurationText.h"
#include "GeneralSystems/GeneralSystemTypes.h"
#include "GeneralSystems/GeneralSystemConfiguration_PDA.h"
#include "GeneralSystems/GorgeousProjectSettingsContribution.h"
#include "FunctionalStructures/GorgeousFunctionalStructure.h"
#include "Misc/App.h"
#include "GeneralSystems/Guards/GorgeousInteractionFoundationGuard.h"
#include "InsightMatrix/GorgeousInsightMatrixSubsystem.h"
#include "GorgeousDevelopmentToolsTestScope.h"
#include "InsightMatrix/GorgeousInsightChartSerialization.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopeExit.h"
#include "HAL/FileManager.h"
#include "Serialization/JsonSerializer.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousChooserMappingRegression,
	"GorgeousCore.Limitations.Chooser.OverlappingMappings",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousChooserMappingRegression::RunTest(const FString& Parameters)
{
	const FGameplayTag A = TAG_Gorgeous_UI_Layer_HUD;
	// HUD.ActionBar inherits HUD and would legitimately match OnlyA via HasAny.
	const FGameplayTag B = TAG_Gorgeous_UI_Layer_TabList;
	TestFalse(TEXT("Fixture tags are independent in the real gameplay tag hierarchy"), B.MatchesTag(A));
	UGorgeousSoundFeedbackEffect* Source = NewObject<UGorgeousSoundFeedbackEffect>();
	Source->Tags.AddTag(B);
	UGorgeousGameplayTagCondition* Condition = NewObject<UGorgeousGameplayTagCondition>();
	Condition->GameplayTagContainerClassReference = Source;
	Condition->GameplayTagContainerUPropertyName = GET_MEMBER_NAME_CHECKED(UGorgeousFeedbackEffect, Tags);
	FGameplayTagContainer OnlyA(A);
	FGameplayTagContainer Both(A);
	Both.AddTag(B);
	Condition->GameplayTagConditionMapping.Add(FGameplayTagContainerWrapper_S(OnlyA), 9);
	Condition->GameplayTagConditionMapping.Add(FGameplayTagContainerWrapper_S(Both), 27);
	for (const EConditionalGameplayTagChooserFightMode_E Mode : {
		EConditionalGameplayTagChooserFightMode_E::FIRST,
		EConditionalGameplayTagChooserFightMode_E::LAST,
		EConditionalGameplayTagChooserFightMode_E::RANDOM })
	{
		Condition->GameplayTagChooserFightMode = Mode;
		TestEqual(TEXT("Only matching mapping supplies the index"), Condition->CheckCondition(), uint8(27));
	}
	Source->Tags.Reset();
	Source->Tags.AddTag(TAG_Gorgeous_UI_Layer_ActionBar);
	Condition->GameplayTagConditionMapping.Remove(FGameplayTagContainerWrapper_S(Both));
	Condition->GameplayTagChooserFightMode = EConditionalGameplayTagChooserFightMode_E::FIRST;
	TestTrue(TEXT("Child tags intentionally match parent-tag mappings"), Source->Tags.HasAny(OnlyA));
	TestEqual(TEXT("Parent-tag mapping remains a valid candidate"), Condition->CheckCondition(), uint8(9));
	Condition->GameplayTagConditionMapping.Add(FGameplayTagContainerWrapper_S(Both), 27);
	FGameplayTagContainer Reversed(B);
	Reversed.AddTag(A);
	TestTrue(TEXT("Container equality ignores insertion order"), FGameplayTagContainerWrapper_S(Both) == FGameplayTagContainerWrapper_S(Reversed));
	TestEqual(TEXT("Equal keys have equal hashes"), GetTypeHash(FGameplayTagContainerWrapper_S(Both)), GetTypeHash(FGameplayTagContainerWrapper_S(Reversed)));
	int32 Value = INDEX_NONE;
	TestTrue(TEXT("Map lookup finds reordered key"), Condition->FindConditionMappingForTagContainer(Reversed, Value));
	TestEqual(TEXT("Reordered key retains mapped value"), Value, 27);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousUIEmptyStackRegression,
	"GorgeousCore.Limitations.CommonUI.EmptyStacksAndProcessor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousUIEmptyStackRegression::RunTest(const FString& Parameters)
{
	if (!TestNotNull(TEXT("Engine is available"), GEngine)) return false;
	ULocalPlayer* Player = NewObject<ULocalPlayer>(GEngine);
	UGorgeousUIFoundationSubsystem* Subsystem = NewObject<UGorgeousUIFoundationSubsystem>(Player);
	TestNull(TEXT("No input bindings is a valid initial state"), Subsystem->GetMostRecentInputBindings());
	// Querying the initial theme must be safe even when a project has no default asset.
	Subsystem->GetMostRecentTheme();
	UGorgeousUITheme_DA* Theme = NewObject<UGorgeousUITheme_DA>();
	Subsystem->SetCurrentTheme(Theme);
	TestEqual(TEXT("Latest applied theme wins"), Subsystem->GetMostRecentTheme(), Theme);
	Subsystem->SetCurrentTheme(nullptr);
	TestEqual(TEXT("Null theme cannot invalidate the active stack"), Subsystem->GetMostRecentTheme(), Theme);
	Subsystem->ProcessorClasses.Add(UTextBlock::StaticClass(), UGorgeousTextProcessor::StaticClass());
	UTextBlock* Widget = NewObject<UTextBlock>();
	UGorgeousUIProcessor* Processor = Subsystem->GetSharedProcessorForWidget(Widget);
	TestNotNull(TEXT("Concrete processor is instantiated rather than rejecting its CDO"), Processor);
	TestEqual(TEXT("Processor instance is shared"), Subsystem->GetSharedProcessorForWidget(Widget), Processor);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousConfigurationArrayRegression,
	"GorgeousCore.Limitations.Configuration.PreserveArrayEntries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousConfigurationArrayRegression::RunTest(const FString& Parameters)
{
	FString Source = TEXT("; authored configuration\n[Collision]\n+Channels=A\n+Channels=B\n[Other]\nKeep=Yes\n");
	TestTrue(TEXT("A new array entry changes the source"), GorgeousConfiguration::ApplyTextEdit(Source, TEXT("Collision"), TEXT("Channels"), TEXT("C"), EGorgeousConfigOperation::Add));
	TestTrue(TEXT("First existing channel survives"), Source.Contains(TEXT("+Channels=A")));
	TestTrue(TEXT("Second existing channel survives"), Source.Contains(TEXT("+Channels=B")));
	TestTrue(TEXT("Third channel is appended"), Source.Contains(TEXT("+Channels=C")));
	TestTrue(TEXT("Other section and comment survive"), Source.Contains(TEXT("[Other]\nKeep=Yes")) && Source.Contains(TEXT("; authored configuration")));
	TestFalse(TEXT("Repeating an Add is idempotent"), GorgeousConfiguration::ApplyTextEdit(Source, TEXT("Collision"), TEXT("Channels"), TEXT("C"), EGorgeousConfigOperation::Add));
	TestTrue(TEXT("Removal records a distinct Unreal remove operation"), GorgeousConfiguration::ApplyTextEdit(Source, TEXT("Collision"), TEXT("Channels"), TEXT("B"), EGorgeousConfigOperation::Remove));
	TestTrue(TEXT("Remove operation preserves unrelated entries"), Source.Contains(TEXT("-Channels=B")) && Source.Contains(TEXT("+Channels=A")) && Source.Contains(TEXT("+Channels=C")));
	const FString BeforeNoOp = Source;
	TestFalse(TEXT("Removing a missing section changes nothing"), GorgeousConfiguration::ApplyTextEdit(Source, TEXT("Missing"), TEXT("Key"), TEXT(""), EGorgeousConfigOperation::RemoveLine));
	TestEqual(TEXT("No empty section is created by a no-op removal"), Source, BeforeNoOp);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousChartContextRegression,
	"GorgeousCore.Limitations.Insight.ChartContextHistoryAndExport",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousChartContextRegression::RunTest(const FString& Parameters)
{
    FGorgeousDevelopmentToolsTestScope ToolsScope;
	if (!TestNotNull(TEXT("Engine is available"), GEngine)) return false;
	class FChartProvider final : public IGorgeousInsightMatrixProvider
	{
	public:
		double Value = 3.0;
		virtual FName GetProviderName() const override { return TEXT("AuditCharts"); }
		virtual FString GetPluginName() const override { return TEXT("GorgeousCore"); }
		virtual void GatherCharts(const FGorgeousInsightGatherContext& Context, TArray<FGorgeousInsightChartDefinition>& Out) const override
		{
			FGorgeousInsightBarValue Bar;
			Bar.Label = FText::FromString(TEXT("A, \"bar\""));
			Bar.Value = Value;
			Out.Add(FGorgeousInsightChartDefinition::MakeBarChart(TEXT("Audit"), FText::FromString(Context.InstanceName), FText::GetEmpty(), { Bar }));
		}
	} Provider;
	UGorgeousInsightMatrixSubsystem* Matrix = NewObject<UGorgeousInsightMatrixSubsystem>(GEngine);
	Matrix->bStatsCacheLoaded = true;
	Matrix->bChartBaselinesLoaded = true;
	Matrix->RegisterProvider(&Provider);
	ON_SCOPE_EXIT { Matrix->UnregisterProvider(&Provider); };
	FGorgeousInsightGatherContext Context;
	Context.Mode = EGorgeousInsightContextMode::Live;
	Context.InstanceName = TEXT("Requested world");
	TArray<FGorgeousInsightChartDefinition> Charts;
	Matrix->GatherProviderCharts(Provider.GetProviderName(), Context, Charts);
	if (!TestEqual(TEXT("Live provider emits a chart"), Charts.Num(), 1)) return false;
	TestEqual(TEXT("Provider receives the requested context"), Charts[0].Title.ToString(), Context.InstanceName);
	TestEqual(TEXT("Live value is sampled"), Charts[0].Bars[0].Value, 3.0);
	Matrix->ProviderBaselineCharts.Add(Provider.GetProviderName(), Charts);
	Matrix->ProviderBaselineCharts.FindChecked(Provider.GetProviderName())[0].Bars[0].Value = 2.0;
	Provider.Value = 7.0;
	Context.Mode = EGorgeousInsightContextMode::Editor;
	Matrix->GatherProviderCharts(Provider.GetProviderName(), Context, Charts);
	TestEqual(TEXT("Editor context retains sampled history rather than querying live provider"), Charts[0].Bars[0].Value, 3.0);
	Context.Mode = EGorgeousInsightContextMode::Baseline;
	Matrix->GatherProviderCharts(Provider.GetProviderName(), Context, Charts);
	TestEqual(TEXT("Baseline context uses its own snapshot"), Charts[0].Bars[0].Value, 2.0);
	if (!TestFalse(TEXT("Live charts create history snapshots"), Matrix->EditorHistory.IsEmpty())) return false;
	Matrix->EditorHistory.Last().Timestamp -= FTimespan::FromSeconds(10.0);
	Context.Mode = EGorgeousInsightContextMode::Live;
	Matrix->GatherProviderCharts(Provider.GetProviderName(), Context, Charts);
	TestEqual(TEXT("New live sample advances value"), Charts[0].Bars[0].Value, 7.0);
	Matrix->ApplyHistorySnapshot(0);
	Context.Mode = EGorgeousInsightContextMode::Editor;
	Matrix->GatherProviderCharts(Provider.GetProviderName(), Context, Charts);
	TestEqual(TEXT("Selecting old history restores old chart value"), Charts[0].Bars[0].Value, 3.0);
	FGorgeousUIMessageResult Payload;
	Payload.ResultTag = TEXT("RoundTrip");
	Charts[0].InstancedPayload = FInstancedStruct::Make(Payload);
	Charts[0].TableRows.AddDefaulted_GetRef().Value = FText::FromString(TEXT("10"));
	FGorgeousInsightChartDefinition Restored;
	if (!TestTrue(TEXT("Chart JSON round trip succeeds"), GorgeousInsightCharts::FromJson(GorgeousInsightCharts::ToJson(Charts[0]), Restored))) return false;
	if (!TestEqual(TEXT("Round trip retains bar entries"), Restored.Bars.Num(), 1) || !TestEqual(TEXT("Round trip retains table entries"), Restored.TableRows.Num(), 1)) return false;
	TestEqual(TEXT("Bar value survives JSON"), Restored.Bars[0].Value, 3.0);
	TestEqual(TEXT("Table data survives JSON"), Restored.TableRows[0].Value.ToString(), FString(TEXT("10")));
	const FGorgeousUIMessageResult* RestoredPayload = Restored.InstancedPayload.GetPtr<FGorgeousUIMessageResult>();
	TestTrue(TEXT("Custom instanced payload survives JSON"), RestoredPayload && RestoredPayload->ResultTag == Payload.ResultTag);
	const FString Stem = FPaths::ProjectSavedDir() / TEXT("GorgeousThings/AuditTests") / FGuid::NewGuid().ToString(EGuidFormats::Digits);
	const FString JsonPath = Stem + TEXT(".json");
	const FString CsvPath = Stem + TEXT(".csv");
	ON_SCOPE_EXIT { IFileManager::Get().Delete(*JsonPath); IFileManager::Get().Delete(*CsvPath); };
	Context.Mode = EGorgeousInsightContextMode::Baseline;
	TestTrue(TEXT("Selected baseline exports to JSON"), Matrix->ExportStatsToJson(JsonPath, Provider.GetProviderName(), Context));
	TestTrue(TEXT("Selected baseline exports to CSV"), Matrix->ExportStatsToCSV(CsvPath, Provider.GetProviderName(), Context));
	FString JsonText;
	FFileHelper::LoadFileToString(JsonText, *JsonPath);
	TSharedPtr<FJsonObject> Export;
	TestTrue(TEXT("Exported JSON parses"), FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonText), Export));
	if (Export.IsValid())
	{
		const auto ExportedCharts = GorgeousInsightCharts::FromJsonArray(Export->GetArrayField(TEXT("charts")));
		TestEqual(TEXT("Export contains chart record"), ExportedCharts.Num(), 1);
		if (!ExportedCharts.IsEmpty()) TestEqual(TEXT("Export matches baseline rather than live"), ExportedCharts[0].Bars[0].Value, 2.0);
	}
	return true;
}

namespace
{
    class FGorgeousActionOutcomeAuditProvider : public IGorgeousInsightMatrixProvider
    {
    public:
        EGorgeousInsightActionStatus Status = EGorgeousInsightActionStatus::Failed;
        virtual FName GetProviderName() const override { return TEXT("Audit.ActionOutcome"); }
        virtual FString GetPluginName() const override { return TEXT("GorgeousCore"); }
        virtual FGorgeousInsightActionResult ExecuteActionWithResult(FName, const FGorgeousInsightActionContext&) override
        {
            FGorgeousInsightActionResult Result; Result.Status = Status; Result.Message = TEXT("Actual action outcome"); return Result;
        }
    };
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousActionOutcomeAudit,
    "GorgeousCore.Limitations.Insight.ActionOutcome",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousActionOutcomeAudit::RunTest(const FString& Parameters)
{
    FGorgeousDevelopmentToolsTestScope ToolsScope;
    const FString StateFile = UGorgeousInsightMatrixSubsystem::GetInsightMatrixIniPath();
    TArray<uint8> PriorState;
    const bool bHadState = FFileHelper::LoadFileToArray(PriorState, *StateFile);
    ON_SCOPE_EXIT
    {
        if (bHadState) FFileHelper::SaveArrayToFile(PriorState, *StateFile);
        else IFileManager::Get().Delete(*StateFile);
    };

    UGorgeousInsightMatrixSubsystem* Subsystem = NewObject<UGorgeousInsightMatrixSubsystem>(GEngine);
    FGorgeousActionOutcomeAuditProvider Provider;
    Subsystem->RegisterProvider(&Provider);
    ON_SCOPE_EXIT { Subsystem->UnregisterProvider(&Provider); };
    FGorgeousInsightActionContext Context;
    TestFalse(TEXT("Real failed outcome propagates to caller"), Subsystem->ExecuteAction(Provider.GetProviderName(), TEXT("Action"), Context));
    UGorgeousInsightMatrixSubsystem::FGorgeousInsightLastActionStats Stats;
    TestTrue(TEXT("Failed action recorded"), Subsystem->GetLastActionStats(Provider.GetProviderName(), TEXT("Action"), Stats));
    TestTrue(TEXT("Recorded failure is an actual result"), Stats.bHasExecutionResult && !Stats.bSucceeded);
    Provider.Status = EGorgeousInsightActionStatus::Dispatched;
    TestTrue(TEXT("Legacy dispatch remains accepted"), Subsystem->ExecuteAction(Provider.GetProviderName(), TEXT("Action"), Context));
    Subsystem->GetLastActionStats(Provider.GetProviderName(), TEXT("Action"), Stats);
    TestFalse(TEXT("Dispatch is not mislabeled actual success"), Stats.bHasExecutionResult || Stats.bSucceeded);
    Provider.Status = EGorgeousInsightActionStatus::Succeeded;
    TestTrue(TEXT("Actual success accepted"), Subsystem->ExecuteAction(Provider.GetProviderName(), TEXT("Action"), Context));
    Subsystem->GetLastActionStats(Provider.GetProviderName(), TEXT("Action"), Stats);
    TestTrue(TEXT("Actual success recorded"), Stats.bHasExecutionResult && Stats.bSucceeded);
    TestFalse(TEXT("Unknown provider is failed test"), Subsystem->RunTest(TEXT("Missing"), TEXT("Missing"), FGorgeousInsightTestContext()).bSuccess);
    FGorgeousInsightTestResult FunctionalFailure;
    FunctionalFailure.bSuccess = false;
    FunctionalFailure.Errors.Add(TEXT("Functional actor assertion failed"));
    Subsystem->RecordFunctionalTestResult(TEXT("World.Actor"), FText::FromString(TEXT("Recorded actor")), FunctionalFailure, 1.0);
    TArray<FGorgeousInsightTest> RecordedTests;
    Subsystem->GatherProviderTests(TEXT("FunctionalTests"), RecordedTests);
    TestEqual(TEXT("Functional actor appears as a real panel test row"), RecordedTests.Num(), 1);
    const auto* Recorded = Subsystem->GetLastTestResult(TEXT("FunctionalTests"), TEXT("World.Actor"));
    TestTrue(TEXT("Panel receives actual actor failure, not default success"), Recorded && !Recorded->bSuccess && Recorded->Errors.Contains(TEXT("Functional actor assertion failed")));
    TestEqual(TEXT("Functional results refresh panel revision"), Subsystem->GetExternalTestResultRevision(), uint64(1));

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousInteractionGuardAliasAudit,
    "GorgeousCore.Limitations.Interaction.PromptGuardCompatibility",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousInteractionGuardAliasAudit::RunTest(const FString& Parameters)
{
    // The previously omitted enabled-branch macro fails compilation at this actual call site.
    UClass* PromptClass = GORGEOUS_CLASS_INTERACTION_FOUNDATION_PROMPT_WIDGET();
#if GORGEOUS_SYSTEM_INSTALLED_INTERACTIONFOUNDATION
    TestNotNull(TEXT("Installed interaction guard resolves the shipped prompt component"), PromptClass);
    TestTrue(TEXT("Prompt is a scene component suitable for guard component initialization"),
        PromptClass && PromptClass->IsChildOf(USceneComponent::StaticClass()));
#else
    TestNull(TEXT("Absent interaction content returns the documented null fallback"), PromptClass);
#endif
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousScenarioAssertionsAudit,
    "GorgeousCore.Limitations.Insight.NoScaffoldedPasses",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousScenarioAssertionsAudit::RunTest(const FString& Parameters)
{
    const auto Scenarios = FGorgeousInsightTestMatrix::GetRegisteredScenarios();
    const auto* ImGui = Scenarios.FindByPredicate([](const auto& D) { return D.ScenarioName == TEXT("Core.DebugAssist.Functional.ImGuiPopulation"); });
    const auto* Performance = Scenarios.FindByPredicate([](const auto& D) { return D.ScenarioName == TEXT("Core.SignalBridge.Performance.LocalDispatch"); });
    if (!TestNotNull(TEXT("Actual ImGui scenario is registered"), ImGui) || !TestNotNull(TEXT("Actual signal performance scenario is registered"), Performance)) return false;
    const auto Request = FGorgeousInsightMatrixRequest::FromParameters(TEXT(""));
    FGorgeousInsightScenarioContext ImGuiContext(Request, TEXT(""), 0, nullptr, *ImGui);
    const auto ImGuiResult = ImGui->Runner(ImGuiContext);
    TestFalse(TEXT("Unimplemented ImGui is not in the default runnable matrix"), ImGui->bEnabledByDefault);
    TestTrue(TEXT("Explicit unavailable ImGui run reports actual failure"), !ImGuiResult.bSuccess && !ImGuiResult.Errors.IsEmpty());
    FGorgeousInsightScenarioContext PerformanceContext(Request, TEXT(""), 0, nullptr, *Performance);
    const auto PerformanceResult = Performance->Runner(PerformanceContext);
    TestTrue(TEXT("Real dispatch benchmark satisfies its functional assertions"), PerformanceResult.bSuccess && PerformanceResult.Errors.IsEmpty());
    TestEqual(TEXT("All10000 actual signals received"), PerformanceResult.Metrics.FindRef(TEXT("Signals Received")), FString(TEXT("10000")));
    TestEqual(TEXT("All10000 nested replies received"), PerformanceResult.Metrics.FindRef(TEXT("Results Sub-Dispatched")), FString(TEXT("10000")));
    TestEqual(TEXT("Actual payload checks return950000 prime calculations"), PerformanceResult.Metrics.FindRef(TEXT("Verified Prime Calculations")), FString(TEXT("950000")));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousStructDefaultAudit,
    "GorgeousCore.Limitations.Structs.DeterministicDamageDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousStructDefaultAudit::RunTest(const FString& Parameters)
{
    TTypeCompatibleBytes<FGorgeousDamageContext_S> Storage;
    FMemory::Memset(&Storage, 0xCD, sizeof(Storage));
    auto* Damage = new (Storage.GetTypedPtr()) FGorgeousDamageContext_S;
    TestEqual(TEXT("Default construction initializes damage even in poisoned storage"), Damage->BaseDamage, 0.0f);
    TestTrue(TEXT("HitLocation has deterministic default"), Damage->HitLocation.IsZero());
    TestTrue(TEXT("ShotDirection has deterministic default"), Damage->ShotDirection.IsZero());
    Damage->~FGorgeousDamageContext_S();
    FGorgeousFunctionalStructure_S FunctionalA, FunctionalB;
    FGorgeousUIMessageRequest MessageA, MessageB;
    TestTrue(TEXT("Functional IDs retain valid per-instance random identity"), FunctionalA.Identifier.IsValid() && FunctionalB.Identifier.IsValid() && FunctionalA.Identifier != FunctionalB.Identifier);
    TestTrue(TEXT("Message request IDs retain valid per-instance random identity"), MessageA.RequestID.IsValid() && MessageB.RequestID.IsValid() && MessageA.RequestID != MessageB.RequestID);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousUnattendedSystemActionAudit,
    "GorgeousCore.Limitations.Configuration.UnattendedActions",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousUnattendedSystemActionAudit::RunTest(const FString& Parameters)
{
    if (!FApp::IsUnattended()) { AddError(TEXT("Automation must be classified unattended by the engine.")); return false; }
    UGeneralSystemConfiguration_PDA* Configuration = NewObject<UGeneralSystemConfiguration_PDA>();
    auto* Prompt = NewObject<UGorgeousTriggerPromptAction>(Configuration);
    auto* Write = NewObject<UGorgeousManageConfigurationAction>(Configuration);
    auto* Reparent = NewObject<UGorgeousReparentBlueprintAction>(Configuration);
    auto* Contribution = NewObject<UGorgeousProjectSettingsContribution>(Configuration);
    TestFalse(TEXT("Prompt blocked before prediction/dispatch"), Prompt->CanExecuteInCurrentSession());
    TestFalse(TEXT("Reparent mutation blocked"), Reparent->CanExecuteInCurrentSession());
    TestFalse(TEXT("Project setup mutation blocked"), Contribution->CanExecuteInCurrentSession());
    Write->Operation = EGorgeousConfigOperation::SetOrNewLine;
    TestFalse(TEXT("Config write blocked"), Write->CanExecuteInCurrentSession());
    const FString Filename = FPaths::ProjectSavedDir() / TEXT("GorgeousThings/AuditTests") / (FGuid::NewGuid().ToString(EGuidFormats::Digits) + TEXT(".ini"));
    Write->ConfigurationTarget = Filename; Write->Section = TEXT("Audit"); Write->Key = TEXT("Value"); Write->Value = TEXT("ShouldNotWrite");
    Write->bPrompt = false; Write->Trigger = EGorgeousConfigTrigger::IfNotPresent;
    ON_SCOPE_EXIT { IFileManager::Get().Delete(*Filename); };
    // Direct native entry must be safe too, even when prediction is bypassed.
    Write->OnSystemActivated(Configuration);
    TestFalse(TEXT("Direct unattended activation never creates a configuration file"), IFileManager::Get().FileExists(*Filename));
    Prompt->OnSystemActivated(Configuration); // Must return, never open FMessageDialog/Slate modal.
    Write->Operation = EGorgeousConfigOperation::Read;
    TestTrue(TEXT("Read-only configuration queries remain available"), Write->CanExecuteInCurrentSession());
    auto* Registry = NewObject<UGorgeousAddDataRegistrySourceAction>(Configuration);
    TestTrue(TEXT("Runtime registry actions remain available"), Registry->CanExecuteInCurrentSession());
    return true;
}

#endif
