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
#include "Interfaces/GorgeousStatisticsInterface.h"

#include "HAL/PlatformMemory.h"
#include "HAL/PlatformTime.h"
#include "Misc/FileHelper.h"
#include "Engine/Engine.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "InsightMatrix/GorgeousInsightMatrixSubsystem.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"

namespace
{
	TWeakObjectPtr<UGorgeousStatisticsInterface> GActiveStatsInterface;
}

UGorgeousStatisticsInterface* UGorgeousStatisticsInterface::GetStatisticsInterface()
{
	if (!GActiveStatsInterface.IsValid())
	{
		UGorgeousStatisticsInterface* NewInterface = NewObject<UGorgeousStatisticsInterface>(GetTransientPackage());
		if (NewInterface)
		{
			NewInterface->AddToRoot();
			GActiveStatsInterface = NewInterface;
		}
	}

	return GActiveStatsInterface.Get();
}

void UGorgeousStatisticsInterface::SetStatisticsInterface(UGorgeousStatisticsInterface* NewInterface)
{
	if (GActiveStatsInterface.IsValid() && GActiveStatsInterface.Get() == NewInterface)
	{
		return;
	}

	if (GActiveStatsInterface.IsValid())
	{
		GActiveStatsInterface->RemoveFromRoot();
	}

	GActiveStatsInterface = NewInterface;
	if (NewInterface)
	{
		NewInterface->AddToRoot();
	}
}

void UGorgeousStatisticsInterface::ClearStatisticsInterface(UGorgeousStatisticsInterface* InterfaceToClear)
{
	if (!InterfaceToClear)
	{
		return;
	}

	if (GActiveStatsInterface.IsValid() && GActiveStatsInterface.Get() == InterfaceToClear)
	{
		GActiveStatsInterface->RemoveFromRoot();
		GActiveStatsInterface.Reset();
	}
}

void UGorgeousStatisticsInterface::EnsureSnapshotForObject(UObject* Object, FName SourceKind, const FGuid& SourceIdentifier)
{
	if (!Object)
	{
		return;
	}

	FGorgeousStatisticsSnapshot& Snapshot = LatestSnapshots.FindOrAdd(Object);
	Snapshot.SourceName = Object->GetName();
	Snapshot.SourceClassName = Object->GetClass() ? Object->GetClass()->GetName() : TEXT("<null>");
	Snapshot.SourceKind = SourceKind;

	if (SourceIdentifier.IsValid())
	{
		Snapshot.SourceIdentifier = SourceIdentifier;
	}
	else if (!Snapshot.SourceIdentifier.IsValid())
	{
		FGuid Resolved;
		if (TryResolveIdentifierFromObject(Object, Resolved))
		{
			Snapshot.SourceIdentifier = Resolved;
		}
	}
}

bool UGorgeousStatisticsInterface::TryResolveIdentifierFromObject(const UObject* Object, FGuid& OutIdentifier)
{
	if (!Object)
	{
		return false;
	}

	for (TFieldIterator<FProperty> It(Object->GetClass()); It; ++It)
	{
		if (It->GetFName() == TEXT("UniqueIdentifier"))
		{
			const FStructProperty* StructProp = CastField<FStructProperty>(*It);
			if (StructProp && StructProp->Struct == TBaseStructure<FGuid>::Get())
			{
				OutIdentifier = *StructProp->ContainerPtrToValuePtr<FGuid>(Object);
				return true;
			}
		}
	}

	return false;
}

void UGorgeousStatisticsInterface::AddObjectToStatistics(UObject* ObjectToAdd, FName SourceKind, FGuid SourceIdentifier)
{
	if (!ObjectToAdd)
	{
		return;
	}

	TrackedObjects.Add(ObjectToAdd);
	EnsureSnapshotForObject(ObjectToAdd, SourceKind, SourceIdentifier);
}

void UGorgeousStatisticsInterface::RemoveObjectFromStatistics(UObject* ObjectToRemove)
{
	if (!ObjectToRemove)
	{
		return;
	}

	TrackedObjects.Remove(ObjectToRemove);
	ActiveSamples.Remove(ObjectToRemove);
	LatestSnapshots.Remove(ObjectToRemove);
}

bool UGorgeousStatisticsInterface::IsObjectInStatistics(UObject* ObjectToCheck) const
{
	if (!ObjectToCheck)
	{
		return false;
	}

	return TrackedObjects.Contains(ObjectToCheck);
}

void UGorgeousStatisticsInterface::GatherCurrentHeapSnapshot()
{
	SnapshotHistory.Reset();

	const FDateTime NowUtc = FDateTime::UtcNow();
	for (const TWeakObjectPtr<UObject>& Object : TrackedObjects)
	{
		if (!Object.IsValid())
		{
			continue;
		}

		if (const FGorgeousStatisticsSnapshot* Existing = LatestSnapshots.Find(Object))
		{
			FGorgeousStatisticsSnapshot Snapshot = *Existing;
			Snapshot.SnapshotTimeUTC = NowUtc;
			SnapshotHistory.Add(Snapshot);
		}
	}
}

