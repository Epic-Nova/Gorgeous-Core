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

#ifndef GORGEOUSCORERUNTIME_ENABLE_NETWORK_METRICS
#define GORGEOUSCORERUNTIME_ENABLE_NETWORK_METRICS 0
#endif

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "HAL/PlatformTime.h"
#include "Math/RandomStream.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector.h"
#include "Net/UnrealNetwork.h"
// NetworkMetricsDatabase only when metrics are enabled so the plugin can compile without engine dependencies.
#if GORGEOUSCORERUNTIME_ENABLE_NETWORK_METRICS
#include "Net/NetworkMetricsDatabase.h"
#endif
#include "QualityOfLife/GorgeousPlayerController.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/NetDriver.h"
#include "Engine/NetConnection.h"
#include "Misc/App.h"
#include "GorgeousObjectVariablePerfTestTypes.generated.h"

USTRUCT()
struct FGorgeousPerfReplicationStimulus
{
	GENERATED_BODY()

	UPROPERTY()
	FVector SampleVector = FVector::ZeroVector;

	UPROPERTY()
	float SampleScalar = 0.0f;

	UPROPERTY()
	float StampSeconds = 0.0f;

	UPROPERTY()
	int32 Sequence = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// PIE Environment Fingerprint, proves a test ran on a real net environment
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Captures immutable facts about the PIE world at the moment a test runs.
 * When embedded in a test result, this proves the scenario executed inside
 * a real PIE multiplayer session, not standalone or a stripped simulation.
 */
USTRUCT()
struct GORGEOUSCORERUNTIME_API FGorgeousPerfEnvironmentFingerprint
{
	GENERATED_BODY()

	UPROPERTY() FString WorldName;
	UPROPERTY() FString NetModeName;          // "DedicatedServer", "ListenServer", "Client", "Standalone"
	UPROPERTY() FString NetDriverName;         // e.g. "IpNetDriver", "DemoNetDriver"
	UPROPERTY() FString NetDriverClassName;
	UPROPERTY() int32   NumServerConnections = 0;
	UPROPERTY() int32   NumPlayerControllers = 0;
	UPROPERTY() bool    bIsPlayInEditor = false;
	UPROPERTY() bool    bHasNetDriver = false;
	UPROPERTY() bool    bIsDedicatedServer = false;
	UPROPERTY() bool    bIsListenServer = false;
	UPROPERTY() bool    bHasBegunPlay = false;
	UPROPERTY() double  TimestampSeconds = 0.0;
	UPROPERTY() FString ProcessId;
	UPROPERTY() int32   PIEInstanceId = INDEX_NONE;
	UPROPERTY() FString MapName;
	UPROPERTY() int32   FrameNumber = 0;

	/** Snapshot the fingerprint from the given world. */
	static FGorgeousPerfEnvironmentFingerprint Capture(UWorld* World)
	{
		FGorgeousPerfEnvironmentFingerprint FP;
		FP.TimestampSeconds = FPlatformTime::Seconds();
		FP.ProcessId = FString::Printf(TEXT("%u"), FPlatformProcess::GetCurrentProcessId());
		FP.FrameNumber = GFrameNumber;

		if (!World)
		{
			FP.NetModeName = TEXT("NoWorld");
			return FP;
		}

		FP.WorldName = World->GetName();
		FP.MapName = World->GetMapName();
		FP.bHasBegunPlay = World->HasBegunPlay();

#if WITH_EDITOR
		FP.bIsPlayInEditor = World->WorldType == EWorldType::PIE;
		FP.PIEInstanceId = World->GetOutermost()->GetPIEInstanceID();
#endif

		const ENetMode NetMode = World->GetNetMode();
		switch (NetMode)
		{
		case NM_DedicatedServer: FP.NetModeName = TEXT("DedicatedServer"); FP.bIsDedicatedServer = true; break;
		case NM_ListenServer:    FP.NetModeName = TEXT("ListenServer");    FP.bIsListenServer = true;    break;
		case NM_Client:          FP.NetModeName = TEXT("Client");          break;
		case NM_Standalone:      FP.NetModeName = TEXT("Standalone");      break;
		default:                 FP.NetModeName = TEXT("Unknown");         break;
		}

		if (UNetDriver* NetDriver = World->GetNetDriver())
		{
			FP.bHasNetDriver = true;
			FP.NetDriverName = NetDriver->GetName();
			FP.NetDriverClassName = NetDriver->GetClass()->GetName();
			if (NetDriver->ServerConnection)
			{
				FP.NumServerConnections = 1; // Client has one server connection
			}
			FP.NumServerConnections += NetDriver->ClientConnections.Num();
		}

		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			++FP.NumPlayerControllers;
		}

