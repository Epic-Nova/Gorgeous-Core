// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Insight Matrix (Runtime)                   |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#include "InsightMatrix/GorgeousInsightMatrixSubsystem.h"
#include "InsightMatrix/GorgeousDevelopmentToolsService.h"
#include "InsightMatrix/GorgeousDevelopmentSuiteSettings.h"
#include "InsightMatrix/GorgeousInsightChartSerialization.h"
#include "UObject/GCObject.h"
#include "UObject/UObjectBase.h"
#if WITH_EDITOR
#include "FunctionalTest.h"
#endif
#include "EngineUtils.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "InsightMatrix/GorgeousInsightHarness.h"
#include "InsightMatrix/GorgeousInsightMatrixProvider.h"
#include "InsightMatrix/GorgeousInsightMatrixTypes.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "InsightMatrix/GorgeousInsightHarness.h"
#include "InsightMatrix/GorgeousInsightBaselineSettings.h"
#include "Helpers/Macros/GorgeousProfilingHelperMacros.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/UObjectIterator.h"
#include "Misc/FileHelper.h"
#include "GeneralSystems/DebugAssist/GorgeousDebugAssistBlueprintFunctionLibrary.h"
#include "Misc/CoreDelegates.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/PlatformTime.h"
#include "Stats/Stats.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/IConsoleManager.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"

/** Displays completed actor tests; running a row retrieves its recorded result, never fakes actor execution. */
class FGorgeousFunctionalResultProvider final : public IGorgeousInsightMatrixProvider
{
public:
    TMap<FName, FGorgeousInsightTestResult> Results;
    TMap<FName, FText> DisplayNames;
    virtual FName GetProviderName() const override { return TEXT("FunctionalTests"); }
    virtual FString GetPluginName() const override { return TEXT("GorgeousCore"); }
    virtual FText GetProviderDisplayName() const override { return NSLOCTEXT("GorgeousInsight", "FunctionalResults", "Functional test results"); }
    virtual void GetTests(TArray<FGorgeousInsightTest>& Out) const override
    {
        for (const auto& Pair : Results)
        {
            FGorgeousInsightTest Test; Test.Id = Pair.Key; Test.DisplayName = DisplayNames.FindRef(Pair.Key);
            Test.Category = TEXT("Recorded actor tests");
            Test.Description = NSLOCTEXT("GorgeousInsight", "FunctionalRecorded", "Recorded result. Execute the functional actor or test volume in its test world to run again.");
            Out.Add(MoveTemp(Test));
        }
    }
    virtual FGorgeousInsightTestResult RunTest(FName Id, const FGorgeousInsightTestContext&) override
    {
        if (const auto* Found = Results.Find(Id)) return *Found;
        FGorgeousInsightTestResult Result; Result.bSuccess = false; Result.Errors.Add(TEXT("Functional actor has not reported a result.")); return Result;
    }
};

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#endif

#if WITH_DEV_AUTOMATION_TESTS
namespace
{

	class FGorgeousInsightRuntimeAutomationTest final : public FAutomationTestBase
	{
	public:
		FGorgeousInsightRuntimeAutomationTest()
			: FAutomationTestBase(TEXT("GorgeousInsight.Runtime"), /*bIsComplexTest*/ false)
		{
		}

		GORGEOUS_55_SWITCH(
			virtual uint32 GetTestFlags() const override
			{
				return static_cast<uint32>(EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);
			},
			virtual EAutomationTestFlags GetTestFlags() const override
			{
				return EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
			})

		virtual uint32 GetRequiredDeviceNum() const override
		{
			return 1;
		}

		virtual FString GetBeautifiedTestName() const override
		{
			return TEXT("Gorgeous Insight Runtime");
		}

		virtual FString GetTestSourceFileName() const override
		{
			return TEXT("GorgeousInsightMatrixSubsystem.cpp");
		}

		virtual int32 GetTestSourceFileLine() const override
		{
			return 0;
		}

		virtual void GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const override
		{
			OutBeautifiedNames.Reset();
			OutTestCommands.Reset();
		}

		virtual bool RunTest(const FString& Parameters) override
		{
			return true;
		}
	};
}
#endif

namespace
{
static FString GetInsightSavedPath(const TCHAR* Filename)
{
	const FString Directory = FPaths::ProjectSavedDir() / TEXT("GorgeousThings/InsightMatrix");
	IFileManager::Get().MakeDirectory(*Directory, true);
	return Directory / Filename;
}

static FString GetBaselineIniPath(FName ProviderName)
{
	return GetInsightSavedPath(TEXT("ProviderBaselines.ini"));
}

static FString GetTestBaselineIniPath()
{
	return GetInsightSavedPath(TEXT("TestBaselines.ini"));
}

// Saved developer data is not part of the engine's layered config hierarchy.
// Use an explicit file transaction, as PanelState already does, so a successful
// in-memory GConfig mutation cannot masquerade as persisted baseline data.
static TArray<FGorgeousInsightStat> ReadProviderBaselineStats(FName ProviderName)
{
	FConfigFile File;
	File.Read(GetBaselineIniPath(ProviderName));
	TArray<FGorgeousInsightStat> Stats;
	const FConfigSection* Section = File.FindSection(TEXT("ProviderBaselineStats_") + ProviderName.ToString());
	if (!Section) return Stats;
	for (const auto& Pair : *Section)
	{
		TArray<FString> Parts;
		Pair.Value.GetValue().ParseIntoArray(Parts, TEXT("|"), false);
		if (Parts.Num() < 8) continue;
		FGorgeousInsightStat Stat;
		Stat.Id = FName(*Parts[0]); Stat.DisplayName = FText::FromString(Parts[1]);
		Stat.Description = FText::FromString(Parts[2]); Stat.Category = FName(*Parts[3]);
		Stat.ValueType = static_cast<EGorgeousInsightStatValueType>(FCString::Atoi(*Parts[4]));
		Stat.NumericValue = FCString::Atod(*Parts[5]); Stat.TextValue = FText::FromString(Parts[6]);
		Stat.Unit = FText::FromString(Parts[7]);
		Stats.Add(MoveTemp(Stat));
	}
	return Stats;
}

static void WriteProviderBaselineStats(FName ProviderName, const TArray<FGorgeousInsightStat>& Stats)
{
	const FString Path = GetBaselineIniPath(ProviderName);
	const FString SectionName = TEXT("ProviderBaselineStats_") + ProviderName.ToString();
	FConfigFile File;
	File.Read(Path); // Preserve baselines belonging to other providers.
	File.Remove(SectionName);
	for (const auto& Stat : Stats)
	{
		const FString Line = FString::Printf(TEXT("%s|%s|%s|%s|%d|%f|%s|%s"),
			*Stat.Id.ToString(), *Stat.DisplayName.ToString(), *Stat.Description.ToString(),
			*Stat.Category.ToString(), static_cast<int32>(Stat.ValueType), Stat.NumericValue,
			*Stat.TextValue.ToString(), *Stat.Unit.ToString());
		File.SetString(*SectionName, *Stat.Id.ToString(), *Line);
	}
	File.Dirty = true; // Also persist removal of the final stat in a section.
	if (!File.Write(Path))
		GT_W_LOG("GT.Core.Insight", TEXT("Could not persist provider baselines to '%s'."), *Path);
}

	static FString MakeBaselineKey(FName ProviderName, FName TestId)
	{
		return UGorgeousInsightBaselineSettings::MakeKey(ProviderName, TestId);
	}

	static bool ResolveProviderForTestId(const TArray<IGorgeousInsightMatrixProvider*>& Providers, FName TestId, FName& OutProvider)
	{
		OutProvider = NAME_None;
		FName Candidate = NAME_None;
		for (IGorgeousInsightMatrixProvider* Provider : Providers)
		{
			if (!Provider)
			{
				continue;
			}
			TArray<FGorgeousInsightTest> Tests;
			Provider->GetTests(Tests);
			if (Tests.ContainsByPredicate([TestId](const FGorgeousInsightTest& Test)
			{
				return Test.Id == TestId;
			}))
			{
				if (Candidate.IsNone())
				{
					Candidate = Provider->GetProviderName();
				}
				else
				{
					OutProvider = NAME_None;
					return false;
				}
			}
		}
		OutProvider = Candidate;
		return !OutProvider.IsNone();
	}

	static void SetBaselineCommand(const TArray<FString>& Args)
	{
		if (Args.Num() < 1)
		{
			GT_W_LOG("GT.Core.Insight", TEXT("Usage: Gorgeous.Insight.SetBaseline <TestId> [ProviderName]"));
			return;
		}

		UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get();
		if (!Subsystem)
		{
			GT_W_LOG("GT.Core.Insight", TEXT("Insight Matrix subsystem not available."));
			return;
		}

		const FName TestId(*Args[0]);
		FName ProviderName = NAME_None;
		if (Args.Num() > 1)
		{
			ProviderName = FName(*Args[1]);
		}
		else
		{
			TArray<IGorgeousInsightMatrixProvider*> Providers = Subsystem->GetProviders();
			if (!ResolveProviderForTestId(Providers, TestId, ProviderName))
			{
				GT_W_LOG("GT.Core.Insight", TEXT("Could not resolve provider for test '%s'. Provide the provider name as second argument."), *TestId.ToString());
				return;
			}
		}

		if (!Subsystem->SetBaselineFromLastResult(ProviderName, TestId))
		{
			GT_W_LOG("GT.Core.Insight", TEXT("No last result found for test '%s' (%s). Run the test first."), *TestId.ToString(), *ProviderName.ToString());
			return;
		}

		Subsystem->RefreshDebugPanel();
		GT_I_LOG("GT.Core.Insight", TEXT("Baseline updated for test '%s' (%s)."), *TestId.ToString(), *ProviderName.ToString());
	}

	static FAutoConsoleCommand GInsightSetBaselineCmd(
		TEXT("Gorgeous.Insight.SetBaseline"),
		TEXT("Sets baseline from last test result. Usage: Gorgeous.Insight.SetBaseline <TestId> [ProviderName]"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&SetBaselineCommand));

