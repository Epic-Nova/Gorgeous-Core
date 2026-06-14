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

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "InsightMatrix/GorgeousInsightMatrixProvider.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "Slate/GorgeousInsightDebugPanel.h"
#include "Widgets/SWindow.h"
#include "Features/IModularFeatures.h"
#include "GorgeousInsightMatrixSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGorgeousInsightProviderChanged, IGorgeousInsightMatrixProvider*);

/**
 * Runtime registry and execution layer for the Insight Matrix.
 * Editor modules can bridge into this subsystem.
 * TODO: Add hooks into Unreal Automation, Insights/Trace, Gauntlet, and profiler pipelines.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousInsightMatrixSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	static UGorgeousInsightMatrixSubsystem* Get();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void OnModularFeatureRegistered(const FName& FeatureName, IModularFeature* Feature);
	void OnModularFeatureUnregistered(const FName& FeatureName, IModularFeature* Feature);

	struct FGorgeousInsightQueuedTestResult
	{
		FName ProviderName;
		FName TestId;
		FGorgeousInsightTestResult Result;
	};

	struct FGorgeousInsightLastRunStats
	{
		bool bHasRun = false;
		FDateTime Timestamp;
		FName ProviderName = NAME_None;
		FName TestId = NAME_None;
		double DurationSeconds = 0.0;
		int32 ErrorCount = 0;
		int32 WarningCount = 0;
		int32 NoteCount = 0;
		int32 ObjectVariablesRegistered = 0;
		double ObjectVariableOverheadMs = 0.0;
	};

	struct FGorgeousInsightLastActionStats
	{
		bool bHasRun = false;
		FDateTime Timestamp;
		FName ProviderName = NAME_None;
		FName ActionId = NAME_None;
		double DurationSeconds = 0.0;
		bool bSucceeded = false;
	};

	/** Opens the unified debug panel window. */
	void ShowDebugPanel(int32 ZOrder = 0);

	/** Hides the unified debug panel window. */
	void HideDebugPanel();

	/** Toggles the unified debug panel window. */
	void ToggleDebugPanel(int32 ZOrder = 0);

	/** Returns true if the debug panel window is open. */
	bool IsDebugPanelOpen() const;

	/** Refreshes the debug panel contents if open. */
	void RefreshDebugPanel();

	/** Opens the in-game debug panel overlay (viewport). */
	void ShowInGamePanel(int32 ZOrder = 0);

	/** Hides the in-game debug panel overlay (viewport). */
	void HideInGamePanel();

	/** Toggles the in-game debug panel overlay (viewport). */
	void ToggleInGamePanel(int32 ZOrder = 0);

	/** Returns true if the in-game panel is open. */
	bool IsInGamePanelOpen() const;

	bool RegisterProvider(IGorgeousInsightMatrixProvider* Provider);
	bool UnregisterProvider(IGorgeousInsightMatrixProvider* Provider);

	TArray<IGorgeousInsightMatrixProvider*> GetProviders() const;
	IGorgeousInsightMatrixProvider* FindProvider(FName ProviderName) const;

	void GatherAllStats(TArray<FGorgeousInsightStat>& OutStats) const;
	void GatherProviderStats(FName ProviderName, TArray<FGorgeousInsightStat>& OutStats) const;
	void GatherProviderCharts(FName ProviderName, TArray<FGorgeousInsightChartDefinition>& OutCharts) const;

	void GatherAllActions(TArray<FGorgeousInsightAction>& OutActions) const;
	void GatherProviderActions(FName ProviderName, TArray<FGorgeousInsightAction>& OutActions) const;

	void GatherAllTests(TArray<FGorgeousInsightTest>& OutTests) const;
	void GatherProviderTests(FName ProviderName, TArray<FGorgeousInsightTest>& OutTests) const;

	bool ExecuteAction(FName ProviderName, FName ActionId, const FGorgeousInsightActionContext& Context);
	FGorgeousInsightTestResult RunTest(FName ProviderName, FName TestId, const FGorgeousInsightTestContext& Context);
	const FGorgeousInsightLastRunStats& GetLastRunStats() const { return LastRunStats; }
	bool GetLastActionStats(FName ProviderName, FName ActionId, FGorgeousInsightLastActionStats& OutStats) const;
	const FGorgeousInsightTestResult* GetBaselineResult(FName ProviderName, FName TestId) const;
	bool SetBaselineFromLastResult(FName ProviderName, FName TestId);

	/** Export provider or all stats to CSV/JSON. */
	bool ExportStatsToCSV(const FString& AbsoluteFilePath, FName ProviderName = NAME_None) const;
	bool ExportStatsToJson(const FString& AbsoluteFilePath, FName ProviderName = NAME_None) const;

	/** Queue a scenario by name for runtime execution. */
	bool EnqueueScenario(FName ScenarioName, const FString& Parameters = TEXT(""), int32 VariantIndex = 0, UObject* WorldContextObject = nullptr);

	/** Run a scenario immediately by name (runtime). */
	FGorgeousInsightScenarioResult RunScenarioByName(FName ScenarioName, const FString& Parameters = TEXT(""), int32 VariantIndex = 0, UObject* WorldContextObject = nullptr);

	/** Drain the queued scenarios and return results in order. */
	TArray<FGorgeousInsightScenarioResult> RunQueuedScenarios();

	/** Returns the number of queued scenarios. */
	int32 GetQueuedScenarioCount() const;

	/** Run the filtered scenario matrix in one call. */
	TArray<FGorgeousInsightScenarioRunResult> RunMatrix(const FString& Parameters = TEXT(""), UObject* WorldContextObject = nullptr);

	/** Queue a provider test for execution. */
	bool EnqueueTest(FName ProviderName, FName TestId, const FGorgeousInsightTestContext& Context);

	/** Drain queued tests and return results in order. */
	TArray<FGorgeousInsightQueuedTestResult> RunQueuedTests();

	/** Returns the number of queued tests. */
	int32 GetQueuedTestCount() const;

	FOnGorgeousInsightProviderChanged OnProviderRegistered;
	FOnGorgeousInsightProviderChanged OnProviderUnregistered;