		return FP;
	}

	/** Returns true if this fingerprint represents a real networked PIE environment. */
	bool IsRealNetEnvironment() const
	{
		return bIsPlayInEditor && bHasNetDriver && bHasBegunPlay
			&& NetModeName != TEXT("Standalone") && NetModeName != TEXT("NoWorld");
	}

	/** Returns true if there are actual network connections (not just standalone loopback). */
	bool HasActiveConnections() const
	{
		return NumServerConnections > 0;
	}

	/** Build a human-readable summary for embedding in test notes. */
	FString ToString() const
	{
		return FString::Printf(
			TEXT("PIE=%s | NetMode=%s | NetDriver=%s (%s) | Connections=%d | Players=%d | Map=%s | PIEInstance=%d | Frame=%d | BegunPlay=%s | PID=%s | T=%.4fs"),
			bIsPlayInEditor ? TEXT("YES") : TEXT("NO"),
			*NetModeName,
			*NetDriverName,
			*NetDriverClassName,
			NumServerConnections,
			NumPlayerControllers,
			*MapName,
			PIEInstanceId,
			FrameNumber,
			bHasBegunPlay ? TEXT("Yes") : TEXT("No"),
			*ProcessId,
			TimestampSeconds);
	}

	/** Emit all fields as individual metrics. */
	void EmitMetrics(FGorgeousInsightScenarioResult& Result, const FString& Prefix) const
	{
		Result.AddMetric(FString::Printf(TEXT("%s.isPIE"), *Prefix), bIsPlayInEditor ? 1.0 : 0.0);
		Result.AddMetric(FString::Printf(TEXT("%s.hasNetDriver"), *Prefix), bHasNetDriver ? 1.0 : 0.0);
		Result.AddMetric(FString::Printf(TEXT("%s.connections"), *Prefix), static_cast<double>(NumServerConnections));
		Result.AddMetric(FString::Printf(TEXT("%s.playerControllers"), *Prefix), static_cast<double>(NumPlayerControllers));
		Result.AddMetric(FString::Printf(TEXT("%s.isDedicatedServer"), *Prefix), bIsDedicatedServer ? 1.0 : 0.0);
		Result.AddMetric(FString::Printf(TEXT("%s.isListenServer"), *Prefix), bIsListenServer ? 1.0 : 0.0);
		Result.AddMetric(FString::Printf(TEXT("%s.pieInstanceId"), *Prefix), static_cast<double>(PIEInstanceId));
		Result.AddMetric(FString::Printf(TEXT("%s.frameNumber"), *Prefix), static_cast<double>(FrameNumber));
		Result.AddMetric(FString::Printf(TEXT("%s.timestampS"), *Prefix), TimestampSeconds, TEXT("s"));
		Result.AddMetric(FString::Printf(TEXT("%s.netMode"), *Prefix), NetModeName);
	}
};

// ═══════════════════════════════════════════════════════════════════════════
// Replication Statistics, aggregated timing and value verification
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Collects all measurable stats from a single replication test operation:
 * timing, throughput, value verification, and payload metadata.
 */
USTRUCT()
struct GORGEOUSCORERUNTIME_API FGorgeousPerfReplicationStats
{
	GENERATED_BODY()