	DECLARE_STATS_GROUP(TEXT("Gorgeous Insight Matrix"), STATGROUP_GorgeousInsightMatrix, STATCAT_Advanced);
	DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Providers"), STAT_GIM_Providers, STATGROUP_GorgeousInsightMatrix);
	DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Stats"), STAT_GIM_Stats, STATGROUP_GorgeousInsightMatrix);
	DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Actions"), STAT_GIM_Actions, STATGROUP_GorgeousInsightMatrix);
	DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Tests"), STAT_GIM_Tests, STATGROUP_GorgeousInsightMatrix);
	DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Actions Executed"), STAT_GIM_ActionsExecuted, STATGROUP_GorgeousInsightMatrix);
	DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Tests Run"), STAT_GIM_TestsRun, STATGROUP_GorgeousInsightMatrix);
	DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("CSV Exports"), STAT_GIM_ExportsCsv, STATGROUP_GorgeousInsightMatrix);
	DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("JSON Exports"), STAT_GIM_ExportsJson, STATGROUP_GorgeousInsightMatrix);

	UObject* ResolveWorldContextObject(UObject* Requested)
	{
		if (Requested)
		{
			return Requested;
		}

		if (!GEngine)
		{
			return nullptr;
		}

		for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
		{
			if (!WorldContext.World())
			{
				continue;
			}

			switch (WorldContext.WorldType)
			{
				case EWorldType::PIE:
				case EWorldType::Game:
				case EWorldType::GamePreview:
					return WorldContext.World();
				default:
					break;
			}
		}

		for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
		{
			if (WorldContext.World())
			{
				return WorldContext.World();
			}
		}

		return nullptr;
	}
}

namespace
{
IGorgeousDevelopmentToolsService* GetDevelopmentToolsService()
{
    auto Services = IModularFeatures::Get().GetModularFeatureImplementations<IGorgeousDevelopmentToolsService>(IGorgeousDevelopmentToolsService::GetFeatureName());
    return Services.Num() == 1 ? Services[0] : nullptr;
}
}

bool UGorgeousInsightMatrixSubsystem::IsDevelopmentToolsEnabled()
{
    auto* Service = GetDevelopmentToolsService();
    if (!Service || !Service->IsAvailable() || IsEngineExitRequested() || !UObjectInitialized()) return false;
    if (!GetDefault<UGorgeousDevelopmentSuiteSettings>()->bEnableInsightMatrix) return false;
#if UE_BUILD_SHIPPING || UE_BUILD_TEST
    return Service->AllowsShippingTools();
#else
    return true;
#endif
}

bool UGorgeousInsightMatrixSubsystem::IsNativeAutomationExecution(const FAutomationTestBase* Test)
{
#if WITH_DEV_AUTOMATION_TESTS && !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    const auto* Current = FAutomationTestFramework::Get().GetCurrentTest();
    return Current && Test && Current == Test;
#else
    return false;
#endif
}

void UGorgeousInsightMatrixSubsystem::SetPanelState(const FGorgeousInsightPanelState& State)
{
    if (!IsDevelopmentToolsEnabled()) return;
    CachedPanelState = State;
    bHasCachedPanelState = true;
    SavePanelState();
}

UGorgeousInsightMatrixSubsystem* UGorgeousInsightMatrixSubsystem::Get()
{
	if (!GEngine)
	{
		return nullptr;
	}

	return GEngine->GetEngineSubsystem<UGorgeousInsightMatrixSubsystem>();
}

void UGorgeousInsightMatrixSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BaselineResultsCache.Reset();
	if (IsDevelopmentToolsEnabled()) if (UGorgeousInsightBaselineSettings* Settings = GetMutableDefault<UGorgeousInsightBaselineSettings>())
	{
		Settings->LoadConfig(UGorgeousInsightBaselineSettings::StaticClass(), *GetTestBaselineIniPath());
		for (const TPair<FString, FGorgeousInsightBaselineEntry>& Pair : Settings->Baselines)
		{
			BaselineResultsCache.Add(Pair.Key, Pair.Value.ToResult());
		}
	}


	
	// --- Modular Feature Integration ---
	IModularFeatures::Get().OnModularFeatureRegistered().AddUObject(this, &UGorgeousInsightMatrixSubsystem::OnModularFeatureRegistered);
	IModularFeatures::Get().OnModularFeatureUnregistered().AddUObject(this, &UGorgeousInsightMatrixSubsystem::OnModularFeatureUnregistered);
	
	// Scan for already registered providers
	TArray<IGorgeousInsightMatrixProvider*> ExistingProviders = IModularFeatures::Get().GetModularFeatureImplementations<IGorgeousInsightMatrixProvider>(IGorgeousInsightMatrixProvider::GetFeatureName());
	for (IGorgeousInsightMatrixProvider* Provider : ExistingProviders)
	{
		RegisterProvider(Provider);
	}
	
	if (auto* Service = GetDevelopmentToolsService()) Service->Attach(this);
	if (IsDevelopmentToolsEnabled()) { LoadCachedStats(); LoadPanelState(); }
}

void UGorgeousInsightMatrixSubsystem::Deinitialize()
{
	if (auto* Service = GetDevelopmentToolsService()) Service->Detach();

	IModularFeatures::Get().OnModularFeatureRegistered().RemoveAll(this);
	IModularFeatures::Get().OnModularFeatureUnregistered().RemoveAll(this);

	HideInGamePanel();
	HideDebugPanel();
    if (FunctionalResultProvider.IsValid()) UnregisterProvider(FunctionalResultProvider.Get());
    FunctionalResultProvider.Reset();
	SaveCachedStats();
	SavePanelState();

	Super::Deinitialize();
}

void UGorgeousInsightMatrixSubsystem::OnModularFeatureRegistered(const FName& FeatureName, IModularFeature* Feature)
{
    if (FeatureName == IGorgeousDevelopmentToolsService::GetFeatureName()) static_cast<IGorgeousDevelopmentToolsService*>(Feature)->Attach(this);
	if (FeatureName == IGorgeousInsightMatrixProvider::GetFeatureName())
	{
		RegisterProvider(static_cast<IGorgeousInsightMatrixProvider*>(Feature));
	}
}

void UGorgeousInsightMatrixSubsystem::OnModularFeatureUnregistered(const FName& FeatureName, IModularFeature* Feature)
{
    if (FeatureName == IGorgeousDevelopmentToolsService::GetFeatureName()) static_cast<IGorgeousDevelopmentToolsService*>(Feature)->Detach();
	if (FeatureName == IGorgeousInsightMatrixProvider::GetFeatureName())
	{
		UnregisterProvider(static_cast<IGorgeousInsightMatrixProvider*>(Feature));
	}
}

void UGorgeousInsightMatrixSubsystem::ShowDebugPanel(int32 ZOrder)
{
    if (auto* Service = GetDevelopmentToolsService(); Service && IsDevelopmentToolsEnabled()) Service->ShowDebugPanel(ZOrder);
}

void UGorgeousInsightMatrixSubsystem::HideDebugPanel()
{
    if (auto* Service = GetDevelopmentToolsService(); Service) Service->HideDebugPanel();
}

void UGorgeousInsightMatrixSubsystem::RefreshDebugPanel()
{
    if (auto* Service = GetDevelopmentToolsService(); Service && IsDevelopmentToolsEnabled()) Service->RefreshDebugPanel();
}

void UGorgeousInsightMatrixSubsystem::ShowInGamePanel(int32 ZOrder)
{
    if (auto* Service = GetDevelopmentToolsService(); Service && IsDevelopmentToolsEnabled()) Service->ShowInGamePanel(ZOrder);
}

void UGorgeousInsightMatrixSubsystem::ShowInGamePanelForPlayer(ULocalPlayer* LocalPlayer, int32 ZOrder)
{
    if (auto* Service = GetDevelopmentToolsService(); Service && IsDevelopmentToolsEnabled()) Service->ShowInGamePanelForPlayer(LocalPlayer, ZOrder);
}

void UGorgeousInsightMatrixSubsystem::HideInGamePanel()
{
    if (auto* Service = GetDevelopmentToolsService(); Service) Service->HideInGamePanel();
}

bool UGorgeousInsightMatrixSubsystem::IsDebugPanelOpen() const
{
    auto* Service = GetDevelopmentToolsService();
    return IsDevelopmentToolsEnabled() && Service && Service->IsDebugPanelOpen();
}

bool UGorgeousInsightMatrixSubsystem::IsInGamePanelOpen() const
{
    auto* Service = GetDevelopmentToolsService();
    return IsDevelopmentToolsEnabled() && Service && Service->IsInGamePanelOpen();
}

void UGorgeousInsightMatrixSubsystem::ToggleDebugPanel(int32 ZOrder)
{
    if (IsDebugPanelOpen()) HideDebugPanel(); else ShowDebugPanel(ZOrder);
}

void UGorgeousInsightMatrixSubsystem::ToggleInGamePanel(int32 ZOrder)
{
    if (IsInGamePanelOpen()) HideInGamePanel(); else ShowInGamePanel(ZOrder);
}

bool UGorgeousInsightMatrixSubsystem::RegisterProvider(IGorgeousInsightMatrixProvider* Provider)
{
	if (!Provider)
	{
		return false;
	}

	FScopeLock Lock(&ProviderMutex);
	if (Providers.Contains(Provider))
	{
		return false;
	}

	Providers.Add(Provider);
	OnProviderRegistered.Broadcast(Provider);
	return true;
}

bool UGorgeousInsightMatrixSubsystem::UnregisterProvider(IGorgeousInsightMatrixProvider* Provider)
{
	if (!Provider)
	{
		return false;
	}

	FScopeLock Lock(&ProviderMutex);
	if (!Providers.Remove(Provider))
	{
		return false;
	}

	OnProviderUnregistered.Broadcast(Provider);
	return true;
}

TArray<IGorgeousInsightMatrixProvider*> UGorgeousInsightMatrixSubsystem::GetProviders() const
{
	GORGEOUS_PROFILE_SCOPE(GIM_GetProviders);
	FScopeLock Lock(&ProviderMutex);
	TArray<IGorgeousInsightMatrixProvider*> Result;
	Result.Reserve(Providers.Num());
	for (IGorgeousInsightMatrixProvider* Provider : Providers)
	{
		Result.Add(Provider);
	}
	SET_DWORD_STAT(STAT_GIM_Providers, Result.Num());
	GORGEOUS_CSV_CUSTOM_STAT_SET(InsightProviders, Result.Num());
	return Result;
}

IGorgeousInsightMatrixProvider* UGorgeousInsightMatrixSubsystem::FindProvider(const FName ProviderName) const
{
	FScopeLock Lock(&ProviderMutex);
	for (IGorgeousInsightMatrixProvider* Provider : Providers)
	{
		if (Provider && Provider->GetProviderName() == ProviderName)
		{
			return Provider;
		}
	}
	return nullptr;
}

