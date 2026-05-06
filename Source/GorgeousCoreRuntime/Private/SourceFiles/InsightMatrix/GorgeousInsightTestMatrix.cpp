// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Insight Matrix (Runtime)                   |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

#include "InsightMatrix/GorgeousInsightTestMatrix.h"

#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/ScopeLock.h"

namespace GorgeousInsightMatrix_Private
{
	static FCriticalSection RegistryMutex;

	struct FScenarioEntry
	{
		FGuid RegistrationId;
		FGorgeousInsightScenarioDescriptor Descriptor;
	};

	static TMap<FName, FScenarioEntry> ScenarioEntries;
}

using namespace GorgeousInsightMatrix_Private;

//=============================================================================
// FGorgeousInsightMatrixRequest
//=============================================================================

FGorgeousInsightMatrixRequest::FGorgeousInsightMatrixRequest()
	: bEnableSharedHarness(true)
	, bRequireMultiSession(true)
	, HarnessTimeoutSeconds(30.0f)
	, HarnessTickSeconds(0.05f)
	, bStopOnFailure(false)
{
}

FGorgeousInsightMatrixRequest FGorgeousInsightMatrixRequest::FromParameters(const FString& Parameters)
{
	FGorgeousInsightMatrixRequest Request;
	Request.RawParameters = Parameters;
	Request.ParameterMap = ParseKeyValueMap(Parameters);

	if (const FString* VariantToken = Request.ParameterMap.Find(TEXT("variant")))
	{
		ParseVariantList(*VariantToken, Request.VariantIndices);
	}
	else if (const FString* VariantsToken = Request.ParameterMap.Find(TEXT("variants")))
	{
		ParseVariantList(*VariantsToken, Request.VariantIndices);
	}
	else if (const FString* PresetToken = Request.ParameterMap.Find(TEXT("preset")))
	{
		ParseVariantList(*PresetToken, Request.VariantIndices);
	}
	else if (const FString* PresetsToken = Request.ParameterMap.Find(TEXT("presets")))
	{
		ParseVariantList(*PresetsToken, Request.VariantIndices);
	}

	if (const FString* ScenarioToken = Request.ParameterMap.Find(TEXT("scenario")))
	{
		ParseNameList(*ScenarioToken, Request.ScenarioWhitelist);
	}

	if (const FString* TagToken = Request.ParameterMap.Find(TEXT("tag")))
	{
		ParseNameList(*TagToken, Request.TagFilters);
	}
	else if (const FString* TagsToken = Request.ParameterMap.Find(TEXT("tags")))
	{
		ParseNameList(*TagsToken, Request.TagFilters);
	}

	if (Request.ScenarioWhitelist.Num() == 0)
	{
		FString ScenarioOverride;
		if (FParse::Value(FCommandLine::Get(), TEXT("GorgeousInsightScenario="), ScenarioOverride) && !ScenarioOverride.IsEmpty())
		{
			ParseNameList(ScenarioOverride, Request.ScenarioWhitelist);
		}
	}

	if (const FString* HarnessToken = Request.ParameterMap.Find(TEXT("harness")))
	{
		Request.bEnableSharedHarness = ParseBoolToken(*HarnessToken, Request.bEnableSharedHarness);
	}

	if (const FString* MultiSessionToken = Request.ParameterMap.Find(TEXT("multisession")))
	{
		Request.bRequireMultiSession = ParseBoolToken(*MultiSessionToken, Request.bRequireMultiSession);
	}

	if (const FString* TimeoutToken = Request.ParameterMap.Find(TEXT("harnesstimeout")))
	{
		Request.HarnessTimeoutSeconds = FCString::Atof(**TimeoutToken);
	}

	if (const FString* TickToken = Request.ParameterMap.Find(TEXT("harnesstick")))
	{
		Request.HarnessTickSeconds = FCString::Atof(**TickToken);
	}

	if (const FString* StopToken = Request.ParameterMap.Find(TEXT("stoponfailure")))
	{
		Request.bStopOnFailure = ParseBoolToken(*StopToken, Request.bStopOnFailure);
	}

	return Request;
}

bool FGorgeousInsightMatrixRequest::IsScenarioAllowed(const FName ScenarioName, const TArray<FName>& ScenarioTags) const
{
	if (ScenarioWhitelist.Num() > 0 && !ScenarioWhitelist.Contains(ScenarioName))
	{
		return false;
	}

	if (TagFilters.Num() == 0)
	{
		return true;
	}

	for (const FName Tag : ScenarioTags)
	{
		if (TagFilters.Contains(Tag))
		{
			return true;
		}
	}

	return false;
}

