// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

#include "CoreMinimal.h"

#if WITH_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/Parse.h"
#include "Misc/ScopeExit.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformMisc.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Engine/NetworkSettings.h"
#include "Net/NetworkMetricsConfig.h"
#include "Misc/ConfigCacheIni.h"
#include "GameFramework/WorldSettings.h"
#include "Math/RandomStream.h"
#include "Containers/Set.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "Helpers/GorgeousObjectVariablePerfTestTypes.h"
#include "GorgeousLoggingBlueprintFunctionLibrary.h"
#include "ModuleCore/GorgeousObjectVariableRootSettings.h"
#include "Serialization/ArchiveCountMem.h"
#include "UObject/Package.h"
#include "Engine/Level.h"
#include "Templates/UniquePtr.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTLS.h"
#include "HAL/CriticalSection.h"
#include "Engine/NetConnection.h"
#include "Engine/PendingNetGame.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Actor.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/SoftObjectPath.h"
#include "UnitTests/Helpers/GorgeousNetworkEmulationHelpers.h"
#include "InsightMatrix/GorgeousInsightHarness.h"

#ifndef GORGEOUS_OVPERF_ENABLE_HARNESS
#define GORGEOUS_OVPERF_ENABLE_HARNESS 0
#endif

struct FPerfServerClientHarnessOptions
{
	double ConnectTimeoutSeconds = 30.0;
	double TickStepSeconds = 1.0 / 120.0;
};

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousObjectVariablePerf, Log, All);

#if GORGEOUSCORERUNTIME_ENABLE_NETWORK_METRICS
void UGorgeousPerfNetworkMetricsListener::HandleReport(const UE::Net::FNetworkMetricSnapshot& Snapshot)
{
	CachedIntMetrics.Reset();
	for (const UE::Net::FNetworkMetric<int64>& Metric : Snapshot.MetricInts)
	{
		CachedIntMetrics.Add(Metric.Name, Metric.Value);
	}

	CachedFloatMetrics.Reset();
	for (const UE::Net::FNetworkMetric<float>& Metric : Snapshot.MetricFloats)
	{
		CachedFloatMetrics.Add(Metric.Name, Metric.Value);
	}
}
#endif

bool UGorgeousPerfNetworkMetricsListener::TryGetIntMetric(FName MetricName, int64& OutValue) const
{
	if (const int64* ValuePtr = CachedIntMetrics.Find(MetricName))
	{
		OutValue = *ValuePtr;
		return true;
	}
	return false;
}

bool UGorgeousPerfNetworkMetricsListener::TryGetFloatMetric(FName MetricName, float& OutValue) const
{
	if (const float* ValuePtr = CachedFloatMetrics.Find(MetricName))
	{
		OutValue = *ValuePtr;
		return true;
	}
	return false;
}

bool UGorgeousPerfNetworkMetricsListener::HasMetric(FName MetricName) const
{
	return CachedIntMetrics.Contains(MetricName) || CachedFloatMetrics.Contains(MetricName);
}

namespace GorgeousObjectVariablePerf
{
	enum class EPerfStatCategory : uint8
	{
		Scenario = 0,
		Performance,
		Replication,
		Networking,
		Memory,
		System,
		Cleanup
	};

	struct FPerfSummaryPanel
	{
		static constexpr int32 MinInnerWidth = 68;
		static constexpr int32 MaxInnerWidth = 108;
		inline static constexpr EPerfStatCategory CategoryOrder[] =
		{
			EPerfStatCategory::Scenario,
			EPerfStatCategory::Performance,
			EPerfStatCategory::Replication,
			EPerfStatCategory::Networking,
			EPerfStatCategory::Memory,
			EPerfStatCategory::System,
			EPerfStatCategory::Cleanup
		};

		struct FSection
		{
			EPerfStatCategory Category = EPerfStatCategory::Performance;
			FString Title;
			int32 Priority = 0;
			TArray<FString> Lines;
		};

		void InitializeDefaults()
		{
			for (EPerfStatCategory Category : CategoryOrder)
			{
				EnsureSection(Category);
			}
		}

		void AddLine(EPerfStatCategory Category, const FString& Line)
		{
			if (Line.IsEmpty())
			{
				return;
			}

			FSection& Section = EnsureSection(Category);
			Section.Lines.Add(NormalizeLine(Line));
		}

		FString BuildPanel() const
		{
			TArray<const FSection*> OrderedSections;
			OrderedSections.Reserve(UE_ARRAY_COUNT(CategoryOrder));
			bool bHasData = false;
			for (EPerfStatCategory Category : CategoryOrder)
			{
				if (const FSection* const Section = Sections.Find(Category))
				{
					OrderedSections.Add(Section);
					bHasData |= Section->Lines.Num() > 0;
				}
			}

			if (!bHasData)
			{
				return FString();
			}

			const int32 WrapWidth = MaxInnerWidth;
			TArray<FString> BodyLines;
			BodyLines.Add(TEXT("OVPerf Summary"));
			for (int32 SectionIndex = 0; SectionIndex < OrderedSections.Num(); ++SectionIndex)
			{
				const FSection* Section = OrderedSections[SectionIndex];
				AppendWrappedLine(FString::Printf(TEXT("[%s]"), *Section->Title), TEXT(""), TEXT(""), WrapWidth, BodyLines);
				if (Section->Lines.Num() == 0)
				{
					AppendWrappedLine(TEXT("(no entries recorded)"), TEXT("  • "), TEXT("    "), WrapWidth, BodyLines);
				}
				else
				{
					for (const FString& Line : Section->Lines)
					{
						AppendWrappedLine(Line, TEXT("  • "), TEXT("    "), WrapWidth, BodyLines);
					}
				}
				if (SectionIndex < OrderedSections.Num() - 1)
				{
					BodyLines.Add(TEXT(""));
				}
			}

			int32 InnerWidth = 0;
			for (const FString& Line : BodyLines)
			{
				InnerWidth = FMath::Max(InnerWidth, Line.Len());
			}
			InnerWidth = FMath::Clamp(InnerWidth, MinInnerWidth, MaxInnerWidth);
			const FString Horizontal = FString::ChrN(InnerWidth + 2, '-');
			TArray<FString> PanelLines;
			PanelLines.Add(TEXT("+") + Horizontal + TEXT("+"));
			for (const FString& Line : BodyLines)
			{
				FString Trimmed = Line;
				if (Trimmed.Len() > InnerWidth)
				{
					Trimmed = Trimmed.Left(InnerWidth);
				}
				const int32 Padding = InnerWidth - Trimmed.Len();
				PanelLines.Add(FString::Printf(TEXT("| %s%s |"), *Trimmed, *FString::ChrN(Padding, ' ')));
			}
			PanelLines.Add(TEXT("+") + Horizontal + TEXT("+"));
			return FString::Join(PanelLines, TEXT("\n"));
		}

	private:
		FSection& EnsureSection(EPerfStatCategory Category)
		{
			FSection& Section = Sections.FindOrAdd(Category);
			if (Section.Title.IsEmpty())
			{
				Section.Category = Category;
				Section.Title = GetTitle(Category);
				Section.Priority = GetPriority(Category);
			}
			return Section;
		}

		static FString NormalizeLine(const FString& Line)
		{
			FString Working = Line;
			Working.ReplaceInline(TEXT(" | "), TEXT("\n| "), ESearchCase::CaseSensitive);
			Working.ReplaceInline(TEXT(" -> "), TEXT("\n-> "), ESearchCase::CaseSensitive);
			return Working;
		}

		static void AppendWrappedLine(const FString& Text, const FString& FirstPrefix, const FString& ContinuationPrefix, int32 MaxWidth, TArray<FString>& OutLines)
		{
			const int32 EffectiveWidth = FMath::Max(MaxWidth, 1);
			TArray<FString> Segments;
			Text.ParseIntoArray(Segments, TEXT("\n"), false);
			if (Segments.Num() == 0)
			{
				Segments.Add(TEXT(""));
			}

			bool bFirstSegment = true;
			for (const FString& Segment : Segments)
			{
				AppendWrappedSegment(Segment, bFirstSegment ? FirstPrefix : ContinuationPrefix, ContinuationPrefix, EffectiveWidth, OutLines);
				bFirstSegment = false;
			}
		}

		static void AppendWrappedSegment(const FString& Segment, const FString& InitialPrefix, const FString& ContinuationPrefix, int32 EffectiveWidth, TArray<FString>& OutLines)
		{
			const int32 FirstPrefixLen = InitialPrefix.Len();
			const int32 ContinuationPrefixLen = ContinuationPrefix.Len();
			if (Segment.IsEmpty())
			{
				OutLines.Add(InitialPrefix);
				return;
			}

			int32 Offset = 0;
			bool bFirstLine = true;
			while (Offset < Segment.Len())
			{
				const FString& Prefix = bFirstLine ? InitialPrefix : ContinuationPrefix;
				const int32 PrefixLen = bFirstLine ? FirstPrefixLen : ContinuationPrefixLen;
				const int32 Available = EffectiveWidth - PrefixLen;
				if (Available <= 0)
				{
					break;
				}

				int32 Remaining = Segment.Len() - Offset;
				int32 Take = FMath::Min(Remaining, Available);
				if (Take < Remaining)
				{
					int32 BreakIndex = INDEX_NONE;
					for (int32 Scan = Offset + Take; Scan > Offset; --Scan)
					{
						if (FChar::IsWhitespace(Segment[Scan - 1]))
						{
							BreakIndex = Scan;
							break;
						}
					}
					if (BreakIndex != INDEX_NONE && BreakIndex > Offset)
					{
						Take = BreakIndex - Offset;
					}
				}

				FString Chunk = Segment.Mid(Offset, Take);
				Chunk.TrimEndInline();
				OutLines.Add(Prefix + Chunk);
				Offset += Take;
				while (Offset < Segment.Len() && FChar::IsWhitespace(Segment[Offset]))
				{
					++Offset;
				}
				bFirstLine = false;
			}
		}

		static FString GetTitle(EPerfStatCategory Category)
		{
			switch (Category)
			{
			case EPerfStatCategory::Scenario: return TEXT("Scenario");
			case EPerfStatCategory::Performance: return TEXT("Performance");
			case EPerfStatCategory::Replication: return TEXT("Replication");
			case EPerfStatCategory::Networking: return TEXT("Networking");
			case EPerfStatCategory::Memory: return TEXT("Memory");
			case EPerfStatCategory::System: return TEXT("System");
			case EPerfStatCategory::Cleanup: return TEXT("Cleanup");
			default: return TEXT("Misc");
			}
		}

		static int32 GetPriority(EPerfStatCategory Category)
		{
			switch (Category)
			{
			case EPerfStatCategory::Scenario: return 0;
			case EPerfStatCategory::Performance: return 10;
			case EPerfStatCategory::Replication: return 20;
			case EPerfStatCategory::Networking: return 30;
			case EPerfStatCategory::Memory: return 40;
			case EPerfStatCategory::System: return 50;
			case EPerfStatCategory::Cleanup: return 60;
			default: return 100;
			}
		}

		TMap<EPerfStatCategory, FSection> Sections;
	};

	struct FPerfScenario
	{
			int32 VariableCount = 300000;
			double SpawnBudgetSeconds = 90.0;
			double LookupBudgetSeconds = 12.0;
			double HierarchyBudgetSeconds = 45.0;
			int32 MaxBranchingFactor = 90;
			int32 AverageBranchingFactor = 45;
			int32 MaxDepth = 10;
			int32 DeepLookupSamples = 4096;
			int32 SubtreeSamples = 1024;
			int32 MemorySampleCount = 4096;
			double TextBloatFrequency = 0.35;
			int32 TextBloatCharLength = 131072;
			int32 TargetRootRegistries = 4;
			double CrossReferenceFrequency = 0.35;
			int32 MaxCrossReferencesPerNode = 4;
			int32 ReferenceHopSamples = 2048;
			int32 ReferenceHopMaxDepth = 48;
			double ReferenceHopBudgetSeconds = 20.0;
			int32 DisplayNameLookupSamples = 2048;
			int32 RegistrationValidationSamples = 2048;
			double ReplicationOptInFrequency = 0.1;
			int32 ReplicationValidationSamples = 512;
			double ReplicationValidationBudgetSeconds = 6.0;
			int32 ScenarioPreset = 3;
			int32 RandomSeed = 1337;
			int32 NetworkEmulationPreset = 2;

			bool HasNetworkEmulationPreset() const
			{
				return NetworkEmulationPreset >= 0;
			}

			void ApplyPresetDefaults()
			{
				auto ApplyCommonMidrange = [this](int32 Count, int32 AvgBranching, int32 MaxBranching, int32 Depth, double SpawnBudget, double LookupBudget, double HierarchyBudget)
				{
					VariableCount = Count;
					AverageBranchingFactor = AvgBranching;
					MaxBranchingFactor = MaxBranching;
					MaxDepth = Depth;
					SpawnBudgetSeconds = SpawnBudget;
					LookupBudgetSeconds = LookupBudget;
					HierarchyBudgetSeconds = HierarchyBudget;
				};

				switch (ScenarioPreset)
				{
				case 0: // Card game style lightweight tables
					ApplyCommonMidrange(1500, 2, 4, 3, 6.0, 1.5, 3.0);
					DeepLookupSamples = 128;
					SubtreeSamples = 64;
					MemorySampleCount = 64;
					TextBloatFrequency = 0.0;
					TextBloatCharLength = 0;
					TargetRootRegistries = 1;
					CrossReferenceFrequency = 0.05;
					MaxCrossReferencesPerNode = 1;
					ReferenceHopSamples = 64;
					ReferenceHopMaxDepth = 6;
					ReferenceHopBudgetSeconds = 0.75;
					DisplayNameLookupSamples = 128;
					RegistrationValidationSamples = 128;
					ReplicationOptInFrequency = 0.0;
					ReplicationValidationSamples = 0;
					ReplicationValidationBudgetSeconds = 1.0;
					break;

				case 1: // Narrative-heavy RPG scene
					ApplyCommonMidrange(20000, 4, 8, 5, 12.0, 2.5, 6.0);
					DeepLookupSamples = 512;
					SubtreeSamples = 256;
					MemorySampleCount = 256;
					TextBloatFrequency = 0.1;
					TextBloatCharLength = 16384;
					TargetRootRegistries = 2;
					CrossReferenceFrequency = 0.15;
					MaxCrossReferencesPerNode = 2;
					ReferenceHopSamples = 256;
					ReferenceHopMaxDepth = 12;
					ReferenceHopBudgetSeconds = 2.5;
					DisplayNameLookupSamples = 256;
					RegistrationValidationSamples = 256;
					ReplicationOptInFrequency = 0.05;
					ReplicationValidationSamples = 128;
					ReplicationValidationBudgetSeconds = 2.0;
					break;

				case 2: // Simulation builder mid-load
					ApplyCommonMidrange(80000, 12, 24, 8, 25.0, 4.0, 12.0);
					DeepLookupSamples = 1024;
					SubtreeSamples = 512;
					MemorySampleCount = 512;
					TextBloatFrequency = 0.2;
					TextBloatCharLength = 32768;
					TargetRootRegistries = 3;
					CrossReferenceFrequency = 0.2;
					MaxCrossReferencesPerNode = 3;
					ReferenceHopSamples = 512;
					ReferenceHopMaxDepth = 18;
					ReferenceHopBudgetSeconds = 6.0;
					DisplayNameLookupSamples = 512;
					RegistrationValidationSamples = 512;
					ReplicationOptInFrequency = 0.08;
					ReplicationValidationSamples = 192;
					ReplicationValidationBudgetSeconds = 3.0;
					break;

				case 3: // MMO shard moderate stress
					ApplyCommonMidrange(150000, 18, 36, 9, 50.0, 7.0, 18.0);
					DeepLookupSamples = 2048;
					SubtreeSamples = 768;
					MemorySampleCount = 1024;
					TextBloatFrequency = 0.25;
					TextBloatCharLength = 65536;
					TargetRootRegistries = 3;
					CrossReferenceFrequency = 0.28;
					MaxCrossReferencesPerNode = 4;
					ReferenceHopSamples = 1024;
					ReferenceHopMaxDepth = 24;
					ReferenceHopBudgetSeconds = 9.0;
					DisplayNameLookupSamples = 1024;
					RegistrationValidationSamples = 1024;
					ReplicationOptInFrequency = 0.12;
					ReplicationValidationSamples = 256;
					ReplicationValidationBudgetSeconds = 4.5;
					break;

				case 4: // Sandbox builder (previous hard default)
					ApplyCommonMidrange(220000, 30, 60, 10, 70.0, 9.0, 30.0);
					DeepLookupSamples = 3072;
					SubtreeSamples = 896;
					MemorySampleCount = 2048;
					TextBloatFrequency = 0.3;
					TextBloatCharLength = 98304;
					TargetRootRegistries = 4;
					CrossReferenceFrequency = 0.32;
					MaxCrossReferencesPerNode = 4;
					ReferenceHopSamples = 1536;
					ReferenceHopMaxDepth = 32;
					ReferenceHopBudgetSeconds = 12.0;
					DisplayNameLookupSamples = 1536;
					RegistrationValidationSamples = 1536;
					ReplicationOptInFrequency = 0.18;
					ReplicationValidationSamples = 512;
					ReplicationValidationBudgetSeconds = 6.5;
					break;

				case 5: // Ultra heavy variable mesh (default)
				default:
					ApplyCommonMidrange(300000, 45, 90, 10, 90.0, 12.0, 45.0);
					DeepLookupSamples = 4096;
					SubtreeSamples = 1024;
					MemorySampleCount = 4096;
					TextBloatFrequency = 0.35;
					TextBloatCharLength = 131072;
					TargetRootRegistries = 4;
					CrossReferenceFrequency = 0.35;
					MaxCrossReferencesPerNode = 5;
					ReferenceHopSamples = 2048;
					ReferenceHopMaxDepth = 48;
					ReferenceHopBudgetSeconds = 20.0;
					DisplayNameLookupSamples = 2048;
					RegistrationValidationSamples = 2048;
					ReplicationOptInFrequency = 0.25;
					ReplicationValidationSamples = 1024;
					ReplicationValidationBudgetSeconds = 9.0;
					break;
				}
			}