void UGorgeousInsightMatrixSubsystem::GatherAllStats(const FGorgeousInsightGatherContext& Context, TArray<FGorgeousInsightStat>& OutStats) const
{
	GORGEOUS_PROFILE_SCOPE(GIM_GatherAllStats);
	FScopeLock Lock(&ProviderMutex);
	for (IGorgeousInsightMatrixProvider* Provider : Providers)
	{
		if (!Provider)
		{
			continue;
		}
		TArray<FGorgeousInsightStat> ProviderStats;
		
		if (Context.Mode == EGorgeousInsightContextMode::Baseline)
		{
			if (const TArray<FGorgeousInsightStat>* Baseline = ProviderBaselineStats.Find(Provider->GetProviderName()))
			{
				ProviderStats = *Baseline;
			}
		}
		else
		{
			Provider->GatherStats(Context, ProviderStats);
		}

		if (!ProviderStats.IsEmpty())
		{
			OutStats.Append(ProviderStats);
			CacheProviderStats(Provider->GetProviderName(), ProviderStats);
		}
		else
		{
			FScopeLock CacheLock(&StatsCacheMutex);
			if (const TArray<FGorgeousInsightStat>* Cached = CachedProviderStats.Find(Provider->GetProviderName()))
			{
				OutStats.Append(*Cached);
			}
		}
	}
	SET_DWORD_STAT(STAT_GIM_Stats, OutStats.Num());
	GORGEOUS_CSV_CUSTOM_STAT_SET(InsightStats, OutStats.Num());
}

void UGorgeousInsightMatrixSubsystem::GatherProviderStats(const FGorgeousInsightGatherContext& Context, const FName ProviderName, TArray<FGorgeousInsightStat>& OutStats) const
{
	GORGEOUS_PROFILE_SCOPE(GIM_GatherProviderStats);
	TArray<FGorgeousInsightStat> ProviderStats;
	
	if (Context.Mode == EGorgeousInsightContextMode::Baseline)
	{
		if (const TArray<FGorgeousInsightStat>* Baseline = ProviderBaselineStats.Find(ProviderName))
		{
			ProviderStats = *Baseline;
		}
		else
		{
			ProviderStats = ReadProviderBaselineStats(ProviderName);
			ProviderBaselineStats.Add(ProviderName, ProviderStats);
		}
	}
	else if (Context.Mode == EGorgeousInsightContextMode::Editor)
	{
		FScopeLock CacheLock(&StatsCacheMutex);
		if (const TArray<FGorgeousInsightStat>* Cached = CachedProviderStats.Find(ProviderName))
		{
			ProviderStats = *Cached;
		}
	}
	else if (IGorgeousInsightMatrixProvider* Provider = FindProvider(ProviderName))
	{
		Provider->GatherStats(Context, ProviderStats);
	}

	if (!ProviderStats.IsEmpty())
	{
		OutStats = ProviderStats;
		if (Context.Mode == EGorgeousInsightContextMode::Live)
		{
			CacheProviderStats(ProviderName, ProviderStats);
		}
	}
	else
	{
		FScopeLock CacheLock(&StatsCacheMutex);
		if (const TArray<FGorgeousInsightStat>* Cached = CachedProviderStats.Find(ProviderName))
		{
			OutStats = *Cached;
		}
	}
	SET_DWORD_STAT(STAT_GIM_Stats, OutStats.Num());
	GORGEOUS_CSV_CUSTOM_STAT_SET(InsightStats, OutStats.Num());
}

void UGorgeousInsightMatrixSubsystem::GatherProviderCharts(FName ProviderName, TArray<FGorgeousInsightChartDefinition>& OutCharts) const
{
	FGorgeousInsightGatherContext Context;
	Context.Mode = EGorgeousInsightContextMode::Live;
	GatherProviderCharts(ProviderName, Context, OutCharts);
}

void UGorgeousInsightMatrixSubsystem::GatherProviderCharts(FName ProviderName, const FGorgeousInsightGatherContext& Context, TArray<FGorgeousInsightChartDefinition>& OutCharts) const
{
	OutCharts.Reset();
	if (Context.Mode == EGorgeousInsightContextMode::Baseline)
	{
		LoadChartBaselines();
		FScopeLock Lock(&StatsCacheMutex);
		if (const auto* Found = ProviderBaselineCharts.Find(ProviderName)) OutCharts = *Found;
		return;
	}
	if (Context.Mode == EGorgeousInsightContextMode::Editor)
	{
		const_cast<UGorgeousInsightMatrixSubsystem*>(this)->LoadCachedStats();
		FScopeLock Lock(&StatsCacheMutex);
		if (const auto* Found = CachedProviderCharts.Find(ProviderName)) OutCharts = *Found;
		return;
	}
	if (IGorgeousInsightMatrixProvider* Provider = FindProvider(ProviderName))
	{
		Provider->GatherCharts(Context, OutCharts);
		CacheProviderCharts(ProviderName, OutCharts);
	}
}

void UGorgeousInsightMatrixSubsystem::CacheProviderCharts(FName ProviderName, const TArray<FGorgeousInsightChartDefinition>& Charts) const
{
	FScopeLock Lock(&StatsCacheMutex);
	const FDateTime Now = FDateTime::UtcNow();
	if (EditorHistory.IsEmpty() || (Now - EditorHistory.Last().Timestamp).GetTotalSeconds() >= 5.0)
	{
		FGorgeousInsightEditorSnapshot Snapshot;
		Snapshot.Timestamp = Now;
		Snapshot.ProviderStats = CachedProviderStats;
		Snapshot.ProviderCharts = CachedProviderCharts;
		EditorHistory.Add(MoveTemp(Snapshot));
		if (EditorHistory.Num() > 100) EditorHistory.RemoveAt(0, EditorHistory.Num() - 100);
	}
	EditorHistory.Last().ProviderStats.FindOrAdd(ProviderName);
	EditorHistory.Last().ProviderCharts.FindOrAdd(ProviderName) = Charts;
	CachedProviderCharts.FindOrAdd(ProviderName) = Charts;
	ActiveHistoryIndex = EditorHistory.Num() - 1;
	bStatsCacheLoaded = true;
	bStatsCacheDirty = true;
}

void UGorgeousInsightMatrixSubsystem::LoadChartBaselines() const
{
    if (!IsDevelopmentToolsEnabled()) return;

	FScopeLock Lock(&StatsCacheMutex);
	if (bChartBaselinesLoaded) return;
	bChartBaselinesLoaded = true;
	FString Contents;
	if (!FFileHelper::LoadFileToString(Contents, *GetInsightSavedPath(TEXT("ChartBaselines.json")))) return;
	TSharedPtr<FJsonObject> Root;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Contents), Root) || !Root.IsValid()) return;
	const TArray<TSharedPtr<FJsonValue>>* SerializedProviders = nullptr;
	if (!Root->TryGetArrayField(TEXT("providers"), SerializedProviders)) return;
	for (const auto& Value : *SerializedProviders)
	{
		if (!Value.IsValid() || Value->Type != EJson::Object) continue;
		const TSharedPtr<FJsonObject> Provider = Value->AsObject();
		const TArray<TSharedPtr<FJsonValue>>* Charts = nullptr;
		FString Name;
		if (Provider->TryGetStringField(TEXT("provider"), Name) && Provider->TryGetArrayField(TEXT("charts"), Charts))
			ProviderBaselineCharts.Add(FName(*Name), GorgeousInsightCharts::FromJsonArray(*Charts));
	}
}

void UGorgeousInsightMatrixSubsystem::SaveChartBaselines() const
{
    if (!IsDevelopmentToolsEnabled()) return;

	FScopeLock Lock(&StatsCacheMutex);
	TArray<TSharedPtr<FJsonValue>> SerializedProviders;
	for (const auto& Pair : ProviderBaselineCharts)
	{
		TSharedRef<FJsonObject> Provider = MakeShared<FJsonObject>();
		Provider->SetStringField(TEXT("provider"), Pair.Key.ToString());
		Provider->SetArrayField(TEXT("charts"), GorgeousInsightCharts::ToJsonArray(Pair.Value));
		SerializedProviders.Add(MakeShared<FJsonValueObject>(Provider));
	}
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetArrayField(TEXT("providers"), SerializedProviders);
	FString Output;
	FJsonSerializer::Serialize(Root, TJsonWriterFactory<>::Create(&Output));
	FFileHelper::SaveStringToFile(Output, *GetInsightSavedPath(TEXT("ChartBaselines.json")));
}

void UGorgeousInsightMatrixSubsystem::AddReferencedObjects(UObject* Object, FReferenceCollector& Collector)
{
	UGorgeousInsightMatrixSubsystem* Self = CastChecked<UGorgeousInsightMatrixSubsystem>(Object);
	auto Visit = [&Collector](auto& Providers)
	{
		for (auto& Pair : Providers)
			for (FGorgeousInsightChartDefinition& Chart : Pair.Value)
				Chart.InstancedPayload.AddStructReferencedObjects(Collector);
	};
	Visit(Self->CachedProviderCharts);
	Visit(Self->ProviderBaselineCharts);
	for (auto& Snapshot : Self->EditorHistory) Visit(Snapshot.ProviderCharts);
	Super::AddReferencedObjects(Object, Collector);
}

