// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

#include "Automation/GorgeousAutomationTestMatrix.h"

#include "Algo/Sort.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	static FString BuildScenarioCommandLine(const FName ScenarioName)
	{
		return ScenarioName.IsNone()
			? FString()
			: FString::Printf(TEXT("Scenario=%s"), *ScenarioName.ToString());
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
				OutLines.Add(Prefix);
				break;
			}

			const int32 Remaining = Segment.Len() - Offset;
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

	class FAutomationResultPanelBuilder
	{
	public:
		explicit FAutomationResultPanelBuilder(FString InHeading)
			: Heading(MoveTemp(InHeading))
		{
		}

		void AddSection(const FString& Title, const TArray<FString>& InLines)
		{
			FSection Section;
			Section.Title = Title;
			for (const FString& Line : InLines)
			{
				if (!Line.IsEmpty())
				{
					Section.Lines.Add(Line);
				}
			}

			if (Section.Lines.Num() == 0)
			{
				return;
			}

			Sections.Add(MoveTemp(Section));
		}

		FString Build() const
		{
			if (Heading.IsEmpty() && Sections.Num() == 0)
			{
				return FString();
			}

			TArray<FString> BodyLines;
			if (!Heading.IsEmpty())
			{
				BodyLines.Add(Heading);
				BodyLines.Add(TEXT(""));
			}

			const int32 WrapWidth = MaxInnerWidth;
			for (int32 SectionIndex = 0; SectionIndex < Sections.Num(); ++SectionIndex)
			{
				const FSection& Section = Sections[SectionIndex];
				AppendWrappedLine(FString::Printf(TEXT("[%s]"), *Section.Title), TEXT(""), TEXT(""), WrapWidth, BodyLines);
				if (Section.Lines.Num() == 0)
				{
					AppendWrappedLine(TEXT("(no entries recorded)"), TEXT("  • "), TEXT("    "), WrapWidth, BodyLines);
				}
				else
				{
					for (const FString& Line : Section.Lines)
					{
						AppendWrappedLine(Line, TEXT("  • "), TEXT("    "), WrapWidth, BodyLines);
					}
				}

				if (SectionIndex < Sections.Num() - 1)
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
		struct FSection
		{
			FString Title;
			TArray<FString> Lines;
		};

		FString Heading;
		TArray<FSection> Sections;

		static constexpr int32 MinInnerWidth = 64;
		static constexpr int32 MaxInnerWidth = 110;
	};

	static FString BuildScenarioResultPanel(const FGorgeousAutomationScenarioContext& Context, const FGorgeousAutomationScenarioResult& Result)
	{
		FAutomationResultPanelBuilder Builder(Context.BuildScenarioLabel());

		TArray<FString> StatusLines;
		StatusLines.Add(FString::Printf(TEXT("Result: %s"), Result.bSuccess ? TEXT("PASS") : TEXT("FAIL")));
		StatusLines.Add(FString::Printf(TEXT("Errors=%d | Warnings=%d | Notes=%d | Metrics=%d"),
			Result.Errors.Num(), Result.Warnings.Num(), Result.Notes.Num(), Result.Metrics.Num()));
		if (!Context.Descriptor.Description.IsEmpty())
		{
			StatusLines.Add(Context.Descriptor.Description);
		}
		Builder.AddSection(TEXT("Status"), StatusLines);

		if (Result.Metrics.Num() > 0)
		{
			TArray<FString> MetricLines;
			Result.Metrics.GenerateKeyArray(MetricLines);
			MetricLines.Sort();
			for (FString& MetricKey : MetricLines)
			{
				const FString* const Value = Result.Metrics.Find(MetricKey);
				MetricKey = Value
					? FString::Printf(TEXT("%s = %s"), *MetricKey, **Value)
					: MetricKey;
			}
			Builder.AddSection(TEXT("Metrics"), MetricLines);
		}

		if (Result.Notes.Num() > 0)
		{
			Builder.AddSection(TEXT("Notes"), Result.Notes);
		}
		if (Result.Warnings.Num() > 0)
		{
			Builder.AddSection(TEXT("Warnings"), Result.Warnings);
		}
		if (Result.Errors.Num() > 0)
		{
			Builder.AddSection(TEXT("Errors"), Result.Errors);
		}

		return Builder.Build();
	}
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FGorgeousAutomationMatrixAutomationTest, "GorgeousCore.Automation.Matrix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
	EAutomationTestFlags::EngineFilter | EAutomationTestFlags::HighPriority);

void FGorgeousAutomationMatrixAutomationTest::GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	OutBeautifiedNames.Add(TEXT("All"));
	OutTestCommands.Add(TEXT(""));
}

