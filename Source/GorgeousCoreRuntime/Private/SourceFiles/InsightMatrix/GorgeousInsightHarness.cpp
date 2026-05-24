// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Insight Matrix Harness (Runtime)            |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#include "InsightMatrix/GorgeousInsightHarness.h"

#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Templates/UniquePtr.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

#if GORGEOUSCORE_WITH_GAUNTLET
#include "GauntletModule.h"
#include "Modules/ModuleManager.h"
#endif

namespace
{
	static FString GInsightHarnessMapPath;
	static FString GInsightHarnessListenAddress;
	static double GInsightHarnessConnectTimeoutSeconds = 0.0;
	static double GInsightHarnessTickStepSeconds = 0.0;
    static bool GInsightGauntletCached = false;
    static bool GInsightGauntletCachedValue = false;
}

bool FGorgeousInsightHarness::StartHarness(const FGorgeousInsightMatrixRequest& Request)
{
	GT_I_LOG("GT.InsightMatrix.Harness.Start", TEXT("[InsightHarness] StartHarness: %s"), *Request.Describe());

	if (!Request.bEnableSharedHarness)
	{
		return true;
	}

	GInsightHarnessConnectTimeoutSeconds = Request.HarnessTimeoutSeconds;
	GInsightHarnessTickStepSeconds = Request.HarnessTickSeconds;

	const FString HarnessMap = Request.GetParameterValue(TEXT("harnessmap"));
	if (!HarnessMap.IsEmpty())
	{
		GInsightHarnessMapPath = HarnessMap;
	}

	const FString ListenAddress = Request.GetParameterValue(TEXT("harnessaddress"));
	if (!ListenAddress.IsEmpty())
	{
		GInsightHarnessListenAddress = ListenAddress;
	}

	if (IsGauntletActive())
	{
		GT_I_LOG("GT.InsightMatrix.Harness.Gauntlet", TEXT("[InsightHarness] Detected Gauntlet command line parameter; using Gauntlet for orchestration."));
		return true;
	}

	GT_W_LOG("GT.InsightMatrix.Harness.Disabled", TEXT("[InsightHarness] Bootstrap disabled. Use Gauntlet for server/client orchestration."));
	return true;
}

void FGorgeousInsightHarness::StopHarness()
{
	GT_I_LOG("GT.InsightMatrix.Harness.Stop", TEXT("[InsightHarness] Stopping harness and cleaning up."));
}

FGorgeousInsightHarnessStatus FGorgeousInsightHarness::GetStatus()
{
	FGorgeousInsightHarnessStatus Status;
	Status.bHarnessEnabled = true;
	Status.bUsingGauntlet = IsGauntletActive();
	Status.bHarnessActive = Status.bUsingGauntlet;
	Status.bRequiresMultiSession = Status.bUsingGauntlet;
	Status.GauntletState = GetGauntletState();
	Status.GauntletStateSeconds = GetGauntletStateSeconds();
	Status.HarnessMapPath = GInsightHarnessMapPath;
	Status.ListenAddress = GInsightHarnessListenAddress;
	Status.ServerPort = 0;
	Status.bHasConnection = false;

	return Status;
}

bool FGorgeousInsightHarness::IsGauntletActive()
{
	if (GInsightGauntletCached)
	{
		return GInsightGauntletCachedValue;
	}

	GInsightGauntletCached = true;
#if GORGEOUSCORE_WITH_GAUNTLET
	FString GauntletValue;
	GInsightGauntletCachedValue = FParse::Value(FCommandLine::Get(), TEXT("gauntlet="), GauntletValue);
#else
	GInsightGauntletCachedValue = false;
#endif
	return GInsightGauntletCachedValue;
}

FGauntletModule* FGorgeousInsightHarness::GetGauntletModule()
{
#if GORGEOUSCORE_WITH_GAUNTLET
	if (FModuleManager::Get().IsModuleLoaded(TEXT("Gauntlet")))
	{
		return &FModuleManager::GetModuleChecked<FGauntletModule>(TEXT("Gauntlet"));
	}
#endif
	return nullptr;
}

void FGorgeousInsightHarness::BroadcastGauntletState(FName NewState)
{
#if GORGEOUSCORE_WITH_GAUNTLET
	if (FGauntletModule* Module = GetGauntletModule())
	{
		Module->BroadcastStateChange(NewState);
	}
#else
	(void)NewState;
#endif
}

FName FGorgeousInsightHarness::GetGauntletState()
{
#if GORGEOUSCORE_WITH_GAUNTLET
	if (FGauntletModule* Module = GetGauntletModule())
	{
		return Module->GetCurrentState();
	}
#endif
	return NAME_None;
}

double FGorgeousInsightHarness::GetGauntletStateSeconds()
{
#if GORGEOUSCORE_WITH_GAUNTLET
	if (FGauntletModule* Module = GetGauntletModule())
	{
		return Module->GetTimeInCurrentState();
	}
#endif
	return 0.0;
}

void FGorgeousInsightHarness::MarkGauntletHeartbeat(const FString& OptionalStatusMessage)
{
#if GORGEOUSCORE_WITH_GAUNTLET
	if (FGauntletModule* Module = GetGauntletModule())
	{
		Module->MarkHeartbeatActive(OptionalStatusMessage);
	}
#else
	(void)OptionalStatusMessage;
#endif
}