			static FString DescribePresetLabel(int32 Preset)
			{
				switch (Preset)
				{
				case 0: return TEXT("CardGame");
				case 1: return TEXT("NarrativeRPG");
				case 2: return TEXT("SimBuilder");
				case 3: return TEXT("MMOShard");
				case 4: return TEXT("Sandbox");
				case 5: return TEXT("UltraHeavy");
				default: return FString::Printf(TEXT("Preset_%d"), Preset);
				}
			}

			static FString DescribeNetworkPresetLabel(int32 Preset)
			{
				switch (Preset)
				{
				case 0: return TEXT("Pristine");
				case 1: return TEXT("ResidentialWiFi");
				case 2: return TEXT("BusyCafe");
				case 3: return TEXT("CongestedMobile");
				case 4: return TEXT("Severe");
				case 5: return TEXT("Chaos");
				default: return TEXT("Disabled");
				}
			}

		static FPerfScenario FromParameters(const FString& Parameters)
		{
			FPerfScenario Scenario;
				Scenario.ApplyPresetDefaults();

				int32 ParsedPreset = Scenario.ScenarioPreset;
				if (FParse::Value(*Parameters, TEXT("Preset="), ParsedPreset))
				{
					Scenario.ScenarioPreset = FMath::Clamp(ParsedPreset, 0, 5);
					Scenario.ApplyPresetDefaults();
				}

			int32 ParsedCount = 0;
			if (FParse::Value(*Parameters, TEXT("Num="), ParsedCount))
			{
				Scenario.VariableCount = FMath::Clamp(ParsedCount, 1, 500000);
			}

			double ParsedBudget = 0.0;
			if (FParse::Value(*Parameters, TEXT("SpawnBudget="), ParsedBudget))
			{
				Scenario.SpawnBudgetSeconds = FMath::Max(ParsedBudget, 0.1);
			}

			if (FParse::Value(*Parameters, TEXT("LookupBudget="), ParsedBudget))
			{
				Scenario.LookupBudgetSeconds = FMath::Max(ParsedBudget, 0.1);
			}

			if (FParse::Value(*Parameters, TEXT("HierarchyBudget="), ParsedBudget))
			{
				Scenario.HierarchyBudgetSeconds = FMath::Max(ParsedBudget, 0.1);
			}

			int32 ParsedInt = 0;
			double ParsedChance = 0.0;
			if (FParse::Value(*Parameters, TEXT("Branching="), ParsedInt))
			{
				Scenario.MaxBranchingFactor = FMath::Clamp(ParsedInt, 2, 128);
			}

			if (FParse::Value(*Parameters, TEXT("AvgBranching="), ParsedInt))
			{
				Scenario.AverageBranchingFactor = FMath::Clamp(ParsedInt, 1, Scenario.MaxBranchingFactor);
			}

			if (FParse::Value(*Parameters, TEXT("Depth="), ParsedInt))
			{
				Scenario.MaxDepth = FMath::Clamp(ParsedInt, 2, 12);
			}

			if (FParse::Value(*Parameters, TEXT("DeepLookups="), ParsedInt))
			{
				Scenario.DeepLookupSamples = FMath::Clamp(ParsedInt, 1, 50000);
			}

			if (FParse::Value(*Parameters, TEXT("SubtreeSamples="), ParsedInt))
			{
				Scenario.SubtreeSamples = FMath::Clamp(ParsedInt, 1, 5000);
			}

			if (FParse::Value(*Parameters, TEXT("MemorySamples="), ParsedInt))
			{
				Scenario.MemorySampleCount = FMath::Clamp(ParsedInt, 1, 100000);
			}

			if (FParse::Value(*Parameters, TEXT("Roots="), ParsedInt))
			{
				Scenario.TargetRootRegistries = FMath::Clamp(ParsedInt, 1, 32);
			}

			if (FParse::Value(*Parameters, TEXT("CrossRefChance="), ParsedChance))
			{
				if (ParsedChance > 1.0)
				{
					ParsedChance *= 0.01;
				}
				Scenario.CrossReferenceFrequency = FMath::Clamp(ParsedChance, 0.0, 1.0);
			}

			if (FParse::Value(*Parameters, TEXT("CrossRefMax="), ParsedInt))
			{
				Scenario.MaxCrossReferencesPerNode = FMath::Clamp(ParsedInt, 0, 32);
			}

			if (FParse::Value(*Parameters, TEXT("RefSamples="), ParsedInt))
			{
				Scenario.ReferenceHopSamples = FMath::Clamp(ParsedInt, 0, 50000);
			}

			if (FParse::Value(*Parameters, TEXT("RefDepth="), ParsedInt))
			{
				Scenario.ReferenceHopMaxDepth = FMath::Clamp(ParsedInt, 1, 256);
			}

			double ParsedRefBudget = 0.0;
			if (FParse::Value(*Parameters, TEXT("RefBudget="), ParsedRefBudget))
			{
				Scenario.ReferenceHopBudgetSeconds = FMath::Max(ParsedRefBudget, 0.1);
			}

			if (FParse::Value(*Parameters, TEXT("DisplayLookups="), ParsedInt))
			{
				Scenario.DisplayNameLookupSamples = FMath::Clamp(ParsedInt, 0, 50000);
			}

			if (FParse::Value(*Parameters, TEXT("RegistrationChecks="), ParsedInt))
			{
				Scenario.RegistrationValidationSamples = FMath::Clamp(ParsedInt, 0, 50000);
			}

			if (FParse::Value(*Parameters, TEXT("Seed="), ParsedInt))
			{
				Scenario.RandomSeed = ParsedInt;
			}

			if (FParse::Value(*Parameters, TEXT("BloatChance="), ParsedChance))
			{
				if (ParsedChance > 1.0)
				{
					ParsedChance *= 0.01;
				}
				Scenario.TextBloatFrequency = FMath::Clamp(ParsedChance, 0.0, 1.0);
			}

			if (FParse::Value(*Parameters, TEXT("BloatChars="), ParsedInt))
			{
				Scenario.TextBloatCharLength = FMath::Clamp(ParsedInt, 0, 2 * 1024 * 1024);
			}

			if (FParse::Value(*Parameters, TEXT("ReplicationChance="), ParsedChance))
			{
				if (ParsedChance > 1.0)
				{
					ParsedChance *= 0.01;
				}
				Scenario.ReplicationOptInFrequency = FMath::Clamp(ParsedChance, 0.0, 1.0);
			}

			if (FParse::Value(*Parameters, TEXT("ReplicationSamples="), ParsedInt))
			{
				Scenario.ReplicationValidationSamples = FMath::Clamp(ParsedInt, 0, 50000);
			}

			double ParsedReplicationBudget = 0.0;
			if (FParse::Value(*Parameters, TEXT("ReplicationBudget="), ParsedReplicationBudget))
			{
				Scenario.ReplicationValidationBudgetSeconds = FMath::Max(ParsedReplicationBudget, 0.1);
			}

			int32 ParsedNetworkPreset = Scenario.NetworkEmulationPreset;
			if (FParse::Value(*Parameters, TEXT("NetworkPreset="), ParsedNetworkPreset) || FParse::Value(*Parameters, TEXT("NetPreset="), ParsedNetworkPreset))
			{
				Scenario.NetworkEmulationPreset = FMath::Clamp(ParsedNetworkPreset, -1, 5);
			}

			Scenario.AverageBranchingFactor = FMath::Clamp(Scenario.AverageBranchingFactor, 1, Scenario.MaxBranchingFactor);

			return Scenario;
		}
	};

	struct FPerfSample
	{
		double Seconds = 0.0;
		int32 Count = 0;
	};

	struct FSpawnRecord
	{
		UGorgeousObjectVariable* Node = nullptr;
		FGuid Identifier;
		int32 Depth = 0;
		UGorgeousRootObjectVariable* Root = nullptr;
		FName RootName = NAME_None;
		FName ResolvedRootKey = NAME_None;
			FName DisplayName = NAME_None;
		bool bNetworkingOptIn = false;
		bool bEnforcedNetworking = false;
		bool bScenarioNetworkingEnabled = false;
		bool bHasReplicatedOwner = false;
		int32 StimulusBaselineSequence = 0;
	};

	struct FReplicationTimingBucket
	{
		double TotalSeconds = 0.0;
		double MaxSeconds = 0.0;
		double MinSeconds = TNumericLimits<double>::Max();
		int32 Samples = 0;

		void AddSample(const double Seconds)
		{
			if (Seconds < 0.0)
			{
				return;
			}

			TotalSeconds += Seconds;
			MaxSeconds = FMath::Max(MaxSeconds, Seconds);
			MinSeconds = FMath::Min(MinSeconds, Seconds);
			++Samples;
		}

		bool HasSamples() const
		{
			return Samples > 0;
		}

		double AverageMilliseconds() const
		{
			return HasSamples() ? (TotalSeconds / Samples) * 1000.0 : 0.0;
		}

		double MinMilliseconds() const
		{
			return HasSamples() ? MinSeconds * 1000.0 : 0.0;
		}

		double MaxMilliseconds() const
		{
			return HasSamples() ? MaxSeconds * 1000.0 : 0.0;
		}
	};

	struct FReplicationTimingAccumulator
	{
		FReplicationTimingBucket ActivationTiming;
		FReplicationTimingBucket ValidationTiming;
		double SimulatedNetworkSeconds = 0.0;
		int32 SimulatedLatencySamples = 0;
		int32 SimulatedDrops = 0;
		int32 SimulatedDeliveries = 0;
		int32 SimulatedFailures = 0;

		void RecordActivation(const double Seconds)
		{
			ActivationTiming.AddSample(Seconds);
		}

		void RecordValidation(const double Seconds)
		{
			ValidationTiming.AddSample(Seconds);
		}

		void RecordSimulatedLatency(const double Seconds)
		{
			if (Seconds < 0.0)
			{
				return;
			}

			SimulatedNetworkSeconds += Seconds;
			++SimulatedLatencySamples;
		}

		void RecordSimulatedDrop()
		{
			++SimulatedDrops;
		}

		void RecordSimulatedOutcome(const bool bDelivered)
		{
			if (bDelivered)
			{
				++SimulatedDeliveries;
			}
			else
			{
				++SimulatedFailures;
			}
		}
	};

	static FGorgeousNetworkEmulation::FRuntimeProfile InitializeNetworkEmulation(const FPerfScenario& Scenario)
	{
		if (!Scenario.HasNetworkEmulationPreset())
		{
			return FGorgeousNetworkEmulation::FRuntimeProfile();
		}

		return FGorgeousNetworkEmulation::ApplyPreset(Scenario.NetworkEmulationPreset);
	}

	struct FActiveRootContext
	{
		UGorgeousRootObjectVariable* Root = nullptr;
		FName RootName = NAME_None;
		FName ResolvedRootKey = NAME_None;
	};

	struct FRootRegistryStats
	{
		UGorgeousRootObjectVariable* Root = nullptr;
		FName RootName = NAME_None;
		FName ResolvedRootKey = NAME_None;
		int32 SpawnedCount = 0;
		SIZE_T TotalExclusiveBytes = 0;
		SIZE_T TotalInclusiveBytes = 0;
		bool bNetworkingMetadataInitialized = false;
		bool bSupportsNetworking = false;
		bool bEnforcesNetworking = false;
		int32 NetworkingOptInCount = 0;
		int32 NetworkingActiveCount = 0;
		int32 EnforcedNetworkingViolations = 0;
		FReplicationTimingBucket ActivationTiming;
		FReplicationTimingBucket ValidationTiming;
	};

	static void InitializeRootNetworkingMetadata(FRootRegistryStats& Entry);
	static FName ResolveMetadataRootName(const FName RawName);

	static FRootRegistryStats& FindOrAddRootStats(TArray<FRootRegistryStats>& Stats, UGorgeousRootObjectVariable* Root, const FName RootName, const FName ResolvedRootKey)
	{
		for (FRootRegistryStats& Entry : Stats)
		{
			if (Entry.Root == Root)
			{
				InitializeRootNetworkingMetadata(Entry);
				return Entry;
			}
		}

		FRootRegistryStats& NewEntry = Stats.AddDefaulted_GetRef();
		NewEntry.Root = Root;
		NewEntry.RootName = RootName;
		NewEntry.ResolvedRootKey = ResolveMetadataRootName(!ResolvedRootKey.IsNone() ? ResolvedRootKey : RootName);
		InitializeRootNetworkingMetadata(NewEntry);
		return NewEntry;
	}

	static FString DescribeRootLabel(const FName RootName)
	{
		return RootName.IsNone() ? TEXT("DefaultRoot") : RootName.ToString();
	}

	static FName ResolveMetadataRootName(const FName RawName)
	{
		if (!RawName.IsNone())
		{
			return RawName;
		}

		if (const UGorgeousObjectVariableRootSettings* Settings = UGorgeousObjectVariableRootSettings::Get())
		{
			const FName DefaultName = Settings->GetDefaultRootName();
			if (!DefaultName.IsNone())
			{
				return DefaultName;
			}
		}

		return FName(TEXT("DefaultRoot"));
	}

	static void InitializeRootNetworkingMetadata(FRootRegistryStats& Entry)
	{
		if (Entry.bNetworkingMetadataInitialized)
		{
			return;
		}

		FName LookupName = Entry.ResolvedRootKey;
		if (LookupName.IsNone())
		{
			LookupName = ResolveMetadataRootName(NAME_None);
		}

		if (const UGorgeousObjectVariableRootSettings* Settings = UGorgeousObjectVariableRootSettings::Get())
		{
			if (!LookupName.IsNone())
			{
				if (const FGorgeousObjectVariableRootEntry* RootEntry = Settings->FindRootEntry(LookupName))
				{
					Entry.ResolvedRootKey = LookupName;
					Entry.bSupportsNetworking = RootEntry->bSupportsNetworking;
					Entry.bEnforcesNetworking = RootEntry->bEnforceNetworking;
					Entry.bNetworkingMetadataInitialized = true;
					return;
				}
			}
		}

		Entry.bNetworkingMetadataInitialized = true;
	}

	static FString DescribeSample(const TCHAR* Label, const FPerfSample& Sample)
	{
		const double Milliseconds = Sample.Seconds * 1000.0;
		const double Rate = (Sample.Seconds > SMALL_NUMBER && Sample.Count > 0) ? (static_cast<double>(Sample.Count) / Sample.Seconds) : 0.0;
		return FString::Printf(TEXT("%s -> %.2f ms total (%.0f ops/s) across %d operations"), Label, Milliseconds, Rate, Sample.Count);
	}

	static FString DescribeTimingBucket(const TCHAR* Label, const FReplicationTimingBucket& Bucket)
	{
		if (!Bucket.HasSamples())
		{
			return FString::Printf(TEXT("%s -> no samples"), Label);
		}

		return FString::Printf(TEXT("%s -> avg %.3f ms (min %.3f ms, max %.3f ms, samples %d)"),
			Label,
			Bucket.AverageMilliseconds(),
			Bucket.MinMilliseconds(),
			Bucket.MaxMilliseconds(),
			Bucket.Samples);
	}

		struct FSystemStatSnapshot
		{
			double TimestampSeconds = 0.0;
			FPlatformMemoryStats MemoryStats{};
			FProcessIOStats IOStats{};
			bool bHasIO = false;
			FCPUTime ProcessCPU{ 0.0f, 0.0f };
			FCPUTime ThreadCPU{ 0.0f, 0.0f };
			double LastProcessCPUSeconds = 0.0;
			double LastThreadCPUSeconds = 0.0;
		};

		static FSystemStatSnapshot CaptureSystemStatsSnapshot()
		{
			FSystemStatSnapshot Snapshot;
			FPlatformTime::UpdateCPUTime(0.0f);
			FPlatformTime::UpdateThreadCPUTime();
			Snapshot.TimestampSeconds = FPlatformTime::Seconds();
			Snapshot.ProcessCPU = FPlatformTime::GetCPUTime();
			Snapshot.ThreadCPU = FPlatformTime::GetThreadCPUTime();
			Snapshot.LastProcessCPUSeconds = FPlatformTime::GetLastIntervalCPUTimeInSeconds();
#if PLATFORM_WINDOWS
			Snapshot.LastThreadCPUSeconds = FPlatformTime::GetLastIntervalThreadCPUTimeInSeconds();
#else
			Snapshot.LastThreadCPUSeconds = 0.0;
#endif
			Snapshot.MemoryStats = FPlatformMemory::GetStats();
			Snapshot.bHasIO = FPlatformMisc::GetBlockingIOStats(Snapshot.IOStats, EInputOutputFlags::All);
			return Snapshot;
		}

		static FString DescribeMemoryUsage(const FPlatformMemoryStats& Stats)
		{
			const auto ToGB = [](uint64 Bytes)
			{
				return static_cast<double>(Bytes) / (1024.0 * 1024.0 * 1024.0);
			};
			return FString::Printf(TEXT("PhysUsed=%.2fGB (Peak %.2fGB) | VirtUsed=%.2fGB (Peak %.2fGB)"),
				ToGB(Stats.UsedPhysical), ToGB(Stats.PeakUsedPhysical), ToGB(Stats.UsedVirtual), ToGB(Stats.PeakUsedVirtual));
		}

		static FString DescribeMemoryDelta(const FPlatformMemoryStats& Before, const FPlatformMemoryStats& After)
		{
			const auto ToMB = [](int64 Bytes)
			{
				return static_cast<double>(Bytes) / (1024.0 * 1024.0);
			};
			return FString::Printf(TEXT("ΔPhys=%.2fMB ΔVirt=%.2fMB"),
				ToMB(static_cast<int64>(After.UsedPhysical) - static_cast<int64>(Before.UsedPhysical)),
				ToMB(static_cast<int64>(After.UsedVirtual) - static_cast<int64>(Before.UsedVirtual)));
		}

		static FString DescribeIOStats(const FProcessIOStats& Stats)
		{
			const auto ToMB = [](uint64 Bytes)
			{
				return static_cast<double>(Bytes) / (1024.0 * 1024.0);
			};
			return FString::Printf(TEXT("InOps=%llu OutOps=%llu OtherOps=%llu | In=%.2fMB Out=%.2fMB Other=%.2fMB"),
				Stats.BlockingInput, Stats.BlockingOutput, Stats.BlockingOther,
				ToMB(Stats.InputBytes), ToMB(Stats.OutputBytes), ToMB(Stats.OtherBytes));
		}

