// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Local Player Registry Subsystem             |
<==========================================================================*/
#include "QualityOfLife/GorgeousLocalPlayerRegistry_GIS.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

//=============================================================================
// UGorgeousLocalPlayerRegistry_GIS Implementation
//=============================================================================

void UGorgeousLocalPlayerRegistry_GIS::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PostLoginHandle = FGameModeEvents::GameModePostLoginEvent.AddUObject(
		this, &UGorgeousLocalPlayerRegistry_GIS::OnGameModePostLogin);

	LogoutHandle = FGameModeEvents::GameModeLogoutEvent.AddUObject(
		this, &UGorgeousLocalPlayerRegistry_GIS::OnGameModeLogout);
}

void UGorgeousLocalPlayerRegistry_GIS::Deinitialize()
{
	FGameModeEvents::GameModePostLoginEvent.Remove(PostLoginHandle);
	FGameModeEvents::GameModeLogoutEvent.Remove(LogoutHandle);
	PostLoginHandle.Reset();
	LogoutHandle.Reset();

	PCToStableId.Reset();
	StableIdToPC.Reset();
	PCToIndex.Reset();

	Super::Deinitialize();
}

// ── Delegate callbacks ─────────────────────────────────────────────────────

void UGorgeousLocalPlayerRegistry_GIS::OnGameModePostLogin(AGameModeBase* /*GameMode*/, APlayerController* NewPC)
{
	if (!NewPC)
	{
		return;
	}

	// Only auto-register PCs that belong to this GameInstance
	if (const UWorld* PCWorld = NewPC->GetWorld())
	{
		if (PCWorld->GetGameInstance() != GetGameInstance())
		{
			return;
		}
	}

	// Auto-register with default "LocalPlayer_N" ID if not already registered
	if (!PCToStableId.Contains(TWeakObjectPtr<APlayerController>(NewPC)))
	{
		// Derive local player index
		int32 PlayerIndex = GetNextAutoIndex();
		if (const ULocalPlayer* LP = NewPC->GetLocalPlayer())
		{
			if (const UGameInstance* GI = LP->GetGameInstance())
			{
				PlayerIndex = GI->GetLocalPlayers().IndexOfByKey(LP);
			}
		}

		const FString DefaultId = FString::Printf(TEXT("LocalPlayer_%d"), PlayerIndex);

		// Avoid collisions by appending until the ID is free
		FString FinalId = DefaultId;
		int32 Suffix = 1;
		while (StableIdToPC.Contains(FinalId) && StableIdToPC[FinalId].IsValid())
		{
			FinalId = FString::Printf(TEXT("%s_%d"), *DefaultId, Suffix++);
		}

		RegisterPC(NewPC, FinalId);
	}
}

void UGorgeousLocalPlayerRegistry_GIS::OnGameModeLogout(AGameModeBase* /*GameMode*/, AController* Exiting)
{
	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		UnregisterPC(PC);
	}
}

// ── Registry API ──────────────────────────────────────────────────────────

bool UGorgeousLocalPlayerRegistry_GIS::RegisterPC(APlayerController* PC, const FString& StableId)
{
	if (!PC || StableId.IsEmpty())
	{
		return false;
	}

	TWeakObjectPtr<APlayerController> WeakPC(PC);

	// If this stable ID is already taken by a *different* live PC, refuse
	if (const TWeakObjectPtr<APlayerController>* Existing = StableIdToPC.Find(StableId))
	{
		if (Existing->IsValid() && Existing->Get() != PC)
		{
			return false;
		}
	}

	// Remove old stable ID for this PC if it had one
	if (const FString* OldId = PCToStableId.Find(WeakPC))
	{
		StableIdToPC.Remove(*OldId);
	}

	// Derive local player index
	int32 PlayerIndex = GetNextAutoIndex();
	if (const ULocalPlayer* LP = PC->GetLocalPlayer())
	{
		if (const UGameInstance* GI = LP->GetGameInstance())
		{
			PlayerIndex = GI->GetLocalPlayers().IndexOfByKey(LP);
		}
	}

	PCToStableId.Add(WeakPC, StableId);
	StableIdToPC.Add(StableId, WeakPC);
	PCToIndex.Add(WeakPC, PlayerIndex);
	return true;
}

