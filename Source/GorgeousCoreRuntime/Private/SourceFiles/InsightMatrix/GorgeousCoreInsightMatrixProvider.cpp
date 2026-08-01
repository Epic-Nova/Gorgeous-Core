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

#include "InsightMatrix/GorgeousCoreInsightMatrixProvider.h"

#include "InsightMatrix/GorgeousInsightHarness.h"
#include "InsightMatrix/GorgeousInsightHarness.h"
#include "InsightMatrix/GorgeousInsightMatrixSubsystem.h"
#include "InsightMatrix/GorgeousInsightStatBuilder.h"
#include "GeneralSystems/DebugAssist/GorgeousDebugAssistBlueprintFunctionLibrary.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousPrimaryGameLayout.h"
#include "GeneralSystems/InteractionFoundation/GorgeousInteractionFoundation.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatComponent_AC.h"
#include "GeneralSystems/FeedbackEffects/GorgeousFeedbackDispatcher.h"
#include "InsightMatrix/GorgeousInsightBlueprintStats_OV.h"
#include "ObjectVariables/GorgeousObjectVariableRegistry_GIS.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStorage_OV.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "ObjectVariables/Slate/GorgeousObjectVariableBrowserWindow.h"
#include "AutoReplication/Slate/SGorgeousNetworkTrafficInspectorWindow.h"
#include "AutoReplication/Slate/SGorgeousRPCInspectorWindow.h"
#include "InsightMatrix/GorgeousInsightActionConfig_DA.h"
#include "AssetRegistry/AssetRegistryModule.h"

// Tab Spawning support
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"

#if WITH_EDITOR
#include "Editor.h"
#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"
#endif
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "ObjectVariables/GorgeousRootNetworkStackSubsystem.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

namespace
{
	const FName ActionOpenOVBrowser(TEXT("Core.OV.Browser"));
	const FName ActionOpenTrafficInspector(TEXT("AR.Window.TrafficInspector"));
	const FName ActionOpenRPCInspector(TEXT("AR.Window.RPCInspector"));

	bool IsCoreScenario(const FGorgeousInsightScenarioDescriptor& Descriptor)
	{
		return Descriptor.Tags.Contains(FName(TEXT("GorgeousCore"))) || Descriptor.Tags.Contains(FName(TEXT("System.Core")));
	}

	bool IsARScenario(const FGorgeousInsightScenarioDescriptor& Descriptor)
	{
		return Descriptor.Tags.Contains(FName(TEXT("System.AutoReplication")));
	}

	FString BuildParameterString(const TMap<FString, FString>& Parameters)
	{
		FString Result;
		for (const auto& Pair : Parameters)
		{
			Result += FString::Printf(TEXT("%s=%s "), *Pair.Key, *Pair.Value);
		}
		return Result;
	}

	FGorgeousInsightTestResult ConvertScenarioResult(const FGorgeousInsightScenarioResult& Result)
	{
		FGorgeousInsightTestResult OutResult;
		OutResult.bSuccess = Result.bSuccess;
		OutResult.Errors = Result.Errors;
		OutResult.Warnings = Result.Warnings;
		OutResult.Notes = Result.Notes;
		OutResult.Metrics = Result.Metrics;
		OutResult.LogCapture = Result.LogCapture;
		return OutResult;
	}
}

FName FGorgeousCoreInsightMatrixProvider::ProviderName()
{
	return FName(TEXT("GorgeousCore"));
}

FText FGorgeousCoreInsightMatrixProvider::GetProviderDisplayName() const
{
	return FText::FromString(TEXT("Gorgeous Core"));
}