		static FString DescribeIODelta(const FProcessIOStats& Before, const FProcessIOStats& After)
		{
			FProcessIOStats Delta;
			Delta.BlockingInput = After.BlockingInput - Before.BlockingInput;
			Delta.BlockingOutput = After.BlockingOutput - Before.BlockingOutput;
			Delta.BlockingOther = After.BlockingOther - Before.BlockingOther;
			Delta.InputBytes = After.InputBytes - Before.InputBytes;
			Delta.OutputBytes = After.OutputBytes - Before.OutputBytes;
			Delta.OtherBytes = After.OtherBytes - Before.OtherBytes;
			return DescribeIOStats(Delta);
		}

		static void LogSystemDelta(const TCHAR* Label, const FSystemStatSnapshot& Before, const FSystemStatSnapshot& After)
		{
			UE_LOG(LogGorgeousObjectVariablePerf, Display,
				TEXT("[OVPerf][Sys] %s | Mem %s (%s) | CPU %.2f%%->%.2f%% Thread %.2f%%->%.2f%% | LastCPU %.4fs LastThread %.4fs"),
				Label,
				*DescribeMemoryUsage(After.MemoryStats), *DescribeMemoryDelta(Before.MemoryStats, After.MemoryStats),
				Before.ProcessCPU.CPUTimePct, After.ProcessCPU.CPUTimePct,
				Before.ThreadCPU.CPUTimePct, After.ThreadCPU.CPUTimePct,
				After.LastProcessCPUSeconds, After.LastThreadCPUSeconds);
			if (Before.bHasIO && After.bHasIO)
			{
				UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf][Sys] %s | IO %s"), Label, *DescribeIODelta(Before.IOStats, After.IOStats));
			}
			else
			{
				UE_LOG(LogGorgeousObjectVariablePerf, Verbose, TEXT("[OVPerf][Sys] %s | IO stats unavailable on this platform"), Label);
			}
		}

	static int32 EnumerateSubtree(UGorgeousObjectVariable* Node)
	{
		if (!IsValid(Node))
		{
			return 0;
		}

		int32 Count = 0;
		TArray<UGorgeousObjectVariable*> Stack;
		Stack.Reserve(64);
		Stack.Push(Node);
		while (Stack.Num() > 0)
		{
			UGorgeousObjectVariable* Current = Stack.Pop(EAllowShrinking::No);
			if (!IsValid(Current))
			{
				continue;
			}

			++Count;
			for (auto& [Key, Child] : Current->VariableRegistry)
			{
				if (IsValid(Child))
				{
					Stack.Push(Child.Get());
				}
			}
		}

		return Count;
	}

	#if WITH_SERVER_CODE && GORGEOUS_OVPERF_ENABLE_HARNESS

	struct FAutomationWorldScope
	{
		bool Create(const FString& InLabel, EWorldType::Type InWorldType, FString& OutError);
		void Tick(float DeltaSeconds);
		void Destroy();

		FWorldContext* WorldContext = nullptr;
		TObjectPtr<UWorld> World = nullptr;
		FString Label;
	};

	struct FPerfNetEndpoint;
	static void ClearEndpointPendingNetGame(FPerfNetEndpoint& Endpoint);
	static bool CompletePendingClientTravel(FPerfNetEndpoint& Endpoint);
	static void CaptureEndpointStatBaseline(FPerfNetEndpoint& Endpoint);

	struct FPerfNetEndpoint
	{
		struct FStatBaseline
		{
			uint64 BaseInBytes = 0;
			uint64 BaseOutBytes = 0;
			uint64 BaseInPackets = 0;
			uint64 BaseOutPackets = 0;
			bool bValid = false;
		};

		void Reset()
		{
			NetDriver = nullptr;
			NetDriverName = NAME_None;
			ClearEndpointPendingNetGame(*this);
			HarnessGameInstance.Reset();

#if GORGEOUSCORERUNTIME_ENABLE_NETWORK_METRICS
			MetricsCollector.Reset();
	#endif
			Scope.Destroy();
			StatBaseline = {};
	#if GORGEOUSCORERUNTIME_ENABLE_NETWORK_METRICS
			RegisteredMetricConfigs.Reset();
			bMetricsCollectorRegistered = false;
	#endif
		}

		FAutomationWorldScope Scope;
		FName NetDriverName = NAME_None;
		UNetDriver* NetDriver = nullptr;
		TStrongObjectPtr<UPendingNetGame> PendingNetGame;
		TStrongObjectPtr<UGameInstance> HarnessGameInstance;
		FStatBaseline StatBaseline;
	#if GORGEOUSCORERUNTIME_ENABLE_NETWORK_METRICS
		TStrongObjectPtr<UGorgeousPerfNetworkMetricsListener> MetricsCollector;
		TArray<FNetworkMetricConfig> RegisteredMetricConfigs;
		bool bMetricsCollectorRegistered = false;
	#endif
	};

	struct FPerfServerClientHarness
	{
		struct FDriverStatSnapshot
		{
			FString Label = TEXT("Harness endpoint");
			uint64 InPackets = 0;
			uint64 OutPackets = 0;
			double InBytes = 0.0;
			double OutBytes = 0.0;
			uint64 InBunches = 0;
			uint64 OutBunches = 0;
			uint64 InPacketsLost = 0;
			uint64 OutPacketsLost = 0;
			double InRateBytesPerSecond = 0.0;
			double OutRateBytesPerSecond = 0.0;
			double InPacketsPerSecond = 0.0;
			double OutPacketsPerSecond = 0.0;
			double ObservedSeconds = 0.0;
			double AvgPingMs = 0.0;
			float AvgJitterMs = 0.0f;
			float InLossPercent = 0.0f;
			float OutLossPercent = 0.0f;
			bool bIsUsingIris = false;
			bool bHasConnection = false;
			EConnectionState ConnectionState = USOCK_Invalid;
			bool bValid = false;

			FString Describe() const;
		};

		struct FCollectedStats
		{
			uint64 OutgoingPackets = 0;
			uint64 IncomingPackets = 0;
			double OutgoingBytes = 0.0;
			double IncomingBytes = 0.0;
			FDriverStatSnapshot ServerSnapshot;
			FDriverStatSnapshot ClientSnapshot;
		};

		bool Initialize(const FPerfServerClientHarnessOptions& Options, FString& OutError);
		void Tick(float DeltaSeconds, int32 Iterations);
		void Shutdown();
		bool IsActive() const { return bInitialized; }
		bool HasLiveConnection() const;
		int32 GetServerPort() const { return ServerPort; }
		FCollectedStats GatherStats() const;
		void CollectRegisteredNetworkMetrics(TArray<FString>& OutLines) const;

	private:
		bool BootstrapServer(FString& OutError);
		bool BootstrapClient(FString& OutError);
		bool WaitForHandshake(double TimeoutSeconds, double TickStepSeconds, FString& OutError);
		bool PumpOnce(float DeltaSeconds);
		int32 AllocateListenPort();
		void EnableStatCollection();
		static FDriverStatSnapshot CaptureNetDriverSnapshot(const TCHAR* Label, UNetDriver* NetDriver, const FPerfNetEndpoint::FStatBaseline* Baseline, double ElapsedSeconds);

	private:
		FPerfNetEndpoint Server;
		FPerfNetEndpoint Client;
		int32 ServerPort = 0;
		bool bInitialized = false;
		bool bStatsCollectionEnabled = false;
		double StatsEnableTimeSeconds = 0.0;
		FString ListenAddress = TEXT("127.0.0.1");
	};

	#else

	struct FPerfServerClientHarness
	{
		bool Initialize(const FPerfServerClientHarnessOptions&, FString& OutError)
		{
			OutError = TEXT("Server/client harness unavailable in this build.");
			return false;
		}

		void Tick(float, int32) {}
		void Shutdown() {}
		bool IsActive() const { return false; }
		bool HasLiveConnection() const { return false; }
		int32 GetServerPort() const { return 0; }

		struct FCollectedStats
		{
			uint64 OutgoingPackets = 0;
			uint64 IncomingPackets = 0;
			double OutgoingBytes = 0.0;
			double IncomingBytes = 0.0;
		};

		FCollectedStats GatherStats() const { return {}; }
		void CollectRegisteredNetworkMetrics(TArray<FString>&) const {}
	};

	#endif // WITH_SERVER_CODE && GORGEOUS_OVPERF_ENABLE_HARNESS

	#if WITH_SERVER_CODE && GORGEOUS_OVPERF_ENABLE_HARNESS

	namespace
	{
		void EnsureHarnessGameInstance(FPerfNetEndpoint& Endpoint, const TCHAR* DebugLabel)
		{
			FWorldContext* WorldContext = Endpoint.Scope.WorldContext;
			UWorld* World = Endpoint.Scope.World;
			if (!WorldContext || !World)
			{
				return;
			}

			UGameInstance* TargetInstance = World->GetGameInstance();
			if (!TargetInstance)
			{
				if (!Endpoint.HarnessGameInstance.IsValid())
				{
					UGorgeousHarnessGameInstance* NewInstance = NewObject<UGorgeousHarnessGameInstance>(GetTransientPackage());
					if (NewInstance)
					{
						NewInstance->AttachWorldContext(WorldContext);
						NewInstance->Init();
						Endpoint.HarnessGameInstance = TStrongObjectPtr<UGameInstance>(NewInstance);
					}
					else
					{
						UE_LOG(LogGorgeousObjectVariablePerf, Warning, TEXT("[OVPerf] %s harness endpoint failed to allocate GameInstance"), DebugLabel);
					}
				}

				TargetInstance = Endpoint.HarnessGameInstance.Get();
			}

			if (TargetInstance)
			{
				WorldContext->OwningGameInstance = TargetInstance;
				World->SetGameInstance(TargetInstance);
			}
			else
			{
				UE_LOG(LogGorgeousObjectVariablePerf, Warning, TEXT("[OVPerf] %s harness endpoint missing GameInstance"), DebugLabel);
			}
		}

		static constexpr int32 GPerfHarnessPortMin = 42000;
		static constexpr int32 GPerfHarnessPortMax = 45000;
		static FCriticalSection GHarnessPortMutex;

		ENetworkMetricEnableMode ParseMetricEnableMode(const FString& ModeString)
		{
			if (ModeString.Equals(TEXT("EnableForIrisOnly"), ESearchCase::IgnoreCase))
			{
				return ENetworkMetricEnableMode::EnableForIrisOnly;
			}
			if (ModeString.Equals(TEXT("EnableForNonIrisOnly"), ESearchCase::IgnoreCase))
			{
				return ENetworkMetricEnableMode::EnableForNonIrisOnly;
			}
			return ENetworkMetricEnableMode::EnableForAllReplication;
		}

	#if GORGEOUSCORERUNTIME_ENABLE_NETWORK_METRICS
		bool ParseNetworkMetricConfigEntry(const FString& RawEntry, FNetworkMetricConfig& OutConfig)
		{
			FString WorkingEntry = RawEntry;
			WorkingEntry.TrimStartAndEndInline();
			if (WorkingEntry.StartsWith(TEXT("(")) && WorkingEntry.EndsWith(TEXT(")")) && WorkingEntry.Len() >= 2)
			{
				WorkingEntry = WorkingEntry.Mid(1, WorkingEntry.Len() - 2);
			}

			TMap<FString, FString> KeyValuePairs;
			TArray<FString> Tokens;
			WorkingEntry.ParseIntoArray(Tokens, TEXT(","), true);
			for (FString Token : Tokens)
			{
				Token.TrimStartAndEndInline();
				FString Key;
				FString Value;
				if (!Token.Split(TEXT("="), &Key, &Value))
				{
					continue;
				}

				Key.TrimStartAndEndInline();
				Key = Key.ToLower();
				Value.TrimStartAndEndInline();
				if (!KeyValuePairs.Contains(Key))
				{
					KeyValuePairs.Add(Key, Value);
				}
			}

			const FString* MetricNameStr = KeyValuePairs.Find(TEXT("metricname"));
			if (!MetricNameStr || MetricNameStr->IsEmpty())
			{
				return false;
			}

			OutConfig = FNetworkMetricConfig();
			OutConfig.MetricName = FName(**MetricNameStr);

			const FString* ClassNameStr = KeyValuePairs.Find(TEXT("classname"));
			const FString* ClassStr = ClassNameStr ? ClassNameStr : KeyValuePairs.Find(TEXT("class"));
			if (ClassStr && !ClassStr->IsEmpty())
			{
				OutConfig.Class = TSoftClassPtr<UNetworkMetricsBaseListener>(FSoftObjectPath(**ClassStr));
			}

			if (const FString* EnableModeStr = KeyValuePairs.Find(TEXT("enablemode")))
			{
				OutConfig.EnableMode = ParseMetricEnableMode(*EnableModeStr);
			}

			return true;
		}

		bool LoadHarnessNetworkMetricConfigs(TArray<FNetworkMetricConfig>& OutConfigs)
		{
			OutConfigs.Reset();
			if (!GConfig)
			{
				return false;
			}

			const FConfigSection* Section = GConfig->GetSection(TEXT("/Script/Engine.NetworkMetricsConfig"), false, GEngineIni);
			if (!Section)
			{
				return false;
			}

			const FName ListenersKey(TEXT("Listeners"));
			for (const auto& Pair : *Section)
			{
				if (Pair.Key != ListenersKey)
				{
					continue;
				}

				FNetworkMetricConfig ParsedConfig;
				if (ParseNetworkMetricConfigEntry(Pair.Value.GetValue(), ParsedConfig))
				{
					OutConfigs.Add(MoveTemp(ParsedConfig));
				}
			}

			return OutConfigs.Num() > 0;
		}

		bool ShouldEmitMetricForDriver(const UNetDriver& NetDriver, const FNetworkMetricConfig& MetricConfig)
		{
			switch (MetricConfig.EnableMode)
			{
			case ENetworkMetricEnableMode::EnableForIrisOnly:
				return NetDriver.IsUsingIrisReplication();
			case ENetworkMetricEnableMode::EnableForNonIrisOnly:
				return !NetDriver.IsUsingIrisReplication();
			default:
				return true;
			}
		}

		// Note: To properly register with the metrics system, UGorgeousPerfNetworkMetricsListener would need
		// to inherit from UNetworkMetricsBaseListener. Since that requires engine-level changes to work
		// around UHT restrictions, we create the collector but do not register it with UNetworkMetricsDatabase.
		// The collector can still be used for manual metric queries if needed.
		void RegisterEndpointMetrics(FPerfNetEndpoint& Endpoint)
		{
			if (!Endpoint.NetDriver || Endpoint.bMetricsCollectorRegistered)
			{
				return;
			}

			UNetworkMetricsDatabase* Metrics = Endpoint.NetDriver->GetMetrics();
			if (!Metrics)
			{
				return;
			}

			TArray<FNetworkMetricConfig> MetricConfigs;
			if (!LoadHarnessNetworkMetricConfigs(MetricConfigs))
			{
				return;
			}

			if (!Endpoint.MetricsCollector.IsValid())
			{
				if (UGorgeousPerfNetworkMetricsListener* NewListener = NewObject<UGorgeousPerfNetworkMetricsListener>(Endpoint.NetDriver))
				{
					Endpoint.MetricsCollector = TStrongObjectPtr<UGorgeousPerfNetworkMetricsListener>(NewListener);
				}
			}

			if (!Endpoint.MetricsCollector.IsValid())
			{
				return;
			}

			Endpoint.RegisteredMetricConfigs = MoveTemp(MetricConfigs);
			
			// Metrics registration disabled - would require UGorgeousPerfNetworkMetricsListener to inherit
			// from UNetworkMetricsBaseListener, which conflicts with UHT preprocessor restrictions.
			// Metric collection is effectively disabled until a proper engine-side solution is implemented.

			Endpoint.bMetricsCollectorRegistered = true;
		}

#endif

		void ForceEndpointConnectionsOpen(FPerfNetEndpoint& Endpoint)
		{
			if (!Endpoint.NetDriver)
			{
				return;
			}

			if (Endpoint.NetDriver->ServerConnection)
			{
				Endpoint.NetDriver->ServerConnection->SetConnectionState(USOCK_Open);
			}

			for (UNetConnection* Connection : Endpoint.NetDriver->ClientConnections)
			{
				if (Connection)
				{
					Connection->SetConnectionState(USOCK_Open);
				}
			}
		}
	}

static void ClearEndpointPendingNetGame(FPerfNetEndpoint& Endpoint)
{
	if (Endpoint.Scope.WorldContext && Endpoint.Scope.WorldContext->PendingNetGame == Endpoint.PendingNetGame.Get())
	{
		Endpoint.Scope.WorldContext->PendingNetGame = nullptr;
	}
	Endpoint.PendingNetGame.Reset();
}

static bool CompletePendingClientTravel(FPerfNetEndpoint& Endpoint)
{
	if (!GEngine)
	{
		return false;
	}

	if (!Endpoint.PendingNetGame.IsValid())
	{
		return false;
	}

	FWorldContext* WorldContext = Endpoint.Scope.WorldContext;
	if (!WorldContext)
	{
		return false;
	}

	UPendingNetGame* PendingNetGame = Endpoint.PendingNetGame.Get();
	if (WorldContext->PendingNetGame != PendingNetGame)
	{
		WorldContext->PendingNetGame = PendingNetGame;
	}

	if (!PendingNetGame->LoadMapCompleted(GEngine, *WorldContext, true, TEXT("")))
	{
		return false;
	}

	PendingNetGame->TravelCompleted(GEngine, *WorldContext);

	WorldContext->PendingNetGame = nullptr;
	Endpoint.PendingNetGame.Reset();
	return true;
}

