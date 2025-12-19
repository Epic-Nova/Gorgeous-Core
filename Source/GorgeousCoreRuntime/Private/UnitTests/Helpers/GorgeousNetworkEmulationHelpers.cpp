#include "UnitTests/Helpers/GorgeousNetworkEmulationHelpers.h"

#include "Engine/Engine.h"
#include "Automation/GorgeousAutomationTestMatrix.h"

namespace FGorgeousNetworkEmulation
{
namespace
{
	TAtomic<int32> GSuitePreset{ INDEX_NONE };
	FRuntimeProfile GSuiteRuntime;
	bool GSuitePresetApplied = false;

	constexpr double MillisecondsToSeconds(const double Ms)
	{
		return Ms <= 0.0 ? 0.0 : Ms / 1000.0;
	}
}

bool BuildPreset(const int32 PresetIndex, FPresetProfile& OutProfile)
{
	OutProfile = FPresetProfile{};
	OutProfile.PresetIndex = PresetIndex;
	OutProfile.Settings.ResetSettings();

	switch (PresetIndex)
	{
	case 0:
		OutProfile.Label = TEXT("Pristine");
		OutProfile.Description = TEXT("LAN-style latency");
		OutProfile.Settings.PktLag = 20;
		OutProfile.Settings.PktLagVariance = 5;
		OutProfile.Settings.PktIncomingLagMin = 10;
		OutProfile.Settings.PktIncomingLagMax = 25;
		OutProfile.Settings.PktLoss = 0;
		OutProfile.Settings.PktIncomingLoss = 0;
		OutProfile.MinSimulatedLatencyMs = 10.0;
		OutProfile.MaxSimulatedLatencyMs = 30.0;
		OutProfile.DropChance = 0.001;
		break;

	case 1:
		OutProfile.Label = TEXT("ResidentialWiFi");
		OutProfile.Description = TEXT("mild jitter, light loss");
		OutProfile.Settings.PktLag = 55;
		OutProfile.Settings.PktLagVariance = 15;
		OutProfile.Settings.PktIncomingLagMin = 35;
		OutProfile.Settings.PktIncomingLagMax = 80;
		OutProfile.Settings.PktLoss = 1;
		OutProfile.Settings.PktIncomingLoss = 1;
		OutProfile.Settings.PktDup = 1;
		OutProfile.MinSimulatedLatencyMs = 35.0;
		OutProfile.MaxSimulatedLatencyMs = 90.0;
		OutProfile.DropChance = 0.01;
		break;

	case 2:
		OutProfile.Label = TEXT("BusyCafe");
		OutProfile.Description = TEXT("high jitter, noticeable loss");
		OutProfile.Settings.PktLag = 95;
		OutProfile.Settings.PktLagVariance = 35;
		OutProfile.Settings.PktIncomingLagMin = 70;
		OutProfile.Settings.PktIncomingLagMax = 160;
		OutProfile.Settings.PktLoss = 2;
		OutProfile.Settings.PktIncomingLoss = 2;
		OutProfile.Settings.PktDup = 2;
		OutProfile.MinSimulatedLatencyMs = 70.0;
		OutProfile.MaxSimulatedLatencyMs = 180.0;
		OutProfile.DropChance = 0.025;
		break;

	case 3:
		OutProfile.Label = TEXT("CongestedMobile");
		OutProfile.Description = TEXT("cell network congestion");
		OutProfile.Settings.PktLag = 140;
		OutProfile.Settings.PktLagVariance = 55;
		OutProfile.Settings.PktIncomingLagMin = 110;
		OutProfile.Settings.PktIncomingLagMax = 240;
		OutProfile.Settings.PktLoss = 4;
		OutProfile.Settings.PktIncomingLoss = 4;
		OutProfile.Settings.PktDup = 3;
		OutProfile.Settings.PktJitter = 30;
		OutProfile.MinSimulatedLatencyMs = 110.0;
		OutProfile.MaxSimulatedLatencyMs = 260.0;
		OutProfile.DropChance = 0.045;
		break;

	case 4:
		OutProfile.Label = TEXT("Severe");
		OutProfile.Description = TEXT("unstable connection");
		OutProfile.Settings.PktLag = 220;
		OutProfile.Settings.PktLagVariance = 80;
		OutProfile.Settings.PktIncomingLagMin = 180;
		OutProfile.Settings.PktIncomingLagMax = 360;
		OutProfile.Settings.PktLoss = 6;
		OutProfile.Settings.PktIncomingLoss = 6;
		OutProfile.Settings.PktDup = 4;
		OutProfile.Settings.PktJitter = 45;
		OutProfile.MinSimulatedLatencyMs = 180.0;
		OutProfile.MaxSimulatedLatencyMs = 380.0;
		OutProfile.DropChance = 0.08;
		break;

	case 5:
		OutProfile.Label = TEXT("Chaos");
		OutProfile.Description = TEXT("near-disaster packet conditions");
		OutProfile.Settings.PktLag = 420;
		OutProfile.Settings.PktLagVariance = 160;
		OutProfile.Settings.PktIncomingLagMin = 320;
		OutProfile.Settings.PktIncomingLagMax = 640;
		OutProfile.Settings.PktLoss = 20;
		OutProfile.Settings.PktIncomingLoss = 22;
		OutProfile.Settings.PktDup = 6;
		OutProfile.Settings.PktJitter = 80;
		OutProfile.Settings.PktOrder = 1;
		OutProfile.Settings.PktLagMin = 250;
		OutProfile.Settings.PktLagMax = 520;
		OutProfile.MinSimulatedLatencyMs = 320.0;
		OutProfile.MaxSimulatedLatencyMs = 700.0;
		OutProfile.DropChance = 0.65;
		OutProfile.bForceReplicationFailures = true;
		OutProfile.FailureProbability = 0.8;
		break;

	default:
		return false;
	}

	OutProfile.Settings.ValidateSettings();
	return true;
}

bool ApplyProfile(const FPresetProfile& Profile, FString& OutWarning)
{
#if DO_ENABLE_NET_TEST
	if (!GEngine)
	{
		OutWarning = TEXT("GEngine is not initialized; cannot target net drivers.");
		return false;
	}

	bool bApplied = false;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		for (const FNamedNetDriver& NetDriverEntry : Context.ActiveNetDrivers)
		{
			if (NetDriverEntry.NetDriver)
			{
				NetDriverEntry.NetDriver->SetPacketSimulationSettings(Profile.Settings);
				bApplied = true;
			}
		}
	}