private:
	void RegisterInputPreProcessor();
	void UnregisterInputPreProcessor();
	FMargin GetInGamePanelOffset() const;
	FVector2D GetInGamePanelPosition() const;
	void OnInGamePanelDragged(const FVector2D& NewPosition);
	void UpdateInGameInputMode(bool bEnable) const;
	void BindPanelState(const TSharedPtr<SGorgeousInsightDebugPanel>& Panel);
	void ApplyPanelStateToAll(const SGorgeousInsightDebugPanel::FInsightPanelState& State);
	void HandlePanelStateChanged(const SGorgeousInsightDebugPanel::FInsightPanelState& State);
	void UpdateLastRunStats(FName ProviderName, FName TestId, const FGorgeousInsightTestResult& Result, double DurationSeconds);
	void LoadCachedStats();
	void SaveCachedStats() const;
	void CacheProviderStats(FName ProviderName, const TArray<FGorgeousInsightStat>& Stats) const;
	static FString GetInsightMatrixIniPath();
	void LoadPanelState();
	void SavePanelState() const;

	struct FGorgeousInsightQueuedScenario
	{
		FName ScenarioName;
		FString Parameters;
		int32 VariantIndex = 0;
		TWeakObjectPtr<UObject> WorldContextObject;
	};

	struct FGorgeousInsightQueuedTest
	{
		FName ProviderName;
		FName TestId;
		FGorgeousInsightTestContext Context;
	};

	mutable FCriticalSection ProviderMutex;
	TSet<IGorgeousInsightMatrixProvider*> Providers;
	mutable FCriticalSection QueueMutex;
	TArray<FGorgeousInsightQueuedScenario> ScenarioQueue;
	TArray<FGorgeousInsightQueuedTest> TestQueue;
	TMap<FName, TMap<FName, FGorgeousInsightTestResult>> LastTestResults;
	TMap<FString, FGorgeousInsightTestResult> BaselineResultsCache;
	FGorgeousInsightLastRunStats LastRunStats;
	TMap<FName, TMap<FName, FGorgeousInsightLastActionStats>> LastActionStats;
	mutable FCriticalSection StatsCacheMutex;
	mutable TMap<FName, TArray<FGorgeousInsightStat>> CachedProviderStats;
	mutable bool bStatsCacheLoaded = false;
	mutable bool bStatsCacheDirty = false;

	TSharedPtr<SWindow> DebugPanelWindow;
	TSharedPtr<SGorgeousInsightDebugPanel> DebugPanelWidget;

	TSharedPtr<class SConstraintCanvas> InGameCanvas;
	TSharedPtr<class SGorgeousInsightDebugPanel> InGamePanelWidget;
	TSharedPtr<class SWidget> InGamePanelContainer;
	FVector2D InGamePanelPosition = FVector2D(80.f, 80.f);
	FVector2D InGamePanelSize = FVector2D(900.f, 620.f);
	bool bInGamePanelVisible = false;
	TSharedPtr<class IInputProcessor> InputPreProcessor;
	SGorgeousInsightDebugPanel::FInsightPanelState CachedPanelState;
	bool bHasCachedPanelState = false;
	bool bApplyingPanelState = false;
};