FString FGorgeousInsightMatrixRequest::Describe() const
{
	TArray<FString> Segments;

	TArray<FString> VariantStrings;
	const TArray<int32> EffectiveVariants = ResolveVariantIndices({ 0 });
	VariantStrings.Reserve(EffectiveVariants.Num());
	for (int32 Variant : EffectiveVariants)
	{
		VariantStrings.Add(FString::Printf(TEXT("%d"), Variant));
	}
	Segments.Add(FString::Printf(TEXT("Variants=[%s]"), *FString::Join(VariantStrings, TEXT(","))));

	Segments.Add(ScenarioWhitelist.Num() > 0
		? FString::Printf(TEXT("Scenarios=%s"), *DescribeScenarioFilter())
		: TEXT("Scenarios=All"));

	if (TagFilters.Num() > 0)
	{
		TArray<FName> Tags = TagFilters.Array();
		Tags.Sort(FNameLexicalLess());
		TArray<FString> TagLabels;
		TagLabels.Reserve(Tags.Num());
		for (const FName Tag : Tags)
		{
			TagLabels.Add(Tag.ToString());
		}
		Segments.Add(FString::Printf(TEXT("Tags=[%s]"), *FString::Join(TagLabels, TEXT(","))));
	}
	else
	{
		Segments.Add(TEXT("Tags=Any"));
	}

	Segments.Add(FString::Printf(TEXT("Harness=%s MultiSession=%s Timeout=%.1fs Tick=%.4fs"),
		bEnableSharedHarness ? TEXT("true") : TEXT("false"),
		bRequireMultiSession ? TEXT("true") : TEXT("false"),
		HarnessTimeoutSeconds,
		HarnessTickSeconds));

	if (bStopOnFailure)
	{
		Segments.Add(TEXT("StopOnFailure=true"));
	}

	return FString::Join(Segments, TEXT(" | "));
}

FString FGorgeousInsightMatrixRequest::DescribeScenarioFilter() const
{
	if (ScenarioWhitelist.Num() == 0)
	{
		return TEXT("<none>");
	}

	TArray<FName> Names = ScenarioWhitelist.Array();
	Names.Sort(FNameLexicalLess());
	TArray<FString> Labels;
	Labels.Reserve(Names.Num());
	for (const FName Name : Names)
	{
		Labels.Add(Name.ToString());
	}
	return FString::Join(Labels, TEXT(","));
}

TArray<int32> FGorgeousInsightMatrixRequest::ResolveVariantIndices(const TArray<int32>& DefaultVariants) const
{
	if (VariantIndices.Num() > 0)
	{
		return VariantIndices;
	}

	return DefaultVariants;
}

FString FGorgeousInsightMatrixRequest::GetParameterValue(const FString& Key) const
{
	if (const FString* Value = ParameterMap.Find(Key.ToLower()))
	{
		return *Value;
	}

	return FString();
}

bool FGorgeousInsightMatrixRequest::GetBoolParameter(const FString& Key, bool bDefaultValue) const
{
	const FString Value = GetParameterValue(Key);
	return ParseBoolToken(Value, bDefaultValue);
}

double FGorgeousInsightMatrixRequest::GetDoubleParameter(const FString& Key, double DefaultValue) const
{
	const FString Value = GetParameterValue(Key);
	return Value.IsEmpty() ? DefaultValue : FCString::Atod(*Value);
}

TMap<FString, FString> FGorgeousInsightMatrixRequest::ParseKeyValueMap(const FString& Parameters)
{
	TMap<FString, FString> Result;
	TArray<FString> Tokens;
	Parameters.ParseIntoArrayWS(Tokens);
	for (FString Token : Tokens)
	{
		Token.TrimStartAndEndInline();
		if (Token.IsEmpty())
		{
			continue;
		}

		FString Key;
		FString Value;
		if (!Token.Split(TEXT("="), &Key, &Value))
		{
			continue;
		}

		Key.TrimStartAndEndInline();
		Value.TrimStartAndEndInline();
		Result.Add(Key.ToLower(), Value);
	}
	return Result;
}

bool FGorgeousInsightMatrixRequest::ParseBoolToken(const FString& Token, const bool bDefaultValue)
{
	if (Token.IsEmpty())
	{
		return bDefaultValue;
	}

	if (Token.Equals(TEXT("true"), ESearchCase::IgnoreCase)
		|| Token.Equals(TEXT("1"), ESearchCase::IgnoreCase)
		|| Token.Equals(TEXT("yes"), ESearchCase::IgnoreCase))
	{
		return true;
	}

	if (Token.Equals(TEXT("false"), ESearchCase::IgnoreCase)
		|| Token.Equals(TEXT("0"), ESearchCase::IgnoreCase)
		|| Token.Equals(TEXT("no"), ESearchCase::IgnoreCase))
	{
		return false;
	}

	return bDefaultValue;
}

