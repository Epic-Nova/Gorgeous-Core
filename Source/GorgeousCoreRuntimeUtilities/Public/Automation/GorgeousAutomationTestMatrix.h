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

#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"

class FAutomationTestBase;

/**
 * Base request shared by every Gorgeous Things automation matrix run.
 * Provides scenario filtering, parameter parsing, and shared harness toggles.
 */
struct GORGEOUSCORERUNTIMEUTILITIES_API FGorgeousAutomationMatrixRequest
{
	FGorgeousAutomationMatrixRequest();

	/** Raw automation parameter string provided by the framework. */
	FString RawParameters;

	/** Normalized parameter map (lowercase keys) for convenience lookups. */
	TMap<FString, FString> ParameterMap;

	/** Variant indices (e.g., network presets) to iterate. */
	TArray<int32> VariantIndices;

	/** Explicit scenario whitelist parsed from Scenario= parameters. */
	TSet<FName> ScenarioWhitelist;

	/** Optional tag filters (scenario must match at least one when provided). */
	TSet<FName> TagFilters;

	/** Whether the shared harness should spin up at all. */
	bool bEnableSharedHarness;

	/** Whether a multi-session (server+client) configuration is required. */
	bool bRequireMultiSession;

	/** Timeout in seconds for harness connections to settle. */
	float HarnessTimeoutSeconds;

	/** Tick delta passed to the shared harness when pumping work. */
	float HarnessTickSeconds;

	/** When true the matrix aborts immediately after the first failure. */
	bool bStopOnFailure;

	/** Builds a request from the automation parameter string. */
	static FGorgeousAutomationMatrixRequest FromParameters(const FString& Parameters);

	/** Returns true when the scenario name/tag combination is permitted by this filter. */
	bool IsScenarioAllowed(FName ScenarioName, const TArray<FName>& ScenarioTags) const;

	/** Returns a human-readable summary of filters and harness toggles. */
	FString Describe() const;

	/** Returns a human-readable description of the scenario filter. */
	FString DescribeScenarioFilter() const;

	/** Returns the variant indices, falling back to DefaultVariants when unset. */
	TArray<int32> ResolveVariantIndices(const TArray<int32>& DefaultVariants) const;

	/** Returns the value for Key or empty string when absent. */
	FString GetParameterValue(const FString& Key) const;

	/** Returns boolean parameter value with fallback. */
	bool GetBoolParameter(const FString& Key, bool bDefaultValue) const;

	/** Returns double parameter value with fallback. */
	double GetDoubleParameter(const FString& Key, double DefaultValue) const;

private:
	static TMap<FString, FString> ParseKeyValueMap(const FString& Parameters);
	static bool ParseBoolToken(const FString& Token, bool bDefaultValue);
	static void ParseVariantList(const FString& Token, TArray<int32>& OutVariants);
	static void ParseNameList(const FString& Token, TSet<FName>& OutNames);
};

/**
 * Result object returned by every registered scenario.
 */
struct GORGEOUSCORERUNTIMEUTILITIES_API FGorgeousAutomationScenarioResult
{
	bool bSuccess = true;
	TArray<FString> Errors;
	TArray<FString> Warnings;
	TArray<FString> Notes;
	TMap<FString, FString> Metrics;

	void AddError(const FString& Message)
	{
		Errors.Add(Message);
		bSuccess = false;
	}

	void AddWarning(const FString& Message)
	{
		Warnings.Add(Message);
	}

	void AddNote(const FString& Message)
	{
		Notes.Add(Message);
	}

	void AddMetric(const FString& Key, const FString& Value)
	{
		Metrics.Add(Key, Value);
	}

	void AddMetric(const FString& Key, double Value, const TCHAR* Unit = TEXT(""))
	{
		const FString ValueString = Unit && *Unit
			? FString::Printf(TEXT("%.4f %s"), Value, Unit)
			: FString::Printf(TEXT("%.4f"), Value);
		AddMetric(Key, ValueString);
	}
};

/**
 * Context passed to each scenario execution.
 */