static void CaptureEndpointStatBaseline(FPerfNetEndpoint& Endpoint)
{
	if (!Endpoint.NetDriver)
	{
		Endpoint.StatBaseline = {};
		return;
	}

	Endpoint.StatBaseline.BaseInBytes = Endpoint.NetDriver->InBytes;
	Endpoint.StatBaseline.BaseOutBytes = Endpoint.NetDriver->OutBytes;
	Endpoint.StatBaseline.BaseInPackets = Endpoint.NetDriver->InPackets;
	Endpoint.StatBaseline.BaseOutPackets = Endpoint.NetDriver->OutPackets;
	Endpoint.StatBaseline.bValid = true;
}

	bool FAutomationWorldScope::Create(const FString& InLabel, EWorldType::Type InWorldType, FString& OutError)
	{
		if (!GEngine)
		{
			OutError = TEXT("GEngine is not initialized.");
			return false;
		}

		Label = InLabel;
		WorldContext = &GEngine->CreateNewWorldContext(InWorldType);
		const FString GuidSuffix = FGuid::NewGuid().ToString(EGuidFormats::Digits);
		const FString ContextHandleString = FString::Printf(TEXT("OVPerfHarness_%s_%s"), *InLabel, *GuidSuffix);
		WorldContext->ContextHandle = FName(*ContextHandleString);

		const FString PackagePath = FString::Printf(TEXT("/Temp/%sHarness_%s"), *Label, *GuidSuffix);
		UPackage* WorldPackage = CreatePackage(*PackagePath);
		if (!WorldPackage)
		{
			OutError = TEXT("Failed to allocate harness world package.");
			return false;
		}
		WorldPackage->SetFlags(RF_Transient | RF_Public);

		const UWorld::InitializationValues InitValues = UWorld::InitializationValues()
			.InitializeScenes(true)
			.AllowAudioPlayback(false)
			.RequiresHitProxies(false)
			.CreatePhysicsScene(false)
			.CreateNavigation(false)
			.CreateAISystem(false)
			.ShouldSimulatePhysics(false)
			.SetTransactional(false)
			.CreateFXSystem(false);

		const FName WorldName(*FString::Printf(TEXT("%sHarnessWorld_%s"), *Label, *GuidSuffix));
		World = UWorld::CreateWorld(InWorldType, false, WorldName, WorldPackage, false, ERHIFeatureLevel::Num, &InitValues);
		if (!World)
		{
			OutError = FString::Printf(TEXT("Failed to create world for %s"), *Label);
			return false;
		}

		World->SetFlags(RF_Transient);

		WorldContext->SetCurrentWorld(World);
		FURL URL;
		World->InitializeActorsForPlay(URL);
		World->BeginPlay();
		return true;
	}

	void FAutomationWorldScope::Tick(float DeltaSeconds)
	{
		if (World)
		{
			World->Tick(ELevelTick::LEVELTICK_All, DeltaSeconds);
		}
	}

	void FAutomationWorldScope::Destroy()
	{
		if (!World)
		{
			WorldContext = nullptr;
			Label.Reset();
			return;
		}

		if (GEngine)
		{
			GEngine->DestroyWorldContext(World);
		}

		World->DestroyWorld(false);
		World = nullptr;
		WorldContext = nullptr;
		Label.Reset();
	}

	bool FPerfServerClientHarness::Initialize(const FPerfServerClientHarnessOptions& Options, FString& OutError)
	{
		if (!BootstrapServer(OutError))
		{
			Shutdown();
			return false;
		}

		if (!BootstrapClient(OutError))
		{
			Shutdown();
			return false;
		}

		if (!WaitForHandshake(Options.ConnectTimeoutSeconds, Options.TickStepSeconds, OutError))
		{
			Shutdown();
			return false;
		}

		ForceEndpointConnectionsOpen(Server);
		ForceEndpointConnectionsOpen(Client);

		if (Client.NetDriver && Client.Scope.World)
		{
			Client.NetDriver->SetWorld(Client.Scope.World);
			Client.Scope.World->SetNetDriver(Client.NetDriver);
		}
		ClearEndpointPendingNetGame(Client);
		EnableStatCollection();

		bInitialized = true;
		UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Harness connected server/client on %s:%d"), *ListenAddress, ServerPort);
		return true;
	}

	void FPerfServerClientHarness::Tick(float DeltaSeconds, int32 Iterations)
	{
		if (!bInitialized)
		{
			return;
		}

		Iterations = FMath::Max(Iterations, 1);
		for (int32 Index = 0; Index < Iterations; ++Index)
		{
			PumpOnce(DeltaSeconds);
		}
	}

	void FPerfServerClientHarness::Shutdown()
	{
		if (Server.NetDriver && GEngine && Server.Scope.World)
		{
			GEngine->DestroyNamedNetDriver(Server.Scope.World, Server.NetDriverName.IsNone() ? NAME_GameNetDriver : Server.NetDriverName);
			Server.NetDriver = nullptr;
			Server.NetDriverName = NAME_None;
		}

		if (Client.NetDriver && GEngine && Client.Scope.World)
		{
			GEngine->DestroyNamedNetDriver(Client.Scope.World, Client.NetDriverName.IsNone() ? NAME_GameNetDriver : Client.NetDriverName);
			Client.NetDriver = nullptr;
			Client.NetDriverName = NAME_None;
		}

		ClearEndpointPendingNetGame(Client);

		Server.Scope.Destroy();
		Client.Scope.Destroy();
		ServerPort = 0;
		bInitialized = false;
		bStatsCollectionEnabled = false;
		StatsEnableTimeSeconds = 0.0;
		Server.StatBaseline = {};
		Client.StatBaseline = {};
	}

	bool FPerfServerClientHarness::HasLiveConnection() const
	{
		const bool bClientStateValid = Client.NetDriver && Client.NetDriver->ServerConnection;
		const EConnectionState ClientState = bClientStateValid
			? Client.NetDriver->ServerConnection->GetConnectionState()
			: USOCK_Invalid;
		const bool bClientReady = bClientStateValid
			&& (ClientState == USOCK_Open || (ClientState == USOCK_Pending && !Client.PendingNetGame.IsValid()));
		const bool bServerReady = Server.NetDriver
			&& Server.NetDriver->ClientConnections.Num() > 0
			&& Server.NetDriver->ClientConnections[0]
			&& Server.NetDriver->ClientConnections[0]->GetConnectionState() == USOCK_Open;
		return bClientReady && bServerReady;
	}

	bool FPerfServerClientHarness::BootstrapServer(FString& OutError)
	{
		if (!Server.Scope.Create(TEXT("OVPerfServer"), EWorldType::Game, OutError))
		{
			return false;
		}

		EnsureHarnessGameInstance(Server, TEXT("Server"));

		auto EnsureHarnessGameMode = [&]() -> bool
		{
			UWorld* ServerWorld = Server.Scope.World;
			if (!ServerWorld)
			{
				return false;
			}

			if (ServerWorld->GetAuthGameMode())
			{
				return true;
			}

			if (AWorldSettings* WorldSettings = ServerWorld->GetWorldSettings())
			{
				WorldSettings->DefaultGameMode = AGorgeousHarnessGameMode::StaticClass();
			}

			const UPackage* WorldPackage = ServerWorld->PersistentLevel ? ServerWorld->PersistentLevel->GetPackage() : ServerWorld->GetPackage();
			const FString HarnessMapName = WorldPackage ? WorldPackage->GetName() : ServerWorld->GetMapName();
			const TCHAR* HarnessMap = HarnessMapName.IsEmpty() ? TEXT("") : *HarnessMapName;
			FURL HarnessURL(nullptr, HarnessMap, TRAVEL_Absolute);
			const FString GameOption = FString::Printf(TEXT("Game=%s"), *AGorgeousHarnessGameMode::StaticClass()->GetPathName());
			HarnessURL.AddOption(*GameOption);
			if (!ServerWorld->SetGameMode(HarnessURL))
			{
				return false;
			}

			if (AGameModeBase* GameMode = ServerWorld->GetAuthGameMode())
			{
				if (!GameMode->IsA(AGorgeousHarnessGameMode::StaticClass()))
				{
					UE_LOG(LogGorgeousObjectVariablePerf, Warning, TEXT("[OVPerf] Harness server GameMode was %s instead of %s"),
						*GameMode->GetClass()->GetName(), *AGorgeousHarnessGameMode::StaticClass()->GetAuthoredName());
				}
				else
				{
					UE_LOG(LogGorgeousObjectVariablePerf, Verbose, TEXT("[OVPerf] Harness server initialized %s"), *GameMode->GetClass()->GetName());
				}
				return true;
			}

			return false;
		};

		if (!EnsureHarnessGameMode())
		{
			Server.Scope.Destroy();
			OutError = TEXT("Failed to initialize harness GameMode.");
			return false;
		}

		const int32 MaxAttempts = 8;
		FString LastError;
		for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
		{
			const int32 CandidatePort = AllocateListenPort();
			const FName ServerDriverName(*FString::Printf(TEXT("OVPerfHarnessServer_%d_%d"), FPlatformTLS::GetCurrentThreadId(), Attempt));
			if (!GEngine->CreateNamedNetDriver(Server.Scope.World, ServerDriverName, NAME_GameNetDriver))
			{
				LastError = TEXT("Failed to create server net driver instance.");
				continue;
			}

			UNetDriver* NewServerDriver = GEngine->FindNamedNetDriver(Server.Scope.World, ServerDriverName);
			if (!NewServerDriver)
			{
				LastError = TEXT("Server net driver lookup failed.");
				GEngine->DestroyNamedNetDriver(Server.Scope.World, ServerDriverName);
				continue;
			}

			NewServerDriver->SetWorld(Server.Scope.World);
			Server.Scope.World->SetNetDriver(NewServerDriver);

			FURL ListenURL(nullptr, *ListenAddress, TRAVEL_Absolute);
			ListenURL.Port = CandidatePort;
			FString ListenError;
			if (!NewServerDriver->InitListen(Server.Scope.World, ListenURL, true, ListenError))
			{
				LastError = ListenError;
				Server.Scope.World->SetNetDriver(nullptr);
				GEngine->DestroyNamedNetDriver(Server.Scope.World, ServerDriverName);
				continue;
			}

			Server.NetDriver = NewServerDriver;
			Server.NetDriverName = ServerDriverName;
			ServerPort = CandidatePort;
			FString LocalAddrString = Server.NetDriver->GetLocalAddr().IsValid() ? Server.NetDriver->GetLocalAddr()->ToString(true) : TEXT("<unknown>");
			UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Harness server listening on %s:%d (LocalAddr=%s Driver=%s)"),
				*ListenAddress, ServerPort, *LocalAddrString, *Server.NetDriver->GetClass()->GetName());
			return true;
		}

		Server.Scope.Destroy();
		OutError = LastError.IsEmpty() ? TEXT("Unable to bind harness server port.") : LastError;
		return false;
	}

	bool FPerfServerClientHarness::BootstrapClient(FString& OutError)
	{
		if (!Client.Scope.Create(TEXT("OVPerfClient"), EWorldType::Game, OutError))
		{
			return false;
		}
		EnsureHarnessGameInstance(Client, TEXT("Client"));

		FURL ConnectURL(nullptr, *ListenAddress, TRAVEL_Absolute);
		ConnectURL.Host = ListenAddress;
		ConnectURL.Port = ServerPort;

		ClearEndpointPendingNetGame(Client);
		UPendingNetGame* PendingNetGame = NewObject<UPendingNetGame>(GetTransientPackage());
		if (!PendingNetGame)
		{
			OutError = TEXT("Failed to allocate pending net game instance.");
			Client.Scope.Destroy();
			return false;
		}
		PendingNetGame->Initialize(ConnectURL);
		Client.PendingNetGame = TStrongObjectPtr<UPendingNetGame>(PendingNetGame);
		if (Client.Scope.WorldContext)
		{
			Client.Scope.WorldContext->PendingNetGame = PendingNetGame;
		}

		const FName ClientDriverName(*FString::Printf(TEXT("OVPerfHarnessClient_%d"), FPlatformTLS::GetCurrentThreadId()));
		if (!GEngine->CreateNamedNetDriver(Client.Scope.World, ClientDriverName, NAME_GameNetDriver))
		{
			OutError = TEXT("Failed to create client net driver instance.");
			ClearEndpointPendingNetGame(Client);
			Client.Scope.Destroy();
			return false;
		}

		Client.NetDriver = GEngine->FindNamedNetDriver(Client.Scope.World, ClientDriverName);
		if (!Client.NetDriver)
		{
			OutError = TEXT("Client net driver lookup failed.");
			GEngine->DestroyNamedNetDriver(Client.Scope.World, ClientDriverName);
			ClearEndpointPendingNetGame(Client);
			Client.Scope.Destroy();
			return false;
		}

		Client.NetDriverName = ClientDriverName;

		FString ConnectError;
		if (!Client.NetDriver->InitConnect(PendingNetGame, ConnectURL, ConnectError))
		{
			OutError = FString::Printf(TEXT("InitConnect failed: %s"), *ConnectError);
			GEngine->DestroyNamedNetDriver(Client.Scope.World, ClientDriverName);
			Client.NetDriver = nullptr;
			ClearEndpointPendingNetGame(Client);
			Client.Scope.Destroy();
			return false;
		}

		PendingNetGame->NetDriver = Client.NetDriver;
		PendingNetGame->BeginHandshake();

		UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Harness client dialing %s:%d"), *ConnectURL.Host, ConnectURL.Port);

		return true;
	}

	bool FPerfServerClientHarness::WaitForHandshake(double TimeoutSeconds, double TickStepSeconds, FString& OutError)
	{
		const double StartTime = FPlatformTime::Seconds();
		const double Deadline = StartTime + TimeoutSeconds;
		const float Step = static_cast<float>(TickStepSeconds);

		auto MaybeSendClientJoin = [&]()
		{
			if (!Client.PendingNetGame.IsValid() || !Client.NetDriver || !Client.NetDriver->ServerConnection)
			{
				return;
			}

			UPendingNetGame* PendingNetGame = Client.PendingNetGame.Get();
			if (!PendingNetGame->bSuccessfullyConnected || PendingNetGame->bSentJoinRequest)
			{
				return;
			}

			if (CompletePendingClientTravel(Client))
			{
				UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Harness client sending join request after handshake ack"));
			}
		};
		while (FPlatformTime::Seconds() < Deadline)
		{
			PumpOnce(Step);
			MaybeSendClientJoin();
			if (HasLiveConnection())
			{
				return true;
			}
			FPlatformProcess::SleepNoStats(TickStepSeconds);
		}

		const double ElapsedSeconds = FPlatformTime::Seconds() - StartTime;
		const int32 PumpIterations = static_cast<int32>(ElapsedSeconds / TickStepSeconds);
		const EConnectionState ClientState = (Client.NetDriver && Client.NetDriver->ServerConnection)
			? Client.NetDriver->ServerConnection->GetConnectionState()
			: USOCK_Invalid;
		const bool bServerHasConnection = Server.NetDriver && Server.NetDriver->ClientConnections.Num() > 0 && Server.NetDriver->ClientConnections[0];
		const EConnectionState ServerState = bServerHasConnection
			? Server.NetDriver->ClientConnections[0]->GetConnectionState()
			: USOCK_Invalid;
		const int32 ServerConnCount = Server.NetDriver ? Server.NetDriver->ClientConnections.Num() : 0;
		OutError = FString::Printf(TEXT("Harness handshake timed out after %.2fs (%d pumps) (ClientState=%s, ServerState=%s, ServerConnCount=%d)."),
			ElapsedSeconds, PumpIterations,
			LexToString(ClientState), LexToString(ServerState), ServerConnCount);
		return false;
	}

	bool FPerfServerClientHarness::PumpOnce(float DeltaSeconds)
	{
		bool bTicked = false;
		const bool bHandshakePhase = !bInitialized;

		auto EnsureWorldNetDriver = [](FPerfNetEndpoint& Endpoint)
		{
			if (Endpoint.Scope.World && Endpoint.NetDriver && Endpoint.Scope.World->GetNetDriver() != Endpoint.NetDriver)
			{
				Endpoint.Scope.World->SetNetDriver(Endpoint.NetDriver);
			}
		};

		auto TickWorld = [&](FPerfNetEndpoint& Endpoint)
		{
			if (!Endpoint.Scope.World)
			{
				return;
			}
			Endpoint.Scope.Tick(DeltaSeconds);
			bTicked = true;
		};

		auto TickNetDriver = [&](FPerfNetEndpoint& Endpoint)
		{
			if (!Endpoint.NetDriver)
			{
				return;
			}
			// Mirror the world-driven order so Iris send-pass bookkeeping stays balanced during handshake ticks.
			EnsureWorldNetDriver(Endpoint);
			Endpoint.NetDriver->TickDispatch(DeltaSeconds);
			Endpoint.NetDriver->PostTickDispatch();
			Endpoint.NetDriver->TickFlush(DeltaSeconds);
			Endpoint.NetDriver->PostTickFlush();
			bTicked = true;
		};

		if (bHandshakePhase)
		{
				EnsureWorldNetDriver(Server);
			TickNetDriver(Server);
			if (Client.PendingNetGame)
			{
				Client.PendingNetGame->Tick(DeltaSeconds);
				bTicked = true;
			}
			else
			{
					EnsureWorldNetDriver(Client);
				TickNetDriver(Client);
			}
			return bTicked;
		}

			EnsureWorldNetDriver(Server);
			EnsureWorldNetDriver(Client);
		TickWorld(Server);
		TickWorld(Client);
		return bTicked;
	}

	int32 FPerfServerClientHarness::AllocateListenPort()
	{
		FScopeLock Lock(&GHarnessPortMutex);
		static int32 CurrentPort = GPerfHarnessPortMin;
		int32 Port = CurrentPort++;
		if (CurrentPort >= GPerfHarnessPortMax)
		{
			CurrentPort = GPerfHarnessPortMin;
		}
		return Port;
	}

	FPerfServerClientHarness::FCollectedStats FPerfServerClientHarness::GatherStats() const
	{
		FCollectedStats Stats;
		const double ElapsedSeconds = (StatsEnableTimeSeconds > 0.0) ? (FPlatformTime::Seconds() - StatsEnableTimeSeconds) : 0.0;
		Stats.ServerSnapshot = CaptureNetDriverSnapshot(TEXT("Harness server"), Server.NetDriver, &Server.StatBaseline, ElapsedSeconds);
		Stats.ClientSnapshot = CaptureNetDriverSnapshot(TEXT("Harness client"), Client.NetDriver, &Client.StatBaseline, ElapsedSeconds);
		Stats.OutgoingPackets = Stats.ServerSnapshot.OutPackets + Stats.ClientSnapshot.OutPackets;
		Stats.IncomingPackets = Stats.ServerSnapshot.InPackets + Stats.ClientSnapshot.InPackets;
		Stats.OutgoingBytes = Stats.ServerSnapshot.OutBytes + Stats.ClientSnapshot.OutBytes;
		Stats.IncomingBytes = Stats.ServerSnapshot.InBytes + Stats.ClientSnapshot.InBytes;
		return Stats;
	}