	// ── Timing ──────────────────────────────────────────────────────────
	UPROPERTY() double OperationStartSeconds = 0.0;
	UPROPERTY() double OperationEndSeconds = 0.0;
	UPROPERTY() double QueueLatencySeconds = 0.0;   // Time to queue the RPC
	UPROPERTY() double DispatchLatencySeconds = 0.0; // Time to dispatch (queue → dequeue/route)
	UPROPERTY() double TotalLatencySeconds = 0.0;    // Full start-to-end

	// ── Counts ──────────────────────────────────────────────────────────
	UPROPERTY() int32 StimulusCount = 0;
	UPROPERTY() int32 NetSendCount = 0;
	UPROPERTY() int32 NetReceiveCount = 0;
	UPROPERTY() int32 PayloadArgumentCount = 0;

	// ── Value Verification (property replication) ───────────────────────
	UPROPERTY() int32  RandomSeed = 0;
	UPROPERTY() float  InjectedScalar = 0.0f;         // Random scalar we injected
	UPROPERTY() float  ReadBackScalar = 0.0f;          // Scalar read back from OV state
	UPROPERTY() bool   bScalarMatch = false;
	UPROPERTY() FVector InjectedVector = FVector::ZeroVector;
	UPROPERTY() FVector ReadBackVector = FVector::ZeroVector;
	UPROPERTY() bool   bVectorMatch = false;
	UPROPERTY() int32  InjectedSequence = 0;
	UPROPERTY() int32  ReadBackSequence = 0;
	UPROPERTY() bool   bSequenceMatch = false;

	// ── Value Verification (RPC argument round-trip) ────────────────────
	UPROPERTY() int32  RPCInputValue = 0;              // Random int we sent through the RPC
	UPROPERTY() int32  RPCExpectedOutput = 0;           // Expected after transformation
	UPROPERTY() int32  RPCActualOutput = 0;             // What we actually got back
	UPROPERTY() bool   bRPCValueMatch = false;
	UPROPERTY() FString RPCInputString;                 // Random string sent
	UPROPERTY() FString RPCReadBackString;              // String read back
	UPROPERTY() bool   bRPCStringMatch = false;

	// ── Environment ─────────────────────────────────────────────────────
	UPROPERTY() FGorgeousPerfEnvironmentFingerprint Fingerprint;

	double GetDurationSeconds() const
	{
		return (OperationEndSeconds > OperationStartSeconds)
			? (OperationEndSeconds - OperationStartSeconds)
			: 0.0;
	}

	bool AllValuesVerified() const
	{
		return bScalarMatch && bVectorMatch && bSequenceMatch;
	}

	bool RPCValuesVerified() const
	{
		return bRPCValueMatch && bRPCStringMatch;
	}

	/** Build a human-readable report. */
	FString BuildReport(const TCHAR* Label) const
	{
		FString R;
		R += FString::Printf(TEXT("=== ReplicationStats [%s] ===\n"), Label);
		R += FString::Printf(TEXT("  Duration: %.6f s\n"), GetDurationSeconds());
		R += FString::Printf(TEXT("  QueueLatency: %.6f s | DispatchLatency: %.6f s\n"), QueueLatencySeconds, DispatchLatencySeconds);
		R += FString::Printf(TEXT("  Stimulus: %d | NetSend: %d | NetReceive: %d | PayloadArgs: %d\n"), StimulusCount, NetSendCount, NetReceiveCount, PayloadArgumentCount);
		R += FString::Printf(TEXT("  Scalar: injected=%.6f readback=%.6f match=%s\n"), InjectedScalar, ReadBackScalar, bScalarMatch ? TEXT("YES") : TEXT("NO"));
		R += FString::Printf(TEXT("  Vector: injected=%s readback=%s match=%s\n"), *InjectedVector.ToString(), *ReadBackVector.ToString(), bVectorMatch ? TEXT("YES") : TEXT("NO"));
		R += FString::Printf(TEXT("  Sequence: injected=%d readback=%d match=%s\n"), InjectedSequence, ReadBackSequence, bSequenceMatch ? TEXT("YES") : TEXT("NO"));
		if (RPCInputValue != 0 || RPCExpectedOutput != 0)
		{
			R += FString::Printf(TEXT("  RPC Int: input=%d expected=%d actual=%d match=%s\n"), RPCInputValue, RPCExpectedOutput, RPCActualOutput, bRPCValueMatch ? TEXT("YES") : TEXT("NO"));
			R += FString::Printf(TEXT("  RPC Str: input='%s' readback='%s' match=%s\n"), *RPCInputString, *RPCReadBackString, bRPCStringMatch ? TEXT("YES") : TEXT("NO"));
		}
		R += FString::Printf(TEXT("  Env: %s\n"), *Fingerprint.ToString());
		return R;
	}

