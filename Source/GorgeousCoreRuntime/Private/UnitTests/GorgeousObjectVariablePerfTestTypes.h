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

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "HAL/PlatformTime.h"
#include "Math/RandomStream.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector.h"
#include "Net/NetworkMetricsDatabase.h"
#include "Net/UnrealNetwork.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InjectReplicationStimulus(float StampSeconds, float RandomScalar);
	int32 GetPerfReplicationStimulusCount() const { return PerfReplicationPayload.Sequence; }

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

private:
	int32 PerfNetSendCount = 0;
	int32 PerfNetReceiveCount = 0;
	double PerfLastNetSendSeconds = 0.0;
	double PerfLastNetReceiveSeconds = 0.0;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<UGorgeousObjectVariable>> CrossReferences;

	bool bScenarioNetworkingEnabled = false;

	UPROPERTY(Replicated)
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
	virtual bool CanControllerAccessVariable_Implementation(AGorgeousPlayerController* Controller) const override
	{
		if (!bUseCustomPolicy || !CustomAuthorizedController.IsValid())
		{
			const bool bDefault = Super::CanControllerAccessVariable_Implementation(Controller);
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

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousPerfNetworkMetricsListener : public UNetworkMetricsBaseListener
{
	GENERATED_BODY()

public:
	UGorgeousPerfNetworkMetricsListener();

	virtual void Report(const UE::Net::FNetworkMetricSnapshot& Snapshot) override;

	bool TryGetIntMetric(FName MetricName, int64& OutValue) const;
	bool TryGetFloatMetric(FName MetricName, float& OutValue) const;
	bool HasMetric(FName MetricName) const;

private:
	UPROPERTY()
	TMap<FName, int64> CachedIntMetrics;

	UPROPERTY()
	TMap<FName, float> CachedFloatMetrics;
};

inline void UGorgeousPerfBaseObjectVariable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGorgeousPerfBaseObjectVariable, PerfReplicationPayload);
}

inline void UGorgeousPerfBaseObjectVariable::InjectReplicationStimulus(float StampSeconds, float RandomScalar)
{
	PerfReplicationPayload.StampSeconds = StampSeconds;
	PerfReplicationPayload.SampleScalar = RandomScalar;
	PerfReplicationPayload.Sequence++;
	const float SequenceAsFloat = static_cast<float>(PerfReplicationPayload.Sequence);
	const float Spread = FMath::Fmod(RandomScalar * 811.0f, 512.0f);
	PerfReplicationPayload.SampleVector = FVector(RandomScalar * 1024.0f, SequenceAsFloat, Spread + StampSeconds * 0.25f);
}