void FGorgeousInsightMatrixRequest::ParseVariantList(const FString& Token, TArray<int32>& OutVariants)
{
	TArray<FString> Parts;
	Token.ParseIntoArray(Parts, TEXT(","), true);
	for (FString Part : Parts)
	{
		Part.TrimStartAndEndInline();
		if (Part.IsEmpty())
		{
			continue;
		}
		OutVariants.Add(FCString::Atoi(*Part));
	}
}

void FGorgeousInsightMatrixRequest::ParseNameList(const FString& Token, TSet<FName>& OutNames)
{
	TArray<FString> Parts;
	Token.ParseIntoArray(Parts, TEXT(","), true);
	for (FString Part : Parts)
	{
		Part.TrimStartAndEndInline();
		if (Part.IsEmpty())
		{
			continue;
		}
		OutNames.Add(FName(*Part));
	}
}

//=============================================================================
// FGorgeousInsightScenarioContext
//=============================================================================

FGorgeousInsightScenarioContext::FGorgeousInsightScenarioContext(const FGorgeousInsightMatrixRequest& InRequest,
	const FString& InParameterString,
	const int32 InVariantIndex,
	FAutomationTestBase* InTest,
	const FGorgeousInsightScenarioDescriptor& InDescriptor,
	UObject* InWorldContextObject)
	: Request(InRequest)
	, ParameterString(InParameterString)
	, VariantIndex(InVariantIndex)
	, Test(InTest)
	, Descriptor(InDescriptor)
	, WorldContextObject(InWorldContextObject)
{
}

FString FGorgeousInsightScenarioContext::BuildScenarioLabel() const
{
	const FString Scenario = Descriptor.GetDisplayName();
	return FString::Printf(TEXT("%s [Variant %d]"), *Scenario, VariantIndex);
}

void FGorgeousInsightScenarioContext::AddInfo(const FString& Message) const
{
	if (Test) Test->AddInfo(Message);
}

void FGorgeousInsightScenarioContext::AddWarning(const FString& Message) const
{
	if (Test) Test->AddWarning(Message);
}

void FGorgeousInsightScenarioContext::AddError(const FString& Message) const
{
	if (Test) Test->AddError(Message);
}

//=============================================================================
// FGorgeousInsightScenarioRegistrar
//=============================================================================

FGorgeousInsightScenarioRegistrar::FGorgeousInsightScenarioRegistrar(FDescriptorFactory InFactory)
	: Factory(MoveTemp(InFactory))
{
	GetRegistrars().Add(this);
}

FGorgeousInsightScenarioRegistrar::~FGorgeousInsightScenarioRegistrar()
{
	Deactivate();
	GetRegistrars().Remove(this);
}

void FGorgeousInsightScenarioRegistrar::Activate()
{
	if (!Handle.IsValid() && Factory)
	{
		CachedDescriptor = Factory();
		Handle = FGorgeousInsightTestMatrix::RegisterScenario(CachedDescriptor);
	}
}

void FGorgeousInsightScenarioRegistrar::Deactivate()
{
	if (Handle.IsValid())
	{
		FGorgeousInsightTestMatrix::UnregisterScenario(Handle);
		Handle = FGorgeousInsightScenarioHandle();
	}
}

void FGorgeousInsightScenarioRegistrar::ActivateAll()
{
	for (FGorgeousInsightScenarioRegistrar* Registrar : GetRegistrars())
	{
		if (Registrar)
		{
			Registrar->Activate();
		}
	}
}

void FGorgeousInsightScenarioRegistrar::DeactivateAll()
{
	for (FGorgeousInsightScenarioRegistrar* Registrar : GetRegistrars())
	{
		if (Registrar)
		{
			Registrar->Deactivate();
		}
	}
}

TArray<FGorgeousInsightScenarioRegistrar*>& FGorgeousInsightScenarioRegistrar::GetRegistrars()
{
	static TArray<FGorgeousInsightScenarioRegistrar*> Registrars;
	return Registrars;
}

//=============================================================================
// FGorgeousInsightTestMatrix
//=============================================================================