TArray<FGorgeousStatisticsSnapshot> UGorgeousStatisticsInterface::GetLastSnapshots() const
{
	return SnapshotHistory;
}

void UGorgeousStatisticsInterface::BeginSample(UObject* Object, const FName OptionalTag, const FGuid SourceIdentifier, const FName SourceKind)
{
	if (!Object)
	{
		return;
	}

	FGorgeousSampleStart Start;
	Start.StartCycles = FPlatformTime::Cycles64();
	const FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
	Start.StartUsedPhysicalBytes = MemoryStats.UsedPhysical;
	Start.StartUsedVirtualBytes = MemoryStats.UsedVirtual;
	Start.SampleTag = OptionalTag;
	Start.SourceKind = SourceKind;
	Start.SourceIdentifier = SourceIdentifier;

	ActiveSamples.Add(Object, Start);
	TrackedObjects.Add(Object);
	EnsureSnapshotForObject(Object, SourceKind, SourceIdentifier);
}

void UGorgeousStatisticsInterface::EndSample(UObject* Object)
{
	if (!Object)
	{
		return;
	}

	FGorgeousSampleStart* Start = ActiveSamples.Find(Object);
	if (!Start)
	{
		return;
	}

	const uint64 EndCycles = FPlatformTime::Cycles64();
	const double ElapsedSeconds = static_cast<double>(EndCycles - Start->StartCycles) * FPlatformTime::GetSecondsPerCycle64();
	const FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
	const int64 DeltaPhysical = static_cast<int64>(MemoryStats.UsedPhysical) - static_cast<int64>(Start->StartUsedPhysicalBytes);
	const int64 DeltaVirtual = static_cast<int64>(MemoryStats.UsedVirtual) - static_cast<int64>(Start->StartUsedVirtualBytes);
	const int64 DeltaCombined = DeltaPhysical != 0 ? DeltaPhysical : DeltaVirtual;

	EnsureSnapshotForObject(Object, Start->SourceKind, Start->SourceIdentifier);
	FGorgeousStatisticsSnapshot& Snapshot = LatestSnapshots.FindOrAdd(Object);
	Snapshot.SnapshotTimeUTC = FDateTime::UtcNow();
	Snapshot.LastCPUTimeSeconds = ElapsedSeconds;
	Snapshot.LastSampleTag = Start->SampleTag;
	Snapshot.SampleCount++;
	Snapshot.AverageCPUTimeSeconds = Snapshot.SampleCount == 1
		? ElapsedSeconds
		: ((Snapshot.AverageCPUTimeSeconds * (Snapshot.SampleCount - 1)) + ElapsedSeconds) / static_cast<double>(Snapshot.SampleCount);
	Snapshot.LastMemoryDeltaBytes = DeltaCombined;
	Snapshot.PeakMemoryDeltaBytes = FMath::Max<int64>(Snapshot.PeakMemoryDeltaBytes, DeltaCombined);

	ActiveSamples.Remove(Object);
}

bool UGorgeousStatisticsInterface::GetStatsSnapshot(UObject* Object, FGorgeousStatisticsSnapshot& OutSnapshot) const
{
	if (!Object)
	{
		return false;
	}

	if (const FGorgeousStatisticsSnapshot* Existing = LatestSnapshots.Find(Object))
	{
		OutSnapshot = *Existing;
		return true;
	}

	return false;
}

void UGorgeousStatisticsInterface::LogStats(UObject* Object) const
{
	FGorgeousStatisticsSnapshot Snapshot;
	if (!GetStatsSnapshot(Object, Snapshot))
	{
		return;
	}

	GT_I_LOG("GT.Core.Stats.Log", TEXT("Stats | Name: %s | Class: %s | Id: %s | Kind: %s | Last CPU: %.6fs | Avg CPU: %.6fs | Last Mem Δ: %lld | Peak Mem Δ: %lld | Samples: %d"),
		*Snapshot.SourceName,
		*Snapshot.SourceClassName,
		*Snapshot.SourceIdentifier.ToString(),
		*Snapshot.SourceKind.ToString(),
		Snapshot.LastCPUTimeSeconds,
		Snapshot.AverageCPUTimeSeconds,
		Snapshot.LastMemoryDeltaBytes,
		Snapshot.PeakMemoryDeltaBytes,
		Snapshot.SampleCount);
}

void UGorgeousStatisticsInterface::LogAllTrackedStats() const
{
	for (const TWeakObjectPtr<UObject>& Object : TrackedObjects)
	{
		if (!Object.IsValid())
		{
			continue;
		}
		LogStats(Object.Get());
	}
}

