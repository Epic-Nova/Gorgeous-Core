#include "Automation/AutoReplicationTestMatrixCore.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"

bool FGorgeousAutoReplicationTestMatrixCore::StartHarness(const FGorgeousAutomationMatrixRequest& Request)
{
	// Stub: real implementation should start PIE/dedicated server harness, apply presets, etc.
	UE_LOG(LogTemp, Log, TEXT("[AutoReplicationTestMatrixCore] StartHarness: %s"), *Request.Describe());
	return true;
}

void FGorgeousAutoReplicationTestMatrixCore::StopHarness()
{
	// Stub: shut down harness resources
	UE_LOG(LogTemp, Log, TEXT("[AutoReplicationTestMatrixCore] StopHarness"));
}

bool FGorgeousAutoReplicationTestMatrixCore::SaveScenarioResult(const FGorgeousAutomationScenarioDescriptor& Descriptor, const FGorgeousAutomationScenarioResult& Result, const FString& Folder)
{
	// Minimal stub serialization to text file under Saved/Automation/AutoReplication
	const FString BaseDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation"), TEXT("AutoReplication"), Folder);
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

	// Ensure directory exists
	IFileManager::Get().MakeDirectory(*BaseDir, true);
	if (!FFileHelper::SaveStringArrayToFile(Lines, *FullPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AutoReplicationTestMatrixCore] Failed to write result file: %s"), *FullPath);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[AutoReplicationTestMatrixCore] Saved result to %s"), *FullPath);
	return true;
}
