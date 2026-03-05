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

#include "AutoReplication/GorgeousAutoReplicationIrisBackend.h"

#if GORGEOUSCORE_WITH_IRIS

#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousRootNetworkStackSubsystem.h"
#include "Engine/World.h"
#include "Algo/Sort.h"

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousAutoReplicationIrisBackend, Log, All);

FGorgeousAutoReplicationIrisBackend::FGorgeousAutoReplicationIrisBackend()
{
}

void FGorgeousAutoReplicationIrisBackend::Initialize(UWorld* InWorld)
{
	CachedWorld = InWorld;
	TrackedStreams.Reset();
	PendingDirtyStreams.Reset();
}

void FGorgeousAutoReplicationIrisBackend::Reset()
{
	TrackedStreams.Reset();
	PendingDirtyStreams.Reset();
	CachedWorld.Reset();
}

void FGorgeousAutoReplicationIrisBackend::RegisterStream(UGorgeousObjectVariable* Variable, const FGorgeousAutoReplicationStreamConfig& Config, const FGuid& StreamGuid)
{
	if (!Variable)
	{
		return;
	}

	FTrackedStream& Stream = TrackedStreams.FindOrAdd(FObjectKey(Variable));
	Stream.Variable = Variable;
	Stream.Config = Config;
	Stream.StreamGuid = StreamGuid.IsValid() ? StreamGuid : FGuid::NewGuid();
	Stream.bPendingInitialState = true;
	Stream.RefreshRatesFromConfig();
	LogStreamUpdate(Stream, TEXT("Registered"));
	MarkStreamDirty(Variable);
}

void FGorgeousAutoReplicationIrisBackend::UnregisterStream(UGorgeousObjectVariable* Variable)
{
	if (!Variable)
	{
		return;
	}

	const FObjectKey StreamKey(Variable);
	if (FTrackedStream* Stream = TrackedStreams.Find(StreamKey))
	{
		LogStreamUpdate(*Stream, TEXT("Unregistered"));
	}
	TrackedStreams.Remove(StreamKey);
	PendingDirtyStreams.Remove(StreamKey);
}

void FGorgeousAutoReplicationIrisBackend::MarkStreamDirty(UGorgeousObjectVariable* Variable)
{
	if (!Variable)
	{
		return;
	}

	const FObjectKey StreamKey(Variable);
	if (!TrackedStreams.Contains(StreamKey))
	{
		return;
	}

	PendingDirtyStreams.AddUnique(StreamKey);
}

void FGorgeousAutoReplicationIrisBackend::Tick(const float DeltaSeconds)
{
	if (!CachedWorld.IsValid())
	{
		TrackedStreams.Reset();
		PendingDirtyStreams.Reset();
		return;
	}

	if (TrackedStreams.Num() == 0)
	{
		PendingDirtyStreams.Reset();
		return;
	}

	for (TPair<FObjectKey, FTrackedStream>& Pair : TrackedStreams)
	{
		FTrackedStream& Stream = Pair.Value;
		Stream.AccumulateTime(DeltaSeconds);
		if (Stream.ShouldForceUpdate())
		{
			PendingDirtyStreams.AddUnique(Pair.Key);
			LogStreamThrottle(Stream, 0.0, TEXT("min frequency guard"));
		}
	}

	if (PendingDirtyStreams.Num() == 0)
	{
		return;
	}

	Algo::Sort(PendingDirtyStreams, [this](const FObjectKey& Lhs, const FObjectKey& Rhs)
	{
		const FTrackedStream* const LhsStream = TrackedStreams.Find(Lhs);
		const FTrackedStream* const RhsStream = TrackedStreams.Find(Rhs);
		const int32 LhsPriority = LhsStream ? LhsStream->Config.Priority : 0;
		const int32 RhsPriority = RhsStream ? RhsStream->Config.Priority : 0;
		if (LhsPriority != RhsPriority)
		{
			return LhsPriority > RhsPriority;
		}

		const float LhsFrequency = LhsStream ? LhsStream->Config.GetEffectiveUpdateFrequency() : 0.f;
		const float RhsFrequency = RhsStream ? RhsStream->Config.GetEffectiveUpdateFrequency() : 0.f;
		return LhsFrequency > RhsFrequency;
	});

	TArray<FObjectKey> DeferredStreams;
	DeferredStreams.Reserve(PendingDirtyStreams.Num());

	for (const FObjectKey& StreamKey : PendingDirtyStreams)
	{
		FTrackedStream* Stream = TrackedStreams.Find(StreamKey);
		if (!Stream)
		{
			continue;
		}

		if (!Stream->CanDispatch())
		{
			LogStreamThrottle(*Stream, Stream->GetTimeUntilTargetDispatch(), TEXT("waiting for target interval"));
			DeferredStreams.Add(StreamKey);
			continue;
		}

		DispatchStream(*Stream, TEXT("Scheduled update"));
	}

	PendingDirtyStreams = MoveTemp(DeferredStreams);
}


FGorgeousAutoReplicationIrisBackend::FTrackedStream::FTrackedStream()
	: StreamGuid()
	, RootNetworkChannel(NAME_None)
	, TimeSinceLastDispatchSeconds(0.0)
	, TargetIntervalSeconds(0.0)
	, MinimumIntervalSeconds(0.0)
	, bPendingInitialState(false)
{
}