	/** Emit all fields as individual metrics into a scenario result. */
	void EmitMetrics(FGorgeousInsightScenarioResult& Result, const FString& Prefix) const
	{
		Result.AddMetric(FString::Printf(TEXT("%s.durationS"), *Prefix), GetDurationSeconds(), TEXT("s"));
		Result.AddMetric(FString::Printf(TEXT("%s.queueLatencyS"), *Prefix), QueueLatencySeconds, TEXT("s"));
		Result.AddMetric(FString::Printf(TEXT("%s.dispatchLatencyS"), *Prefix), DispatchLatencySeconds, TEXT("s"));
		Result.AddMetric(FString::Printf(TEXT("%s.totalLatencyS"), *Prefix), TotalLatencySeconds, TEXT("s"));
		Result.AddMetric(FString::Printf(TEXT("%s.stimulusCount"), *Prefix), static_cast<double>(StimulusCount));
		Result.AddMetric(FString::Printf(TEXT("%s.netSendCount"), *Prefix), static_cast<double>(NetSendCount));
		Result.AddMetric(FString::Printf(TEXT("%s.netReceiveCount"), *Prefix), static_cast<double>(NetReceiveCount));
		Result.AddMetric(FString::Printf(TEXT("%s.payloadArgCount"), *Prefix), static_cast<double>(PayloadArgumentCount));
		Result.AddMetric(FString::Printf(TEXT("%s.randomSeed"), *Prefix), static_cast<double>(RandomSeed));
		Result.AddMetric(FString::Printf(TEXT("%s.scalarMatch"), *Prefix), bScalarMatch ? 1.0 : 0.0);
		Result.AddMetric(FString::Printf(TEXT("%s.vectorMatch"), *Prefix), bVectorMatch ? 1.0 : 0.0);
		Result.AddMetric(FString::Printf(TEXT("%s.sequenceMatch"), *Prefix), bSequenceMatch ? 1.0 : 0.0);
		if (RPCInputValue != 0 || RPCExpectedOutput != 0)
		{
			Result.AddMetric(FString::Printf(TEXT("%s.rpcInput"), *Prefix), static_cast<double>(RPCInputValue));
			Result.AddMetric(FString::Printf(TEXT("%s.rpcExpected"), *Prefix), static_cast<double>(RPCExpectedOutput));
			Result.AddMetric(FString::Printf(TEXT("%s.rpcActual"), *Prefix), static_cast<double>(RPCActualOutput));
			Result.AddMetric(FString::Printf(TEXT("%s.rpcValueMatch"), *Prefix), bRPCValueMatch ? 1.0 : 0.0);
			Result.AddMetric(FString::Printf(TEXT("%s.rpcStringMatch"), *Prefix), bRPCStringMatch ? 1.0 : 0.0);
		}
		Fingerprint.EmitMetrics(Result, Prefix + TEXT(".env"));
	}
};