	if (!bApplied)
	{
		OutWarning = TEXT("No active net drivers were found; preset applied logically only.");
	}
	return bApplied;
#else
	OutWarning = TEXT("Network emulation is disabled for this build configuration.");
	return false;
#endif
}

static void DescribeRuntimeNote(const FRuntimeProfile& Runtime, FGorgeousAutomationScenarioResult* OptionalResult, const TCHAR* NotePrefix)
{
	if (!OptionalResult)
	{
		return;
	}

	if (!Runtime.bEnabled)
	{
		OptionalResult->AddNote(TEXT("Network emulation preset: disabled."));
		return;
	}

	const FString Label = FString::Printf(TEXT("Preset %d (%s) - %s"), Runtime.PresetIndex, *Runtime.Label, *Runtime.Description);
	OptionalResult->AddNote(FString::Printf(TEXT("%sNetwork emulation: %s"), NotePrefix ? NotePrefix : TEXT(""), *Label));

	if (!Runtime.DriverWarning.IsEmpty())
	{
		OptionalResult->AddWarning(FString::Printf(TEXT("%sNetwork emulation warning: %s"), NotePrefix ? NotePrefix : TEXT(""), *Runtime.DriverWarning));
	}
}

FRuntimeProfile ApplyPreset(const int32 PresetIndex, FGorgeousAutomationScenarioResult* OptionalResult, const TCHAR* NotePrefix)
{
	FRuntimeProfile Runtime;
	if (PresetIndex < 0)
	{
		DescribeRuntimeNote(Runtime, OptionalResult, NotePrefix);
		return Runtime;
	}

	FPresetProfile Profile;
	if (!BuildPreset(PresetIndex, Profile))
	{
		if (OptionalResult)
		{
			OptionalResult->AddError(FString::Printf(TEXT("Unknown network emulation preset %d."), PresetIndex));
		}
		return Runtime;
	}

	Runtime.bEnabled = true;
	Runtime.PresetIndex = Profile.PresetIndex;
	Runtime.Label = Profile.Label;
	Runtime.Description = Profile.Description;
	Runtime.MinLatencySeconds = MillisecondsToSeconds(Profile.MinSimulatedLatencyMs);
	Runtime.MaxLatencySeconds = MillisecondsToSeconds(Profile.MaxSimulatedLatencyMs);
	Runtime.DropChance = FMath::Clamp(Profile.DropChance, 0.0, 1.0);
	Runtime.bForceReplicationFailures = Profile.bForceReplicationFailures;
	Runtime.ReplicationFailureProbability = FMath::Clamp(Profile.FailureProbability, 0.0, 1.0);

	FString Warning;
	Runtime.bDriverSettingsApplied = ApplyProfile(Profile, Warning);
	Runtime.DriverWarning = Warning;
	DescribeRuntimeNote(Runtime, OptionalResult, NotePrefix);
	return Runtime;
}

void SetSuiteDefaultPreset(const int32 PresetIndex)
{
	GSuitePreset = PresetIndex;
	GSuitePresetApplied = false;
	GSuiteRuntime = FRuntimeProfile();
}

bool HasSuiteDefaultPreset()
{
	return GSuitePreset.Load() >= 0;
}

int32 GetSuiteDefaultPreset()
{
	return GSuitePreset.Load();
}

FRuntimeProfile EnsureSuitePresetApplied(FGorgeousAutomationScenarioResult* OptionalResult)
{
	const int32 RequestedPreset = GSuitePreset.Load();
	if (RequestedPreset < 0)
	{
		return FRuntimeProfile();
	}

	if (!GSuitePresetApplied)
	{
		GSuiteRuntime = ApplyPreset(RequestedPreset, OptionalResult, TEXT("suite."));
		GSuitePresetApplied = true;
	}

	return GSuiteRuntime;
}

void AppendRuntimeMetrics(const FRuntimeProfile& Runtime, FGorgeousAutomationScenarioResult& Result, const TCHAR* MetricPrefix)
{
	if (!Runtime.bEnabled)
	{
		return;
	}

	const FString Prefix = MetricPrefix ? MetricPrefix : TEXT("netemu.");
	Result.AddMetric(Prefix + TEXT("minLatency"), Runtime.MinLatencySeconds * 1000.0);
	Result.AddMetric(Prefix + TEXT("maxLatency"), Runtime.MaxLatencySeconds * 1000.0);
	Result.AddMetric(Prefix + TEXT("dropChance"), Runtime.DropChance);
	Result.AddMetric(Prefix + TEXT("forceFailures"), Runtime.bForceReplicationFailures ? 1.0 : 0.0);
	Result.AddMetric(Prefix + TEXT("failureProb"), Runtime.ReplicationFailureProbability);
}

} // namespace FGorgeousNetworkEmulation