FGorgeousAutoReplicationIrisBackend::FTrackedStream* FGorgeousAutoReplicationIrisBackend::FindStream(UGorgeousObjectVariable* Variable)
{
	return Variable ? TrackedStreams.Find(FObjectKey(Variable)) : nullptr;
}

void FGorgeousAutoReplicationIrisBackend::DispatchStream(FTrackedStream& Stream, const TCHAR* Reason)
{
	UGorgeousObjectVariable* Variable = Stream.Variable.Get();
	if (!Variable)
	{
		return;
	}

	if (Stream.RootNetworkChannel.IsNone() && Variable->IsRootNetworkStackEnabled())
	{
		Stream.RootNetworkChannel = Variable->GetEffectiveNetworkChannel();
	}

	if (!Stream.RootNetworkChannel.IsNone())
	{
		if (UWorld* World = CachedWorld.Get())
		{
			if (UGorgeousRootNetworkStackSubsystem* RootNetworkStack = UGorgeousRootNetworkStackSubsystem::Get(World))
			{
				if (!RootNetworkStack->HasChannelSubscribers(Stream.RootNetworkChannel))
				{
					UE_LOG(LogGorgeousAutoReplicationIrisBackend, VeryVerbose,
						TEXT("AutoReplication Iris stream [%s] skipped dispatch because channel %s has no subscribers."),
						*Variable->GetName(), *Stream.RootNetworkChannel.ToString());
					return;
				}
			}
		}
	}

	LogStreamUpdate(Stream, Reason);
	Stream.MarkDispatched();
}

void FGorgeousAutoReplicationIrisBackend::LogStreamUpdate(const FTrackedStream& Stream, const TCHAR* Reason) const
{
	UGorgeousObjectVariable* Variable = Stream.Variable.Get();
	const FString VariableName = Variable ? Variable->GetName() : TEXT("<invalid>");
	UE_LOG(LogGorgeousAutoReplicationIrisBackend, Verbose, TEXT("AutoReplication Iris stream [%s] %s (Guid: %s, UpdateFrequency: %.2f Hz)."),
		*VariableName, Reason, *Stream.StreamGuid.ToString(), Stream.Config.UpdateFrequency);
}

void FGorgeousAutoReplicationIrisBackend::LogStreamThrottle(const FTrackedStream& Stream, const double TimeRemaining, const TCHAR* Reason) const
{
	if (!UE_LOG_ACTIVE(LogGorgeousAutoReplicationIrisBackend, Verbose))
	{
		return;
	}

	UGorgeousObjectVariable* Variable = Stream.Variable.Get();
	const FString VariableName = Variable ? Variable->GetName() : TEXT("<invalid>");
	const double TargetHz = Stream.Config.UpdateFrequency;
	const double MinHz = Stream.Config.MinUpdateFrequency;
	UE_LOG(LogGorgeousAutoReplicationIrisBackend, Verbose,
		TEXT("AutoReplication Iris stream [%s] throttled (%s, priority=%d, target=%.2f Hz, min=%.2f Hz, remaining=%.3fs)."),
		*VariableName,
		Reason ? Reason : TEXT("unspecified"),
		Stream.Config.Priority,
		TargetHz,
		MinHz,
		TimeRemaining);
}

void FGorgeousAutoReplicationIrisBackend::FTrackedStream::RefreshRatesFromConfig()
{
	const float EffectiveUpdateHz = FMath::Max(Config.UpdateFrequency, 0.f);
	const float EffectiveMinHz = (Config.MinUpdateFrequency > 0.f) ? FMath::Min(Config.MinUpdateFrequency, EffectiveUpdateHz) : 0.f;
	TargetIntervalSeconds = (EffectiveUpdateHz > 0.f) ? (1.0 / static_cast<double>(EffectiveUpdateHz)) : 0.0;
	MinimumIntervalSeconds = (EffectiveMinHz > 0.f) ? (1.0 / static_cast<double>(EffectiveMinHz)) : 0.0;
	TimeSinceLastDispatchSeconds = 0.0;
}

void FGorgeousAutoReplicationIrisBackend::FTrackedStream::AccumulateTime(const double DeltaSeconds)
{
	TimeSinceLastDispatchSeconds += DeltaSeconds;
}

bool FGorgeousAutoReplicationIrisBackend::FTrackedStream::ShouldForceUpdate() const
{
	return MinimumIntervalSeconds > 0.0 && TimeSinceLastDispatchSeconds >= MinimumIntervalSeconds;
}

bool FGorgeousAutoReplicationIrisBackend::FTrackedStream::CanDispatch() const
{
	if (bPendingInitialState)
	{
		return true;
	}

	return TargetIntervalSeconds <= 0.0 || TimeSinceLastDispatchSeconds >= TargetIntervalSeconds;
}

double FGorgeousAutoReplicationIrisBackend::FTrackedStream::GetTimeUntilTargetDispatch() const
{
	if (bPendingInitialState || TargetIntervalSeconds <= 0.0)
	{
		return 0.0;
	}

	return FMath::Max(0.0, TargetIntervalSeconds - TimeSinceLastDispatchSeconds);
}

void FGorgeousAutoReplicationIrisBackend::FTrackedStream::MarkDispatched()
{
	TimeSinceLastDispatchSeconds = 0.0;
	bPendingInitialState = false;
}

#endif // GORGEOUSCORE_WITH_IRIS
