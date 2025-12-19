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

#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/ScopeLock.h"

namespace GorgeousAutomationMatrix_Private
{
	static FCriticalSection RegistryMutex;

	struct FScenarioEntry
	{
		FGuid RegistrationId;
		FGorgeousAutomationScenarioDescriptor Descriptor;
	};

	static TMap<FName, FScenarioEntry> ScenarioEntries;
}

using namespace GorgeousAutomationMatrix_Private;

//=============================================================================
// FGorgeousAutomationMatrixRequest
//=============================================================================

FGorgeousAutomationMatrixRequest::FGorgeousAutomationMatrixRequest()
	: bEnableSharedHarness(true)
	, bRequireMultiSession(true)
	, HarnessTimeoutSeconds(30.0f)
	, HarnessTickSeconds(0.05f)
	, bStopOnFailure(false)
{
}

FGorgeousAutomationMatrixRequest FGorgeousAutomationMatrixRequest::FromParameters(const FString& Parameters)
{
	FGorgeousAutomationMatrixRequest Request;
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
		if (FParse::Value(FCommandLine::Get(), TEXT("GorgeousMatrixScenario="), ScenarioOverride) && !ScenarioOverride.IsEmpty())
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

bool FGorgeousAutomationMatrixRequest::IsScenarioAllowed(FName ScenarioName, const TArray<FName>& ScenarioTags) const
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

FString FGorgeousAutomationMatrixRequest::Describe() const
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

FString FGorgeousAutomationMatrixRequest::DescribeScenarioFilter() const
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

TArray<int32> FGorgeousAutomationMatrixRequest::ResolveVariantIndices(const TArray<int32>& DefaultVariants) const
{
	if (VariantIndices.Num() > 0)
	{
		return VariantIndices;
	}

	return DefaultVariants;
}

FString FGorgeousAutomationMatrixRequest::GetParameterValue(const FString& Key) const
{
	if (const FString* Value = ParameterMap.Find(Key.ToLower()))
	{
		return *Value;
	}

	return FString();
}

bool FGorgeousAutomationMatrixRequest::GetBoolParameter(const FString& Key, bool bDefaultValue) const
{
	const FString Value = GetParameterValue(Key);
	return ParseBoolToken(Value, bDefaultValue);
}

double FGorgeousAutomationMatrixRequest::GetDoubleParameter(const FString& Key, double DefaultValue) const
{
	const FString Value = GetParameterValue(Key);
	return Value.IsEmpty() ? DefaultValue : FCString::Atod(*Value);
}

TMap<FString, FString> FGorgeousAutomationMatrixRequest::ParseKeyValueMap(const FString& Parameters)
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

		Result.Add(Key.ToLower(), Value);
	}

	return Result;
}

bool FGorgeousAutomationMatrixRequest::ParseBoolToken(const FString& Token, bool bDefaultValue)
{
	if (Token.IsEmpty())
	{
		return bDefaultValue;
	}

	const FString Normalized = Token.ToLower();
	if (Normalized == TEXT("true") || Normalized == TEXT("1") || Normalized == TEXT("yes") || Normalized == TEXT("on"))
	{
		return true;
	}

	if (Normalized == TEXT("false") || Normalized == TEXT("0") || Normalized == TEXT("no") || Normalized == TEXT("off"))
	{
		return false;
	}

	return bDefaultValue;
}