#if GORGEOUSCORERUNTIME_ENABLE_NETWORK_METRICS
	void FPerfServerClientHarness::CollectRegisteredNetworkMetrics(TArray<FString>& OutLines) const
	{
		bool bEmittedAnyMetricLine = false;
		bool bObservedNonZeroMetric = false;

		auto DescribeEndpoint = [&](const TCHAR* Label, const FPerfNetEndpoint& Endpoint)
		{
			if (!Endpoint.NetDriver || !Endpoint.MetricsCollector.IsValid() || Endpoint.RegisteredMetricConfigs.Num() == 0)
			{
				return;
			}

			UNetworkMetricsDatabase* Metrics = Endpoint.NetDriver->GetMetrics();
			if (!Metrics)
			{
				return;
			}

			Metrics->ProcessListeners();

			for (const FNetworkMetricConfig& MetricConfig : Endpoint.RegisteredMetricConfigs)
			{
				if (!Metrics->Contains(MetricConfig.MetricName))
				{
					continue;
				}

				if (!ShouldEmitMetricForDriver(*Endpoint.NetDriver, MetricConfig))
				{
					continue;
				}

				FString ValueString;
				int64 IntValue = 0;
				float FloatValue = 0.0f;
				bool bHasValue = false;
				bool bValueNonZero = false;
				if (Endpoint.MetricsCollector->TryGetIntMetric(MetricConfig.MetricName, IntValue))
				{
					ValueString = LexToString(IntValue);
					bHasValue = true;
					bValueNonZero = IntValue != 0;
				}
				else if (Endpoint.MetricsCollector->TryGetFloatMetric(MetricConfig.MetricName, FloatValue))
				{
					ValueString = FString::SanitizeFloat(FloatValue);
					bHasValue = true;
					bValueNonZero = !FMath::IsNearlyZero(FloatValue);
				}
				else
				{
					ValueString = TEXT("<unavailable>");
				}

				OutLines.Add(FString::Printf(TEXT("%s metric %s.%s = %s"),
					Label,
					*MetricConfig.Class.ToString(),
					*MetricConfig.MetricName.ToString(),
					*ValueString));
				bEmittedAnyMetricLine = true;
				if (bHasValue && bValueNonZero)
				{
					bObservedNonZeroMetric = true;
				}
			}
		};

		DescribeEndpoint(TEXT("Harness server"), Server);
		DescribeEndpoint(TEXT("Harness client"), Client);

		if (bEmittedAnyMetricLine && !bObservedNonZeroMetric)
		{
			OutLines.Add(TEXT("Harness network metrics listeners returned only zero values (CSV/perf counter metrics focus on actor replication, not object variables)."));
		}
	}

#else

	void FPerfServerClientHarness::CollectRegisteredNetworkMetrics(TArray<FString>&) const
	{
	}

#endif

	void FPerfServerClientHarness::EnableStatCollection()
	{
		if (bStatsCollectionEnabled)
		{
			return;
		}

		auto EnableForEndpoint = [](FPerfNetEndpoint& Endpoint, const TCHAR* EndpointLabel)
		{
			if (!Endpoint.NetDriver)
			{
				return;
			}

			Endpoint.NetDriver->bCollectNetStats = true;
			Endpoint.NetDriver->bSkipLocalStats = false;
			Endpoint.NetDriver->ProfileStats = true;
			Endpoint.NetDriver->StatPeriod = FMath::Max(Endpoint.NetDriver->StatPeriod, 0.25f);
			Endpoint.NetDriver->StatUpdateTime = FPlatformTime::Seconds();
			UE_LOG(LogGorgeousObjectVariablePerf, Display,
				TEXT("[OVPerf] Harness %s driver enabling automation stat capture (Driver=%s)"),
				EndpointLabel,
				*Endpoint.NetDriver->GetClass()->GetName());
	#if GORGEOUSCORERUNTIME_ENABLE_NETWORK_METRICS
				RegisterEndpointMetrics(Endpoint);
	#endif
		};

		EnableForEndpoint(Server, TEXT("server"));
		EnableForEndpoint(Client, TEXT("client"));
		CaptureEndpointStatBaseline(Server);
		CaptureEndpointStatBaseline(Client);
		StatsEnableTimeSeconds = FPlatformTime::Seconds();
		UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Harness issuing stat net / stat irisreplication style capture (headless logging)"));
		bStatsCollectionEnabled = true;
	}

	FPerfServerClientHarness::FDriverStatSnapshot FPerfServerClientHarness::CaptureNetDriverSnapshot(const TCHAR* Label, UNetDriver* NetDriver, const FPerfNetEndpoint::FStatBaseline* Baseline, double ElapsedSeconds)
	{
		FDriverStatSnapshot Snapshot;
		if (Label && *Label)
		{
			Snapshot.Label = Label;
		}

		if (!NetDriver)
		{
			return Snapshot;
		}

		Snapshot.bValid = true;
		Snapshot.InPackets = NetDriver->InPackets;
		Snapshot.OutPackets = NetDriver->OutPackets;
		Snapshot.InBytes = NetDriver->InBytes;
		Snapshot.OutBytes = NetDriver->OutBytes;
		Snapshot.InBunches = NetDriver->InBunches;
		Snapshot.OutBunches = NetDriver->OutBunches;
		Snapshot.InPacketsLost = NetDriver->InPacketsLost;
		Snapshot.OutPacketsLost = NetDriver->OutPacketsLost;
		Snapshot.ObservedSeconds = ElapsedSeconds;
		Snapshot.bIsUsingIris = NetDriver->IsUsingIrisReplication();

		const bool bUseBaseline = Baseline && Baseline->bValid && ElapsedSeconds > KINDA_SMALL_NUMBER;
		if (bUseBaseline)
		{
			auto ComputeRate = [ElapsedSeconds](double Delta)
			{
				return Delta <= 0.0 ? 0.0 : Delta / ElapsedSeconds;
			};

			const double DeltaInBytes = static_cast<double>(NetDriver->InBytes) - static_cast<double>(Baseline->BaseInBytes);
			const double DeltaOutBytes = static_cast<double>(NetDriver->OutBytes) - static_cast<double>(Baseline->BaseOutBytes);
			const double DeltaInPackets = static_cast<double>(NetDriver->InPackets) - static_cast<double>(Baseline->BaseInPackets);
			const double DeltaOutPackets = static_cast<double>(NetDriver->OutPackets) - static_cast<double>(Baseline->BaseOutPackets);
			Snapshot.InRateBytesPerSecond = ComputeRate(DeltaInBytes);
			Snapshot.OutRateBytesPerSecond = ComputeRate(DeltaOutBytes);
			Snapshot.InPacketsPerSecond = ComputeRate(DeltaInPackets);
			Snapshot.OutPacketsPerSecond = ComputeRate(DeltaOutPackets);
		}
		else
		{
			Snapshot.InRateBytesPerSecond = NetDriver->InBytesPerSecond;
			Snapshot.OutRateBytesPerSecond = NetDriver->OutBytesPerSecond;
		}

		UNetConnection* ActiveConnection = NetDriver->ServerConnection;
		if (!ActiveConnection && NetDriver->ClientConnections.Num() > 0)
		{
			ActiveConnection = NetDriver->ClientConnections[0];
		}

		if (ActiveConnection)
		{
			Snapshot.bHasConnection = true;
			Snapshot.ConnectionState = ActiveConnection->GetConnectionState();
			Snapshot.AvgPingMs = ActiveConnection->AvgLag * 1000.0;
			Snapshot.AvgJitterMs = ActiveConnection->GetAverageJitterInMS();
			Snapshot.InLossPercent = ActiveConnection->GetInLossPercentage().GetAvgLossPercentage() * 100.0f;
			Snapshot.OutLossPercent = ActiveConnection->GetOutLossPercentage().GetAvgLossPercentage() * 100.0f;
			if (!bUseBaseline)
			{
				Snapshot.InPacketsPerSecond = ActiveConnection->InPacketsPerSecond;
				Snapshot.OutPacketsPerSecond = ActiveConnection->OutPacketsPerSecond;
			}
		}

		return Snapshot;
	}

	FString FPerfServerClientHarness::FDriverStatSnapshot::Describe() const
	{
		if (Label.IsEmpty())
		{
			return FString();
		}

		if (!bValid)
		{
			return FString::Printf(TEXT("%s stat net unavailable (driver offline)"), *Label);
		}

		const double InRateKBs = static_cast<double>(InRateBytesPerSecond) / 1024.0;
		const double OutRateKBs = static_cast<double>(OutRateBytesPerSecond) / 1024.0;
		const TCHAR* IrisState = bIsUsingIris ? TEXT("true") : TEXT("false");

		TArray<FString> Lines;
		Lines.Add(FString::Printf(TEXT("%s stat net"), *Label));
		Lines.Add(FString::Printf(TEXT("  Rate: In=%.1f KB/s Out=%.1f KB/s"), InRateKBs, OutRateKBs));

		if (!bHasConnection)
		{
			Lines.Add(FString::Printf(TEXT("  Iris=%s | Connection=<none>"), IrisState));
			return FString::Join(Lines, TEXT("\n"));
		}

		const FString PacketLine = FString::Printf(TEXT("  Packets: In=%llu (%.0f/s) Out=%llu (%.0f/s)"),
			static_cast<unsigned long long>(InPackets),
			static_cast<double>(InPacketsPerSecond),
			static_cast<unsigned long long>(OutPackets),
			static_cast<double>(OutPacketsPerSecond));
		Lines.Add(PacketLine);
		Lines.Add(FString::Printf(TEXT("  Loss: In=%.2f%%%% Out=%.2f%%%%"), InLossPercent, OutLossPercent));
		Lines.Add(FString::Printf(TEXT("  Latency: Ping=%.2f ms | Jitter=%.2f ms"), AvgPingMs, AvgJitterMs));
		Lines.Add(FString::Printf(TEXT("  Iris=%s | ConnState=%s"), IrisState, LexToString(ConnectionState)));
		return FString::Join(Lines, TEXT("\n"));
	}

	#endif // WITH_SERVER_CODE
	}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousObjectVariableRegistryScaleTest, "GorgeousCore.ObjectVariables.RegistryScale",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::HighPriority);

