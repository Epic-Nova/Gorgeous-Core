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

#pragma once

#include "CoreMinimal.h"
#include "AutoReplication/GorgeousAutoReplicationTypes.h"

class UGorgeousObjectVariable;
class UWorld;

#if GORGEOUSCORE_WITH_IRIS

/** Lightweight coordinator-owned backend that tracks Iris stream state. */
class FGorgeousAutoReplicationIrisBackend
{
public:
	FGorgeousAutoReplicationIrisBackend();

	void Initialize(UWorld* InWorld);
	void Reset();

	void RegisterStream(UGorgeousObjectVariable* Variable, const FGorgeousAutoReplicationStreamConfig& Config, const FGuid& StreamGuid);
	void UnregisterStream(UGorgeousObjectVariable* Variable);
	void MarkStreamDirty(UGorgeousObjectVariable* Variable);

	void Tick(float DeltaSeconds);

private:
	struct FTrackedStream
	{
		FTrackedStream();

		void RefreshRatesFromConfig();
		void AccumulateTime(double DeltaSeconds);
		bool ShouldForceUpdate() const;
		bool CanDispatch() const;
		void MarkDispatched();
		double GetTimeUntilTargetDispatch() const;

		TWeakObjectPtr<UGorgeousObjectVariable> Variable;
		FGorgeousAutoReplicationStreamConfig Config;
		FGuid StreamGuid;
		FName RootNetworkChannel;
		double TimeSinceLastDispatchSeconds;
		double TargetIntervalSeconds;
		double MinimumIntervalSeconds;
		bool bPendingInitialState;
	};

	FTrackedStream* FindStream(UGorgeousObjectVariable* Variable);
	void DispatchStream(FTrackedStream& Stream, const TCHAR* Reason);
	void LogStreamUpdate(const FTrackedStream& Stream, const TCHAR* Reason) const;
	void LogStreamThrottle(const FTrackedStream& Stream, double TimeRemaining, const TCHAR* Reason) const;

private:
	TWeakObjectPtr<UWorld> CachedWorld;
	TMap<FObjectKey, FTrackedStream> TrackedStreams;
	TArray<FObjectKey> PendingDirtyStreams;
};

#endif // GORGEOUSCORE_WITH_IRIS