void FGorgeousCoreInsightMatrixProvider::GatherStats(const FGorgeousInsightGatherContext& Context, TArray<FGorgeousInsightStat>& OutStats) const
{
	// Purged. Will pull published stats instead.
	UGorgeousInsightStatBuilder::GatherPublishedStats(ProviderName(), OutStats);

	// --- 1.2 SIGNAL BRIDGE STATS ---
	FGorgeousInsightStat DispatchedStat;
	DispatchedStat.Id = TEXT("Core.SignalBridge.Dispatched");
	DispatchedStat.DisplayName = FText::FromString(TEXT("Signals Dispatched"));
	DispatchedStat.Description = FText::FromString(TEXT("Total local signals fired since boot."));
	DispatchedStat.Category = FName(TEXT("Signal Bridge"));
	DispatchedStat.NumericValue = (USignalBridgeStorage_OV::GetTotalLocalSignalsFired());
	OutStats.Add(DispatchedStat);

	FGorgeousInsightStat DroppedStat;
	DroppedStat.Id = TEXT("Core.SignalBridge.Dropped");
	DroppedStat.DisplayName = FText::FromString(TEXT("Dropped Signals"));
	DroppedStat.Description = FText::FromString(TEXT("Signals fired with zero active listeners."));
	DroppedStat.Category = FName(TEXT("Signal Bridge"));
	DroppedStat.NumericValue = (USignalBridgeStorage_OV::GetTotalNoListenersFound());
	OutStats.Add(DroppedStat);

	if (GEngine && GEngine->GetWorldContexts().Num() > 0)
	{
		if (UWorld* World = GEngine->GetWorldContexts()[0].World())
		{
			if (USignalBridgeStorage_OV* Bridge = USignalBridgeBlueprintFunctionLibrary::GetSignalBridgeStorage(World, false))
			{
				FGorgeousInsightStat ListenersStat;
				ListenersStat.Id = TEXT("Core.SignalBridge.Listeners");
				ListenersStat.DisplayName = FText::FromString(TEXT("Active Tag Listeners"));
				ListenersStat.Description = FText::FromString(TEXT("Number of unique gameplay tags currently being listened to."));
				ListenersStat.Category = FName(TEXT("Signal Bridge"));
				ListenersStat.NumericValue = (Bridge->GetTotalActiveListeners());
				OutStats.Add(ListenersStat);
			}
		}
	}

#if GORGEOUS_SYSTEM_INSTALLED_STATSFOUNDATION
	// Stats now displayed as a Bar Chart
#endif // GORGEOUS_SYSTEM_INSTALLED_STATSFOUNDATION

#if GORGEOUS_SYSTEM_INSTALLED_INTERACTIONFOUNDATION
	{
		FGorgeousInsightStat ActiveInteractions;
		ActiveInteractions.Id = TEXT("Core.InteractionFoundation.ActiveTrackedActors");
		ActiveInteractions.DisplayName = FText::FromString(TEXT("Active Tracked Interaction Actors"));
		ActiveInteractions.Category = FName(TEXT("Interaction Foundation"));
		ActiveInteractions.NumericValue = static_cast<double>(UGorgeousInteractionFoundation::GetActiveTrackedInteractionActors());
		OutStats.Add(ActiveInteractions);
	}
#endif // GORGEOUS_SYSTEM_INSTALLED_INTERACTIONFOUNDATION

#if GORGEOUS_SYSTEM_INSTALLED_COMMONUIFOUNDATION
	// Stats now displayed as a Bar Chart
#endif // GORGEOUS_SYSTEM_INSTALLED_COMMONUIFOUNDATION

#if GORGEOUS_SYSTEM_INSTALLED_DEBUGASSIST
	{
		FGorgeousInsightStat ActiveBeacons;
		ActiveBeacons.Id = TEXT("Core.DebugAssist.ActiveBeacons");
		ActiveBeacons.DisplayName = FText::FromString(TEXT("Active Debug Beacons"));
		ActiveBeacons.Category = FName(TEXT("Debug Assist"));
		ActiveBeacons.NumericValue = static_cast<double>(UGorgeousDebugAssistBlueprintFunctionLibrary::GetTotalActiveDebugBeacons());
		OutStats.Add(ActiveBeacons);

		FGorgeousInsightStat RenderOverhead;
		RenderOverhead.Id = TEXT("Core.DebugAssist.RenderOverheadMS");
		RenderOverhead.DisplayName = FText::FromString(TEXT("Render Overhead (MS)"));
		RenderOverhead.Category = FName(TEXT("Debug Assist"));
		RenderOverhead.ValueType = EGorgeousInsightStatValueType::TimeSeconds;
		RenderOverhead.NumericValue = UGorgeousDebugAssistBlueprintFunctionLibrary::GetDebugAssistRenderOverheadMS() / 1000.0;
		OutStats.Add(RenderOverhead);
	}
#endif // GORGEOUS_SYSTEM_INSTALLED_DEBUGASSIST

	{
		FGorgeousInsightStat AliveOVs;
		AliveOVs.Id = TEXT("Core.ObjectVariables.AliveCount");
		AliveOVs.DisplayName = FText::FromString(TEXT("Active Object Variables (Memory)"));
		AliveOVs.Category = FName(TEXT("Object Variables"));
		AliveOVs.NumericValue = static_cast<double>(UGorgeousObjectVariable::GetTotalAliveObjectVariables());
		OutStats.Add(AliveOVs);
	}

	// ── FEEDBACK EFFECTS STATS ───────────────────────────────────────────
	if (GEngine && GEngine->GetWorldContexts().Num() > 0)
	{
		if (UWorld* World = GEngine->GetWorldContexts()[0].World())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				if (UGorgeousFeedbackDispatcher* Dispatcher = GameInstance->GetSubsystem<UGorgeousFeedbackDispatcher>())
				{
					FGorgeousInsightStat RegisteredProviders;
					RegisteredProviders.Id = TEXT("Core.Feedback.RegisteredProviders");
					RegisteredProviders.DisplayName = FText::FromString(TEXT("Registered Providers"));
					RegisteredProviders.Description = FText::FromString(TEXT("Number of feedback providers currently registered with the dispatcher."));
					RegisteredProviders.Category = FName(TEXT("Feedback Effects"));
					RegisteredProviders.NumericValue = static_cast<double>(Dispatcher->GetRegisteredProviderCount());
					OutStats.Add(RegisteredProviders);

					FGorgeousInsightStat TotalTriggers;
					TotalTriggers.Id = TEXT("Core.Feedback.TotalTriggers");
					TotalTriggers.DisplayName = FText::FromString(TEXT("Total Triggers"));
					TotalTriggers.Description = FText::FromString(TEXT("Total TriggerFeedback calls since the dispatcher was created."));
					TotalTriggers.Category = FName(TEXT("Feedback Effects"));
					TotalTriggers.NumericValue = static_cast<double>(Dispatcher->GetTotalTriggers());
					OutStats.Add(TotalTriggers);

					FGorgeousInsightStat TriggersResolved;
					TriggersResolved.Id = TEXT("Core.Feedback.TriggersResolved");
					TriggersResolved.DisplayName = FText::FromString(TEXT("Triggers Resolved"));
					TriggersResolved.Description = FText::FromString(TEXT("TriggerFeedback calls that resolved a non-empty definition via a provider."));
					TriggersResolved.Category = FName(TEXT("Feedback Effects"));
					TriggersResolved.NumericValue = static_cast<double>(Dispatcher->GetTotalTriggersResolved());
					OutStats.Add(TriggersResolved);

					FGorgeousInsightStat TriggersUnresolved;
					TriggersUnresolved.Id = TEXT("Core.Feedback.TriggersUnresolved");
					TriggersUnresolved.DisplayName = FText::FromString(TEXT("Triggers Unresolved"));
					TriggersUnresolved.Description = FText::FromString(TEXT("TriggerFeedback calls that found no provider able to resolve a definition."));
					TriggersUnresolved.Category = FName(TEXT("Feedback Effects"));
					TriggersUnresolved.NumericValue = static_cast<double>(Dispatcher->GetTotalTriggersUnresolved());
					OutStats.Add(TriggersUnresolved);

					FGorgeousInsightStat EffectsExecuted;
					EffectsExecuted.Id = TEXT("Core.Feedback.EffectsExecuted");
					EffectsExecuted.DisplayName = FText::FromString(TEXT("Effects Executed"));
					EffectsExecuted.Description = FText::FromString(TEXT("Total individual effects executed across all definitions played."));
					EffectsExecuted.Category = FName(TEXT("Feedback Effects"));
					EffectsExecuted.NumericValue = static_cast<double>(Dispatcher->GetTotalEffectsExecuted());
					OutStats.Add(EffectsExecuted);

					// Recent executed effects (DisplayName + Description) surfaced as the feedback
					// debug feed for designers.
					const TArray<FString>& RecentEffects = Dispatcher->GetRecentEffects();
					for (int32 Index = 0; Index < RecentEffects.Num(); ++Index)
					{
						FGorgeousInsightStat RecentStat;
						RecentStat.Id = FName(*FString::Printf(TEXT("Core.Feedback.Recent.%d"), Index));
						RecentStat.DisplayName = FText::FromString(FString::Printf(TEXT("Recent Effect %d"), Index + 1));
						RecentStat.Description = FText::FromString(TEXT("Most recently executed feedback effect (newest last)."));
						RecentStat.Category = FName(TEXT("Feedback Effects"));
						RecentStat.ValueType = EGorgeousInsightStatValueType::Text;
						RecentStat.TextValue = FText::FromString(RecentEffects[Index]);
						OutStats.Add(RecentStat);
					}
				}
			}
		}
	}

	if (GEngine && GEngine->GetWorldContexts().Num() > 0)
	{
		if (UWorld* World = GEngine->GetWorldContexts()[0].World())
		{
			TArray<UGorgeousObjectVariable*> FoundOVs = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry();
			if (FoundOVs.Num() > 0)
			{
				for (UGorgeousObjectVariable* OV : FoundOVs)
				{
					if (UGorgeousInsightBlueprintStats_OV* StatsOV = Cast<UGorgeousInsightBlueprintStats_OV>(OV))
					{
						for (const auto& Pair : StatsOV->SystemStatsMap)
						{
							FName CategoryName = Pair.Key;
							for (const auto& StatPair : Pair.Value.NumericStats)
							{
								FGorgeousInsightStat BPStat;
								BPStat.Id = FName(*FString::Printf(TEXT("Blueprint.%s.%s"), *CategoryName.ToString(), *StatPair.Key));
								BPStat.DisplayName = FText::FromString(StatPair.Key);
								BPStat.Category = CategoryName;
								BPStat.NumericValue = StatPair.Value;
								OutStats.Add(BPStat);
							}
						}
					}
				}
			}
		}
	}
}