void UGorgeousInsightMatrixSubsystem::LoadCachedStats()
{
    if (!IsDevelopmentToolsEnabled()) return;

	FScopeLock CacheLock(&StatsCacheMutex);
	if (bStatsCacheLoaded)
	{
		return;
	}

	const FString Directory = FPaths::ProjectSavedDir() / TEXT("GorgeousThings");
	const FString FilePath = Directory / TEXT("EditorHistory.json");
	
	EditorHistory.Reset();
	ActiveHistoryIndex = -1;
	CachedProviderStats.Reset();
	CachedProviderCharts.Reset();

	FString Contents;
	if (FFileHelper::LoadFileToString(Contents, *FilePath))
	{
		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Contents);
		if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
		{
			const TArray<TSharedPtr<FJsonValue>>* HistoryArray = nullptr;
			if (Root->TryGetArrayField(TEXT("history"), HistoryArray) && HistoryArray)
			{
				for (const TSharedPtr<FJsonValue>& SnapValue : *HistoryArray)
				{
					const TSharedPtr<FJsonObject> SnapObj = SnapValue.IsValid() ? SnapValue->AsObject() : nullptr;
					if (!SnapObj.IsValid()) continue;
					
					FGorgeousInsightEditorSnapshot Snapshot;
					FString TimeStr;
					if (SnapObj->TryGetStringField(TEXT("timestamp"), TimeStr))
					{
						FDateTime::ParseIso8601(*TimeStr, Snapshot.Timestamp);
					}
					else
					{
						Snapshot.Timestamp = FDateTime::UtcNow();
					}

					const TArray<TSharedPtr<FJsonValue>>* ProvidersArray = nullptr;
					if (SnapObj->TryGetArrayField(TEXT("providers"), ProvidersArray) && ProvidersArray)
					{
						for (const TSharedPtr<FJsonValue>& ProviderValue : *ProvidersArray)
						{
							const TSharedPtr<FJsonObject> ProviderObj = ProviderValue.IsValid() ? ProviderValue->AsObject() : nullptr;
							if (!ProviderObj.IsValid()) continue;
							
							const FString ProviderName = ProviderObj->GetStringField(TEXT("provider"));
							const TArray<TSharedPtr<FJsonValue>>* StatsArray = nullptr;
							if (!ProviderObj->TryGetArrayField(TEXT("stats"), StatsArray) || !StatsArray) continue;

							TArray<FGorgeousInsightStat> Stats;
							for (const TSharedPtr<FJsonValue>& StatValue : *StatsArray)
							{
								const TSharedPtr<FJsonObject> StatObj = StatValue.IsValid() ? StatValue->AsObject() : nullptr;
								if (!StatObj.IsValid()) continue;
								
								FGorgeousInsightStat Stat;
								Stat.Id = FName(*StatObj->GetStringField(TEXT("id")));
								Stat.DisplayName = FText::FromString(StatObj->GetStringField(TEXT("displayName")));
								Stat.Description = FText::FromString(StatObj->GetStringField(TEXT("description")));
								Stat.Category = FName(*StatObj->GetStringField(TEXT("category")));
								Stat.ValueType = static_cast<EGorgeousInsightStatValueType>(StatObj->GetIntegerField(TEXT("valueType")));
								Stat.NumericValue = StatObj->GetNumberField(TEXT("numericValue"));
								Stat.TextValue = FText::FromString(StatObj->GetStringField(TEXT("textValue")));
								Stat.Unit = FText::FromString(StatObj->GetStringField(TEXT("unit")));
								Stats.Add(Stat);
							}
							Snapshot.ProviderStats.Add(FName(*ProviderName), Stats);
							const TArray<TSharedPtr<FJsonValue>>* ChartsArray = nullptr;
							if (ProviderObj->TryGetArrayField(TEXT("charts"), ChartsArray))
								Snapshot.ProviderCharts.Add(FName(*ProviderName), GorgeousInsightCharts::FromJsonArray(*ChartsArray));
						}
					}
					EditorHistory.Add(Snapshot);
				}
			}
		}
	}
	
	if (EditorHistory.Num() > 0)
	{
		ActiveHistoryIndex = EditorHistory.Num() - 1;
		CachedProviderStats = EditorHistory.Last().ProviderStats;
		CachedProviderCharts = EditorHistory.Last().ProviderCharts;
	}

	bStatsCacheLoaded = true;
}


void UGorgeousInsightMatrixSubsystem::ApplyHistorySnapshot(int32 Index)
{
	FScopeLock CacheLock(&StatsCacheMutex);
	if (EditorHistory.IsValidIndex(Index))
	{
		ActiveHistoryIndex = Index;
		CachedProviderStats = EditorHistory[Index].ProviderStats;
		CachedProviderCharts = EditorHistory[Index].ProviderCharts;
	}
}
void UGorgeousInsightMatrixSubsystem::SaveCachedStats() const
{
    if (!IsDevelopmentToolsEnabled()) return;

	FScopeLock CacheLock(&StatsCacheMutex);
	if (!bStatsCacheLoaded || !bStatsCacheDirty)
	{
		return;
	}
	
	TArray<TSharedPtr<FJsonValue>> HistoryJsonArray;
	for (const FGorgeousInsightEditorSnapshot& Snapshot : EditorHistory)
	{
		TSharedRef<FJsonObject> SnapObj = MakeShared<FJsonObject>();
		SnapObj->SetStringField(TEXT("timestamp"), Snapshot.Timestamp.ToIso8601());
		
		TArray<TSharedPtr<FJsonValue>> ProviderArray;
		for (const TPair<FName, TArray<FGorgeousInsightStat>>& Pair : Snapshot.ProviderStats)
		{
			TArray<TSharedPtr<FJsonValue>> StatArray;
			for (const FGorgeousInsightStat& Stat : Pair.Value)
			{
				TSharedRef<FJsonObject> StatObj = MakeShared<FJsonObject>();
				StatObj->SetStringField(TEXT("id"), Stat.Id.ToString());
				StatObj->SetStringField(TEXT("displayName"), Stat.DisplayName.ToString());
				StatObj->SetStringField(TEXT("description"), Stat.Description.ToString());
				StatObj->SetStringField(TEXT("category"), Stat.Category.ToString());
				StatObj->SetNumberField(TEXT("valueType"), static_cast<int32>(Stat.ValueType));
				StatObj->SetNumberField(TEXT("numericValue"), Stat.NumericValue);
				StatObj->SetStringField(TEXT("textValue"), Stat.TextValue.ToString());
				StatObj->SetStringField(TEXT("unit"), Stat.Unit.ToString());
				StatArray.Add(MakeShared<FJsonValueObject>(StatObj));
			}
			TSharedRef<FJsonObject> ProviderObj = MakeShared<FJsonObject>();
			ProviderObj->SetStringField(TEXT("provider"), Pair.Key.ToString());
			ProviderObj->SetArrayField(TEXT("stats"), StatArray);
			if (const auto* Charts = Snapshot.ProviderCharts.Find(Pair.Key)) ProviderObj->SetArrayField(TEXT("charts"), GorgeousInsightCharts::ToJsonArray(*Charts));
			ProviderArray.Add(MakeShared<FJsonValueObject>(ProviderObj));
		}
		
		SnapObj->SetArrayField(TEXT("providers"), ProviderArray);
		HistoryJsonArray.Add(MakeShared<FJsonValueObject>(SnapObj));
	}

	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetArrayField(TEXT("history"), HistoryJsonArray);

	FString Output;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
	if (!FJsonSerializer::Serialize(Root, Writer))
	{
		return;
	}

	const FString Directory = FPaths::ProjectSavedDir() / TEXT("GorgeousThings");
	IFileManager::Get().MakeDirectory(*Directory, true);
	const FString FilePath = Directory / TEXT("EditorHistory.json");
	if (FFileHelper::SaveStringToFile(Output, *FilePath))
	{
		bStatsCacheDirty = false;
	}
}

void UGorgeousInsightMatrixSubsystem::CacheProviderStats(FName ProviderName, const TArray<FGorgeousInsightStat>& Stats) const
{
	FScopeLock CacheLock(&StatsCacheMutex);
	
	// Create a new snapshot if there isn't one from the last few seconds
	const FDateTime Now = FDateTime::UtcNow();
	
	// We want to avoid creating a new snapshot for every single provider that updates during a frame/tick.
	// We'll bundle them if they happen within a 5 second window of the active snapshot.
	bool bCreateNewSnapshot = true;
	if (EditorHistory.Num() > 0)
	{
		FGorgeousInsightEditorSnapshot& LastSnap = EditorHistory.Last();
		if ((Now - LastSnap.Timestamp).GetTotalSeconds() < 5.0)
		{
			LastSnap.ProviderStats.FindOrAdd(ProviderName) = Stats;
			bCreateNewSnapshot = false;
		}
	}
	
	if (bCreateNewSnapshot)
	{
		FGorgeousInsightEditorSnapshot NewSnap;
		NewSnap.Timestamp = Now;
		NewSnap.ProviderStats = CachedProviderStats;
		NewSnap.ProviderCharts = CachedProviderCharts;
		NewSnap.ProviderStats.Add(ProviderName, Stats);
		EditorHistory.Add(NewSnap);
		// Keep history reasonably sized
		if (EditorHistory.Num() > 100)
		{
			EditorHistory.RemoveAt(0, EditorHistory.Num() - 100);
		}
	}
	
	ActiveHistoryIndex = EditorHistory.Num() - 1;
	CachedProviderStats.FindOrAdd(ProviderName) = Stats;
	
	bStatsCacheLoaded = true;
	bStatsCacheDirty = true;
}

void UGorgeousInsightMatrixSubsystem::GatherAllActions(TArray<FGorgeousInsightAction>& OutActions) const
{
	GORGEOUS_PROFILE_SCOPE(GIM_GatherAllActions);
	FScopeLock Lock(&ProviderMutex);
	for (IGorgeousInsightMatrixProvider* Provider : Providers)
	{
		if (!Provider)
		{
			continue;
		}
		Provider->GetActions(OutActions);
	}
	SET_DWORD_STAT(STAT_GIM_Actions, OutActions.Num());
	GORGEOUS_CSV_CUSTOM_STAT_SET(InsightActions, OutActions.Num());
}

void UGorgeousInsightMatrixSubsystem::GatherProviderActions(const FName ProviderName, TArray<FGorgeousInsightAction>& OutActions) const
{
	GORGEOUS_PROFILE_SCOPE(GIM_GatherProviderActions);
	if (IGorgeousInsightMatrixProvider* Provider = FindProvider(ProviderName))
	{
		Provider->GetActions(OutActions);
	}
	SET_DWORD_STAT(STAT_GIM_Actions, OutActions.Num());
	GORGEOUS_CSV_CUSTOM_STAT_SET(InsightActions, OutActions.Num());
}

void UGorgeousInsightMatrixSubsystem::GatherAllTests(TArray<FGorgeousInsightTest>& OutTests) const
{
	GORGEOUS_PROFILE_SCOPE(GIM_GatherAllTests);
	FScopeLock Lock(&ProviderMutex);
	for (IGorgeousInsightMatrixProvider* Provider : Providers)
	{
		if (!Provider)
		{
			continue;
		}
		Provider->GetTests(OutTests);
	}
	SET_DWORD_STAT(STAT_GIM_Tests, OutTests.Num());
	GORGEOUS_CSV_CUSTOM_STAT_SET(InsightTests, OutTests.Num());
}

void UGorgeousInsightMatrixSubsystem::GatherProviderTests(const FName ProviderName, TArray<FGorgeousInsightTest>& OutTests) const
{
	GORGEOUS_PROFILE_SCOPE(GIM_GatherProviderTests);
	if (IGorgeousInsightMatrixProvider* Provider = FindProvider(ProviderName))
	{
		Provider->GetTests(OutTests);
	}
	SET_DWORD_STAT(STAT_GIM_Tests, OutTests.Num());
	GORGEOUS_CSV_CUSTOM_STAT_SET(InsightTests, OutTests.Num());
}