void FGorgeousAutomationMatrixRequest::ParseVariantList(const FString& Token, TArray<int32>& OutVariants)
{
	TArray<FString> Parts;
	Token.ParseIntoArray(Parts, TEXT(","), true);
	if (Parts.Num() == 0)
	{
		Token.ParseIntoArray(Parts, TEXT("|"), true);
	}

	for (FString& Part : Parts)
	{
		Part.TrimStartAndEndInline();
		if (Part.IsEmpty())
		{
			continue;
		}

		if (Part.Equals(TEXT("all"), ESearchCase::IgnoreCase))
		{
			OutVariants.Reset();
			break;
		}

		int32 Value = 0;
		if (Part.Contains(TEXT("-")))
		{
			FString RangeStart;
			FString RangeEnd;
			if (Part.Split(TEXT("-"), &RangeStart, &RangeEnd))
			{
				const int32 StartValue = FCString::Atoi(*RangeStart);
				const int32 EndValue = FCString::Atoi(*RangeEnd);
				if (EndValue >= StartValue)
				{
					for (int32 Iter = StartValue; Iter <= EndValue; ++Iter)
					{
						OutVariants.AddUnique(Iter);
					}
				}
			}
			continue;
		}

		if (LexTryParseString(Value, *Part))
		{
			OutVariants.AddUnique(Value);
		}
	}
}

void FGorgeousAutomationMatrixRequest::ParseNameList(const FString& Token, TSet<FName>& OutNames)
{
	TArray<FString> Parts;
	Token.ParseIntoArray(Parts, TEXT(","), true);
	if (Parts.Num() == 0)
	{
		Token.ParseIntoArray(Parts, TEXT("|"), true);
	}
	if (Parts.Num() == 0)
	{
		Token.ParseIntoArray(Parts, TEXT("+"), true);
	}

	for (FString& Part : Parts)
	{
		Part.TrimStartAndEndInline();
		if (!Part.IsEmpty())
		{
			OutNames.Add(*Part);
		}
	}
}

//=============================================================================
// FGorgeousAutomationScenarioContext
//=============================================================================

FGorgeousAutomationScenarioContext::FGorgeousAutomationScenarioContext(const FGorgeousAutomationMatrixRequest& InRequest,
	const FString& InParameterString,
	int32 InVariantIndex,
	FAutomationTestBase& InTest,
	const FGorgeousAutomationScenarioDescriptor& InDescriptor)
	: Request(InRequest)
	, ParameterString(InParameterString)
	, VariantIndex(InVariantIndex)
	, Test(InTest)
	, Descriptor(InDescriptor)
{
}

FString FGorgeousAutomationScenarioContext::BuildScenarioLabel() const
{
	return FString::Printf(TEXT("%s [Variant=%d]"), *Descriptor.GetDisplayName(), VariantIndex);
}

void FGorgeousAutomationScenarioContext::AddInfo(const FString& Message) const
{
	Test.AddInfo(Message);
}

void FGorgeousAutomationScenarioContext::AddWarning(const FString& Message) const
{
	Test.AddWarning(Message);
}

void FGorgeousAutomationScenarioContext::AddError(const FString& Message) const
{
	Test.AddError(Message);
}

//=============================================================================
// FGorgeousAutomationScenarioRegistrar
//=============================================================================

TArray<FGorgeousAutomationScenarioRegistrar*>& FGorgeousAutomationScenarioRegistrar::GetRegistrars()
{
	static TArray<FGorgeousAutomationScenarioRegistrar*> Registrars;
	return Registrars;
}

FGorgeousAutomationScenarioRegistrar::FGorgeousAutomationScenarioRegistrar(FDescriptorFactory InFactory)
	: Factory(MoveTemp(InFactory))
{
	GetRegistrars().Add(this);
}

FGorgeousAutomationScenarioRegistrar::~FGorgeousAutomationScenarioRegistrar()
{
	Deactivate();
	GetRegistrars().RemoveSwap(this);
}

void FGorgeousAutomationScenarioRegistrar::Activate()
{
	if (!Handle.IsValid() && Factory)
	{
		CachedDescriptor = Factory();
		Handle = FGorgeousAutomationTestMatrix::RegisterScenario(CachedDescriptor);
	}
}

void FGorgeousAutomationScenarioRegistrar::Deactivate()
{
	if (Handle.IsValid())
	{
		FGorgeousAutomationTestMatrix::UnregisterScenario(Handle);
		Handle = FGorgeousAutomationScenarioHandle();
		CachedDescriptor = FGorgeousAutomationScenarioDescriptor();
	}
}