void FGorgeousCoreInsightMatrixProvider::GatherCharts(TArray<FGorgeousInsightChartDefinition>& OutCharts) const
{
	// 1. Interaction Foundation (Pie Chart)
#if GORGEOUS_SYSTEM_INSTALLED_INTERACTIONFOUNDATION
	{
		FGorgeousInsightChartDefinition InterChart;
		InterChart.Id = TEXT("Core.Charts.Interactions");
		InterChart.Type = EGorgeousInsightChartType::Pie;
		InterChart.Title = FText::FromString(TEXT("Interaction Results"));
		InterChart.bPieDonut = false;
		
		FGorgeousInsightPieSlice SuccessSlice;
		SuccessSlice.Label = FText::FromString(TEXT("Successful"));
		SuccessSlice.Value = UGorgeousInteractionFoundation::GetTotalSuccessfulInteractions();
		SuccessSlice.Color = FLinearColor::Green;
		InterChart.PieSlices.Add(SuccessSlice);

		FGorgeousInsightPieSlice RejectedSlice;
		RejectedSlice.Label = FText::FromString(TEXT("Rejected"));
		RejectedSlice.Value = UGorgeousInteractionFoundation::GetTotalRejectedInteractions();
		RejectedSlice.Color = FLinearColor::Yellow;
		InterChart.PieSlices.Add(RejectedSlice);

		FGorgeousInsightPieSlice DeniedSlice;
		DeniedSlice.Label = FText::FromString(TEXT("Permission Denied"));
		DeniedSlice.Value = UGorgeousInteractionFoundation::GetTotalPermissionDeniedInteractions();
		DeniedSlice.Color = FLinearColor::Red;
		InterChart.PieSlices.Add(DeniedSlice);

		OutCharts.Add(InterChart);
	}
#endif

	// 2. Object Variables (Donut Chart)
	if (GEngine && GEngine->GetWorldContexts().Num() > 0)
	{
		if (UWorld* World = GEngine->GetWorldContexts()[0].World())
		{
			TArray<UGorgeousObjectVariable*> FoundOVs = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry();
				
				TMap<FString, int32> ClassCounts;
				for (UGorgeousObjectVariable* OV : FoundOVs)
				{
					if (OV)
					{
						FString ClassName = OV->GetClass()->GetName();
						ClassName.RemoveFromEnd(TEXT("_OV"));
						ClassName.RemoveFromEnd(TEXT("_C"));
						int32& Count = ClassCounts.FindOrAdd(ClassName);
						Count++;
					}
				}

				if (ClassCounts.Num() > 0)
				{
					FGorgeousInsightChartDefinition OVChart;
					OVChart.Id = TEXT("Core.Charts.ObjectVariables");
					OVChart.Type = EGorgeousInsightChartType::Pie;
					OVChart.Title = FText::FromString(TEXT("Object Variables Distribution"));
					OVChart.bPieDonut = true;

					int32 ColorIndex = 0;
					TArray<FLinearColor> Palette = { FLinearColor::Blue, FLinearColor::Red, FLinearColor::Green, FLinearColor::Yellow, FLinearColor(1.f, 0.f, 1.f, 1.f), FLinearColor(0.f, 1.f, 1.f, 1.f) };

					for (const auto& Pair : ClassCounts)
					{
						FGorgeousInsightPieSlice Slice;
						Slice.Label = FText::FromString(Pair.Key);
						Slice.Value = Pair.Value;
						Slice.Color = Palette[ColorIndex % Palette.Num()];
						OVChart.PieSlices.Add(Slice);
						ColorIndex++;
					}
					OutCharts.Add(OVChart);
				}
		}
	}

#if GORGEOUS_SYSTEM_INSTALLED_STATSFOUNDATION
	{
		FGorgeousInsightChartDefinition StatsChart;
		StatsChart.Id = TEXT("Core.Charts.StatsFoundation");
		StatsChart.Type = EGorgeousInsightChartType::Bar;
		StatsChart.Title = FText::FromString(TEXT("Stat Component Activity"));

		FGorgeousInsightBarValue ActiveComps;
		ActiveComps.Label = FText::FromString(TEXT("Active Components"));
		ActiveComps.Value = UGorgeousStatComponent_AC::GetTotalActiveComponents();
		StatsChart.Bars.Add(ActiveComps);

		FGorgeousInsightBarValue Modifiers;
		Modifiers.Label = FText::FromString(TEXT("Total Modifiers Applied"));
		Modifiers.Value = UGorgeousStatComponent_AC::GetTotalModifiersApplied();
		StatsChart.Bars.Add(Modifiers);

		OutCharts.Add(StatsChart);
	}
#endif

#if GORGEOUS_SYSTEM_INSTALLED_COMMONUIFOUNDATION
	{
		FGorgeousInsightChartDefinition UIChart;
		UIChart.Id = TEXT("Core.Charts.UIFoundation");
		UIChart.Type = EGorgeousInsightChartType::Bar;
		UIChart.Title = FText::FromString(TEXT("UI Foundation Activity"));

		FGorgeousInsightBarValue Overlays;
		Overlays.Label = FText::FromString(TEXT("Active UI Overlays"));
		Overlays.Value = UGorgeousPrimaryGameLayout::GetTotalActiveOverlays();
		UIChart.Bars.Add(Overlays);

		FGorgeousInsightBarValue ThemeSwaps;
		ThemeSwaps.Label = FText::FromString(TEXT("Total Theme Swaps"));
		ThemeSwaps.Value = UGorgeousUIFoundationSubsystem::GetTotalThemeSwapsTriggered();
		UIChart.Bars.Add(ThemeSwaps);

		OutCharts.Add(UIChart);
	}
#endif

	// 3. Feedback Effects (Bar Chart)
	if (GEngine && GEngine->GetWorldContexts().Num() > 0)
	{
		if (UWorld* World = GEngine->GetWorldContexts()[0].World())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				if (UGorgeousFeedbackDispatcher* Dispatcher = GameInstance->GetSubsystem<UGorgeousFeedbackDispatcher>())
				{
					FGorgeousInsightChartDefinition FeedbackChart;
					FeedbackChart.Id = TEXT("Core.Charts.FeedbackEffects");
					FeedbackChart.Type = EGorgeousInsightChartType::Bar;
					FeedbackChart.Title = FText::FromString(TEXT("Feedback Effects Activity"));

					FGorgeousInsightBarValue ProvidersBar;
					ProvidersBar.Label = FText::FromString(TEXT("Registered Providers"));
					ProvidersBar.Value = Dispatcher->GetRegisteredProviderCount();
					FeedbackChart.Bars.Add(ProvidersBar);

					FGorgeousInsightBarValue TriggersBar;
					TriggersBar.Label = FText::FromString(TEXT("Total Triggers"));
					TriggersBar.Value = Dispatcher->GetTotalTriggers();
					FeedbackChart.Bars.Add(TriggersBar);

					FGorgeousInsightBarValue ResolvedBar;
					ResolvedBar.Label = FText::FromString(TEXT("Triggers Resolved"));
					ResolvedBar.Value = Dispatcher->GetTotalTriggersResolved();
					FeedbackChart.Bars.Add(ResolvedBar);

					FGorgeousInsightBarValue UnresolvedBar;
					UnresolvedBar.Label = FText::FromString(TEXT("Triggers Unresolved"));
					UnresolvedBar.Value = Dispatcher->GetTotalTriggersUnresolved();
					FeedbackChart.Bars.Add(UnresolvedBar);

					FGorgeousInsightBarValue EffectsBar;
					EffectsBar.Label = FText::FromString(TEXT("Effects Executed"));
					EffectsBar.Value = Dispatcher->GetTotalEffectsExecuted();
					FeedbackChart.Bars.Add(EffectsBar);

					OutCharts.Add(FeedbackChart);
				}
			}
		}
	}

	// Will pull published blueprint charts as well.
	UGorgeousInsightStatBuilder::GatherPublishedCharts(ProviderName(), OutCharts);
}