bool FGorgeousObjectVariableRegistryScaleTest::RunTest(const FString& Parameters)
{
	using namespace GorgeousObjectVariablePerf;
	// Gauntlet provides the multiplayer harness; we only sample active net drivers when available.
	const bool bGauntletActive = FGorgeousInsightHarness::IsGauntletActive();

	const FPerfScenario Scenario = FPerfScenario::FromParameters(Parameters);
	const FString PresetLabel = FPerfScenario::DescribePresetLabel(Scenario.ScenarioPreset);
	const FName RegistrationLoggingKey(TEXT("GT.ObjectVariables.Registration.Successful"));
	const bool bRegistrationLogWasSuppressed = UGorgeousLoggingBlueprintFunctionLibrary::IsLoggingKeySuppressed(RegistrationLoggingKey);
	UGorgeousLoggingBlueprintFunctionLibrary::SetLoggingKeySuppressed(RegistrationLoggingKey, true);
	ON_SCOPE_EXIT
	{
		UGorgeousLoggingBlueprintFunctionLibrary::SetLoggingKeySuppressed(RegistrationLoggingKey, bRegistrationLogWasSuppressed);
	};
	const FString ScenarioSummary = FString::Printf(
		TEXT("[OVPerf] Starting registry perf test | Preset=%s (%d) Num=%d SpawnBudget=%.2fs LookupBudget=%.2fs HierarchyBudget=%.2fs AvgBranching=%d MaxBranching=%d Depth<=%d Seed=%d BloatChance=%.2f BloatChars=%d MemorySamples=%d RootsTarget=%d CrossRefChance=%.2f CrossRefMax=%d ReplicationChance=%.2f ReplicationSamples=%d NetworkPreset=%s (%d) Gauntlet=%s"),
		*PresetLabel, Scenario.ScenarioPreset,
		Scenario.VariableCount, Scenario.SpawnBudgetSeconds, Scenario.LookupBudgetSeconds, Scenario.HierarchyBudgetSeconds,
		Scenario.AverageBranchingFactor, Scenario.MaxBranchingFactor, Scenario.MaxDepth, Scenario.RandomSeed,
		Scenario.TextBloatFrequency, Scenario.TextBloatCharLength, Scenario.MemorySampleCount, Scenario.TargetRootRegistries,
		Scenario.CrossReferenceFrequency, Scenario.MaxCrossReferencesPerNode,
		Scenario.ReplicationOptInFrequency, Scenario.ReplicationValidationSamples,
		*FPerfScenario::DescribeNetworkPresetLabel(Scenario.NetworkEmulationPreset), Scenario.NetworkEmulationPreset,
		bGauntletActive ? TEXT("true") : TEXT("false"));
	UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("%s"), *ScenarioSummary);

	FPerfSummaryPanel SummaryPanel;
	SummaryPanel.InitializeDefaults();
	auto RecordStat = [&](EPerfStatCategory Category, const FString& Line)
	{
		if (Line.IsEmpty())
		{
			return;
		}

		SummaryPanel.AddLine(Category, Line);
	};
	RecordStat(EPerfStatCategory::Scenario, ScenarioSummary);

	auto TickHarness = [](int32 Iterations = 1)
	{
		(void)Iterations;
	};
	auto PumpHarnessWork = []() {};

	TickHarness();

	const FGorgeousNetworkEmulation::FRuntimeProfile NetworkRuntime = InitializeNetworkEmulation(Scenario);
	if (NetworkRuntime.bEnabled)
	{
		const FString NetSummary = FString::Printf(TEXT("Network preset %s (%d) -> %s | latency %.1f-%.1f ms | dropChance %.2f%%"),
			*NetworkRuntime.Label,
			NetworkRuntime.PresetIndex,
			*NetworkRuntime.Description,
			NetworkRuntime.MinLatencySeconds * 1000.0,
			NetworkRuntime.MaxLatencySeconds * 1000.0,
			NetworkRuntime.DropChance * 100.0);
		RecordStat(EPerfStatCategory::Networking, NetSummary);
		if (NetworkRuntime.bDriverSettingsApplied)
		{
			UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] %s (applied to active net drivers)"), *NetSummary);
		}
		else if (!NetworkRuntime.DriverWarning.IsEmpty())
		{
			UE_LOG(LogGorgeousObjectVariablePerf, Warning, TEXT("[OVPerf] %s | %s"), *NetSummary, *NetworkRuntime.DriverWarning);
			AddWarning(NetworkRuntime.DriverWarning);
		}
	}

	FRandomStream RandomStream(Scenario.RandomSeed);

	TArray<FName> RegisteredRootNames = UGorgeousRootObjectVariable::GetRegisteredRootNames();
	TArray<FName> CandidateRootNames;
	CandidateRootNames.Reserve(RegisteredRootNames.Num() + 1);
	CandidateRootNames.AddUnique(NAME_None);
	for (const FName RootName : RegisteredRootNames)
	{
		CandidateRootNames.AddUnique(RootName);
	}
	if (CandidateRootNames.Num() == 0)
	{
		CandidateRootNames.Add(NAME_None);
	}

	TArray<FActiveRootContext> ActiveRoots;
	const int32 DesiredRootCount = FMath::Clamp(Scenario.TargetRootRegistries, 1, CandidateRootNames.Num());
	while (CandidateRootNames.Num() > 0 && ActiveRoots.Num() < DesiredRootCount)
	{
		const int32 PickIndex = RandomStream.RandRange(0, CandidateRootNames.Num() - 1);
		const FName SelectedName = CandidateRootNames[PickIndex];
		CandidateRootNames.RemoveAtSwap(PickIndex);
		if (UGorgeousRootObjectVariable* RootInstance = UGorgeousRootObjectVariable::GetRootObjectVariable(SelectedName))
		{
			const bool bAlreadyTracked = ActiveRoots.ContainsByPredicate([RootInstance](const FActiveRootContext& Existing)
			{
				return Existing.Root == RootInstance;
			});
			if (bAlreadyTracked)
			{
				continue;
			}
			FActiveRootContext& Context = ActiveRoots.AddDefaulted_GetRef();
			Context.Root = RootInstance;
			Context.RootName = SelectedName;
			Context.ResolvedRootKey = ResolveMetadataRootName(SelectedName);
		}
	}

	if (ActiveRoots.Num() == 0)
	{
		if (UGorgeousRootObjectVariable* DefaultRoot = UGorgeousRootObjectVariable::GetRootObjectVariable())
		{
			FActiveRootContext& Context = ActiveRoots.AddDefaulted_GetRef();
			Context.Root = DefaultRoot;
			Context.RootName = NAME_None;
			Context.ResolvedRootKey = ResolveMetadataRootName(NAME_None);
		}
	}

	if (ActiveRoots.Num() == 0)
	{
		AddError(TEXT("Unable to activate any root registries for the perf test."));
		return false;
	}

	for (const FActiveRootContext& Context : ActiveRoots)
	{
		if (!TestNotNull(*FString::Printf(TEXT("Root %s available"), *DescribeRootLabel(Context.RootName)), Context.Root))
		{
			return false;
		}
	}

	TArray<FString> RootLabels;
	RootLabels.Reserve(ActiveRoots.Num());
	for (const FActiveRootContext& Context : ActiveRoots)
	{
		RootLabels.Add(DescribeRootLabel(Context.RootName));
	}
	UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Activated %d/%d root registries -> %s"),
		ActiveRoots.Num(), Scenario.TargetRootRegistries, *FString::Join(RootLabels, TEXT(", ")));

	const FSystemStatSnapshot StatsBeforeSpawn = CaptureSystemStatsSnapshot();
	RecordStat(EPerfStatCategory::System, FString::Printf(TEXT("System baseline: %s"), *DescribeMemoryUsage(StatsBeforeSpawn.MemoryStats)));
	UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf][Sys] Baseline captured | %s"), *DescribeMemoryUsage(StatsBeforeSpawn.MemoryStats));

	const EGorgeousObjectVariableOrphanResolution PreviousResolution = UGorgeousRootObjectVariable::GetDefaultOrphanResolution();
	UGorgeousRootObjectVariable::SetDefaultOrphanResolution(EGorgeousObjectVariableOrphanResolution::DestroyOrphans);
	ON_SCOPE_EXIT
	{
		UGorgeousRootObjectVariable::SetDefaultOrphanResolution(PreviousResolution);
	};

	TArray<UGorgeousObjectVariable*> SpawnedVariables;
	SpawnedVariables.Reserve(Scenario.VariableCount);
	TArray<FGuid> Identifiers;
	Identifiers.Reserve(Scenario.VariableCount);
	TArray<FSpawnRecord> SpawnRecords;
	SpawnRecords.Reserve(Scenario.VariableCount);
	const TArray<TSubclassOf<UGorgeousObjectVariable>> ClassRotation = {
		UGorgeousPerfObjectVariable::StaticClass(),
		UGorgeousPerfHeavyObjectVariable::StaticClass(),
		UGorgeousPerfNarrativeObjectVariable::StaticClass()
	};
	int64 TotalBranchingChildren = 0;
	int32 BranchingParentCount = 0;
	int32 TextBloatApplications = 0;

	bool bManualCleanupCompleted = false;
	ON_SCOPE_EXIT
	{
		if (bManualCleanupCompleted)
		{
			return;
		}

		for (UGorgeousObjectVariable* Variable : SpawnedVariables)
		{
			UGorgeousRootObjectVariable::RemoveVariableFromRegistry(Variable);
		}
	};

	const double SpawnStart = FPlatformTime::Seconds();
	UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Spawning %d test variables with branching fanout <= %d..."), Scenario.VariableCount, Scenario.MaxBranchingFactor);

	struct FBranchWorkItem
	{
		UGorgeousObjectVariable* Parent = nullptr;
		UGorgeousRootObjectVariable* Root = nullptr;
		FName RootName = NAME_None;
		FName ResolvedRootKey = NAME_None;
		int32 Depth = 0;
	};

	TArray<FBranchWorkItem> Pending;
	Pending.Reserve(ActiveRoots.Num() * Scenario.AverageBranchingFactor);
	for (const FActiveRootContext& Context : ActiveRoots)
	{
		if (IsValid(Context.Root))
		{
			Pending.Add({ Context.Root, Context.Root, Context.RootName, Context.ResolvedRootKey, 0 });
		}
	}

	TArray<FRootRegistryStats> RootStats;
	RootStats.Reserve(ActiveRoots.Num());
	for (const FActiveRootContext& Context : ActiveRoots)
	{
		FindOrAddRootStats(RootStats, Context.Root, Context.RootName, Context.ResolvedRootKey);
	}

	FReplicationTimingAccumulator ReplicationTiming;

	int32 NetworkingEnableCandidates = 0;
	int32 NetworkingEnableFailures = 0;
	int32 ReplicatedOwnerCandidates = 0;
	int32 ReplicatedOwnerFailures = 0;

	auto HasReplicatedOwnerContext = [](UGorgeousObjectVariable* Node) -> bool
	{
		if (!Node)
		{
			return false;
		}

		auto HasReplicatedNetDriver = [](UWorld* CandidateWorld) -> bool
		{
			return CandidateWorld && CandidateWorld->GetNetMode() != NM_Standalone && CandidateWorld->GetNetDriver() != nullptr;
		};

		if (UWorld* DirectWorld = FGorgeousObjectVariablePerfTestAccess::ResolveWorld(Node))
		{
			if (HasReplicatedNetDriver(DirectWorld))
			{
				return true;
			}
		}

		if (AActor* OwnerActor = FGorgeousObjectVariablePerfTestAccess::ResolveReplicationOwner(Node))
		{
			if (OwnerActor->GetIsReplicated() && HasReplicatedNetDriver(OwnerActor->GetWorld()))
			{
				return true;
			}
		}

		return false;
	};

	auto ConfigureNetworkingForRecord = [&](UGorgeousObjectVariable* Node, FRootRegistryStats& RootStat, FSpawnRecord& Record)
	{
		Record.bEnforcedNetworking = RootStat.bEnforcesNetworking;
		Record.bNetworkingOptIn = false;

		if (!Node)
		{
			return;
		}

		UGorgeousPerfBaseObjectVariable* PerfNode = Cast<UGorgeousPerfBaseObjectVariable>(Node);
		if (!PerfNode)
		{
			if (RootStat.bEnforcesNetworking && RootStat.bSupportsNetworking)
			{
				++RootStat.EnforcedNetworkingViolations;
				UE_LOG(LogGorgeousObjectVariablePerf, Warning,
					TEXT("[OVPerf] Root %s enforces networking but node %s is missing the perf networking helper."),
					*DescribeRootLabel(RootStat.RootName), *Node->GetName());
			}
			return;
		}

		const bool bSupportsNetworking = RootStat.bSupportsNetworking;
		bool bEnableNetworking = false;
		if (bSupportsNetworking)
		{
			if (RootStat.bEnforcesNetworking)
			{
				bEnableNetworking = true;
			}
			else if (Scenario.ReplicationOptInFrequency > 0.0)
			{
				bEnableNetworking = RandomStream.GetFraction() < Scenario.ReplicationOptInFrequency;
			}
		}

		if (!bEnableNetworking)
		{
			return;
		}

		++NetworkingEnableCandidates;
		++ReplicatedOwnerCandidates;
		const double ActivationStart = FPlatformTime::Seconds();
		PerfNode->ConfigureScenarioNetworking(true, RootStat.bEnforcesNetworking);
		const double ActivationSeconds = FPlatformTime::Seconds() - ActivationStart;
		ReplicationTiming.RecordActivation(ActivationSeconds);
		RootStat.ActivationTiming.AddSample(ActivationSeconds);
		Record.bNetworkingOptIn = true;
		Record.bScenarioNetworkingEnabled = PerfNode->IsScenarioNetworkingEnabled();
		if (!Record.bScenarioNetworkingEnabled)
		{
			++NetworkingEnableFailures;
		}
		Record.bHasReplicatedOwner = HasReplicatedOwnerContext(PerfNode);
		if (!Record.bHasReplicatedOwner)
		{
			++ReplicatedOwnerFailures;
		}
		const bool bReplicationActive = PerfNode->IsReplicationActive();
		RootStat.NetworkingActiveCount += bReplicationActive ? 1 : 0;
		if (RootStat.bEnforcesNetworking && !bReplicationActive)
		{
			++RootStat.EnforcedNetworkingViolations;
		}
		++RootStat.NetworkingOptInCount;
	};

	while (SpawnedVariables.Num() < Scenario.VariableCount && Pending.Num() > 0)
	{
		PumpHarnessWork();
		const int32 WorkIndex = RandomStream.RandRange(0, Pending.Num() - 1);
		FBranchWorkItem WorkItem = Pending[WorkIndex];
		Pending.RemoveAtSwap(WorkIndex);
		if (!IsValid(WorkItem.Parent) || !IsValid(WorkItem.Root))
		{
			continue;
		}

		const float VarianceRange = static_cast<float>(Scenario.MaxBranchingFactor - Scenario.AverageBranchingFactor);
		const float CenteredSample = (RandomStream.GetFraction() - 0.5f) * 2.0f; // [-1,1]
		const int32 DesiredChildren = FMath::Clamp(
			FMath::RoundToInt(static_cast<float>(Scenario.AverageBranchingFactor) + VarianceRange * CenteredSample),
			1,
			Scenario.MaxBranchingFactor);
		int32 ChildrenSpawnedForParent = 0;
		for (int32 ChildIndex = 0; ChildIndex < DesiredChildren && SpawnedVariables.Num() < Scenario.VariableCount; ++ChildIndex)
		{
			PumpHarnessWork();
			const TSubclassOf<UGorgeousObjectVariable> SelectedClass = ClassRotation[RandomStream.RandRange(0, ClassRotation.Num() - 1)];
			const FString DisplayName = FString::Printf(TEXT("PerfNode_%d_%d"), WorkItem.Depth, SpawnedVariables.Num());
			FGuid Identifier;
			UGorgeousObjectVariable* Variable = WorkItem.Parent->NewObjectVariable(SelectedClass, Identifier, WorkItem.Parent, false, DisplayName);
			if (!TestNotNull(TEXT("Variable successfully created"), Variable))
			{
				continue;
			}

			SpawnedVariables.Add(Variable);
			Identifiers.Add(Identifier);
			FSpawnRecord& Record = SpawnRecords.AddDefaulted_GetRef();
			Record.Node = Variable;
			Record.Identifier = Identifier;
			Record.Depth = WorkItem.Depth + 1;
			Record.Root = WorkItem.Root;
			Record.RootName = WorkItem.RootName;
			Record.ResolvedRootKey = WorkItem.ResolvedRootKey;
			Record.DisplayName = FName(*DisplayName);
			FRootRegistryStats& RootStat = FindOrAddRootStats(RootStats, WorkItem.Root, WorkItem.RootName, WorkItem.ResolvedRootKey);
			++RootStat.SpawnedCount;
			ConfigureNetworkingForRecord(Record.Node, RootStat, Record);

			if (UGorgeousPerfHeavyObjectVariable* Heavy = Cast<UGorgeousPerfHeavyObjectVariable>(Variable))
			{
				Heavy->InitializePayload(RandomStream);
			}
			else if (UGorgeousPerfNarrativeObjectVariable* Narrative = Cast<UGorgeousPerfNarrativeObjectVariable>(Variable))
			{
				const bool bShouldBloat = RandomStream.GetFraction() < Scenario.TextBloatFrequency;
				TextBloatApplications += bShouldBloat ? 1 : 0;
				Narrative->InitializePayload(RandomStream, bShouldBloat, Scenario.TextBloatCharLength);
			}

			if (WorkItem.Depth + 1 < Scenario.MaxDepth && RandomStream.GetFraction() > 0.25)
			{
				Pending.Add({ Variable, WorkItem.Root, WorkItem.RootName, WorkItem.ResolvedRootKey, WorkItem.Depth + 1 });
			}

			++ChildrenSpawnedForParent;
		}

		if (ChildrenSpawnedForParent > 0)
		{
			++BranchingParentCount;
			TotalBranchingChildren += ChildrenSpawnedForParent;
		}
	}

	while (SpawnedVariables.Num() < Scenario.VariableCount)
	{
		PumpHarnessWork();
		const FActiveRootContext& RootContext = ActiveRoots[RandomStream.RandRange(0, ActiveRoots.Num() - 1)];
		const TSubclassOf<UGorgeousObjectVariable> SelectedClass = ClassRotation[RandomStream.RandRange(0, ClassRotation.Num() - 1)];
		const FString DisplayName = FString::Printf(TEXT("PerfFlat_%d"), SpawnedVariables.Num());
		FGuid Identifier;
		UGorgeousObjectVariable* Variable = RootContext.Root->NewObjectVariable(SelectedClass, Identifier, RootContext.Root, false, DisplayName);
		if (!TestNotNull(TEXT("Variable successfully created"), Variable))
		{
			break;
		}

		SpawnedVariables.Add(Variable);
		Identifiers.Add(Identifier);
		FSpawnRecord& Record = SpawnRecords.AddDefaulted_GetRef();
		Record.Node = Variable;
		Record.Identifier = Identifier;
		Record.Depth = 1;
		Record.Root = RootContext.Root;
		Record.RootName = RootContext.RootName;
		Record.ResolvedRootKey = RootContext.ResolvedRootKey;
		Record.DisplayName = FName(*DisplayName);
		FRootRegistryStats& RootStat = FindOrAddRootStats(RootStats, RootContext.Root, RootContext.RootName, RootContext.ResolvedRootKey);
		++RootStat.SpawnedCount;
		ConfigureNetworkingForRecord(Record.Node, RootStat, Record);

		if (UGorgeousPerfHeavyObjectVariable* Heavy = Cast<UGorgeousPerfHeavyObjectVariable>(Variable))
		{
			Heavy->InitializePayload(RandomStream);
		}
		else if (UGorgeousPerfNarrativeObjectVariable* Narrative = Cast<UGorgeousPerfNarrativeObjectVariable>(Variable))
		{
			const bool bShouldBloat = RandomStream.GetFraction() < Scenario.TextBloatFrequency;
			TextBloatApplications += bShouldBloat ? 1 : 0;
			Narrative->InitializePayload(RandomStream, bShouldBloat, Scenario.TextBloatCharLength);
		}
	}
	const FPerfSample SpawnSample{ FPlatformTime::Seconds() - SpawnStart, SpawnedVariables.Num() };
	RecordStat(EPerfStatCategory::Performance, DescribeSample(TEXT("Spawn"), SpawnSample));
	UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Spawned %d variables in %.2f seconds"), SpawnSample.Count, SpawnSample.Seconds);
	TestTrue(TEXT("Spawn budget respected"), SpawnSample.Seconds <= Scenario.SpawnBudgetSeconds);
	if (BranchingParentCount > 0)
		{
			const double ActualAverageBranching = static_cast<double>(TotalBranchingChildren) / static_cast<double>(BranchingParentCount);
			const double BloatPercent = (SpawnRecords.Num() > 0) ? (static_cast<double>(TextBloatApplications) / static_cast<double>(SpawnRecords.Num())) * 100.0 : 0.0;
			UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Branching parents=%d avgChildren=%.2f | Text bloat hits=%d (%.2f%%)"),
				BranchingParentCount, ActualAverageBranching, TextBloatApplications, BloatPercent);
			RecordStat(EPerfStatCategory::Performance, FString::Printf(TEXT("Branching parents=%d avgChildren=%.2f"), BranchingParentCount, ActualAverageBranching));
		}

		if (SpawnSample.Count > 0 && RootStats.Num() > 0)
		{
			TArray<FString> RootShareSummaries;
			RootShareSummaries.Reserve(RootStats.Num());
			for (const FRootRegistryStats& Stats : RootStats)
			{
				const double SharePercent = (static_cast<double>(Stats.SpawnedCount) / SpawnSample.Count) * 100.0;
				const FString RootLabel = DescribeRootLabel(Stats.RootName);
				UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Root %s captured %d vars (%.2f%% of run)"), *RootLabel, Stats.SpawnedCount, SharePercent);
				RootShareSummaries.Add(FString::Printf(TEXT("%s=%d (%.2f%%)"), *RootLabel, Stats.SpawnedCount, SharePercent));
			}
			RecordStat(EPerfStatCategory::Performance, FString::Printf(TEXT("Root distribution -> %s"), *FString::Join(RootShareSummaries, TEXT(", "))));
		}

		const FSystemStatSnapshot StatsAfterSpawn = CaptureSystemStatsSnapshot();
		LogSystemDelta(TEXT("PostSpawn"), StatsBeforeSpawn, StatsAfterSpawn);
		TickHarness(4);

	int64 TotalReferenceLinks = 0;
	int32 NodesWithReferences = 0;
	const bool bShouldAssignCrossRefs = (Scenario.MaxCrossReferencesPerNode > 0) && (Scenario.CrossReferenceFrequency > 0.0) && SpawnRecords.Num() > 1;
	if (bShouldAssignCrossRefs)
	{
		const double CrossRefStart = FPlatformTime::Seconds();
		for (FSpawnRecord& Record : SpawnRecords)
		{
			PumpHarnessWork();
			UGorgeousPerfBaseObjectVariable* PerfNode = Cast<UGorgeousPerfBaseObjectVariable>(Record.Node);
			if (!PerfNode)
			{
				continue;
			}

			PerfNode->ResetCrossReferences();
			if (RandomStream.GetFraction() > Scenario.CrossReferenceFrequency)
			{
				continue;
			}

			const int32 DesiredLinks = FMath::Clamp(RandomStream.RandRange(1, Scenario.MaxCrossReferencesPerNode), 1, Scenario.MaxCrossReferencesPerNode);
			int32 LinksAddedForNode = 0;
			for (int32 LinkIndex = 0; LinkIndex < DesiredLinks; ++LinkIndex)
			{
				const FSpawnRecord& TargetRecord = SpawnRecords[RandomStream.RandRange(0, SpawnRecords.Num() - 1)];
				if (!IsValid(TargetRecord.Node) || TargetRecord.Node == Record.Node)
				{
					continue;
				}

				PerfNode->AddCrossReference(TargetRecord.Node);
				++LinksAddedForNode;
			}

			if (LinksAddedForNode > 0)
			{
				++NodesWithReferences;
				TotalReferenceLinks += LinksAddedForNode;
			}
		}

		const FPerfSample CrossRefSample{ FPlatformTime::Seconds() - CrossRefStart, NodesWithReferences };
		RecordStat(EPerfStatCategory::Performance, DescribeSample(TEXT("Cross-reference assignment"), CrossRefSample));
		const double AverageLinks = (NodesWithReferences > 0) ? static_cast<double>(TotalReferenceLinks) / NodesWithReferences : 0.0;
		UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Assigned %lld reference links across %d nodes (avg %.2f) in %.2f seconds"),
			TotalReferenceLinks, NodesWithReferences, AverageLinks, CrossRefSample.Seconds);
	}

	const FSystemStatSnapshot StatsAfterCrossRefs = bShouldAssignCrossRefs ? CaptureSystemStatsSnapshot() : StatsAfterSpawn;
	if (bShouldAssignCrossRefs)
	{
		LogSystemDelta(TEXT("PostCrossRefs"), StatsAfterSpawn, StatsAfterCrossRefs);
	}
	TickHarness(4);

	const double LookupStart = FPlatformTime::Seconds();
	UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Beginning identifier lookup validation (%d lookups)..."), Identifiers.Num());
	for (const FGuid& Identifier : Identifiers)
	{
		PumpHarnessWork();
		UGorgeousObjectVariable* Found = UGorgeousRootObjectVariable::FindVariableByIdentifier(Identifier);
		TestTrue(TEXT("Lookup should succeed"), IsValid(Found));
	}
	const FPerfSample LookupSample{ FPlatformTime::Seconds() - LookupStart, Identifiers.Num() };
	RecordStat(EPerfStatCategory::Performance, DescribeSample(TEXT("Identifier lookup"), LookupSample));
	UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Completed identifier lookups in %.2f seconds"), LookupSample.Seconds);
	TestTrue(TEXT("Lookup budget respected"), LookupSample.Seconds <= Scenario.LookupBudgetSeconds);

	const int32 ActualDisplayNameSamples = FMath::Min(Scenario.DisplayNameLookupSamples, SpawnRecords.Num());
	if (ActualDisplayNameSamples > 0)
	{
		const double DisplayLookupStart = FPlatformTime::Seconds();
		int32 SuccessfulDisplayLookups = 0;
		for (int32 SampleIndex = 0; SampleIndex < ActualDisplayNameSamples; ++SampleIndex)
		{
			PumpHarnessWork();
			const FSpawnRecord& Record = SpawnRecords[RandomStream.RandRange(0, SpawnRecords.Num() - 1)];
			if (Record.DisplayName.IsNone())
			{
				continue;
			}

			UGorgeousObjectVariable* FoundByName = UGorgeousRootObjectVariable::FindVariableByDisplayName(Record.DisplayName);
			TestTrue(TEXT("Display-name lookup should succeed"), IsValid(FoundByName));
			SuccessfulDisplayLookups += IsValid(FoundByName) ? 1 : 0;
		}

		const FPerfSample DisplayLookupSample{ FPlatformTime::Seconds() - DisplayLookupStart, ActualDisplayNameSamples };
		RecordStat(EPerfStatCategory::Performance, DescribeSample(TEXT("Display-name lookup"), DisplayLookupSample));
		UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Display-name lookups succeeded %d/%d in %.2f seconds"),
			SuccessfulDisplayLookups, ActualDisplayNameSamples, DisplayLookupSample.Seconds);
	}

	const int32 ActualRegistrationSamples = FMath::Min(Scenario.RegistrationValidationSamples, SpawnRecords.Num());
	if (ActualRegistrationSamples > 0)
	{
		const double RegistrationStart = FPlatformTime::Seconds();
		for (int32 SampleIndex = 0; SampleIndex < ActualRegistrationSamples; ++SampleIndex)
		{
			PumpHarnessWork();
			const FSpawnRecord& Record = SpawnRecords[RandomStream.RandRange(0, SpawnRecords.Num() - 1)];
			TestTrue(TEXT("IsVariableRegistered should report true"), UGorgeousRootObjectVariable::IsVariableRegistered(Record.Node));
		}

		const FPerfSample RegistrationSample{ FPlatformTime::Seconds() - RegistrationStart, ActualRegistrationSamples };
		RecordStat(EPerfStatCategory::Performance, DescribeSample(TEXT("Registry membership checks"), RegistrationSample));
		UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Registry membership checks completed in %.2f seconds"), RegistrationSample.Seconds);
	}

	const double HierarchyStart = FPlatformTime::Seconds();
	UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Flattening hierarchy across %d roots (expecting %d entries)..."), ActiveRoots.Num(), SpawnedVariables.Num());
	TArray<UGorgeousObjectVariable*> FlattenedRegistry;
	FlattenedRegistry.Reserve(SpawnedVariables.Num());
	for (const FActiveRootContext& Context : ActiveRoots)
	{
		TArray<UGorgeousObjectVariable*> RootEntries = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry(Context.RootName);
		FlattenedRegistry.Append(RootEntries);
		UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Root %s hierarchy contributed %d entries"), *DescribeRootLabel(Context.RootName), RootEntries.Num());
	}
	const FPerfSample HierarchySample{ FPlatformTime::Seconds() - HierarchyStart, FlattenedRegistry.Num() };
	RecordStat(EPerfStatCategory::Performance, DescribeSample(TEXT("Hierarchy query"), HierarchySample));
	UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Hierarchy traversal returned %d entries in %.2f seconds"), HierarchySample.Count, HierarchySample.Seconds);
	TestTrue(TEXT("Hierarchy traversal budget respected"), HierarchySample.Seconds <= Scenario.HierarchyBudgetSeconds);

	const int32 ActualDeepLookupSamples = FMath::Min(Scenario.DeepLookupSamples, SpawnRecords.Num());
	if (ActualDeepLookupSamples > 0)
	{
		const double DeepLookupStart = FPlatformTime::Seconds();
		int64 TotalDepth = 0;
		int32 MaxObservedDepth = 0;
		for (int32 SampleIndex = 0; SampleIndex < ActualDeepLookupSamples; ++SampleIndex)
		{
			PumpHarnessWork();
			const FSpawnRecord& Record = SpawnRecords[RandomStream.RandRange(0, SpawnRecords.Num() - 1)];
			UGorgeousObjectVariable* Found = UGorgeousRootObjectVariable::FindVariableByIdentifier(Record.Identifier);
			TestTrue(TEXT("Deep lookup should succeed"), IsValid(Found));

			int32 DepthSteps = 0;
			UGorgeousRootObjectVariable* OwningRoot = Record.Root;
			if (!OwningRoot && ActiveRoots.Num() > 0)
			{
				OwningRoot = ActiveRoots[0].Root;
			}
			for (UGorgeousObjectVariable* Current = Found; Current; Current = Current->GetParent())
			{
				++DepthSteps;
				if (Current == OwningRoot)
				{
					break;
				}
			}

			TotalDepth += DepthSteps;
			MaxObservedDepth = FMath::Max(MaxObservedDepth, DepthSteps);
		}

		const FPerfSample DeepLookupSample{ FPlatformTime::Seconds() - DeepLookupStart, ActualDeepLookupSamples };
		const double AverageDepth = (ActualDeepLookupSamples > 0) ? static_cast<double>(TotalDepth) / ActualDeepLookupSamples : 0.0;
		RecordStat(EPerfStatCategory::Performance, DescribeSample(TEXT("Deep identifier lookup"), DeepLookupSample));
		RecordStat(EPerfStatCategory::Performance, FString::Printf(TEXT("Average depth %.2f (max %d)"), AverageDepth, MaxObservedDepth));
		UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Deep lookup sample (%d paths, avg depth %.2f, max %d) finished in %.2f seconds"),
			ActualDeepLookupSamples, AverageDepth, MaxObservedDepth, DeepLookupSample.Seconds);
	}

	const int32 ActualSubtreeSamples = FMath::Min(Scenario.SubtreeSamples, SpawnRecords.Num());
	if (ActualSubtreeSamples > 0)
	{
		const double SubtreeStart = FPlatformTime::Seconds();
		int64 TotalEnumerated = 0;
		int32 MaxEnumerated = 0;
		for (int32 SampleIndex = 0; SampleIndex < ActualSubtreeSamples; ++SampleIndex)
		{
			PumpHarnessWork();
			const FSpawnRecord& Record = SpawnRecords[RandomStream.RandRange(0, SpawnRecords.Num() - 1)];
			const int32 Enumerated = EnumerateSubtree(Record.Node);
			TotalEnumerated += Enumerated;
			MaxEnumerated = FMath::Max(MaxEnumerated, Enumerated);
		}

		const FPerfSample SubtreeSample{ FPlatformTime::Seconds() - SubtreeStart, ActualSubtreeSamples };
		const double AverageEnumerated = (ActualSubtreeSamples > 0) ? static_cast<double>(TotalEnumerated) / ActualSubtreeSamples : 0.0;
		RecordStat(EPerfStatCategory::Performance, DescribeSample(TEXT("Subtree enumeration"), SubtreeSample));
		RecordStat(EPerfStatCategory::Performance, FString::Printf(TEXT("Average subtree size %.2f (max %d)"), AverageEnumerated, MaxEnumerated));
		UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Subtree enumeration sample (%d probes, avg size %.2f, max %d) finished in %.2f seconds"),
			ActualSubtreeSamples, AverageEnumerated, MaxEnumerated, SubtreeSample.Seconds);
	}

	const int32 ActualReferenceSamples = FMath::Min(Scenario.ReferenceHopSamples, SpawnRecords.Num());
	if (bShouldAssignCrossRefs && ActualReferenceSamples > 0)
	{
		const double HopStart = FPlatformTime::Seconds();
		int64 TotalHops = 0;
		int32 SuccessfulChains = 0;
		int32 MaxChain = 0;
		for (int32 SampleIndex = 0; SampleIndex < ActualReferenceSamples; ++SampleIndex)
		{
			PumpHarnessWork();
			const FSpawnRecord& Record = SpawnRecords[RandomStream.RandRange(0, SpawnRecords.Num() - 1)];
			UGorgeousPerfBaseObjectVariable* Current = Cast<UGorgeousPerfBaseObjectVariable>(Record.Node);
			if (!Current)
			{
				continue;
			}

			TSet<UGorgeousPerfBaseObjectVariable*> Visited;
			Visited.Reserve(Scenario.ReferenceHopMaxDepth);
			int32 HopCount = 0;
			while (HopCount < Scenario.ReferenceHopMaxDepth && Current)
			{
				const TArray<TWeakObjectPtr<UGorgeousObjectVariable>>& Links = Current->GetCrossReferences();
				if (Links.Num() == 0)
				{
					break;
				}

				UGorgeousPerfBaseObjectVariable* NextNode = nullptr;
				for (int32 Attempt = 0; Attempt < Links.Num(); ++Attempt)
				{
					UGorgeousObjectVariable* Candidate = Links[RandomStream.RandRange(0, Links.Num() - 1)].Get();
					if (UGorgeousPerfBaseObjectVariable* CandidatePerf = Cast<UGorgeousPerfBaseObjectVariable>(Candidate))
					{
						if (!Visited.Contains(CandidatePerf) && CandidatePerf != Current)
						{
							NextNode = CandidatePerf;
							break;
						}
					}
				}

				if (!NextNode)
				{
					break;
				}

				Visited.Add(Current);
				Current = NextNode;
				++HopCount;
				if (HopCount >= Scenario.ReferenceHopMaxDepth)
				{
					break;
				}
			}

			if (HopCount > 0)
			{
				++SuccessfulChains;
				TotalHops += HopCount;
				MaxChain = FMath::Max(MaxChain, HopCount);
			}

		}

		const FPerfSample ReferenceHopSample{ FPlatformTime::Seconds() - HopStart, ActualReferenceSamples };
		RecordStat(EPerfStatCategory::Performance, DescribeSample(TEXT("Reference hopping"), ReferenceHopSample));
		const double AverageHopDepth = (SuccessfulChains > 0) ? static_cast<double>(TotalHops) / SuccessfulChains : 0.0;
		UE_LOG(LogGorgeousObjectVariablePerf, Display,
			TEXT("[OVPerf] Reference hopping sampled %d chains | success %d | avg hops %.2f | max %d | %.2f seconds"),
			ActualReferenceSamples, SuccessfulChains, AverageHopDepth, MaxChain, ReferenceHopSample.Seconds);
		RecordStat(EPerfStatCategory::Performance, FString::Printf(TEXT("Reference hops avg %.2f (max %d, success %d/%d)"), AverageHopDepth, MaxChain, SuccessfulChains, ActualReferenceSamples));
		TestTrue(TEXT("Reference hopping budget respected"), ReferenceHopSample.Seconds <= Scenario.ReferenceHopBudgetSeconds);
	}
	else if (Scenario.ReferenceHopSamples > 0)
	{
		RecordStat(EPerfStatCategory::Performance, TEXT("Reference hopping skipped (no cross references available)."));
	}

	const bool bHasNetworkingRoots = RootStats.ContainsByPredicate([](const FRootRegistryStats& Stats)
	{
		return Stats.bSupportsNetworking;
	});

	const int32 TotalNetworkingCandidates = NetworkingEnableCandidates;
	if (TotalNetworkingCandidates > 0)
	{
		RecordStat(EPerfStatCategory::Replication, FString::Printf(TEXT("Networking candidates %d | SetNetworking() failures %d | missing replicated net context %d"),
			TotalNetworkingCandidates,
			NetworkingEnableFailures,
			ReplicatedOwnerFailures));
		TestTrue(TEXT("Scenario networking toggled SetNetworkingEnabled on all opted-in perf variables"), NetworkingEnableFailures == 0);
		if (ReplicatedOwnerFailures > 0)
		{
			const FString WarningLine = FString::Printf(TEXT("%d networking-enabled variables lacked a replicated world with an active NetDriver; replication batches may stay at zero."), ReplicatedOwnerFailures);
			AddWarning(WarningLine);
			RecordStat(EPerfStatCategory::Replication, WarningLine);
		}
	}
	else if (bHasNetworkingRoots)
	{
		AddWarning(TEXT("Networking-enabled roots detected, but no perf variables opted into networking in this scenario."));
	}
	else
	{
		RecordStat(EPerfStatCategory::Replication, TEXT("Networking-enabled perf variables were not spawned for this scenario."));
	}

	if (Scenario.ReplicationValidationSamples > 0)
	{
		if (bHasNetworkingRoots)
		{
			TArray<int32> ReplicationCandidates;
			ReplicationCandidates.Reserve(SpawnRecords.Num());
			for (int32 Index = 0; Index < SpawnRecords.Num(); ++Index)
			{
				const FSpawnRecord& Record = SpawnRecords[Index];
				if (!Record.Node)
				{
					continue;
				}

				if (Record.bEnforcedNetworking || Record.bNetworkingOptIn)
				{
					ReplicationCandidates.Add(Index);
				}
			}

			const int32 DesiredReplicationSamples = FMath::Min(Scenario.ReplicationValidationSamples, ReplicationCandidates.Num());
			if (DesiredReplicationSamples > 0)
			{
				const double ReplicationStart = FPlatformTime::Seconds();
				for (const int32 CandidateIndex : ReplicationCandidates)
				{
					if (UGorgeousPerfBaseObjectVariable* PerfCandidate = Cast<UGorgeousPerfBaseObjectVariable>(SpawnRecords[CandidateIndex].Node))
					{
						SpawnRecords[CandidateIndex].StimulusBaselineSequence = PerfCandidate->GetPerfReplicationStimulusCount();
					}
				}
				int32 ActiveMatches = 0;
				int32 EnforcedFailures = 0;
				int32 OptionalFailures = 0;
				int32 StimulusApplications = 0;
				TSet<int32> StimulatedReplicationIndices;
				StimulatedReplicationIndices.Reserve(DesiredReplicationSamples);
				for (int32 SampleIndex = 0; SampleIndex < DesiredReplicationSamples; ++SampleIndex)
				{
					const int32 CandidateIndex = ReplicationCandidates[RandomStream.RandRange(0, ReplicationCandidates.Num() - 1)];
					const FSpawnRecord& Record = SpawnRecords[CandidateIndex];
					if (UGorgeousPerfBaseObjectVariable* StimulusNode = Cast<UGorgeousPerfBaseObjectVariable>(Record.Node))
					{
						const double StimulusStamp = FPlatformTime::Seconds();
						StimulusNode->InjectReplicationStimulus(static_cast<float>(StimulusStamp), RandomStream.FRandRange(-1.0f, 1.0f));
						StimulatedReplicationIndices.Add(CandidateIndex);
						++StimulusApplications;
					}
					PumpHarnessWork();
					bool bSimulatedDelivered = true;
					if (NetworkRuntime.bEnabled && NetworkRuntime.MaxLatencySeconds >= NetworkRuntime.MinLatencySeconds)
					{
						const double SimulatedLatencySeconds = (NetworkRuntime.MaxLatencySeconds > NetworkRuntime.MinLatencySeconds)
							? RandomStream.FRandRange(NetworkRuntime.MinLatencySeconds, NetworkRuntime.MaxLatencySeconds)
							: NetworkRuntime.MaxLatencySeconds;
						ReplicationTiming.RecordSimulatedLatency(SimulatedLatencySeconds);
						if (RandomStream.GetFraction() < NetworkRuntime.DropChance)
						{
							bSimulatedDelivered = false;
							ReplicationTiming.RecordSimulatedDrop();
						}
						else if (NetworkRuntime.bForceReplicationFailures && RandomStream.GetFraction() < NetworkRuntime.ReplicationFailureProbability)
						{
							bSimulatedDelivered = false;
						}
					}

					if (NetworkRuntime.bEnabled)
					{
						ReplicationTiming.RecordSimulatedOutcome(bSimulatedDelivered);
					}
					const bool bSimulatedFailure = NetworkRuntime.bForceReplicationFailures && !bSimulatedDelivered;
					const double CheckStart = FPlatformTime::Seconds();
					bool bReplicationActive = Record.Node && Record.Node->IsReplicationActive();
					if (bSimulatedFailure)
					{
						bReplicationActive = false;
					}
					const double CheckSeconds = FPlatformTime::Seconds() - CheckStart;
					ReplicationTiming.RecordValidation(CheckSeconds);
					FRootRegistryStats& RootEntry = FindOrAddRootStats(RootStats, Record.Root, Record.RootName, Record.ResolvedRootKey);
					RootEntry.ValidationTiming.AddSample(CheckSeconds);
					if (bReplicationActive)
					{
						++ActiveMatches;
					}
					else if (Record.bEnforcedNetworking)
					{
						++EnforcedFailures;
					}
					else
					{
						++OptionalFailures;
					}
				}

				const int32 PostStimulusFlushPasses = 3;
				for (int32 FlushIndex = 0; FlushIndex < PostStimulusFlushPasses; ++FlushIndex)
				{
					PumpHarnessWork();
				}

				if (StimulusApplications > 0 && StimulatedReplicationIndices.Num() > 0)
				{
					int32 TotalStimulatedSends = 0;
					int32 TotalStimulatedReceives = 0;
					double LatestStimulatedSend = 0.0;
					int32 StimulusMutationFailures = 0;
					for (const int32 StimulatedIndex : StimulatedReplicationIndices)
					{
						if (StimulatedIndex >= 0 && StimulatedIndex < SpawnRecords.Num())
						{
							if (UGorgeousPerfBaseObjectVariable* StimulatedNode = Cast<UGorgeousPerfBaseObjectVariable>(SpawnRecords[StimulatedIndex].Node))
							{
								TotalStimulatedSends += StimulatedNode->GetPerfNetSendCount();
								TotalStimulatedReceives += StimulatedNode->GetPerfNetReceiveCount();
								LatestStimulatedSend = FMath::Max(LatestStimulatedSend, StimulatedNode->GetPerfLastNetSendSeconds());
								const int32 BaselineSequence = SpawnRecords[StimulatedIndex].StimulusBaselineSequence;
								if (StimulatedNode->GetPerfReplicationStimulusCount() <= BaselineSequence)
								{
									++StimulusMutationFailures;
								}
							}
						}
					}
					const FString StimulusSummary = FString::Printf(TEXT("Replication stimulus applied to %d nodes (%d mutations) -> sends=%d receives=%d lastSend=%.3fs"),
						StimulatedReplicationIndices.Num(),
						StimulusApplications,
						TotalStimulatedSends,
						TotalStimulatedReceives,
						LatestStimulatedSend);
					RecordStat(EPerfStatCategory::Replication, StimulusSummary);
					UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] %s"), *StimulusSummary);
					TestTrue(TEXT("Replication stimulus dirtied perf payloads"), StimulusMutationFailures == 0);
					if (StimulusMutationFailures > 0)
					{
						RecordStat(EPerfStatCategory::Replication, TEXT("One or more replication stimuli failed to mutate their payloads (check stimulus configuration)."));
					}
				}
				else
				{
					RecordStat(EPerfStatCategory::Replication, TEXT("Replication stimulus skipped (no eligible replication-enabled nodes located)."));
				}

				const FPerfSample ReplicationSample{ FPlatformTime::Seconds() - ReplicationStart, DesiredReplicationSamples };
				RecordStat(EPerfStatCategory::Replication, DescribeSample(TEXT("Replication validation"), ReplicationSample));
				RecordStat(EPerfStatCategory::Replication, FString::Printf(TEXT("Replication active %d/%d (enforced failures %d | optional failures %d)"),
					ActiveMatches, DesiredReplicationSamples, EnforcedFailures, OptionalFailures));
				UE_LOG(LogGorgeousObjectVariablePerf, Display,
					TEXT("[OVPerf] Replication validation checked %d nodes | active=%d | enforcedFail=%d | optionalFail=%d"),
					DesiredReplicationSamples, ActiveMatches, EnforcedFailures, OptionalFailures);
				TestTrue(TEXT("Replication validation budget respected"),
					ReplicationSample.Seconds <= Scenario.ReplicationValidationBudgetSeconds);
				if (!NetworkRuntime.bForceReplicationFailures)
				{
					TestTrue(TEXT("Networking roots honor enforced replication"), EnforcedFailures == 0);
				}
				else if (EnforcedFailures > 0)
				{
					AddWarning(TEXT("Chaos network preset forced replication failures; ignoring enforced replication assertion."));
				}
			}
			else
			{
				RecordStat(EPerfStatCategory::Replication, TEXT("Replication validation skipped (no replication-enabled entries were spawned)."));
			}
		}
		else
		{
			RecordStat(EPerfStatCategory::Replication, TEXT("Replication validation skipped (no networking-capable roots were active)."));
		}
	}

	if (ReplicationTiming.ActivationTiming.HasSamples())
	{
		const FString ActivationSummary = DescribeTimingBucket(TEXT("Replication activation timing"), ReplicationTiming.ActivationTiming);
		RecordStat(EPerfStatCategory::Replication, ActivationSummary);
		UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] %s"), *ActivationSummary);
	}

	if (ReplicationTiming.ValidationTiming.HasSamples())
	{
		const FString ValidationSummary = DescribeTimingBucket(TEXT("Replication check timing"), ReplicationTiming.ValidationTiming);
		RecordStat(EPerfStatCategory::Replication, ValidationSummary);
		UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] %s"), *ValidationSummary);
	}

	if (NetworkRuntime.bEnabled)
	{
		if (ReplicationTiming.SimulatedLatencySamples > 0)
		{
			const double AvgLatencyMs = (ReplicationTiming.SimulatedNetworkSeconds / ReplicationTiming.SimulatedLatencySamples) * 1000.0;
			const int32 SimulatedTotal = ReplicationTiming.SimulatedDeliveries + ReplicationTiming.SimulatedFailures;
			const FString SimSummary = FString::Printf(
				TEXT("Network emulation %s -> avg latency %.2f ms (range %.1f-%.1f ms) | simulated drops %d/%d | simulated deliveries %d/%d"),
				*NetworkRuntime.Label,
				AvgLatencyMs,
				NetworkRuntime.MinLatencySeconds * 1000.0,
				NetworkRuntime.MaxLatencySeconds * 1000.0,
				ReplicationTiming.SimulatedDrops,
				ReplicationTiming.SimulatedLatencySamples,
				ReplicationTiming.SimulatedDeliveries,
				SimulatedTotal);
			RecordStat(EPerfStatCategory::Networking, SimSummary);
			UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] %s"), *SimSummary);
		}
		else
		{
			RecordStat(EPerfStatCategory::Networking, TEXT("Network emulation preset active but no replication samples were collected."));
		}
	}

	if (bHasNetworkingRoots)
	{
		for (const FRootRegistryStats& Stats : RootStats)
		{
			if (!Stats.bSupportsNetworking)
			{
				continue;
			}

			const FString RootLabel = DescribeRootLabel(Stats.RootName);
			UE_LOG(LogGorgeousObjectVariablePerf, Display,
				TEXT("[OVPerf] Root %s networking -> enforce=%s | optIns=%d | active=%d | violations=%d"),
				*RootLabel,
				Stats.bEnforcesNetworking ? TEXT("true") : TEXT("false"),
				Stats.NetworkingOptInCount,
				Stats.NetworkingActiveCount,
				Stats.EnforcedNetworkingViolations);
			RecordStat(EPerfStatCategory::Replication, FString::Printf(TEXT("%s networking: optIns=%d active=%d violations=%d"),
				*RootLabel, Stats.NetworkingOptInCount, Stats.NetworkingActiveCount, Stats.EnforcedNetworkingViolations));
			if (Stats.ActivationTiming.HasSamples())
			{
				const FString ActivationLabel = FString::Printf(TEXT("%s replication activation"), *RootLabel);
				RecordStat(EPerfStatCategory::Replication, DescribeTimingBucket(*ActivationLabel, Stats.ActivationTiming));
			}

			if (Stats.ValidationTiming.HasSamples())
			{
				const FString ValidationLabel = FString::Printf(TEXT("%s replication validation"), *RootLabel);
				RecordStat(EPerfStatCategory::Replication, DescribeTimingBucket(*ValidationLabel, Stats.ValidationTiming));
			}
		}
	}