bool FGorgeousAutomationMatrixAutomationTest::RunTest(const FString& Parameters)
{
	const FGorgeousAutomationMatrixRequest Request = FGorgeousAutomationMatrixRequest::FromParameters(Parameters);
	AddInfo(FString::Printf(TEXT("[AutomationMatrix] Request: %s"), *Request.Describe()));

	const TArray<FGorgeousAutomationScenarioDescriptor> Registered = FGorgeousAutomationTestMatrix::GetRegisteredScenarios();
	TArray<FGorgeousAutomationScenarioDescriptor> Targets = FGorgeousAutomationTestMatrix::GetFilteredScenarios(Request);

	if (Targets.Num() == 0)
	{
		if (Registered.Num() == 0)
		{
			AddWarning(TEXT("[AutomationMatrix] No scenarios registered. Ensure modules register their suites."));
			return true;
		}

		if (Request.ScenarioWhitelist.Num() > 0 || Request.TagFilters.Num() > 0)
		{
			AddError(FString::Printf(TEXT("[AutomationMatrix] Scenario filter produced no matches (%s)."), *Request.DescribeScenarioFilter()));
			return false;
		}

		AddWarning(TEXT("[AutomationMatrix] All registered scenarios are disabled."));
		return true;
	}

	const TArray<int32> Variants = Request.ResolveVariantIndices({ 0 });
	bool bAllPassed = true;

	for (const FGorgeousAutomationScenarioDescriptor& Descriptor : Targets)
	{
		for (const int32 Variant : Variants)
		{
			FGorgeousAutomationScenarioContext Context(Request, Parameters, Variant, *this, Descriptor);
			const FString ScenarioLabel = Context.BuildScenarioLabel();
			AddInfo(FString::Printf(TEXT("[AutomationMatrix] >>> %s"), *ScenarioLabel));

			const FGorgeousAutomationScenarioResult Result = FGorgeousAutomationTestMatrix::ExecuteScenario(Descriptor, Context);
			for (const FString& Note : Result.Notes)
			{
				AddInfo(FString::Printf(TEXT("[AutomationMatrix][%s] %s"), *ScenarioLabel, *Note));
			}

			for (const TPair<FString, FString>& Metric : Result.Metrics)
			{
				AddInfo(FString::Printf(TEXT("[AutomationMatrix][%s] Metric %s = %s"), *ScenarioLabel, *Metric.Key, *Metric.Value));
			}

			for (const FString& Warning : Result.Warnings)
			{
				AddWarning(FString::Printf(TEXT("[AutomationMatrix][%s] %s"), *ScenarioLabel, *Warning));
			}

			for (const FString& Error : Result.Errors)
			{
				AddError(FString::Printf(TEXT("[AutomationMatrix][%s] %s"), *ScenarioLabel, *Error));
			}

			const FString PanelText = BuildScenarioResultPanel(Context, Result);
			if (!PanelText.IsEmpty())
			{
				TArray<FString> PanelLines;
				PanelText.ParseIntoArrayLines(PanelLines, true);
				AddInfo(TEXT(""));
				for (const FString& Line : PanelLines)
				{
					AddInfo(Line);
				}
			}

			if (!Result.bSuccess)
			{
				bAllPassed = false;
				if (Request.bStopOnFailure)
				{
					AddWarning(TEXT("[AutomationMatrix] StopOnFailure triggered. Aborting remaining scenarios."));
					return false;
				}
			}
		}
	}

	return bAllPassed;
}

#endif // WITH_DEV_AUTOMATION_TESTS