/**
 * Common functionality for perf test object variables (disable persistence/networking and expose cross references).
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API UGorgeousPerfBaseObjectVariable : public UGorgeousObjectVariable
{
	GENERATED_BODY()

public:
	UGorgeousPerfBaseObjectVariable()
	{
		bSupportsNetworking = false;
		bPersistent = false;
	}
	
	/** Registers PerfReplicationPayload for the auto-replication property system when replication activates. */
	virtual void OnReplicationActivated_Implementation(const FGorgeousAutoReplicationContext& Context) override
	{
		Super::OnReplicationActivated_Implementation(Context);
		RegisterReplicatedProperty(FName(TEXT("PerfReplicationPayload")));
	}

	void InjectReplicationStimulus(float StampSeconds, float RandomScalar);
	int32 GetPerfReplicationStimulusCount() const { return PerfReplicationPayload.Sequence; }

	/** Read back the stimulus payload for value verification in tests. */
	const FGorgeousPerfReplicationStimulus& GetPerfReplicationPayload() const { return PerfReplicationPayload; }

	void ConfigureScenarioNetworking(const bool bEnableReplication, const bool bForceRootStack)
	{
		bSupportsNetworking = bEnableReplication;
		RootNetworkConfig.bExposeThroughRootNetworkStack = bEnableReplication && bForceRootStack;
		if (bEnableReplication)
		{
			ReplicationMode = EGorgeousObjectVariableReplicationMode::EManual;
			SetNetworkingEnabled(true);
			bScenarioNetworkingEnabled = true;
		}
		else
		{
			SetNetworkingEnabled(false);
			bScenarioNetworkingEnabled = false;
		}
	}

	bool IsScenarioNetworkingEnabled() const
	{
		return bScenarioNetworkingEnabled;
	}

	void ResetCrossReferences()
	{
		CrossReferences.Reset();
	}

	void AddCrossReference(UGorgeousObjectVariable* Target)
	{
		if (IsValid(Target) && Target != this)
		{
			CrossReferences.Add(Target);
		}
	}

	const TArray<TWeakObjectPtr<UGorgeousObjectVariable>>& GetCrossReferences() const
	{
		return CrossReferences;
	}

protected:
	virtual void Serialize(FArchive& Ar) override
	{
		const bool bIsNetArchive = Ar.IsNetArchive();
		const bool bNetSaving = bIsNetArchive && Ar.IsSaving();
		const bool bNetLoading = bIsNetArchive && Ar.IsLoading();
		Super::Serialize(Ar);

#if WITH_AUTOMATION_TESTS
		if (!bIsNetArchive)
		{
			return;
		}

		const double NowSeconds = FPlatformTime::Seconds();
		if (bNetSaving)
		{
			++PerfNetSendCount;
			PerfLastNetSendSeconds = NowSeconds;
		}
		else if (bNetLoading)
		{
			++PerfNetReceiveCount;
			PerfLastNetReceiveSeconds = NowSeconds;
		}
#else
		(void)bIsNetArchive;
		(void)bNetSaving;
		(void)bNetLoading;
#endif
	}

public:
	int32 GetPerfNetSendCount() const { return PerfNetSendCount; }
	int32 GetPerfNetReceiveCount() const { return PerfNetReceiveCount; }
	double GetPerfLastNetSendSeconds() const { return PerfLastNetSendSeconds; }
	double GetPerfLastNetReceiveSeconds() const { return PerfLastNetReceiveSeconds; }

	/**
	 * Intercept the mixin property-payload apply path so that PerfNetReceiveCount is
	 * incremented regardless of whether data arrived via UE standard subobject replication
	 * (which goes through Serialize()) or via the auto-replication RPC relay
	 * (which bypasses Serialize() entirely).  Both paths therefore increment the same
	 * counter that TickReplicationUntilReceived polls as its completion signal.
	 */
	virtual bool ApplyAutoReplicationPropertyPayload(const FGorgeousAutoReplicationPropertyPayload& Payload, UPackageMap* PackageMap = nullptr, bool bSyncChangeShadow = true) override
	{
		const bool bApplied = Super::ApplyAutoReplicationPropertyPayload(Payload, PackageMap, bSyncChangeShadow);
#if WITH_AUTOMATION_TESTS
		if (bApplied)
		{
			++PerfNetReceiveCount;
			PerfLastNetReceiveSeconds = FPlatformTime::Seconds();
		}
#endif
		return bApplied;
	}