void UGorgeousLocalPlayerRegistry_GIS::UnregisterPC(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	TWeakObjectPtr<APlayerController> WeakPC(PC);
	if (const FString* StableId = PCToStableId.Find(WeakPC))
	{
		StableIdToPC.Remove(*StableId);
	}
	PCToStableId.Remove(WeakPC);
	PCToIndex.Remove(WeakPC);
}

bool UGorgeousLocalPlayerRegistry_GIS::RenameLocalPlayer(APlayerController* PC, const FString& NewStableId)
{
	if (!PC || NewStableId.IsEmpty())
	{
		return false;
	}

	TWeakObjectPtr<APlayerController> WeakPC(PC);

	// Must already be registered
	if (!PCToStableId.Contains(WeakPC))
	{
		return false;
	}

	// New ID taken by a different live PC?
	if (const TWeakObjectPtr<APlayerController>* Occupant = StableIdToPC.Find(NewStableId))
	{
		if (Occupant->IsValid() && Occupant->Get() != PC)
		{
			return false;
		}
	}

	const FString OldId = PCToStableId[WeakPC];
	StableIdToPC.Remove(OldId);
	PCToStableId[WeakPC] = NewStableId;
	StableIdToPC.Add(NewStableId, WeakPC);
	return true;
}

FString UGorgeousLocalPlayerRegistry_GIS::GetStableId(const APlayerController* PC) const
{
	if (!PC)
	{
		return FString();
	}

	const TWeakObjectPtr<APlayerController> WeakPC(const_cast<APlayerController*>(PC));
	if (const FString* Id = PCToStableId.Find(WeakPC))
	{
		return *Id;
	}
	return FString();
}

APlayerController* UGorgeousLocalPlayerRegistry_GIS::GetPCForStableId(const FString& StableId) const
{
	if (const TWeakObjectPtr<APlayerController>* WeakPC = StableIdToPC.Find(StableId))
	{
		return WeakPC->Get();
	}
	return nullptr;
}

void UGorgeousLocalPlayerRegistry_GIS::GetAllEntries(TArray<FString>& OutStableIds, TArray<int32>& OutPlayerIndices)
{
	OutStableIds.Reset();
	OutPlayerIndices.Reset();

	// Prune stale pointers while iterating
	TArray<TWeakObjectPtr<APlayerController>> StaleKeys;

	for (auto& [WeakPC, StableId] : PCToStableId)
	{
		if (!WeakPC.IsValid())
		{
			StaleKeys.Add(WeakPC);
			continue;
		}
		const int32 Index = PCToIndex.Contains(WeakPC) ? PCToIndex[WeakPC] : -1;
		OutStableIds.Add(StableId);
		OutPlayerIndices.Add(Index);
	}

	for (const TWeakObjectPtr<APlayerController>& Stale : StaleKeys)
	{
		if (const FString* Id = PCToStableId.Find(Stale))
		{
			StableIdToPC.Remove(*Id);
		}
		PCToStableId.Remove(Stale);
		PCToIndex.Remove(Stale);
	}
}

int32 UGorgeousLocalPlayerRegistry_GIS::GetNextAutoIndex() const
{
	int32 MaxIndex = -1;
	for (const auto& [WeakPC, Index] : PCToIndex)
	{
		if (WeakPC.IsValid())
		{
			MaxIndex = FMath::Max(MaxIndex, Index);
		}
	}
	return MaxIndex + 1;
}

// ── Static helpers ─────────────────────────────────────────────────────────

UGorgeousLocalPlayerRegistry_GIS* UGorgeousLocalPlayerRegistry_GIS::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UGorgeousLocalPlayerRegistry_GIS>();
		}
	}
	return nullptr;
}

UGorgeousLocalPlayerRegistry_GIS* UGorgeousLocalPlayerRegistry_GIS::GetFromPC(const APlayerController* PC)
{
	return PC ? Get(PC) : nullptr;
}