#if WITH_AUTOMATION_TESTS
	{
		int32 ObservedReplicationVariables = 0;
		int64 TotalSendBatches = 0;
		int64 TotalReceiveBatches = 0;
		TArray<FString> HighlightedVariables;
		TMap<FName, int64> RootSendBreakdown;
		for (int32 RecordIndex = 0; RecordIndex < SpawnRecords.Num(); ++RecordIndex)
		{
			const FSpawnRecord& Record = SpawnRecords[RecordIndex];
			const UGorgeousPerfBaseObjectVariable* PerfNode = Cast<UGorgeousPerfBaseObjectVariable>(Record.Node);
			if (!PerfNode)
			{
				continue;
			}

			const int32 SendCount = PerfNode->GetPerfNetSendCount();
			const int32 ReceiveCount = PerfNode->GetPerfNetReceiveCount();
			if (SendCount == 0 && ReceiveCount == 0)
			{
				continue;
			}

			++ObservedReplicationVariables;
			TotalSendBatches += SendCount;
			TotalReceiveBatches += ReceiveCount;
			RootSendBreakdown.FindOrAdd(Record.RootName) += SendCount;
			if (HighlightedVariables.Num() < 8)
			{
				const FString NodeLabel = !Record.DisplayName.IsNone()
					? Record.DisplayName.ToString()
					: FString::Printf(TEXT("PerfVar_%d"), RecordIndex);
				HighlightedVariables.Add(FString::Printf(TEXT("%s send=%d recv=%d"), *NodeLabel, SendCount, ReceiveCount));
			}
		}

		const FString PacketSummary = FString::Printf(TEXT("Perf variable net serialization\n  Server batches=%lld\n  Client batches=%lld\n  Instrumented variables=%d"),
			TotalSendBatches,
			TotalReceiveBatches,
			ObservedReplicationVariables);
		RecordStat(EPerfStatCategory::Replication, PacketSummary);
		UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] %s"), *PacketSummary);
		if (ObservedReplicationVariables == 0)
		{
			RecordStat(EPerfStatCategory::Replication, TEXT("No automation perf variables were serialized across the network (check replication activation / opt-in rate)."));
		}
		else if (HighlightedVariables.Num() > 0)
		{
			RecordStat(EPerfStatCategory::Replication, FString::Printf(TEXT("Sampled variable batches -> %s"), *FString::Join(HighlightedVariables, TEXT(", "))));
		}

		if (RootSendBreakdown.Num() > 0)
		{
			TArray<FString> RootBreakdownLines;
			RootBreakdownLines.Reserve(RootSendBreakdown.Num());
			for (const TPair<FName, int64>& Pair : RootSendBreakdown)
			{
				const FString RootLabel = DescribeRootLabel(Pair.Key);
				RootBreakdownLines.Add(FString::Printf(TEXT("%s=%lld"), *RootLabel, Pair.Value));
			}
			RootBreakdownLines.Sort();
			RecordStat(EPerfStatCategory::Replication, FString::Printf(TEXT("Replication batches per root -> %s"), *FString::Join(RootBreakdownLines, TEXT(", "))));
		}
	}