private:
	int32 PerfNetSendCount = 0;
	int32 PerfNetReceiveCount = 0;
	double PerfLastNetSendSeconds = 0.0;
	double PerfLastNetReceiveSeconds = 0.0;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<UGorgeousObjectVariable>> CrossReferences;

	bool bScenarioNetworkingEnabled = false;
	
	UPROPERTY()
	FGorgeousPerfReplicationStimulus PerfReplicationPayload;
};

/**
 * Lightweight concrete object variable used by automation perf tests so we can stress the registry
 * without touching any gameplay specific subclasses.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousPerfObjectVariable : public UGorgeousPerfBaseObjectVariable
{
	GENERATED_BODY()
};

/**
 * Heavier payload variant containing arrays, maps, and sets to emulate diverse data footprints.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousPerfHeavyObjectVariable : public UGorgeousPerfBaseObjectVariable
{
	GENERATED_BODY()

public:
	UGorgeousPerfHeavyObjectVariable() = default;

	void InitializePayload(FRandomStream& RandomStream)
	{
		const int32 FloatCount = RandomStream.RandRange(8, 24);
		FloatingValues.SetNum(FloatCount);
		for (int32 Index = 0; Index < FloatCount; ++Index)
		{
			FloatingValues[Index] = RandomStream.GetFraction() * 2048.0 - 1024.0;
		}

		const int32 VectorCount = RandomStream.RandRange(4, 12);
		VectorSamples.SetNum(VectorCount);
		for (int32 Index = 0; Index < VectorCount; ++Index)
		{
			VectorSamples[Index] = FVector(RandomStream.FRandRange(-1000.f, 1000.f), RandomStream.FRandRange(-1000.f, 1000.f), RandomStream.FRandRange(-1000.f, 1000.f));
		}

		Counters.Empty();
		const int32 CounterCount = RandomStream.RandRange(5, 15);
		for (int32 Index = 0; Index < CounterCount; ++Index)
		{
			Counters.Add(FName(*FString::Printf(TEXT("Counter_%d"), Index)), RandomStream.RandRange(-5000, 5000));
		}

		GuidSet.Reset();
		const int32 GuidCount = RandomStream.RandRange(4, 10);
		for (int32 Index = 0; Index < GuidCount; ++Index)
		{
			GuidSet.Add(FGuid::NewGuid());
		}
	}

private:
	UPROPERTY()
	TArray<double> FloatingValues;

	UPROPERTY()
	TArray<FVector> VectorSamples;

	UPROPERTY()
	TMap<FName, int32> Counters;

	UPROPERTY()
	TSet<FGuid> GuidSet;
};

/**
 * Text-heavy variant storing blobs of narrative/content tags to mimic data-diverse branches.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousPerfNarrativeObjectVariable : public UGorgeousPerfBaseObjectVariable
{
	GENERATED_BODY()

public:
	UGorgeousPerfNarrativeObjectVariable() = default;

	void InitializePayload(FRandomStream& RandomStream, bool bApplyTextBloat, int32 TextBloatCharLength)
	{
		const int32 KeywordCount = RandomStream.RandRange(6, 18);
		Keywords.SetNum(KeywordCount);
		for (int32 Index = 0; Index < KeywordCount; ++Index)
		{
			Keywords[Index] = GenerateToken(RandomStream, RandomStream.RandRange(6, 12));
		}

		const int32 BlobSize = RandomStream.RandRange(64, 256);
		BinaryBlob.SetNum(BlobSize);
		for (int32 Index = 0; Index < BlobSize; ++Index)
		{
			BinaryBlob[Index] = static_cast<uint8>(RandomStream.RandRange(0, 255));
		}

		Narrative = FString::Printf(TEXT("%s:%s:%s"), *GenerateToken(RandomStream, 10), *GenerateToken(RandomStream, 12), *GenerateToken(RandomStream, 8));
		if (bApplyTextBloat && TextBloatCharLength > 0)
		{
			LargeNarrative = GenerateTextBlock(RandomStream, TextBloatCharLength);
		}
		else
		{
			LargeNarrative.Reset();
		}
	}

private:
	static FString GenerateToken(FRandomStream& RandomStream, int32 Length)
	{
		static const TCHAR Alphabet[] = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
		const int32 AlphabetCount = UE_ARRAY_COUNT(Alphabet) - 1;
		FString Result;
		Result.Reserve(Length);
		for (int32 CharIndex = 0; CharIndex < Length; ++CharIndex)
		{
			const int32 AlphabetIndex = RandomStream.RandRange(0, AlphabetCount - 1);
			Result.AppendChar(Alphabet[AlphabetIndex]);
		}
		return Result;
	}

	static FString GenerateTextBlock(FRandomStream& RandomStream, int32 Length)
	{
		static const TCHAR Alphabet[] = TEXT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 .,;:-_+/\\\n");
		const int32 AlphabetCount = UE_ARRAY_COUNT(Alphabet) - 1;
		FString Result;
		Result.Reserve(Length);
		for (int32 CharIndex = 0; CharIndex < Length; ++CharIndex)
		{
			const int32 AlphabetIndex = RandomStream.RandRange(0, AlphabetCount - 1);
			Result.AppendChar(Alphabet[AlphabetIndex]);
		}
		return Result;
	}

	UPROPERTY()
	FString Narrative;

	UPROPERTY()
	FString LargeNarrative;

	UPROPERTY()
	TArray<FString> Keywords;

	UPROPERTY()
	TArray<uint8> BinaryBlob;
};

/**
 * Access-policy aware variant that lets automation toggle allow/deny decisions at runtime.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousPerfAccessPolicyObjectVariable : public UGorgeousPerfObjectVariable
{
	GENERATED_BODY()

public:
	void EnableCustomAccessPolicy(AGorgeousPlayerController* AuthorizedController)
	{
		CustomAuthorizedController = AuthorizedController;
		bUseCustomPolicy = AuthorizedController != nullptr;
		ResetAccessPolicyCounters();
	}

	void SetCustomAccessAllowed(const bool bInAllow)
	{
		bAllowAuthorizedController = bInAllow;
	}

	void ResetAccessPolicyCounters()
	{
		AllowedEvaluations = 0;
		DeniedEvaluations = 0;
	}

	int32 GetAccessPolicyAllowCount() const { return AllowedEvaluations; }
	int32 GetAccessPolicyDenyCount() const { return DeniedEvaluations; }

protected:
	virtual bool CanControllerAccessVariable_Implementation(AGorgeousPlayerController* Controller, FName PropertyName) const override
	{
		if (!bUseCustomPolicy || !CustomAuthorizedController.IsValid())
		{
			const bool bDefault = Super::CanControllerAccessVariable_Implementation(Controller, PropertyName);
			bDefault ? ++AllowedEvaluations : ++DeniedEvaluations;
			return bDefault;
		}

		AGorgeousPlayerController* OwningController = nullptr;
#if WITH_AUTOMATION_TESTS
		OwningController = FGorgeousObjectVariablePerfTestAccess::ResolveOwningPlayerController(this);
#endif
		if (Controller && Controller == OwningController)
		{
			++AllowedEvaluations;
			return true;
		}

		if (Controller && Controller == CustomAuthorizedController.Get())
		{
			if (bAllowAuthorizedController)
			{
				++AllowedEvaluations;
				return true;
			}
			++DeniedEvaluations;
			return false;
		}

		++DeniedEvaluations;
		return false;
	}

private:
	TWeakObjectPtr<AGorgeousPlayerController> CustomAuthorizedController;
	mutable int32 AllowedEvaluations = 0;
	mutable int32 DeniedEvaluations = 0;
	bool bUseCustomPolicy = false;
	bool bAllowAuthorizedController = true;
};

// The listener caches network metrics when enabled; always inherits from UObject to satisfy UHT.
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousPerfNetworkMetricsListener : public UObject
{
	GENERATED_BODY()

public:
	bool TryGetIntMetric(FName MetricName, int64& OutValue) const;
	bool TryGetFloatMetric(FName MetricName, float& OutValue) const;
	bool HasMetric(FName MetricName) const;

#if GORGEOUSCORERUNTIME_ENABLE_NETWORK_METRICS
	// Called by the network metrics system when metrics are available.
	void HandleReport(const UE::Net::FNetworkMetricSnapshot& Snapshot);
#endif

private:
	UPROPERTY()
	TMap<FName, int64> CachedIntMetrics;

	UPROPERTY()
	TMap<FName, float> CachedFloatMetrics;
};

// ═══════════════════════════════════════════════════════════════════════════
// Scoped Log Capture, captures Warning and Error log lines during lifetime
// ═══════════════════════════════════════════════════════════════════════════

/**
 * RAII helper that registers as a global log output device on construction
 * and unregisters on destruction.  While alive every Warning and Error line
 * is captured into CapturedLines, and ALL log lines are captured into
 * FullLogLines.  After destruction, move the arrays into a result.
 */
