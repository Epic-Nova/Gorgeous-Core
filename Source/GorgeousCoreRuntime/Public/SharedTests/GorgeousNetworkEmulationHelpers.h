// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "Engine/NetDriver.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
struct FGorgeousInsightScenarioResult;
//<------------------------------------------------------------->
namespace FGorgeousNetworkEmulation
{
struct FPresetProfile
{
	int32 PresetIndex = INDEX_NONE;
	const TCHAR* Label = TEXT("Disabled");
	const TCHAR* Description = TEXT("");
	FPacketSimulationSettings Settings;
	double MinSimulatedLatencyMs = 0.0;
	double MaxSimulatedLatencyMs = 0.0;
	double DropChance = 0.0;
	bool bForceReplicationFailures = false;
	double FailureProbability = 0.0;
};

struct FRuntimeProfile
{
	bool bEnabled = false;
	bool bDriverSettingsApplied = false;
	int32 PresetIndex = INDEX_NONE;
	FString Label;
	FString Description;
	FString DriverWarning;
	double MinLatencySeconds = 0.0;
	double MaxLatencySeconds = 0.0;
	double DropChance = 0.0;
	bool bForceReplicationFailures = false;
	double ReplicationFailureProbability = 0.0;
};

bool BuildPreset(int32 PresetIndex, FPresetProfile& OutProfile);
bool ApplyProfile(const FPresetProfile& Profile, FString& OutWarning);
FRuntimeProfile ApplyPreset(int32 PresetIndex, FGorgeousInsightScenarioResult* OptionalResult = nullptr, const TCHAR* NotePrefix = TEXT("netemu"));

void SetSuiteDefaultPreset(int32 PresetIndex);
bool HasSuiteDefaultPreset();
int32 GetSuiteDefaultPreset();
FRuntimeProfile EnsureSuitePresetApplied(FGorgeousInsightScenarioResult* OptionalResult = nullptr);
void AppendRuntimeMetrics(const FRuntimeProfile& Runtime, FGorgeousInsightScenarioResult& Result, const TCHAR* MetricPrefix = TEXT("netemu."));
}