void FGorgeousAutomationScenarioRegistrar::ActivateAll()
{
	for (FGorgeousAutomationScenarioRegistrar* Registrar : GetRegistrars())
	{
		if (Registrar)
		{
			Registrar->Activate();
		}
	}
}

void FGorgeousAutomationScenarioRegistrar::DeactivateAll()
{
	for (FGorgeousAutomationScenarioRegistrar* Registrar : GetRegistrars())
	{
		if (Registrar)
		{
			Registrar->Deactivate();
		}
	}
}

//=============================================================================
// FGorgeousAutomationTestMatrix
//=============================================================================

FGorgeousAutomationScenarioHandle FGorgeousAutomationTestMatrix::RegisterScenario(const FGorgeousAutomationScenarioDescriptor& Descriptor)
{
	FGorgeousAutomationScenarioHandle Handle;
	if (Descriptor.ScenarioName.IsNone())
	{
		return Handle;
	}

	FScopeLock Lock(&RegistryMutex);

	FScenarioEntry& Entry = ScenarioEntries.FindOrAdd(Descriptor.ScenarioName);
	Entry.Descriptor = Descriptor;
	Entry.RegistrationId = FGuid::NewGuid();

	Handle.RegistrationId = Entry.RegistrationId;
	Handle.ScenarioName = Descriptor.ScenarioName;

	return Handle;
}

bool FGorgeousAutomationTestMatrix::UnregisterScenario(const FGorgeousAutomationScenarioHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return false;
	}

	FScopeLock Lock(&RegistryMutex);
	if (FScenarioEntry* Existing = ScenarioEntries.Find(Handle.ScenarioName))
	{
		if (Existing->RegistrationId == Handle.RegistrationId)
		{
			ScenarioEntries.Remove(Handle.ScenarioName);
			return true;
		}
	}

	return false;
}

TArray<FGorgeousAutomationScenarioDescriptor> FGorgeousAutomationTestMatrix::GetRegisteredScenarios()
{
	FScopeLock Lock(&RegistryMutex);

	TArray<FGorgeousAutomationScenarioDescriptor> Result;
	Result.Reserve(ScenarioEntries.Num());
	for (const TPair<FName, FScenarioEntry>& Pair : ScenarioEntries)
	{
		Result.Add(Pair.Value.Descriptor);
	}

	Result.Sort([](const FGorgeousAutomationScenarioDescriptor& LHS, const FGorgeousAutomationScenarioDescriptor& RHS)
	{
		if (LHS.Priority == RHS.Priority)
		{
			return LHS.GetDisplayName() < RHS.GetDisplayName();
		}
		return LHS.Priority > RHS.Priority;
	});

	return Result;
}

TArray<FGorgeousAutomationScenarioDescriptor> FGorgeousAutomationTestMatrix::GetFilteredScenarios(const FGorgeousAutomationMatrixRequest& Request)
{
	TArray<FGorgeousAutomationScenarioDescriptor> Registered = GetRegisteredScenarios();

	Registered = Registered.FilterByPredicate([&Request](const FGorgeousAutomationScenarioDescriptor& Descriptor)
	{
		const bool bExplicitlySelected = Request.ScenarioWhitelist.Contains(Descriptor.ScenarioName);
		if (!Descriptor.bEnabledByDefault && !bExplicitlySelected)
		{
			return false;
		}

		return Request.IsScenarioAllowed(Descriptor.ScenarioName, Descriptor.Tags);
	});

	return Registered;
}

FGorgeousAutomationScenarioResult FGorgeousAutomationTestMatrix::ExecuteScenario(const FGorgeousAutomationScenarioDescriptor& Descriptor, const FGorgeousAutomationScenarioContext& Context)
{
	if (!Descriptor.Runner)
	{
		FGorgeousAutomationScenarioResult Result;
		Result.bSuccess = false;
		Result.AddError(FString::Printf(TEXT("Scenario '%s' is missing a runner delegate."), *Descriptor.GetDisplayName()));
		return Result;
	}

	return Descriptor.Runner(Context);
}