#endif // WITH_AUTOMATION_TESTS

	if (Scenario.MemorySampleCount > 0 && SpawnRecords.Num() > 0)
	{
		const int32 ActualMemorySamples = FMath::Min(Scenario.MemorySampleCount, SpawnRecords.Num());
		const double MemorySampleStart = FPlatformTime::Seconds();
		SIZE_T TotalExclusiveBytes = 0;
		SIZE_T TotalInclusiveBytes = 0;
		SIZE_T MinExclusiveBytes = TNumericLimits<SIZE_T>::Max();
		SIZE_T MaxExclusiveBytes = 0;
		SIZE_T MinInclusiveBytes = TNumericLimits<SIZE_T>::Max();
		SIZE_T MaxInclusiveBytes = 0;
		for (int32 SampleIndex = 0; SampleIndex < ActualMemorySamples; ++SampleIndex)
		{
			PumpHarnessWork();
			const FSpawnRecord& Record = SpawnRecords[RandomStream.RandRange(0, SpawnRecords.Num() - 1)];
			FArchiveCountMem MemCounter(Record.Node);
			const SIZE_T ExclusiveBytes = MemCounter.GetMax();
			const SIZE_T InclusiveBytes = MemCounter.GetNum();
			TotalExclusiveBytes += ExclusiveBytes;
			TotalInclusiveBytes += InclusiveBytes;
			MinExclusiveBytes = FMath::Min(MinExclusiveBytes, ExclusiveBytes);
			MaxExclusiveBytes = FMath::Max(MaxExclusiveBytes, ExclusiveBytes);
			MinInclusiveBytes = FMath::Min(MinInclusiveBytes, InclusiveBytes);
			MaxInclusiveBytes = FMath::Max(MaxInclusiveBytes, InclusiveBytes);
		}

		const FPerfSample MemorySample{ FPlatformTime::Seconds() - MemorySampleStart, ActualMemorySamples };
		const double AvgExclusiveKB = (ActualMemorySamples > 0) ? (static_cast<double>(TotalExclusiveBytes) / ActualMemorySamples) / 1024.0 : 0.0;
		const double AvgInclusiveKB = (ActualMemorySamples > 0) ? (static_cast<double>(TotalInclusiveBytes) / ActualMemorySamples) / 1024.0 : 0.0;
		RecordStat(EPerfStatCategory::Memory, DescribeSample(TEXT("Memory sampling"), MemorySample));
		RecordStat(EPerfStatCategory::Memory, FString::Printf(TEXT("Exclusive avg %.2f KB (min %.2f KB, max %.2f KB)"), AvgExclusiveKB, MinExclusiveBytes / 1024.0, MaxExclusiveBytes / 1024.0));
		RecordStat(EPerfStatCategory::Memory, FString::Printf(TEXT("Inclusive avg %.2f KB (min %.2f KB, max %.2f KB)"), AvgInclusiveKB, MinInclusiveBytes / 1024.0, MaxInclusiveBytes / 1024.0));
		UE_LOG(LogGorgeousObjectVariablePerf, Display,
			TEXT("[OVPerf] Memory sample (%d nodes) exclusive avg %.2f KB [min %.2f KB | max %.2f KB], inclusive avg %.2f KB [min %.2f KB | max %.2f KB]"),
			ActualMemorySamples,
			AvgExclusiveKB, MinExclusiveBytes / 1024.0, MaxExclusiveBytes / 1024.0,
			AvgInclusiveKB, MinInclusiveBytes / 1024.0, MaxInclusiveBytes / 1024.0);
	}

	if (SpawnRecords.Num() > 0)
	{
		const double RegistrySweepStart = FPlatformTime::Seconds();
		SIZE_T CombinedExclusiveBytes = 0;
		SIZE_T CombinedInclusiveBytes = 0;
		for (const FSpawnRecord& Record : SpawnRecords)
		{
			PumpHarnessWork();
			if (!IsValid(Record.Node))
			{
				continue;
			}

			FArchiveCountMem MemCounter(Record.Node);
			const SIZE_T ExclusiveBytes = MemCounter.GetMax();
			const SIZE_T InclusiveBytes = MemCounter.GetNum();
			CombinedExclusiveBytes += ExclusiveBytes;
			CombinedInclusiveBytes += InclusiveBytes;
			FRootRegistryStats& RootEntry = FindOrAddRootStats(RootStats, Record.Root, Record.RootName, Record.ResolvedRootKey);
			RootEntry.TotalExclusiveBytes += ExclusiveBytes;
			RootEntry.TotalInclusiveBytes += InclusiveBytes;
		}

		const double RegistrySweepSeconds = FPlatformTime::Seconds() - RegistrySweepStart;
		const double CombinedExclusiveMB = CombinedExclusiveBytes / (1024.0 * 1024.0);
		const double CombinedInclusiveMB = CombinedInclusiveBytes / (1024.0 * 1024.0);
		RecordStat(EPerfStatCategory::Memory, FString::Printf(TEXT("Registry footprint exclusive %.2f MB | inclusive %.2f MB (sweep %.2fs)"), CombinedExclusiveMB, CombinedInclusiveMB, RegistrySweepSeconds));
		UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Registry footprint -> exclusive %.2f MB | inclusive %.2f MB (sweep %.2fs)"),
			CombinedExclusiveMB, CombinedInclusiveMB, RegistrySweepSeconds);

		for (const FRootRegistryStats& Stats : RootStats)
		{
			const double RootExclusiveMB = Stats.TotalExclusiveBytes / (1024.0 * 1024.0);
			const double RootInclusiveMB = Stats.TotalInclusiveBytes / (1024.0 * 1024.0);
			UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Root %s footprint -> %d vars | exclusive %.2f MB inclusive %.2f MB"),
				*DescribeRootLabel(Stats.RootName), Stats.SpawnedCount, RootExclusiveMB, RootInclusiveMB);
			RecordStat(EPerfStatCategory::Memory, FString::Printf(TEXT("%s footprint: exclusive %.2f MB | inclusive %.2f MB"), *DescribeRootLabel(Stats.RootName), RootExclusiveMB, RootInclusiveMB));
		}
	}

	const FSystemStatSnapshot StatsAfterValidation = CaptureSystemStatsSnapshot();
	LogSystemDelta(TEXT("PostValidation"), StatsAfterCrossRefs, StatsAfterValidation);
	TickHarness(4);

	const double CleanupStart = FPlatformTime::Seconds();
	UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Cleaning up %d test variables..."), SpawnedVariables.Num());
	for (UGorgeousObjectVariable* Variable : SpawnedVariables)
	{
		PumpHarnessWork();
		UGorgeousRootObjectVariable::RemoveVariableFromRegistry(Variable);
	}
	bManualCleanupCompleted = true;
	const FPerfSample CleanupSample{ FPlatformTime::Seconds() - CleanupStart, SpawnedVariables.Num() };
	RecordStat(EPerfStatCategory::Cleanup, DescribeSample(TEXT("Cleanup"), CleanupSample));
	UE_LOG(LogGorgeousObjectVariablePerf, Display, TEXT("[OVPerf] Cleanup finished in %.2f seconds"), CleanupSample.Seconds);

	const FSystemStatSnapshot StatsAfterCleanup = CaptureSystemStatsSnapshot();
	LogSystemDelta(TEXT("PostCleanup"), StatsAfterValidation, StatsAfterCleanup);
	TickHarness(4);

	if (bGauntletActive && GEngine)
	{
		UNetDriver* NetDriver = nullptr;
		for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
		{
			UWorld* World = WorldContext.World();
			if (!World)
			{
				continue;
			}
			NetDriver = World->GetNetDriver();
			if (NetDriver)
			{
				RecordStat(EPerfStatCategory::Networking, FString::Printf(TEXT("Gauntlet world=%s | NetDriver=%s"), *World->GetName(), *NetDriver->GetName()));
				RecordStat(EPerfStatCategory::Networking, FString::Printf(TEXT("Gauntlet net traffic | InBytes=%llu OutBytes=%llu InPackets=%llu OutPackets=%llu"),
					static_cast<unsigned long long>(NetDriver->InBytes),
					static_cast<unsigned long long>(NetDriver->OutBytes),
					static_cast<unsigned long long>(NetDriver->InPackets),
					static_cast<unsigned long long>(NetDriver->OutPackets)));
				UNetConnection* Connection = NetDriver->ServerConnection;
				if (!Connection && NetDriver->ClientConnections.Num() > 0)
				{
					Connection = NetDriver->ClientConnections[0];
				}
				if (Connection)
				{
					RecordStat(EPerfStatCategory::Networking, FString::Printf(TEXT("Gauntlet conn state=%s | AvgPing=%.2f ms"),
						LexToString(Connection->GetConnectionState()),
						Connection->AvgLag * 1000.0));
				}
				break;
			}
		}
		if (!NetDriver)
		{
			RecordStat(EPerfStatCategory::Networking, TEXT("Gauntlet net stats unavailable: no active NetDriver found."));
		}
	}

	const FString SummaryPanelText = SummaryPanel.BuildPanel();
	if (!SummaryPanelText.IsEmpty())
	{
		TArray<FString> PanelLines;
		SummaryPanelText.ParseIntoArrayLines(PanelLines, true);
		if (PanelLines.Num() > 0)
		{
			AddInfo(TEXT(""));
			for (const FString& Line : PanelLines)
			{
				AddInfo(Line);
			}
		}
	}

	return true;
}

#endif // WITH_AUTOMATION_TESTS