bool UGorgeousInsightMatrixSubsystem::ExecuteAction(const FName ProviderName, const FName ActionId, const FGorgeousInsightActionContext& Context)
{
    if (!IsDevelopmentToolsEnabled()) return false;

	GORGEOUS_PROFILE_SCOPE(GIM_ExecuteAction);
	if (IGorgeousInsightMatrixProvider* Provider = FindProvider(ProviderName))
	{
		FGorgeousInsightActionContext ResolvedContext = Context;
		ResolvedContext.WorldContextObject = ResolveWorldContextObject(Context.WorldContextObject);
		const double StartSeconds = FPlatformTime::Seconds();
		const FGorgeousInsightActionResult Outcome = Provider->ExecuteActionWithResult(ActionId, ResolvedContext);
		const double DurationSeconds = FPlatformTime::Seconds() - StartSeconds;
		FGorgeousInsightLastActionStats& ActionStats = LastActionStats.FindOrAdd(ProviderName).FindOrAdd(ActionId);
		ActionStats.bHasRun = true;
		ActionStats.Timestamp = FDateTime::UtcNow();
		ActionStats.ProviderName = ProviderName;
		ActionStats.ActionId = ActionId;
		ActionStats.DurationSeconds = DurationSeconds;
		ActionStats.bSucceeded = Outcome.Status == EGorgeousInsightActionStatus::Succeeded;
        ActionStats.bHasExecutionResult = Outcome.Status != EGorgeousInsightActionStatus::Dispatched;
        ActionStats.ResultMessage = Outcome.Message;
		INC_DWORD_STAT(STAT_GIM_ActionsExecuted);
		GORGEOUS_CSV_CUSTOM_STAT_ACCUMULATE(InsightActionsExecuted, 1);
		GORGEOUS_TRACE_BOOKMARK(TEXT("InsightAction %s.%s"), *ProviderName.ToString(), *ActionId.ToString());
		SavePanelState(); // Save after action execution
		return Outcome.Status != EGorgeousInsightActionStatus::Failed;
	}
	return false;
}

bool UGorgeousInsightMatrixSubsystem::GetLastActionStats(const FName ProviderName, const FName ActionId, FGorgeousInsightLastActionStats& OutStats) const
{
	if (const TMap<FName, FGorgeousInsightLastActionStats>* ProviderStats = LastActionStats.Find(ProviderName))
	{
		if (const FGorgeousInsightLastActionStats* Found = ProviderStats->Find(ActionId))
		{
			OutStats = *Found;
			return true;
		}
	}
	return false;
}

FGorgeousInsightTestResult UGorgeousInsightMatrixSubsystem::RunTest(const FName ProviderName, const FName TestId, const FGorgeousInsightTestContext& Context)
{
    if (!IsDevelopmentToolsEnabled()) { FGorgeousInsightTestResult Disabled; Disabled.bSuccess = false; Disabled.Errors.Add(TEXT("Development Suite is absent, disabled, or not approved for this target.")); return Disabled; }

	GORGEOUS_PROFILE_SCOPE(GIM_RunTest);
	if (IGorgeousInsightMatrixProvider* Provider = FindProvider(ProviderName))
	{
		FGorgeousInsightTestContext ResolvedContext = Context;
		ResolvedContext.WorldContextObject = ResolveWorldContextObject(Context.WorldContextObject);
		const double StartSeconds = FPlatformTime::Seconds();
		FGorgeousInsightTestResult Result = Provider->RunTest(TestId, ResolvedContext);
		const double DurationSeconds = FPlatformTime::Seconds() - StartSeconds;
		UpdateLastRunStats(ProviderName, TestId, Result, DurationSeconds);
		LastTestResults.FindOrAdd(ProviderName).Add(TestId, Result);
		INC_DWORD_STAT(STAT_GIM_TestsRun);
		GORGEOUS_CSV_CUSTOM_STAT_ACCUMULATE(InsightTestsRun, 1);
		GORGEOUS_TRACE_BOOKMARK(TEXT("InsightTest %s.%s"), *ProviderName.ToString(), *TestId.ToString());
		SavePanelState(); // Save after test execution
		return Result;
	}
	FGorgeousInsightTestResult Missing;
    Missing.bSuccess = false;
    Missing.Errors.Add(FString::Printf(TEXT("Insight provider %s was not registered."), *ProviderName.ToString()));
    return Missing;
}

void UGorgeousInsightMatrixSubsystem::RecordFunctionalTestResult(FName TestId, const FText& DisplayName, const FGorgeousInsightTestResult& Result, double DurationSeconds)
{
    const bool bNewProvider = !FunctionalResultProvider.IsValid();
    if (bNewProvider) FunctionalResultProvider = MakeShared<FGorgeousFunctionalResultProvider>();
    FunctionalResultProvider->Results.Add(TestId, Result);
    FunctionalResultProvider->DisplayNames.Add(TestId, DisplayName);
    const FName ProviderName = FunctionalResultProvider->GetProviderName();
    LastTestResults.FindOrAdd(ProviderName).Add(TestId, Result);
    UpdateLastRunStats(ProviderName, TestId, Result, DurationSeconds);
    ++ExternalTestResultRevision;
    if (bNewProvider) RegisterProvider(FunctionalResultProvider.Get());
    SavePanelState();
}

const FGorgeousInsightTestResult* UGorgeousInsightMatrixSubsystem::GetLastTestResult(FName ProviderName, FName TestId) const
{
    const auto* Results = LastTestResults.Find(ProviderName);
    return Results ? Results->Find(TestId) : nullptr;
}

void UGorgeousInsightMatrixSubsystem::UpdateLastRunStats(const FName ProviderName, const FName TestId, const FGorgeousInsightTestResult& Result, const double DurationSeconds)
{
	LastRunStats.bHasRun = true;
	LastRunStats.Timestamp = FDateTime::UtcNow();
	LastRunStats.ProviderName = ProviderName;
	LastRunStats.TestId = TestId;
	LastRunStats.DurationSeconds = DurationSeconds;
	LastRunStats.ErrorCount = Result.Errors.Num();
	LastRunStats.WarningCount = Result.Warnings.Num();
	LastRunStats.NoteCount = Result.Notes.Num();

	const double OvStart = FPlatformTime::Seconds();
	LastRunStats.ObjectVariablesRegistered = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry().Num();
	LastRunStats.ObjectVariableOverheadMs = (FPlatformTime::Seconds() - OvStart) * 1000.0;
}

const FGorgeousInsightTestResult* UGorgeousInsightMatrixSubsystem::GetBaselineResult(FName ProviderName, FName TestId) const
{
	const FString Key = MakeBaselineKey(ProviderName, TestId);
	return BaselineResultsCache.Find(Key);
}

bool UGorgeousInsightMatrixSubsystem::SetBaselineFromLastResult(FName ProviderName, FName TestId)
{
    if (!IsDevelopmentToolsEnabled()) return false;

	const TMap<FName, FGorgeousInsightTestResult>* ProviderMap = LastTestResults.Find(ProviderName);
	if (!ProviderMap)
	{
		return false;
	}

	const FGorgeousInsightTestResult* LastResult = ProviderMap->Find(TestId);
	if (!LastResult)
	{
		return false;
	}

	UGorgeousInsightBaselineSettings* Settings = GetMutableDefault<UGorgeousInsightBaselineSettings>();
	if (!Settings)
	{
		return false;
	}

	const FString Key = MakeBaselineKey(ProviderName, TestId);
	FGorgeousInsightBaselineEntry Entry = FGorgeousInsightBaselineEntry::FromResult(*LastResult);
	Settings->Baselines.Add(Key, Entry);
	
	// Write to specific INI file
	const FString IniPath = GetTestBaselineIniPath();
	FString SectionName = TEXT("BaselineTest_") + Key;
	
	// We use standard Config array formats for simplicity, but saving the full Baseline entry 
	// via standard UE config paths is complex.
	// Actually, UObject::SaveConfig(CPF_Config, *IniPath) saves ALL properties on the object!
	// So we can just use Settings->SaveConfig(CPF_Config, *IniPath);
	Settings->SaveConfig(CPF_Config, *IniPath);
	BaselineResultsCache.Add(Key, *LastResult);
	return true;
}

bool UGorgeousInsightMatrixSubsystem::ExportStatsToCSV(const FString& AbsoluteFilePath, FName ProviderName, const FGorgeousInsightGatherContext& Context) const
{
    if (!IsDevelopmentToolsEnabled()) return false;

	GORGEOUS_PROFILE_SCOPE(GIM_ExportStatsCsv);
	if (AbsoluteFilePath.IsEmpty())
	{
		return false;
	}
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(AbsoluteFilePath), true);

	TArray<FGorgeousInsightStat> Stats;
	if (ProviderName.IsNone())
	{
		GatherAllStats(Context, Stats);
	}
	else
	{
		GatherProviderStats(Context, ProviderName, Stats);
	}

	auto QuoteCsv = [](FString Value) { Value.ReplaceInline(TEXT("\""), TEXT("\"\"")); return TEXT("\"") + Value + TEXT("\""); };
	FString Csv = TEXT("Id,DisplayName,Category,ValueType,NumericValue,TextValue,Unit,RecordType,Provider,ChartJson\n");
	for (const FGorgeousInsightStat& Stat : Stats)
	{
		Csv += FString::Printf(TEXT("%s,%s,%s,%d,%.6f,%s,%s,Stat,,\n"),
			*QuoteCsv(Stat.Id.ToString()),
			*QuoteCsv(Stat.DisplayName.ToString()),
			*QuoteCsv(Stat.Category.ToString()),
			static_cast<int32>(Stat.ValueType),
			Stat.NumericValue,
			*QuoteCsv(Stat.TextValue.ToString()),
			*QuoteCsv(Stat.Unit.ToString()));
	}

	for (IGorgeousInsightMatrixProvider* Provider : GetProviders())
	{
		if (!Provider || (!ProviderName.IsNone() && Provider->GetProviderName() != ProviderName)) continue;
		TArray<FGorgeousInsightChartDefinition> Charts;
		GatherProviderCharts(Provider->GetProviderName(), Context, Charts);
		for (const auto& Chart : Charts)
		{
			FString Json;
			FJsonSerializer::Serialize(GorgeousInsightCharts::ToJson(Chart), TJsonWriterFactory<>::Create(&Json));
			Csv += FString::Printf(TEXT("%s,%s,,,,,,Chart,%s,%s\n"), *QuoteCsv(Chart.Id.ToString()), *QuoteCsv(Chart.Title.ToString()), *QuoteCsv(Provider->GetProviderName().ToString()), *QuoteCsv(Json));
		}
	}

	const bool bSaved = FFileHelper::SaveStringToFile(Csv, *AbsoluteFilePath);
	if (bSaved)
	{
		INC_DWORD_STAT(STAT_GIM_ExportsCsv);
		GORGEOUS_CSV_CUSTOM_STAT_ACCUMULATE(InsightExportsCsv, 1);
		GORGEOUS_TRACE_BOOKMARK(TEXT("InsightExportCSV %s"), *AbsoluteFilePath);
	}
	return bSaved;
}