void FGorgeousCoreInsightMatrixProvider::GetActions(TArray<FGorgeousInsightAction>& OutActions) const
{
	FGorgeousInsightAction BrowserAction;
	BrowserAction.Id = ActionOpenOVBrowser;
	BrowserAction.DisplayName = FText::FromString(TEXT("Object Variable Browser"));
	BrowserAction.Description = FText::FromString(TEXT("Opens the general-purpose Object Variable tree browser window."));
	BrowserAction.Category = FName(TEXT("Windows"));
	OutActions.Add(BrowserAction);

	// ── AutoReplication merged actions ────────────────────────────────────
	{
		FGorgeousInsightAction Action;
		Action.Id = ActionOpenTrafficInspector;
		Action.DisplayName = FText::FromString(TEXT("Network Traffic Inspector"));
		Action.Description = FText::FromString(TEXT("Opens the Network Traffic Inspector companion window."));
		Action.Category = FName(TEXT("Windows"));
		OutActions.Add(Action);
	}
	{
		FGorgeousInsightAction Action;
		Action.Id = ActionOpenRPCInspector;
		Action.DisplayName = FText::FromString(TEXT("RPC Inspector"));
		Action.Description = FText::FromString(TEXT("Opens the RPC Inspector companion window."));
		Action.Category = FName(TEXT("Windows"));
		OutActions.Add(Action);
	}

	// ── DataAsset Actions ──────────────────────────────────────────────────
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	AssetRegistryModule.Get().GetAssetsByClass(UGorgeousInsightActionConfig_DA::StaticClass()->GetClassPathName(), AssetData);

	for (const FAssetData& Asset : AssetData)
	{
		if (UGorgeousInsightActionConfig_DA* Config = Cast<UGorgeousInsightActionConfig_DA>(Asset.GetAsset()))
		{
			// Only load actions for this provider or globally?
			// Since this is the Core provider, we can load ALL of them and pipe them in, or 
			// let each provider do it. Wait, the Insight Matrix gathers from all providers.
			// Let's filter by ProviderName if we want, but since Core is the main one, we can 
			// just append all of them to Core for now, or match ProviderName.
			if (Config->ProviderName == ProviderName() || Config->ProviderName.IsNone())
			{
				for (const FGorgeousBlueprintInsightAction& BlueprintAction : Config->Actions)
				{
					FGorgeousInsightAction Action;
					Action.Id = BlueprintAction.ActionId;
					Action.DisplayName = BlueprintAction.DisplayName;
					Action.Description = BlueprintAction.Description;
					Action.Category = BlueprintAction.Category;
					OutActions.Add(Action);
				}
			}
		}
	}

	// ── Blueprint Actions ──────────────────────────────────────────────────
	if (GEngine && GEngine->GetWorldContexts().Num() > 0)
	{
		if (UWorld* World = GEngine->GetWorldContexts()[0].World())
		{
			TArray<UGorgeousObjectVariable*> FoundOVs = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry();
			if (FoundOVs.Num() > 0)
			{
				for (UGorgeousObjectVariable* OV : FoundOVs)
				{
					if (UGorgeousInsightBlueprintStats_OV* StatsOV = Cast<UGorgeousInsightBlueprintStats_OV>(OV))
					{
						for (const auto& Pair : StatsOV->SystemStatsMap)
						{
							FName CategoryName = Pair.Key;
							for (const FGorgeousBlueprintStatsInsightAction& BPAction : Pair.Value.Actions)
							{
								FGorgeousInsightAction Action;
								Action.Id = FName(*FString::Printf(TEXT("Blueprint.%s.%s"), *CategoryName.ToString(), *BPAction.ActionName.ToString()));
								Action.DisplayName = FText::FromName(BPAction.ActionName);
								Action.Description = FText::FromString(FString::Printf(TEXT("Triggers Action via Signal Bridge: %s"), *BPAction.SignalBridgeTag.ToString()));
								Action.Category = CategoryName;
								OutActions.Add(Action);
							}
						}
					}
				}
			}
		}
	}
}