FGorgeousInsightScenarioHandle FGorgeousInsightTestMatrix::RegisterScenario(const FGorgeousInsightScenarioDescriptor& Descriptor)
{
	FScopeLock Lock(&RegistryMutex);
	FGorgeousInsightScenarioHandle Handle;
	if (Descriptor.ScenarioName.IsNone())
	{
		return Handle;
	}

	FScenarioEntry Entry;
	Entry.RegistrationId = FGuid::NewGuid();
	Entry.Descriptor = Descriptor;
	ScenarioEntries.Add(Descriptor.ScenarioName, Entry);

	Handle.RegistrationId = Entry.RegistrationId;
	Handle.ScenarioName = Descriptor.ScenarioName;
	return Handle;
}

bool FGorgeousInsightTestMatrix::UnregisterScenario(const FGorgeousInsightScenarioHandle& Handle)
{
	FScopeLock Lock(&RegistryMutex);
	if (!Handle.IsValid())
	{
		return false;
	}

	FScenarioEntry* Entry = ScenarioEntries.Find(Handle.ScenarioName);
	if (!Entry || Entry->RegistrationId != Handle.RegistrationId)
	{
		return false;
	}

	ScenarioEntries.Remove(Handle.ScenarioName);
	return true;
}

TArray<FGorgeousInsightScenarioDescriptor> FGorgeousInsightTestMatrix::GetRegisteredScenarios()
{
	FScopeLock Lock(&RegistryMutex);
	TArray<FGorgeousInsightScenarioDescriptor> Results;
	Results.Reserve(ScenarioEntries.Num());
	for (const TPair<FName, FScenarioEntry>& Pair : ScenarioEntries)
	{
		Results.Add(Pair.Value.Descriptor);
	}
	return Results;
}

TArray<FGorgeousInsightScenarioDescriptor> FGorgeousInsightTestMatrix::GetFilteredScenarios(const FGorgeousInsightMatrixRequest& Request)
{
	FScopeLock Lock(&RegistryMutex);
	TArray<FGorgeousInsightScenarioDescriptor> Results;
	for (const TPair<FName, FScenarioEntry>& Pair : ScenarioEntries)
	{
		const FGorgeousInsightScenarioDescriptor& Descriptor = Pair.Value.Descriptor;
		if (!Descriptor.bEnabledByDefault)
		{
			continue;
		}
		if (!Request.IsScenarioAllowed(Descriptor.ScenarioName, Descriptor.Tags))
		{
			continue;
		}
		Results.Add(Descriptor);
	}

	Results.Sort([](const FGorgeousInsightScenarioDescriptor& A, const FGorgeousInsightScenarioDescriptor& B)
	{
		if (A.Priority == B.Priority)
		{
			return A.ScenarioName.LexicalLess(B.ScenarioName);
		}
		return A.Priority > B.Priority;
	});

	return Results;
}

FGorgeousInsightScenarioResult FGorgeousInsightTestMatrix::ExecuteScenario(const FGorgeousInsightScenarioDescriptor& Descriptor, const FGorgeousInsightScenarioContext& Context)
{
	if (!Descriptor.Runner)
	{
		FGorgeousInsightScenarioResult Result;
		Result.AddError(TEXT("No scenario runner registered."));
		return Result;
	}

	return Descriptor.Runner(Context);
}

TArray<FGorgeousInsightScenarioRunResult> FGorgeousInsightTestMatrix::ExecuteMatrix(const FGorgeousInsightMatrixRequest& Request,
	const FString& ParameterString,
	FAutomationTestBase& Test,
	UObject* WorldContextObject)
{
	TArray<FGorgeousInsightScenarioRunResult> Results;
	const TArray<FGorgeousInsightScenarioDescriptor> Targets = GetFilteredScenarios(Request);
	if (Targets.Num() == 0)
	{
		return Results;
	}

	const TArray<int32> Variants = Request.ResolveVariantIndices({ 0 });
	bool bStopRequested = false;
	for (const FGorgeousInsightScenarioDescriptor& Descriptor : Targets)
	{
		for (const int32 Variant : Variants)
		{
			FGorgeousInsightScenarioContext Context(Request, ParameterString, Variant, &Test, Descriptor, WorldContextObject);
			FGorgeousInsightScenarioRunResult Entry;
			Entry.Descriptor = Descriptor;
			Entry.VariantIndex = Variant;
			Entry.Result = ExecuteScenario(Descriptor, Context);
			Results.Add(Entry);

			if (Request.bStopOnFailure && !Entry.Result.bSuccess)
			{
				bStopRequested = true;
				break;
			}
		}

		if (bStopRequested)
		{
			break;
		}
	}

	return Results;
}