bool UGorgeousInsightMatrixSubsystem::ExportStatsToJson(const FString& AbsoluteFilePath, FName ProviderName, const FGorgeousInsightGatherContext& Context) const
{
    if (!IsDevelopmentToolsEnabled()) return false;

	GORGEOUS_PROFILE_SCOPE(GIM_ExportStatsJson);
	if (AbsoluteFilePath.IsEmpty())
	{
		return false;
	}
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(AbsoluteFilePath), true);

	TArray<FGorgeousInsightStat> Stats;
	if (ProviderName.IsNone())
	{
		GatherAllStats(Context, Stats);
	}
	else
	{
		GatherProviderStats(Context, ProviderName, Stats);
	}

	TArray<TSharedPtr<FJsonValue>> StatArray;
	StatArray.Reserve(Stats.Num());
	for (const FGorgeousInsightStat& Stat : Stats)
	{
		TSharedRef<FJsonObject> StatObj = MakeShared<FJsonObject>();
		StatObj->SetStringField(TEXT("id"), Stat.Id.ToString());
		StatObj->SetStringField(TEXT("displayName"), Stat.DisplayName.ToString());
		StatObj->SetStringField(TEXT("category"), Stat.Category.ToString());
		StatObj->SetNumberField(TEXT("valueType"), static_cast<int32>(Stat.ValueType));
		StatObj->SetNumberField(TEXT("numericValue"), Stat.NumericValue);
		StatObj->SetStringField(TEXT("textValue"), Stat.TextValue.ToString());
		StatObj->SetStringField(TEXT("unit"), Stat.Unit.ToString());
		StatArray.Add(MakeShared<FJsonValueObject>(StatObj));
	}

	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetArrayField(TEXT("stats"), StatArray);
	TArray<TSharedPtr<FJsonValue>> ChartsJson;
	for (IGorgeousInsightMatrixProvider* Provider : GetProviders())
	{
		if (!Provider || (!ProviderName.IsNone() && Provider->GetProviderName() != ProviderName)) continue;
		TArray<FGorgeousInsightChartDefinition> Charts;
		GatherProviderCharts(Provider->GetProviderName(), Context, Charts);
		for (const auto& Chart : Charts)
		{
			TSharedRef<FJsonObject> ChartJson = GorgeousInsightCharts::ToJson(Chart);
			ChartJson->SetStringField(TEXT("provider"), Provider->GetProviderName().ToString());
			ChartsJson.Add(MakeShared<FJsonValueObject>(ChartJson));
		}
	}
	Root->SetArrayField(TEXT("charts"), ChartsJson);
	if (!ProviderName.IsNone())
	{
		Root->SetStringField(TEXT("provider"), ProviderName.ToString());
	}

	FString Output;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
	if (!FJsonSerializer::Serialize(Root, Writer))
	{
		return false;
	}

	const bool bSaved = FFileHelper::SaveStringToFile(Output, *AbsoluteFilePath);
	if (bSaved)
	{
		INC_DWORD_STAT(STAT_GIM_ExportsJson);
		GORGEOUS_CSV_CUSTOM_STAT_ACCUMULATE(InsightExportsJson, 1);
		GORGEOUS_TRACE_BOOKMARK(TEXT("InsightExportJSON %s"), *AbsoluteFilePath);
	}
	return bSaved;
}

bool UGorgeousInsightMatrixSubsystem::EnqueueScenario(FName ScenarioName, const FString& Parameters, int32 VariantIndex, UObject* WorldContextObject)
{
    if (!IsDevelopmentToolsEnabled()) return false;

	if (ScenarioName.IsNone())
	{
		return false;
	}

	FGorgeousInsightQueuedScenario Entry;
	Entry.ScenarioName = ScenarioName;
	Entry.Parameters = Parameters;
	Entry.VariantIndex = VariantIndex;
	Entry.WorldContextObject = WorldContextObject;

	{
		FScopeLock QueueLock(&QueueMutex);
		ScenarioQueue.Add(MoveTemp(Entry));
	}

	return true;
}

FGorgeousInsightScenarioResult UGorgeousInsightMatrixSubsystem::RunScenarioByName(FName ScenarioName, const FString& Parameters, int32 VariantIndex, UObject* WorldContextObject)
{
    if (!IsDevelopmentToolsEnabled()) { FGorgeousInsightScenarioResult Disabled; Disabled.bSuccess = false; Disabled.Errors.Add(TEXT("Development Suite is absent, disabled, or not approved for this target.")); return Disabled; }

	FGorgeousInsightScenarioResult Result;
	(void)WorldContextObject;

	if (ScenarioName.IsNone())
	{
		Result.bSuccess = false;
		Result.AddError(TEXT("Scenario name is empty"));
		return Result;
	}

	FGorgeousInsightScenarioDescriptor Descriptor;
	bool bFoundDescriptor = false;
	{
		const TArray<FGorgeousInsightScenarioDescriptor> Scenarios = FGorgeousInsightTestMatrix::GetRegisteredScenarios();
		for (const FGorgeousInsightScenarioDescriptor& Candidate : Scenarios)
		{
			if (Candidate.ScenarioName == ScenarioName)
			{
				Descriptor = Candidate;
				bFoundDescriptor = true;
				break;
			}
		}
	}

	if (!bFoundDescriptor)
	{
		Result.bSuccess = false;
		Result.AddError(FString::Printf(TEXT("Scenario '%s' not found"), *ScenarioName.ToString()));
		return Result;
	}

	FGorgeousInsightMatrixRequest Request = FGorgeousInsightMatrixRequest::FromParameters(Parameters);
	FGorgeousInsightScenarioResult ScenarioResult;

	if (Request.bEnableSharedHarness)
	{
		if (!FGorgeousInsightHarness::StartHarness(Request))
		{
			ScenarioResult.bSuccess = false;
			ScenarioResult.AddError(TEXT("Insight harness failed to start"));
			return ScenarioResult;
		}
	}

#if WITH_DEV_AUTOMATION_TESTS
	FGorgeousInsightRuntimeAutomationTest RuntimeTest;
	FGorgeousInsightScenarioContext ScenarioContext(Request, Parameters, VariantIndex, &RuntimeTest, Descriptor, WorldContextObject);
	ScenarioResult = FGorgeousInsightTestMatrix::ExecuteScenario(Descriptor, ScenarioContext);
	ScenarioResult.AddNote(TEXT("Executed via Insight Matrix runtime"));
#else
	ScenarioResult.bSuccess = false;
	ScenarioResult.AddError(TEXT("Runtime execution requires WITH_DEV_AUTOMATION_TESTS"));
#endif

	if (Request.bEnableSharedHarness)
	{
		FGorgeousInsightHarness::StopHarness();
	}

	FGorgeousInsightHarness::SaveScenarioResult(Descriptor, ScenarioResult, TEXT("Runtime"));

#if GORGEOUS_SYSTEM_INSTALLED_DEBUGASSIST
	if (WorldContextObject && WorldContextObject->GetWorld())
	{
		const FLinearColor BeaconColor = ScenarioResult.bSuccess ? FLinearColor::Green : FLinearColor::Red;
		const FVector DefaultLocation(0.0f, 0.0f, 100.0f); 
		// If there is an instigator or target, we would draw it there. For now, draw a central beacon.
		
		FGorgeousDebugAssistVisualParameters DebugParams;
		DebugParams.Duration = 10.0f;
		DebugParams.bEnabled = true;
		
		UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistPointWithState(
			WorldContextObject, 
			DefaultLocation, 
			ScenarioResult.bSuccess ? EGorgeousDebugAssistPointState::InBounds : EGorgeousDebugAssistPointState::OutOfBounds, 
			DebugParams
		);
	}
#endif

	return ScenarioResult;
}

TArray<FGorgeousInsightScenarioResult> UGorgeousInsightMatrixSubsystem::RunQueuedScenarios()
{
    if (!IsDevelopmentToolsEnabled()) { FGorgeousInsightScenarioResult Disabled; Disabled.bSuccess = false; Disabled.Errors.Add(TEXT("Development Suite is absent, disabled, or not approved for this target.")); return { MoveTemp(Disabled) }; }

	TArray<FGorgeousInsightQueuedScenario> ToRun;
	{
		FScopeLock QueueLock(&QueueMutex);
		ToRun = MoveTemp(ScenarioQueue);
		ScenarioQueue.Reset();
	}

	TArray<FGorgeousInsightScenarioResult> Results;
	Results.Reserve(ToRun.Num());
	for (const FGorgeousInsightQueuedScenario& Entry : ToRun)
	{
		Results.Add(RunScenarioByName(Entry.ScenarioName, Entry.Parameters, Entry.VariantIndex, Entry.WorldContextObject.Get()));
	}

	return Results;
}

bool UGorgeousInsightMatrixSubsystem::EnqueueTest(FName ProviderName, FName TestId, const FGorgeousInsightTestContext& Context)
{
    if (!IsDevelopmentToolsEnabled()) return false;

	if (ProviderName.IsNone() || TestId.IsNone())
	{
		return false;
	}

	FGorgeousInsightQueuedTest Entry;
	Entry.ProviderName = ProviderName;
	Entry.TestId = TestId;
	Entry.Context = Context;

	{
		FScopeLock QueueLock(&QueueMutex);
		TestQueue.Add(MoveTemp(Entry));
	}

	return true;
}

TArray<UGorgeousInsightMatrixSubsystem::FGorgeousInsightQueuedTestResult> UGorgeousInsightMatrixSubsystem::RunQueuedTests()
{
    if (!IsDevelopmentToolsEnabled()) { FGorgeousInsightQueuedTestResult Disabled; Disabled.Result.bSuccess = false; Disabled.Result.Errors.Add(TEXT("Development Suite is absent, disabled, or not approved for this target.")); return { MoveTemp(Disabled) }; }

	TArray<FGorgeousInsightQueuedTest> ToRun;
	{
		FScopeLock QueueLock(&QueueMutex);
		ToRun = MoveTemp(TestQueue);
		TestQueue.Reset();
	}

	TArray<FGorgeousInsightQueuedTestResult> Results;
	Results.Reserve(ToRun.Num());
	for (const FGorgeousInsightQueuedTest& Entry : ToRun)
	{
		FGorgeousInsightQueuedTestResult ResultEntry;
		ResultEntry.ProviderName = Entry.ProviderName;
		ResultEntry.TestId = Entry.TestId;
		ResultEntry.Result = RunTest(Entry.ProviderName, Entry.TestId, Entry.Context);
		Results.Add(MoveTemp(ResultEntry));
	}

	return Results;
}

int32 UGorgeousInsightMatrixSubsystem::GetQueuedTestCount() const
{
	FScopeLock QueueLock(&QueueMutex);
	return TestQueue.Num();
}