void FGorgeousCoreInsightMatrixProvider::ExecuteAction(FName ActionId, const FGorgeousInsightActionContext& Context)
{
	if (ActionId == ActionOpenOVBrowser)
	{
		SGorgeousObjectVariableBrowserWindow::Open();
		return;
	}

	// ── Blueprint Actions ──────────────────────────────────────────────────
	if (ActionId.ToString().StartsWith(TEXT("Blueprint.")))
	{
		UObject* ContextObj = Context.WorldContextObject;
		if (!ContextObj && GEngine && GEngine->GetWorldContexts().Num() > 0)
		{
			ContextObj = GEngine->GetWorldContexts()[0].World();
		}

		if (ContextObj)
		{
			TArray<UGorgeousObjectVariable*> FoundOVs = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry();
			if (FoundOVs.Num() > 0)
			{
				for (UGorgeousObjectVariable* OV : FoundOVs)
				{
					if (UGorgeousInsightBlueprintStats_OV* StatsOV = Cast<UGorgeousInsightBlueprintStats_OV>(OV))
					{
						for (const auto& Pair : StatsOV->SystemStatsMap)
						{
							FName CategoryName = Pair.Key;
							for (const FGorgeousBlueprintStatsInsightAction& BPAction : Pair.Value.Actions)
							{
								FName FormattedActionId = FName(*FString::Printf(TEXT("Blueprint.%s.%s"), *CategoryName.ToString(), *BPAction.ActionName.ToString()));
								if (FormattedActionId == ActionId)
								{
									// Dispatch via Signal Bridge
									FInstancedStruct EmptyPayload;
									USignalBridgeBlueprintFunctionLibrary::DispatchLocal(ContextObj, BPAction.SignalBridgeTag, EmptyPayload);
									return;
								}
							}
						}
					}
				}
			}
		}
	}

	// ── AutoReplication merged action handlers ───────────────────────────
	if (ActionId == ActionOpenTrafficInspector)
	{
		SGorgeousNetworkTrafficInspectorWindow::Open();
		return;
	}

	if (ActionId == ActionOpenRPCInspector)
	{
		SGorgeousRPCInspectorWindow::Open();
		return;
	}

	// ── DataAsset Actions ──────────────────────────────────────────────────
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	AssetRegistryModule.Get().GetAssetsByClass(UGorgeousInsightActionConfig_DA::StaticClass()->GetClassPathName(), AssetData);

	for (const FAssetData& Asset : AssetData)
	{
		if (UGorgeousInsightActionConfig_DA* Config = Cast<UGorgeousInsightActionConfig_DA>(Asset.GetAsset()))
		{
			for (const FGorgeousBlueprintInsightAction& BlueprintAction : Config->Actions)
			{
				if (BlueprintAction.ActionId == ActionId)
				{
					// Action Found!
#if WITH_EDITOR
					if (BlueprintAction.EditorWidgetClass.IsValid() || !BlueprintAction.EditorWidgetClass.IsNull())
					{
						UClass* LoadedClass = BlueprintAction.EditorWidgetClass.LoadSynchronous();
						if (LoadedClass && LoadedClass->IsChildOf(UEditorUtilityWidget::StaticClass()))
						{
							if (UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>())
							{
								if (UEditorUtilityWidgetBlueprint* WidgetBP = Cast<UEditorUtilityWidgetBlueprint>(LoadedClass->ClassGeneratedBy))
								{
									EditorUtilitySubsystem->SpawnAndRegisterTab(WidgetBP);
								}
								else
								{
									// It might be a native class or already compiled. Fallback:
									FName TabID = FName(*(FString(TEXT("BlueprintActionTab_")) + ActionId.ToString()));
									if (!FGlobalTabmanager::Get()->HasTabSpawner(TabID))
									{
										FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
											TabID,
											FOnSpawnTab::CreateLambda([LoadedClass](const FSpawnTabArgs& Args)
											{
												UWorld* World = GEditor->GetEditorWorldContext().World();
												UUserWidget* WidgetInstance = CreateWidget<UUserWidget>(World, LoadedClass);
												
												return SNew(SDockTab)
													.TabRole(ETabRole::NomadTab)
													[
														WidgetInstance->TakeWidget()
													];
											}))
											.SetDisplayName(BlueprintAction.DisplayName)
											.SetMenuType(ETabSpawnerMenuType::Hidden);
									}
									FGlobalTabmanager::Get()->TryInvokeTab(TabID);
								}
							}
						}
					}
#endif
					// Also handle RuntimeWidgetClass if playing in PIE / Game
					if (Context.WorldContextObject && Context.WorldContextObject->GetWorld())
					{
						if (BlueprintAction.RuntimeWidgetClass.IsValid() || !BlueprintAction.RuntimeWidgetClass.IsNull())
						{
							UClass* LoadedRuntimeClass = BlueprintAction.RuntimeWidgetClass.LoadSynchronous();
							if (LoadedRuntimeClass)
							{
								UUserWidget* WidgetInstance = CreateWidget<UUserWidget>(Context.WorldContextObject->GetWorld(), LoadedRuntimeClass);
								if (WidgetInstance)
								{
									WidgetInstance->AddToViewport();
								}
							}
						}
					}

					return; // Done executing
				}
			}
		}
	}
}

