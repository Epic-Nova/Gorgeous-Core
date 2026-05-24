// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationStorage_OV.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationSettings.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationListenerStructures.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "QualityOfLife/GorgeousWorldSettings.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

UGorgeousStatFoundationStorage_OV::UGorgeousStatFoundationStorage_OV()
{
	bSupportsNetworking = true;
	ReplicationMode = EGorgeousObjectVariableReplicationMode::EFullAutoReplication;
	bUseSharedNetworkStack = true;
}

void UGorgeousStatFoundationStorage_OV::OnReplicationActivated_Implementation(const FGorgeousAutoReplicationContext& Context)
{
	Super::OnReplicationActivated_Implementation(Context);
	
	// Register our internal map for custom payload replication.
	// This ensures BuildCustomAutoReplicationPayload is called whenever the stream is dirty.
	RegisterReplicatedProperty(TEXT("AllActorStats"), EGorgeousReplicationMode::ECustomPayload);
}

bool UGorgeousStatFoundationStorage_OV::CanControllerAccessVariable_Implementation(AGorgeousPlayerController* Controller, FName PropertyName) const
{
	return true;
}

bool UGorgeousStatFoundationStorage_OV::BuildCustomAutoReplicationPayload_Implementation(FName PropertyName, TArray<uint8>& OutPayload, const struct FGorgeousAutoReplicationConditionContext& ConditionContext)
{
	AGorgeousPlayerController* Controller = ConditionContext.TargetController;
	if (!Controller) return false;

	TMap<FString, FGorgeousStatValues_S> FilteredStats;
	for (const auto& ActorPair : AllActorStats)
	{
		AActor* Actor = ActorPair.Key.Get();
		if (!Actor) continue;

		FGorgeousStatValues_S AuthorizedStats;
		for (const auto& StatPair : ActorPair.Value.Values)
		{
			if (EvaluateStatAccess(Controller, Actor, StatPair.Key))
			{
				AuthorizedStats.Values.Add(StatPair.Key, StatPair.Value);
			}
		}

		if (AuthorizedStats.Values.Num() > 0)
		{
			FilteredStats.Add(Actor->GetPathName(), AuthorizedStats);
		}
	}

	FMemoryWriter Writer(OutPayload);
	int32 MapSize = FilteredStats.Num();
	Writer << MapSize;

	for (auto& Pair : FilteredStats)
	{
		FString ActorPath = Pair.Key;
		Writer << ActorPath;
		
		int32 StatCount = Pair.Value.Values.Num();
		Writer << StatCount;
		for (auto& StatPair : Pair.Value.Values)
		{
			FGameplayTag Tag = StatPair.Key;
			float Value = StatPair.Value;
			Writer << Tag;
			Writer << Value;
		}
	}

	return true;
}

bool UGorgeousStatFoundationStorage_OV::ApplyCustomAutoReplicationPayload_Implementation(FName PropertyName, const TArray<uint8>& Payload, const struct FGorgeousAutoReplicationConditionContext& ConditionContext)
{
	if (Payload.Num() == 0) return false;

	FMemoryReader Reader(Payload);
	int32 MapSize = 0;
	Reader << MapSize;

	for (int32 i = 0; i < MapSize; ++i)
	{
		FString ActorPath;
		Reader << ActorPath;

		int32 StatCount = 0;
		Reader << StatCount;

		AActor* Actor = FindObject<AActor>(nullptr, *ActorPath);
		if (!Actor)
		{
			for (int32 j = 0; j < StatCount; ++j)
			{
				FGameplayTag DummyTag;
				float DummyValue;
				Reader << DummyTag;
				Reader << DummyValue;
			}
			continue;
		}

		FGorgeousStatValues_S& ActorStats = AllActorStats.FindOrAdd(Actor);
		FGorgeousStatValues_S& LastActorStats = LastKnownValues.FindOrAdd(Actor);

		for (int32 j = 0; j < StatCount; ++j)
		{
			FGameplayTag Tag;
			float Value;
			Reader << Tag;
			Reader << Value;

			float* LastValue = LastActorStats.Values.Find(Tag);
			if (!LastValue || *LastValue != Value)
			{
				ActorStats.Values.Add(Tag, Value);
				LastActorStats.Values.Add(Tag, Value);
				OnStatChanged.Broadcast(Tag, Value);
			}
		}
	}

	return true;
}

