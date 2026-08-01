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
#include "CoreMinimal.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "GorgeousStatisticsInterface.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Snapshot structure capturing statistics for a specific runtime source.
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousStatisticsSnapshot
{
	GENERATED_BODY()

	// The UTC time when the snapshot was taken.
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Statistics")
	FDateTime SnapshotTimeUTC;

	// The name of the source.
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Statistics")
	FString SourceName;

	// The class name of the source.
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Statistics")
	FString SourceClassName;

	// The unique identifier of the source, if available.
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Statistics")
	FGuid SourceIdentifier;

	// The kind/category of the source (Events, ObjectVariables, etc.).
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Statistics")
	FName SourceKind = NAME_None;

	// The optional tag associated with the last sample.
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Statistics")
	FName LastSampleTag = NAME_None;

	// The last CPU time sample.
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Statistics")
	double LastCPUTimeSeconds = 0.0;

	// The average CPU time over all samples.
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Statistics")
	double AverageCPUTimeSeconds = 0.0;

	// The last memory delta sample.
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Statistics")
	int64 LastMemoryDeltaBytes = 0;

	// The peak memory delta observed so far.
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Statistics")
	int64 PeakMemoryDeltaBytes = 0;

	// The number of samples taken.
	UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Statistics")
	int32 SampleCount = 0;
};

/**
 * Interface providing runtime statistics tracking for events, object variables, and other systems.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousStatisticsInterface : public UGorgeousInterface
{
	GENERATED_BODY()

public:
	/** Returns the active registered statistics interface. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Statistics", meta = (CompactNodeTitle = "Statistics"))
	static UGorgeousStatisticsInterface* GetStatisticsInterface();

	/** Overrides the active statistics interface instance. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Statistics")
	static void SetStatisticsInterface(UGorgeousStatisticsInterface* NewInterface);

	/** Clears the active statistics interface instance if it matches. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Statistics")
	static void ClearStatisticsInterface(UGorgeousStatisticsInterface* InterfaceToClear);

	/** Adds a source object to statistics tracking. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Statistics")
	virtual void AddObjectToStatistics(UObject* ObjectToAdd, FName SourceKind = NAME_None, FGuid SourceIdentifier = FGuid());

	/** Removes a source object from statistics tracking. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Statistics")
	virtual void RemoveObjectFromStatistics(UObject* ObjectToRemove);

	/** Checks if a source object is being tracked for statistics. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Statistics")
	virtual bool IsObjectInStatistics(UObject* ObjectToCheck) const;

	/** Gathers a heap snapshot for all currently tracked objects. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Statistics")
	virtual void GatherCurrentHeapSnapshot();

	/** Retrieves the latest statistics snapshots for all tracked objects. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Statistics")
	TArray<FGorgeousStatisticsSnapshot> GetLastSnapshots() const;

	/** Begins sampling statistics for a specific object. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Statistics")
	virtual void BeginSample(UObject* Object, FName OptionalTag = NAME_None, FGuid SourceIdentifier = FGuid(), FName SourceKind = NAME_None);

	/** Ends sampling statistics for a specific object. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Statistics")
	virtual void EndSample(UObject* Object);

	/** Retrieves the latest statistics snapshot for a specific object. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Statistics")
	bool GetStatsSnapshot(UObject* Object, FGorgeousStatisticsSnapshot& OutSnapshot) const;

	/** Logs the statistics of a specific object to the output log. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Statistics")
	void LogStats(UObject* Object) const;

	/** Logs the statistics of all tracked objects to the output log. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Statistics")
	void LogAllTrackedStats() const;

	/** Exports the latest statistics snapshots to a CSV file. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Statistics")
	bool ExportSnapshotsToCSV(const FString& AbsoluteFilePath) const;

	/** Exports the latest statistics snapshots to a JSON file. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Statistics")
	bool ExportSnapshotsToJson(const FString& AbsoluteFilePath) const;

	/** Displays the statistics of all tracked objects on the screen. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Statistics")
	void ShowAllTrackedStatsOnScreen(float DurationSeconds = 4.0f, FLinearColor Color = FLinearColor::Green) const;

	/** Shows the statistics widget on the screen. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Statistics")
	void ShowStatsWidget(int32 ZOrder = 0);

	/** Updates the statistics widget with the latest data. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Statistics")
	void UpdateStatsWidget();

	/** Hides the statistics widget from the screen. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Statistics")
	void HideStatsWidget();

	/** Enables or disables automatic sampling during the processing loop. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Statistics")
	void SetAutoSampleProcessingLoop(bool bEnable);

	/** Checks if automatic sampling during the processing loop is enabled. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Statistics")
	bool IsAutoSampleProcessingLoopEnabled() const;

private:
	struct FGorgeousSampleStart
	{
		uint64 StartCycles = 0;
		uint64 StartUsedPhysicalBytes = 0;
		uint64 StartUsedVirtualBytes = 0;
		FName SampleTag = NAME_None;
		FName SourceKind = NAME_None;
		FGuid SourceIdentifier;
	};

	void EnsureSnapshotForObject(UObject* Object, FName SourceKind, const FGuid& SourceIdentifier);
	static bool TryResolveIdentifierFromObject(const UObject* Object, FGuid& OutIdentifier);

	TSet<TWeakObjectPtr<UObject>> TrackedObjects;
	TMap<TWeakObjectPtr<UObject>, FGorgeousSampleStart> ActiveSamples;
	TMap<TWeakObjectPtr<UObject>, FGorgeousStatisticsSnapshot> LatestSnapshots;
	TArray<FGorgeousStatisticsSnapshot> SnapshotHistory;
	bool bAutoSampleProcessingLoop = false;
};