void FGorgeousCoreInsightMatrixProvider::GetTests(TArray<FGorgeousInsightTest>& OutTests) const
{
	const TArray<FGorgeousInsightScenarioDescriptor> Scenarios = FGorgeousInsightTestMatrix::GetRegisteredScenarios();
	for (const FGorgeousInsightScenarioDescriptor& Descriptor : Scenarios)
	{
		// Include both Core-tagged and AR-tagged scenarios
		if (!IsCoreScenario(Descriptor) && !IsARScenario(Descriptor))
		{
			continue;
		}

		FGorgeousInsightTest Test;
		Test.Id = Descriptor.ScenarioName;
		Test.DisplayName = FText::FromString(Descriptor.GetDisplayName());
		Test.Description = FText::FromString(Descriptor.Description);
		if (IsARScenario(Descriptor)) {
			Test.Category = FName(TEXT("AutoReplication"));
		} else if (Descriptor.Tags.Contains(TEXT("signal-bridge"))) {
			Test.Category = FName(TEXT("Signal Bridge"));
		} else {
			Test.Category = FName(TEXT("Tests"));
		}
		Test.Tags = Descriptor.Tags;
		Test.Inputs = Descriptor.Inputs;
		OutTests.Add(Test);
	}
}

FGorgeousInsightTestResult FGorgeousCoreInsightMatrixProvider::RunTest(const FName TestId, const FGorgeousInsightTestContext& Context)
{
	UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get();
	if (!Subsystem)
	{
		FGorgeousInsightTestResult Result;
		Result.bSuccess = false;
		Result.Errors.Add(TEXT("Insight Matrix subsystem unavailable"));
		return Result;
	}

	const FString Parameters = BuildParameterString(Context.Parameters);
	const FGorgeousInsightScenarioResult ScenarioResult = Subsystem->RunScenarioByName(TestId, Parameters, /*VariantIndex*/ 0, Context.WorldContextObject);
	return ConvertScenarioResult(ScenarioResult);
}
