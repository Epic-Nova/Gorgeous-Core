// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/GorgeousUIPolicy.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousPrimaryGameLayout.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSettings.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"


//Heavily oriented on Lyra

UGorgeousUIPolicy::UGorgeousUIPolicy(const FObjectInitializer& ObjectInitializer)
{
}

void UGorgeousUIPolicy::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UGameInstance* GI = GetGameInstance();
	check(GI);

	// Bind for future player changes
	GI->OnLocalPlayerAddedEvent.AddUObject(this, &UGorgeousUIPolicy::HandleLocalPlayerAdded);
	GI->OnLocalPlayerRemovedEvent.AddUObject(this, &UGorgeousUIPolicy::HandleLocalPlayerRemoved);

	// Handle any players that already exist (common in PIE)
	for (ULocalPlayer* LP : GI->GetLocalPlayers())
	{
		CreateLayoutForPlayer(LP);
	}
}

void UGorgeousUIPolicy::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		GI->OnLocalPlayerAddedEvent.RemoveAll(this);
		GI->OnLocalPlayerRemovedEvent.RemoveAll(this);
	}

	// Remove all layouts from viewport
	for (auto& Pair : Layouts)
	{
		if (Pair.Value)
		{
			OnLayoutRemoving(Pair.Key, Pair.Value);
			Pair.Value->RemoveFromParent();
		}
	}
	Layouts.Empty();

	Super::Deinitialize();
}

/*static*/ UGorgeousUIPolicy* UGorgeousUIPolicy::GetCurrent(const UObject* WorldContextObject)
{
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UGorgeousUIPolicy>();
		}
	}
	return nullptr;
}

UGorgeousPrimaryGameLayout* UGorgeousUIPolicy::GetRootLayout(const ULocalPlayer* LocalPlayer) const
{
	if (!LocalPlayer) return nullptr;
	if (const TObjectPtr<UGorgeousPrimaryGameLayout>* Found = Layouts.Find(const_cast<ULocalPlayer*>(LocalPlayer)))
	{
		return Found->Get();
	}
	return nullptr;
}

UGorgeousPrimaryGameLayout* UGorgeousUIPolicy::GetRootLayoutForController(const APlayerController* PC) const
{
	return PC ? GetRootLayout(Cast<ULocalPlayer>(PC->Player)) : nullptr;
}

/*static*/ UGorgeousPrimaryGameLayout* UGorgeousUIPolicy::GetPrimaryPlayerLayout(const UObject* WorldContextObject)
{
	if (const UGorgeousUIPolicy* Policy = GetCurrent(WorldContextObject))
	{
		const APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
		return Policy->GetRootLayoutForController(PC);
	}
	return nullptr;
}

void UGorgeousUIPolicy::CreateLayoutForPlayer(ULocalPlayer* LocalPlayer)
{
	const UGorgeousUIFoundationSettings* Settings = GetDefault<UGorgeousUIFoundationSettings>();
	if (!LocalPlayer || !Settings || Settings->DefaultLayoutClass.IsNull()) return;
	if (Layouts.Contains(LocalPlayer)) return; // Already created

	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(Settings->DefaultLayoutClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateWeakLambda(this, [this, LocalPlayer, Settings]()
		{
			UClass* LoadedClass = Settings->DefaultLayoutClass.Get();
			if (!LoadedClass || !IsValid(LocalPlayer)) return;
			if (Layouts.Contains(LocalPlayer)) return; // Race guard

			APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld());
			if (!PC) return;

			if (UGorgeousPrimaryGameLayout* Layout = CreateWidget<UGorgeousPrimaryGameLayout>(PC, LoadedClass))
			{
				Layouts.Add(LocalPlayer, Layout);
				Layout->AddToViewport(0);
				OnLayoutCreated(LocalPlayer, Layout);
				OnLayoutCreatedDelegate.Broadcast(LocalPlayer, Layout);
			}
		}));
}

void UGorgeousUIPolicy::RemoveLayoutForPlayer(ULocalPlayer* LocalPlayer)
{
	if (UGorgeousPrimaryGameLayout* Layout = GetRootLayout(LocalPlayer))
	{
		OnLayoutRemoving(LocalPlayer, Layout);
		Layout->RemoveFromParent();
	}
	Layouts.Remove(LocalPlayer);
}

void UGorgeousUIPolicy::HandleLocalPlayerAdded(ULocalPlayer* LocalPlayer)
{
	CreateLayoutForPlayer(LocalPlayer);
}

void UGorgeousUIPolicy::HandleLocalPlayerRemoved(ULocalPlayer* LocalPlayer)
{
	RemoveLayoutForPlayer(LocalPlayer);
}