bool UGorgeousStatisticsInterface::ExportSnapshotsToCSV(const FString& AbsoluteFilePath) const
{
	if (AbsoluteFilePath.IsEmpty())
	{
		return false;
	}

	FString Csv = TEXT("SnapshotTimeUTC,SourceName,SourceClassName,SourceIdentifier,SourceKind,LastSampleTag,LastCPUTimeSeconds,AverageCPUTimeSeconds,LastMemoryDeltaBytes,PeakMemoryDeltaBytes,SampleCount\n");
	for (const FGorgeousStatisticsSnapshot& Snapshot : SnapshotHistory)
	{
		Csv += FString::Printf(TEXT("%s,%s,%s,%s,%s,%s,%.6f,%.6f,%lld,%lld,%d\n"),
			*Snapshot.SnapshotTimeUTC.ToString(),
			*Snapshot.SourceName,
			*Snapshot.SourceClassName,
			*Snapshot.SourceIdentifier.ToString(),
			*Snapshot.SourceKind.ToString(),
			*Snapshot.LastSampleTag.ToString(),
			Snapshot.LastCPUTimeSeconds,
			Snapshot.AverageCPUTimeSeconds,
			Snapshot.LastMemoryDeltaBytes,
			Snapshot.PeakMemoryDeltaBytes,
			Snapshot.SampleCount);
	}

	return FFileHelper::SaveStringToFile(Csv, *AbsoluteFilePath);
}

bool UGorgeousStatisticsInterface::ExportSnapshotsToJson(const FString& AbsoluteFilePath) const
{
	if (AbsoluteFilePath.IsEmpty())
	{
		return false;
	}

	FString Json = TEXT("[\n");
	for (int32 Index = 0; Index < SnapshotHistory.Num(); ++Index)
	{
		const FGorgeousStatisticsSnapshot& Snapshot = SnapshotHistory[Index];
		Json += FString::Printf(TEXT("  {\"SnapshotTimeUTC\":\"%s\",\"SourceName\":\"%s\",\"SourceClassName\":\"%s\",\"SourceIdentifier\":\"%s\",\"SourceKind\":\"%s\",\"LastSampleTag\":\"%s\",\"LastCPUTimeSeconds\":%.6f,\"AverageCPUTimeSeconds\":%.6f,\"LastMemoryDeltaBytes\":%lld,\"PeakMemoryDeltaBytes\":%lld,\"SampleCount\":%d}%s\n"),
			*Snapshot.SnapshotTimeUTC.ToString(),
			*Snapshot.SourceName.ReplaceCharWithEscapedChar(),
			*Snapshot.SourceClassName.ReplaceCharWithEscapedChar(),
			*Snapshot.SourceIdentifier.ToString(),
			*Snapshot.SourceKind.ToString(),
			*Snapshot.LastSampleTag.ToString(),
			Snapshot.LastCPUTimeSeconds,
			Snapshot.AverageCPUTimeSeconds,
			Snapshot.LastMemoryDeltaBytes,
			Snapshot.PeakMemoryDeltaBytes,
			Snapshot.SampleCount,
			Index < SnapshotHistory.Num() - 1 ? TEXT(",") : TEXT(""));
	}
	Json += TEXT("]\n");

	return FFileHelper::SaveStringToFile(Json, *AbsoluteFilePath);
}

void UGorgeousStatisticsInterface::ShowAllTrackedStatsOnScreen(const float DurationSeconds, const FLinearColor Color) const
{
	if (!GEngine)
	{
		return;
	}

	for (const TWeakObjectPtr<UObject>& Object : TrackedObjects)
	{
		if (!Object.IsValid())
		{
			continue;
		}

		FGorgeousStatisticsSnapshot Snapshot;
		if (!GetStatsSnapshot(Object.Get(), Snapshot))
		{
			continue;
		}

		const FString Message = FString::Printf(TEXT("%s | CPU: %.6fs (avg %.6fs) | Mem Δ: %lld (peak %lld) | Samples: %d"),
			*Snapshot.SourceName,
			Snapshot.LastCPUTimeSeconds,
			Snapshot.AverageCPUTimeSeconds,
			Snapshot.LastMemoryDeltaBytes,
			Snapshot.PeakMemoryDeltaBytes,
			Snapshot.SampleCount);

		GEngine->AddOnScreenDebugMessage(-1, DurationSeconds, Color.ToFColor(true), Message);
	}
}

void UGorgeousStatisticsInterface::ShowStatsWidget(const int32 ZOrder)
{
	GatherCurrentHeapSnapshot();
	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		Subsystem->ShowDebugPanel(ZOrder);
		Subsystem->RefreshDebugPanel();
	}
}

void UGorgeousStatisticsInterface::UpdateStatsWidget()
{
	GatherCurrentHeapSnapshot();
	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		Subsystem->RefreshDebugPanel();
	}
}

void UGorgeousStatisticsInterface::HideStatsWidget()
{
	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		Subsystem->HideDebugPanel();
	}
}

void UGorgeousStatisticsInterface::SetAutoSampleProcessingLoop(const bool bEnable)
{
	bAutoSampleProcessingLoop = bEnable;
}

bool UGorgeousStatisticsInterface::IsAutoSampleProcessingLoopEnabled() const
{
	return bAutoSampleProcessingLoop;
}