struct GORGEOUSCORERUNTIMEUTILITIES_API FGorgeousAutomationScenarioContext
{
	FGorgeousAutomationScenarioContext(const FGorgeousAutomationMatrixRequest& InRequest,
		const FString& InParameterString,
		int32 InVariantIndex,
		FAutomationTestBase& InTest,
		const struct FGorgeousAutomationScenarioDescriptor& InDescriptor);

	const FGorgeousAutomationMatrixRequest& Request;
	const FString ParameterString;
	const int32 VariantIndex;
	FAutomationTestBase& Test;
	const struct FGorgeousAutomationScenarioDescriptor& Descriptor;

	FString BuildScenarioLabel() const;
	void AddInfo(const FString& Message) const;
	void AddWarning(const FString& Message) const;
	void AddError(const FString& Message) const;
};

using FGorgeousAutomationScenarioRunner = TFunction<FGorgeousAutomationScenarioResult(const FGorgeousAutomationScenarioContext&)>;

/**
 * Metadata describing a single registered scenario.
 */
struct GORGEOUSCORERUNTIMEUTILITIES_API FGorgeousAutomationScenarioDescriptor
{
	FName ScenarioName;
	FString DisplayName;
	FString Description;
	TArray<FName> Tags;
	int32 Priority = 0;
	bool bEnabledByDefault = true;
	FGorgeousAutomationScenarioRunner Runner;

	FString GetDisplayName() const
	{
		return DisplayName.IsEmpty() ? ScenarioName.ToString() : DisplayName;
	}
};

/** Handle returned during scenario registration. */
struct GORGEOUSCORERUNTIMEUTILITIES_API FGorgeousAutomationScenarioHandle
{
	FGuid RegistrationId;
	FName ScenarioName;

	bool IsValid() const
	{
		return RegistrationId.IsValid() && !ScenarioName.IsNone();
	}
};

/**
 * RAII helper that auto-registers a scenario at module load and unregisters during shutdown.
 */
class GORGEOUSCORERUNTIMEUTILITIES_API FGorgeousAutomationScenarioRegistrar
{
public:
	using FDescriptorFactory = TFunction<FGorgeousAutomationScenarioDescriptor()>;

	explicit FGorgeousAutomationScenarioRegistrar(FDescriptorFactory InFactory);
	~FGorgeousAutomationScenarioRegistrar();

	void Activate();
	void Deactivate();

	static void ActivateAll();
	static void DeactivateAll();

private:
	static TArray<FGorgeousAutomationScenarioRegistrar*>& GetRegistrars();

	FDescriptorFactory Factory;
	FGorgeousAutomationScenarioDescriptor CachedDescriptor;
	FGorgeousAutomationScenarioHandle Handle;
};

/**
 * Static utility that manages scenario registration and matrix execution helpers.
 */
class GORGEOUSCORERUNTIMEUTILITIES_API FGorgeousAutomationTestMatrix
{
public:
	static FGorgeousAutomationScenarioHandle RegisterScenario(const FGorgeousAutomationScenarioDescriptor& Descriptor);
	static bool UnregisterScenario(const FGorgeousAutomationScenarioHandle& Handle);
	static TArray<FGorgeousAutomationScenarioDescriptor> GetRegisteredScenarios();
	static TArray<FGorgeousAutomationScenarioDescriptor> GetFilteredScenarios(const FGorgeousAutomationMatrixRequest& Request);
	static FGorgeousAutomationScenarioResult ExecuteScenario(const FGorgeousAutomationScenarioDescriptor& Descriptor, const FGorgeousAutomationScenarioContext& Context);
};

#define GORGEOUS_AUTOMATION_CONCAT_INTERNAL(A, B) A##B
#define GORGEOUS_AUTOMATION_CONCAT(A, B) GORGEOUS_AUTOMATION_CONCAT_INTERNAL(A, B)

#define REGISTER_GORGEOUS_AUTOMATION_SCENARIO(DescriptorInit) \
	namespace \
	{ \
		static FGorgeousAutomationScenarioRegistrar GORGEOUS_AUTOMATION_CONCAT(GGorgeousAutomationScenarioRegistrar_, __LINE__)([]() { return DescriptorInit; }); \
	}