void UGorgeousStatFoundationStorage_OV::SetStatValue(AActor* Actor, FGameplayTag Tag, float Value, AGorgeousPlayerController* Requester)
{
	if (!HasAuthority() || !Actor) return;

	if (Requester && !EvaluateStatAccess(Requester, Actor, Tag)) return;

	const UGorgeousStatFoundationSettings* Settings = GetDefault<UGorgeousStatFoundationSettings>();
	if (const FGorgeousStat_S* Metadata = Settings->FindStatMetadata(Tag))
	{
		// Apply clamping if enabled
		if (Metadata->bClamped)
		{
			Value = FMath::Clamp(Value, Metadata->MinValue, Metadata->MaxValue);
		}

		// Apply rounding rules
		switch (Metadata->RoundingRule)
		{
		case EGorgeousStatRoundingRule::RoundToNearest:
			Value = FMath::RoundToFloat(Value);
			break;
		case EGorgeousStatRoundingRule::Floor:
			Value = FMath::FloorToFloat(Value);
			break;
		case EGorgeousStatRoundingRule::Ceil:
			Value = FMath::CeilToFloat(Value);
			break;
		case EGorgeousStatRoundingRule::TwoDecimals:
			Value = FMath::RoundToFloat(Value * 100.0f) / 100.0f;
			break;
		case EGorgeousStatRoundingRule::None:
		default:
			break;
		}
	}

	FGorgeousStatValues_S& ActorStats = AllActorStats.FindOrAdd(Actor);
	float& CurrentValue = ActorStats.Values.FindOrAdd(Tag);
	
	if (CurrentValue != Value)
	{
		CurrentValue = Value;
		OnStatChanged.Broadcast(Tag, Value);
		MarkPropertyDirty(NAME_None);
	}
}

float UGorgeousStatFoundationStorage_OV::GetStatValue(AActor* Actor, FGameplayTag Tag) const
{
	if (const FGorgeousStatValues_S* ActorStats = AllActorStats.Find(Actor))
	{
		if (const float* Value = ActorStats->Values.Find(Tag))
		{
			return *Value;
		}
	}

	if (const FGorgeousStat_S* Metadata = GetDefault<UGorgeousStatFoundationSettings>()->FindStatMetadata(Tag))
	{
		return Metadata->DefaultValue;
	}

	return 0.0f;
}

bool UGorgeousStatFoundationStorage_OV::HasStat(AActor* Actor, FGameplayTag Tag) const
{
	return AllActorStats.Contains(Actor) || GetDefault<UGorgeousStatFoundationSettings>()->StatRegistry.Contains(Tag);
}

void UGorgeousStatFoundationStorage_OV::AddAllowedController(AActor* Actor, FGameplayTag Tag, AGorgeousPlayerController* Controller)
{
	if (HasAuthority() && Actor && Controller)
	{
		FGorgeousStatListener_S& Registry = GetOrCreateListenerRegistry(Actor);
		FGorgeousStatControllerArray_S& List = Registry.TagSpecificListeners.FindOrAdd(Tag);
		List.Controllers.AddUnique(Controller);
		MarkPropertyDirty(NAME_None);
	}
}

void UGorgeousStatFoundationStorage_OV::RemoveAllowedController(AActor* Actor, FGameplayTag Tag, AGorgeousPlayerController* Controller)
{
	if (HasAuthority() && Actor && Controller)
	{
		if (FGorgeousStatListener_S* Registry = DictionaryAssociations.Find(Actor))
		{
			if (FGorgeousStatControllerArray_S* List = Registry->TagSpecificListeners.Find(Tag))
			{
				List->Controllers.Remove(Controller);
				MarkPropertyDirty(NAME_None);
			}
		}
	}
}

bool UGorgeousStatFoundationStorage_OV::EvaluateStatAccess(AGorgeousPlayerController* Controller, AActor* Actor, FGameplayTag Tag) const
{
	if (!Actor) return false;

	if (const FGorgeousStatListener_S* Registry = DictionaryAssociations.Find(Actor))
	{
		if (Registry->RegisteredListeners.Contains(Controller)) return true;
		if (const FGorgeousStatControllerArray_S* List = Registry->TagSpecificListeners.Find(Tag))
		{
			if (List->Controllers.Contains(Controller)) return true;
		}
	}

	if (const FGorgeousStat_S* Metadata = GetDefault<UGorgeousStatFoundationSettings>()->FindStatMetadata(Tag))
	{
		const FGorgeousStatAccessRules_S& Rules = Metadata->AccessRules;
		switch (Rules.AccessPolicy)
		{
		case EGorgeousObjectVariableAccessPolicy::Everyone: return true;
		case EGorgeousObjectVariableAccessPolicy::OwningControllerOnly:
			return Controller != nullptr && Actor->GetInstigatorController() == Controller;
		case EGorgeousObjectVariableAccessPolicy::Custom:
			if (!Controller) return false;
			for (const auto& AllowedClass : Rules.AllowedClasses)
			{
				if (Controller->IsA(AllowedClass)) return true;
			}
			return false;
		default: break;
		}
	}
	return false;
}

FGorgeousStatListener_S& UGorgeousStatFoundationStorage_OV::GetOrCreateListenerRegistry(AActor* Actor)
{
	if (FGorgeousStatListener_S* Existing = DictionaryAssociations.Find(Actor))
	{
		return *Existing;
	}

	FGorgeousStatListener_S& NewRegistry = DictionaryAssociations.Add(Actor);
	NewRegistry.TargetActor = Actor;
	return NewRegistry;
}