int32 UGorgeousInsightMatrixSubsystem::GetQueuedScenarioCount() const
{
	FScopeLock QueueLock(&QueueMutex);
	return ScenarioQueue.Num();
}

void UGorgeousInsightMatrixSubsystem::AddBaselineStats(FName ProviderName, const TArray<FGorgeousInsightStat>& Stats)
{
    if (!IsDevelopmentToolsEnabled()) return;

	LoadChartBaselines();
	TArray<FGorgeousInsightChartDefinition> Charts;
	FGorgeousInsightGatherContext LiveContext;
	LiveContext.Mode = EGorgeousInsightContextMode::Live;
	if (IGorgeousInsightMatrixProvider* Provider = FindProvider(ProviderName)) Provider->GatherCharts(LiveContext, Charts);
	{
	FScopeLock CacheLock(&StatsCacheMutex);
	ProviderBaselineCharts.FindOrAdd(ProviderName) = Charts;
	if (!ProviderBaselineStats.Contains(ProviderName))
		ProviderBaselineStats.Add(ProviderName, ReadProviderBaselineStats(ProviderName));
	TArray<FGorgeousInsightStat>& ProviderStats = ProviderBaselineStats.FindOrAdd(ProviderName);
	
	// Add or update
	for (const FGorgeousInsightStat& NewStat : Stats)
	{
		bool bFound = false;
		for (FGorgeousInsightStat& ExistingStat : ProviderStats)
		{
			if (ExistingStat.Id == NewStat.Id)
			{
				ExistingStat = NewStat;
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			ProviderStats.Add(NewStat);
		}
	}
	
	WriteProviderBaselineStats(ProviderName, ProviderStats);
	}
	SaveChartBaselines();
}

void UGorgeousInsightMatrixSubsystem::RemoveBaselineStat(FName ProviderName, FName StatId)
{
    if (!IsDevelopmentToolsEnabled()) return;

	FScopeLock CacheLock(&StatsCacheMutex);
	if (!ProviderBaselineStats.Contains(ProviderName))
		ProviderBaselineStats.Add(ProviderName, ReadProviderBaselineStats(ProviderName));
	if (TArray<FGorgeousInsightStat>* ProviderStats = ProviderBaselineStats.Find(ProviderName))
	{
		ProviderStats->RemoveAll([StatId](const FGorgeousInsightStat& Stat) { return Stat.Id == StatId; });
		WriteProviderBaselineStats(ProviderName, *ProviderStats);
	}
}

TArray<FGorgeousInsightScenarioRunResult> UGorgeousInsightMatrixSubsystem::RunMatrix(const FString& Parameters, UObject* WorldContextObject)
{
    if (!IsDevelopmentToolsEnabled()) { FGorgeousInsightScenarioRunResult Disabled; Disabled.Result.bSuccess = false; Disabled.Result.Errors.Add(TEXT("Development Suite is absent, disabled, or not approved for this target.")); return { MoveTemp(Disabled) }; }

	const FGorgeousInsightMatrixRequest Request = FGorgeousInsightMatrixRequest::FromParameters(Parameters);
	TArray<FGorgeousInsightScenarioRunResult> Results;

	bool bHarnessStarted = false;
	if (Request.bEnableSharedHarness)
	{
		bHarnessStarted = FGorgeousInsightHarness::StartHarness(Request);
		if (!bHarnessStarted)
		{
			return Results;
		}
	}

#if WITH_DEV_AUTOMATION_TESTS
	FGorgeousInsightRuntimeAutomationTest RuntimeTest;
	Results = FGorgeousInsightTestMatrix::ExecuteMatrix(Request, Parameters, RuntimeTest, WorldContextObject);
#else
	(void)WorldContextObject;
#endif

	if (bHarnessStarted)
	{
		FGorgeousInsightHarness::StopHarness();
	}

	for (const FGorgeousInsightScenarioRunResult& Entry : Results)
	{
		FGorgeousInsightHarness::SaveScenarioResult(Entry.Descriptor, Entry.Result, TEXT("Runtime"));
	}

	return Results;
}

FString UGorgeousInsightMatrixSubsystem::GetInsightMatrixIniPath()
{
	return GetInsightSavedPath(TEXT("PanelState.ini"));
}

void UGorgeousInsightMatrixSubsystem::LoadPanelState()
{
    if (!IsDevelopmentToolsEnabled()) return;

	const FString IniPath = GetInsightMatrixIniPath();
	
	if (!FPaths::FileExists(IniPath))
	{
		return;
	}
	
	FConfigFile ConfigFile;
	ConfigFile.Read(IniPath);
	
	// Load Panel State
	const FString PanelSectionName = TEXT("InsightPanelState");
	if (const FConfigSection* Section = ConfigFile.FindSection(PanelSectionName))
	{
		// Load SelectedProvider
		if (const FConfigValue* Value = Section->Find(TEXT("SelectedProvider")))
		{
			CachedPanelState.SelectedProvider = FName(*Value->GetValue());
		}
		
		// Load ProviderFilter
		if (const FConfigValue* Value = Section->Find(TEXT("ProviderFilter")))
		{
			CachedPanelState.ProviderFilter = Value->GetValue();
		}
		
		// Load bRunWithHarness
		if (const FConfigValue* Value = Section->Find(TEXT("bRunWithHarness")))
		{
			CachedPanelState.bRunWithHarness = Value->GetValue().ToBool();
		}
		
		// Load StatWarningThresholds
		CachedPanelState.StatWarningThresholds.Reset();
		TArray<FConfigValue> WarningThresholds;
		Section->MultiFind(TEXT("StatWarningThreshold"), WarningThresholds);
		for (const FConfigValue& Entry : WarningThresholds)
		{
			FString KeyPart, ValuePart;
			if (Entry.GetValue().Split(TEXT("="), &KeyPart, &ValuePart))
			{
				CachedPanelState.StatWarningThresholds.Add(FName(*KeyPart), FCString::Atod(*ValuePart));
			}
		}
		
		// Load StatCriticalThresholds
		CachedPanelState.StatCriticalThresholds.Reset();
		TArray<FConfigValue> CriticalThresholds;
		Section->MultiFind(TEXT("StatCriticalThreshold"), CriticalThresholds);
		for (const FConfigValue& Entry : CriticalThresholds)
		{
			FString KeyPart, ValuePart;
			if (Entry.GetValue().Split(TEXT("="), &KeyPart, &ValuePart))
			{
				CachedPanelState.StatCriticalThresholds.Add(FName(*KeyPart), FCString::Atod(*ValuePart));
			}
		}
		
		// Load TestInputValues
		CachedPanelState.TestInputValues.Reset();
		TArray<FConfigValue> TestInputs;
		Section->MultiFind(TEXT("TestInputValue"), TestInputs);
		for (const FConfigValue& Entry : TestInputs)
		{
			// Format: ProviderName.TestId.InputId=Value
			FString Remainder, InputValue;
			if (Entry.GetValue().Split(TEXT("="), &Remainder, &InputValue))
			{
				FString ProviderPart, TestAndInputPart;
				if (Remainder.Split(TEXT("."), &ProviderPart, &TestAndInputPart))
				{
					FString TestPart, InputPart;
					if (TestAndInputPart.Split(TEXT("."), &TestPart, &InputPart))
					{
						const FName ProviderName(*ProviderPart);
						const FName TestInputKey(*FString::Printf(TEXT("%s.%s"), *TestPart, *InputPart));
						CachedPanelState.TestInputValues.FindOrAdd(ProviderName).Add(TestInputKey, InputValue);
					}
				}
			}
		}
		
		bHasCachedPanelState = true;
	}
	
	// Load Last Run Stats
	const FString LastRunSectionName = TEXT("LastRunStats");
	if (const FConfigSection* Section = ConfigFile.FindSection(LastRunSectionName))
	{
		if (const FConfigValue* Value = Section->Find(TEXT("bHasRun")))
		{
			LastRunStats.bHasRun = Value->GetValue().ToBool();
		}
		if (const FConfigValue* Value = Section->Find(TEXT("Timestamp")))
		{
			FDateTime::Parse(Value->GetValue(), LastRunStats.Timestamp);
		}
		if (const FConfigValue* Value = Section->Find(TEXT("ProviderName")))
		{
			LastRunStats.ProviderName = FName(*Value->GetValue());
		}
		if (const FConfigValue* Value = Section->Find(TEXT("TestId")))
		{
			LastRunStats.TestId = FName(*Value->GetValue());
		}
		if (const FConfigValue* Value = Section->Find(TEXT("DurationSeconds")))
		{
			LastRunStats.DurationSeconds = FCString::Atod(*Value->GetValue());
		}
		if (const FConfigValue* Value = Section->Find(TEXT("ErrorCount")))
		{
			LastRunStats.ErrorCount = FCString::Atoi(*Value->GetValue());
		}
		if (const FConfigValue* Value = Section->Find(TEXT("WarningCount")))
		{
			LastRunStats.WarningCount = FCString::Atoi(*Value->GetValue());
		}
		if (const FConfigValue* Value = Section->Find(TEXT("NoteCount")))
		{
			LastRunStats.NoteCount = FCString::Atoi(*Value->GetValue());
		}
		if (const FConfigValue* Value = Section->Find(TEXT("ObjectVariablesRegistered")))
		{
			LastRunStats.ObjectVariablesRegistered = FCString::Atoi(*Value->GetValue());
		}
		if (const FConfigValue* Value = Section->Find(TEXT("ObjectVariableOverheadMs")))
		{
			LastRunStats.ObjectVariableOverheadMs = FCString::Atod(*Value->GetValue());
		}
	}
	
	// Load Last Test Results
	LastTestResults.Reset();
	TArray<FString> SectionNames;
	ConfigFile.GetKeys(SectionNames);
	for (const FString& SectionName : SectionNames)
	{
		if (!SectionName.StartsWith(TEXT("TestResult.")))
		{
			continue;
		}
		
		// Format: TestResult.ProviderName.TestId
		FString Remainder = SectionName.Mid(11); // Skip "TestResult."
		FString ProviderPart, TestPart;
		if (!Remainder.Split(TEXT("."), &ProviderPart, &TestPart))
		{
			continue;
		}
		
		const FConfigSection* Section = ConfigFile.FindSection(SectionName);
		if (!Section)
		{
			continue;
		}
		
		FGorgeousInsightTestResult Result;
		
		if (const FConfigValue* Value = Section->Find(TEXT("bSuccess")))
		{
			Result.bSuccess = Value->GetValue().ToBool();
		}
		
		TArray<FConfigValue> Errors;
		Section->MultiFind(TEXT("Error"), Errors);
		for (const FConfigValue& Entry : Errors)
		{
			Result.Errors.Add(Entry.GetValue());
		}
		
		TArray<FConfigValue> Warnings;
		Section->MultiFind(TEXT("Warning"), Warnings);
		for (const FConfigValue& Entry : Warnings)
		{
			Result.Warnings.Add(Entry.GetValue());
		}
		
		TArray<FConfigValue> Notes;
		Section->MultiFind(TEXT("Note"), Notes);
		for (const FConfigValue& Entry : Notes)
		{
			Result.Notes.Add(Entry.GetValue());
		}
		
		TArray<FConfigValue> Metrics;
		Section->MultiFind(TEXT("Metric"), Metrics);
		for (const FConfigValue& Entry : Metrics)
		{
			FString KeyPart, ValuePart;
			if (Entry.GetValue().Split(TEXT("="), &KeyPart, &ValuePart))
			{
				Result.Metrics.Add(KeyPart, ValuePart);
			}
		}
		
		LastTestResults.FindOrAdd(FName(*ProviderPart)).Add(FName(*TestPart), Result);
	}
	
	// Load Last Action Stats
	LastActionStats.Reset();
	for (const FString& SectionName : SectionNames)
	{
		if (!SectionName.StartsWith(TEXT("ActionStats.")))
		{
			continue;
		}
		
		// Format: ActionStats.ProviderName.ActionId
		FString Remainder = SectionName.Mid(12); // Skip "ActionStats."
		FString ProviderPart, ActionPart;
		if (!Remainder.Split(TEXT("."), &ProviderPart, &ActionPart))
		{
			continue;
		}
		
		const FConfigSection* Section = ConfigFile.FindSection(SectionName);
		if (!Section)
		{
			continue;
		}
		
		FGorgeousInsightLastActionStats Stats;
		
		if (const FConfigValue* Value = Section->Find(TEXT("bHasRun")))
		{
			Stats.bHasRun = Value->GetValue().ToBool();
		}
		if (const FConfigValue* Value = Section->Find(TEXT("Timestamp")))
		{
			FDateTime::Parse(Value->GetValue(), Stats.Timestamp);
		}
		Stats.ProviderName = FName(*ProviderPart);
		Stats.ActionId = FName(*ActionPart);
		if (const FConfigValue* Value = Section->Find(TEXT("DurationSeconds")))
		{
			Stats.DurationSeconds = FCString::Atod(*Value->GetValue());
		}
		if (const FConfigValue* Value = Section->Find(TEXT("bSucceeded")))
		{
			Stats.bSucceeded = Value->GetValue().ToBool();
		}
        if (const FConfigValue* Value = Section->Find(TEXT("bHasExecutionResult"))) Stats.bHasExecutionResult = Value->GetValue().ToBool();
        if (const FConfigValue* Value = Section->Find(TEXT("ResultMessage"))) Stats.ResultMessage = Value->GetValue();
		
		LastActionStats.FindOrAdd(FName(*ProviderPart)).Add(FName(*ActionPart), Stats);
	}
}

void UGorgeousInsightMatrixSubsystem::SavePanelState() const
{
    if (!IsDevelopmentToolsEnabled()) return;

	const FString IniPath = GetInsightMatrixIniPath();
	
	FConfigFile ConfigFile;
	if (FPaths::FileExists(IniPath))
	{
		ConfigFile.Read(IniPath);
	}
	
	// Save Panel State
	const FString PanelSectionName = TEXT("InsightPanelState");
	ConfigFile.Remove(PanelSectionName);
	
	if (bHasCachedPanelState)
	{
		// Save SelectedProvider
		ConfigFile.SetString(*PanelSectionName, TEXT("SelectedProvider"), *CachedPanelState.SelectedProvider.ToString());
		
		// Save ProviderFilter
		ConfigFile.SetString(*PanelSectionName, TEXT("ProviderFilter"), *CachedPanelState.ProviderFilter);
		
		// Save bRunWithHarness
		ConfigFile.SetString(*PanelSectionName, TEXT("bRunWithHarness"), CachedPanelState.bRunWithHarness ? TEXT("true") : TEXT("false"));
		
		// Save StatWarningThresholds
		for (const TPair<FName, double>& Pair : CachedPanelState.StatWarningThresholds)
		{
			ConfigFile.AddToSection(*PanelSectionName, TEXT("StatWarningThreshold"), FString::Printf(TEXT("%s=%f"), *Pair.Key.ToString(), Pair.Value));
		}
		
		// Save StatCriticalThresholds
		for (const TPair<FName, double>& Pair : CachedPanelState.StatCriticalThresholds)
		{
			ConfigFile.AddToSection(*PanelSectionName, TEXT("StatCriticalThreshold"), FString::Printf(TEXT("%s=%f"), *Pair.Key.ToString(), Pair.Value));
		}
		
		// Save TestInputValues
		for (const TPair<FName, TMap<FName, FString>>& ProviderPair : CachedPanelState.TestInputValues)
		{
			for (const TPair<FName, FString>& InputPair : ProviderPair.Value)
			{
				ConfigFile.AddToSection(*PanelSectionName, TEXT("TestInputValue"), FString::Printf(TEXT("%s.%s=%s"), 
					*ProviderPair.Key.ToString(), *InputPair.Key.ToString(), *InputPair.Value));
			}
		}
	}
	
	// Save Last Run Stats
	const FString LastRunSectionName = TEXT("LastRunStats");
	ConfigFile.Remove(LastRunSectionName);
	
	if (LastRunStats.bHasRun)
	{
		ConfigFile.SetString(*LastRunSectionName, TEXT("bHasRun"), TEXT("true"));
		ConfigFile.SetString(*LastRunSectionName, TEXT("Timestamp"), *LastRunStats.Timestamp.ToString());
		ConfigFile.SetString(*LastRunSectionName, TEXT("ProviderName"), *LastRunStats.ProviderName.ToString());
		ConfigFile.SetString(*LastRunSectionName, TEXT("TestId"), *LastRunStats.TestId.ToString());
		ConfigFile.SetString(*LastRunSectionName, TEXT("DurationSeconds"), *FString::Printf(TEXT("%f"), LastRunStats.DurationSeconds));
		ConfigFile.SetString(*LastRunSectionName, TEXT("ErrorCount"), *FString::Printf(TEXT("%d"), LastRunStats.ErrorCount));
		ConfigFile.SetString(*LastRunSectionName, TEXT("WarningCount"), *FString::Printf(TEXT("%d"), LastRunStats.WarningCount));
		ConfigFile.SetString(*LastRunSectionName, TEXT("NoteCount"), *FString::Printf(TEXT("%d"), LastRunStats.NoteCount));
		ConfigFile.SetString(*LastRunSectionName, TEXT("ObjectVariablesRegistered"), *FString::Printf(TEXT("%d"), LastRunStats.ObjectVariablesRegistered));
		ConfigFile.SetString(*LastRunSectionName, TEXT("ObjectVariableOverheadMs"), *FString::Printf(TEXT("%f"), LastRunStats.ObjectVariableOverheadMs));
	}
	
	// Clear old test result sections
	TArray<FString> SectionNames;
	ConfigFile.GetKeys(SectionNames);
	for (const FString& SectionName : SectionNames)
	{
		if (SectionName.StartsWith(TEXT("TestResult.")) || SectionName.StartsWith(TEXT("ActionStats.")))
		{
			ConfigFile.Remove(SectionName);
		}
	}
	
	// Save Last Test Results
	for (const TPair<FName, TMap<FName, FGorgeousInsightTestResult>>& ProviderPair : LastTestResults)
	{
		for (const TPair<FName, FGorgeousInsightTestResult>& TestPair : ProviderPair.Value)
		{
			const FString SectionName = FString::Printf(TEXT("TestResult.%s.%s"), *ProviderPair.Key.ToString(), *TestPair.Key.ToString());
			
			ConfigFile.SetString(*SectionName, TEXT("bSuccess"), TestPair.Value.bSuccess ? TEXT("true") : TEXT("false"));
			
			for (const FString& Error : TestPair.Value.Errors)
			{
				ConfigFile.AddToSection(*SectionName, TEXT("Error"), Error);
			}
			
			for (const FString& Warning : TestPair.Value.Warnings)
			{
				ConfigFile.AddToSection(*SectionName, TEXT("Warning"), Warning);
			}
			
			for (const FString& Note : TestPair.Value.Notes)
			{
				ConfigFile.AddToSection(*SectionName, TEXT("Note"), Note);
			}
			
			for (const TPair<FString, FString>& Metric : TestPair.Value.Metrics)
			{
				ConfigFile.AddToSection(*SectionName, TEXT("Metric"), FString::Printf(TEXT("%s=%s"), *Metric.Key, *Metric.Value));
			}
		}
	}
	
	// Save Last Action Stats
	for (const TPair<FName, TMap<FName, FGorgeousInsightLastActionStats>>& ProviderPair : LastActionStats)
	{
		for (const TPair<FName, FGorgeousInsightLastActionStats>& ActionPair : ProviderPair.Value)
		{
			const FString SectionName = FString::Printf(TEXT("ActionStats.%s.%s"), *ProviderPair.Key.ToString(), *ActionPair.Key.ToString());
			
			ConfigFile.SetString(*SectionName, TEXT("bHasRun"), ActionPair.Value.bHasRun ? TEXT("true") : TEXT("false"));
			ConfigFile.SetString(*SectionName, TEXT("Timestamp"), *ActionPair.Value.Timestamp.ToString());
			ConfigFile.SetString(*SectionName, TEXT("DurationSeconds"), *FString::Printf(TEXT("%f"), ActionPair.Value.DurationSeconds));
			ConfigFile.SetString(*SectionName, TEXT("bSucceeded"), ActionPair.Value.bSucceeded ? TEXT("true") : TEXT("false"));
            ConfigFile.SetString(*SectionName, TEXT("bHasExecutionResult"), ActionPair.Value.bHasExecutionResult ? TEXT("true") : TEXT("false"));
            ConfigFile.SetString(*SectionName, TEXT("ResultMessage"), *ActionPair.Value.ResultMessage);
		}
	}
	
	ConfigFile.Dirty = true;
	ConfigFile.Write(IniPath);
}

void UGorgeousInsightMatrixSubsystem::RunAllLocalFunctionalTests(UObject* WorldContextObject)
{
    if (!IsDevelopmentToolsEnabled()) return;

#if WITH_EDITOR
	if (!WorldContextObject)
	{
		return;
	}

	if (UWorld* World = WorldContextObject->GetWorld())
	{
		for (TActorIterator<AFunctionalTest> It(World); It; ++It)
		{
			AFunctionalTest* TestActor = *It;
			if (TestActor)
			{
				// Triggers the functional test logic, allowing the beacon to process state.
				TestActor->RunTest(TArray<FString>());
			}
		}
	}
#endif
}