struct GORGEOUSCORERUNTIME_API FGorgeousInsightScopedLogCapture final : public FOutputDevice
{
	TArray<FString> CapturedLines;  // Warnings + Errors only
	TArray<FString> FullLogLines;   // Everything

	FGorgeousInsightScopedLogCapture()
	{
		GLog->AddOutputDevice(this);
	}

	~FGorgeousInsightScopedLogCapture()
	{
		GLog->RemoveOutputDevice(this);
	}

	// FOutputDevice interface
	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override
	{
		// LogHAL emits blank separator lines between every real log entry, strip them out.
		if (Category == FName(TEXT("LogHAL")))
		{
			return;
		}

		const FString Line = FString::Printf(TEXT("[%s] %s"), *Category.ToString(), V);
		FullLogLines.Add(Line);
		if (Verbosity == ELogVerbosity::Warning || Verbosity == ELogVerbosity::Error || Verbosity == ELogVerbosity::Fatal)
		{
			const TCHAR* Prefix = (Verbosity == ELogVerbosity::Warning) ? TEXT("WARN") : TEXT("ERROR");
			CapturedLines.Add(FString::Printf(TEXT("[%s][%s] %s"), Prefix, *Category.ToString(), V));
		}
	}

	/** Merge captured lines into a scenario result. */
	void ApplyToResult(FGorgeousInsightScenarioResult& Result) const
	{
		for (const FString& Line : CapturedLines)
		{
			if (Line.StartsWith(TEXT("[ERROR]")) || Line.StartsWith(TEXT("[FATAL]")))
			{
				Result.Errors.Add(FString::Printf(TEXT("LOG-CAPTURE: %s"), *Line));
			}
			else
			{
				Result.Warnings.Add(FString::Printf(TEXT("LOG-CAPTURE: %s"), *Line));
			}
		}
		// Copy the full log transcript so SaveScenarioResult writes it to the capture txt file.
		Result.LogCapture.Append(FullLogLines);
		Result.AddMetric(TEXT("logCapture.warningsAndErrors"), static_cast<double>(CapturedLines.Num()));
		Result.AddMetric(TEXT("logCapture.totalLines"), static_cast<double>(FullLogLines.Num()));
	}

	// Non-copyable, non-movable
	FGorgeousInsightScopedLogCapture(const FGorgeousInsightScopedLogCapture&) = delete;
	FGorgeousInsightScopedLogCapture& operator=(const FGorgeousInsightScopedLogCapture&) = delete;
};

inline void UGorgeousPerfBaseObjectVariable::InjectReplicationStimulus(float StampSeconds, float RandomScalar)
{
	PerfReplicationPayload.StampSeconds = StampSeconds;
	PerfReplicationPayload.SampleScalar = RandomScalar;
	PerfReplicationPayload.Sequence++;
	const float SequenceAsFloat = static_cast<float>(PerfReplicationPayload.Sequence);
	const float Spread = FMath::Fmod(RandomScalar * 811.0f, 512.0f);
	PerfReplicationPayload.SampleVector = FVector(RandomScalar * 1024.0f, SequenceAsFloat, Spread + StampSeconds * 0.25f);
}