bool FGorgeousInsightHarness::SaveScenarioResult(const FGorgeousInsightScenarioDescriptor& Descriptor, const FGorgeousInsightScenarioResult& Result, const FString& Folder)
{
	const FString BaseDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation"), TEXT("InsightMatrix"), Folder);
	FString FileName = Descriptor.ScenarioName.IsNone() ? Descriptor.GetDisplayName() : Descriptor.ScenarioName.ToString();
	FileName = FileName.Replace(TEXT("/"), TEXT("_"));
	const FString FullPath = FPaths::Combine(BaseDir, FileName + TEXT(".txt"));

	TArray<FString> Lines;
	Lines.Add(FString::Printf(TEXT("Scenario: %s"), *Descriptor.GetDisplayName()));
	Lines.Add(FString::Printf(TEXT("Success: %s"), Result.bSuccess ? TEXT("true") : TEXT("false")));
	if (Result.Errors.Num() > 0)
	{
		Lines.Add(TEXT("Errors:"));
		for (const FString& E : Result.Errors)
		{
			Lines.Add(FString::Printf(TEXT(" - %s"), *E));
		}
	}
	if (Result.Warnings.Num() > 0)
	{
		Lines.Add(TEXT("Warnings:"));
		for (const FString& W : Result.Warnings)
		{
			Lines.Add(FString::Printf(TEXT(" - %s"), *W));
		}
	}
	if (Result.Notes.Num() > 0)
	{
		Lines.Add(TEXT("Notes:"));
		for (const FString& N : Result.Notes)
		{
			Lines.Add(FString::Printf(TEXT(" - %s"), *N));
		}
	}
	if (Result.Metrics.Num() > 0)
	{
		Lines.Add(TEXT("Metrics:"));
		for (const TPair<FString, FString>& M : Result.Metrics)
		{
			Lines.Add(FString::Printf(TEXT(" - %s = %s"), *M.Key, *M.Value));
		}
	}
	if (Result.LogCapture.Num() > 0)
	{
		Lines.Add(TEXT("Log Capture:"));
		for (const FString& L : Result.LogCapture)
		{
			Lines.Add(FString::Printf(TEXT(" %s"), *L));
		}
	}

	IFileManager::Get().MakeDirectory(*BaseDir, true);
	if (!FFileHelper::SaveStringArrayToFile(Lines, *FullPath))
	{
		GT_W_LOG("GT.InsightMatrix.Harness.Save.Fail", TEXT("[InsightHarness] Failed to write result file: %s"), *FullPath);
		return false;
	}

	GT_I_LOG("GT.InsightMatrix.Harness.Save", TEXT("[InsightHarness] Saved result to %s"), *FullPath);
	return true;
}

bool FGorgeousInsightHarness::SaveTestResult(const FName ProviderName, const FGorgeousInsightTest& Test, const FGorgeousInsightTestResult& Result, const FString& Folder)
{
	const FString BaseDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation"), TEXT("InsightMatrix"), Folder, ProviderName.ToString());
	FString FileName = Test.Id.IsNone() ? Test.DisplayName.ToString() : Test.Id.ToString();
	FileName = FileName.Replace(TEXT("/"), TEXT("_"));
	const FString FullPath = FPaths::Combine(BaseDir, FileName + TEXT(".txt"));

	TArray<FString> Lines;
	Lines.Add(FString::Printf(TEXT("Provider: %s"), *ProviderName.ToString()));
	Lines.Add(FString::Printf(TEXT("Test: %s"), *Test.DisplayName.ToString()));
	Lines.Add(FString::Printf(TEXT("Success: %s"), Result.bSuccess ? TEXT("true") : TEXT("false")));

	if (Result.Errors.Num() > 0)
	{
		Lines.Add(TEXT("Errors:"));
		for (const FString& E : Result.Errors)
		{
			Lines.Add(FString::Printf(TEXT(" - %s"), *E));
		}
	}
	if (Result.Warnings.Num() > 0)
	{
		Lines.Add(TEXT("Warnings:"));
		for (const FString& W : Result.Warnings)
		{
			Lines.Add(FString::Printf(TEXT(" - %s"), *W));
		}
	}
	if (Result.Notes.Num() > 0)
	{
		Lines.Add(TEXT("Notes:"));
		for (const FString& N : Result.Notes)
		{
			Lines.Add(FString::Printf(TEXT(" - %s"), *N));
		}
	}
	if (Result.Metrics.Num() > 0)
	{
		Lines.Add(TEXT("Metrics:"));
		for (const TPair<FString, FString>& M : Result.Metrics)
		{
			Lines.Add(FString::Printf(TEXT(" - %s = %s"), *M.Key, *M.Value));
		}
	}
	if (Result.LogCapture.Num() > 0)
	{
		Lines.Add(TEXT("Log Capture:"));
		for (const FString& L : Result.LogCapture)
		{
			Lines.Add(FString::Printf(TEXT(" %s"), *L));
		}
	}

	IFileManager::Get().MakeDirectory(*BaseDir, true);
	if (!FFileHelper::SaveStringArrayToFile(Lines, *FullPath))
	{
		GT_W_LOG("GT.InsightMatrix.Harness.Save.Fail", TEXT("[InsightHarness] Failed to write test file: %s"), *FullPath);
		return false;
	}

	GT_I_LOG("GT.InsightMatrix.Harness.Save", TEXT("[InsightHarness] Saved test result to %s"), *FullPath);
	return true;
}
