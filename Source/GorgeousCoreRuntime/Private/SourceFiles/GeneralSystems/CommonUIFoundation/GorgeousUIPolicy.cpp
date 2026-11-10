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
#include "GameFramework/PlayerController.h"
#include "GameFramework/InputSettings.h"
#include "EnhancedPlayerInput.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"


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

	LayoutRefreshTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UGorgeousUIPolicy::RefreshPlayerLayouts), 0.5f);

	// Handle any players that already exist (common in PIE)
	for (ULocalPlayer* LP : GI->GetLocalPlayers())
	{
		CreateLayoutForPlayer(LP);
	}
}

void UGorgeousUIPolicy::Deinitialize()
{
	FTSTicker::GetCoreTicker().RemoveTicker(LayoutRefreshTicker);
	LayoutRefreshTicker.Reset();
	PendingLayoutLoads.Empty();
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

bool UGorgeousUIPolicy::IsPlayerInputReadyForLayout(const ULocalPlayer* LocalPlayer, const APlayerController* Controller)
{
	if (!IsValid(LocalPlayer) || !IsValid(Controller)) return false;
	const UWorld* PlayerWorld = Controller->GetWorld();
	// Existing worldless widget/preview fixtures do not run a game input system.
	if (!PlayerWorld || !PlayerWorld->IsGameWorld()) return true;
	if (!IsValid(Controller->PlayerInput)) return false;
	const UClass* OverrideClass = Controller->GetOverridePlayerInputClass().Get();
	const UClass* InputClass = OverrideClass ? OverrideClass : UInputSettings::GetDefaultPlayerInputClass();
	if (InputClass && InputClass->IsChildOf(UEnhancedPlayerInput::StaticClass()))
	{
		const UEnhancedPlayerInput* EnhancedInput = Cast<UEnhancedPlayerInput>(Controller->PlayerInput);
		if (!EnhancedInput) return false;
		if (const auto* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			return InputSubsystem->GetPlayerInput() == EnhancedInput;
	}
	// A legacy PlayerInput is valid when the controller/project chose legacy input.
	return true;
}

void UGorgeousUIPolicy::CreateLayoutForPlayer(ULocalPlayer* LocalPlayer)
{
	const UGorgeousUIFoundationSettings* Settings = GetDefault<UGorgeousUIFoundationSettings>();
	if (!LocalPlayer || !Settings || Settings->DefaultLayoutClass.IsNull()) return;
	if (!IsPlayerInputReadyForLayout(LocalPlayer, LocalPlayer->GetPlayerController(GetWorld()))) return;
	if (Layouts.Contains(LocalPlayer) || PendingLayoutLoads.Contains(LocalPlayer)) return; // Already created or loading
	PendingLayoutLoads.Add(LocalPlayer);

	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(Settings->DefaultLayoutClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateWeakLambda(this, [this, LocalPlayer, Settings]()
		{
			PendingLayoutLoads.Remove(LocalPlayer);
			UClass* LoadedClass = Settings->DefaultLayoutClass.Get();
			if (!LoadedClass || !IsValid(LocalPlayer)) return;
			if (Layouts.Contains(LocalPlayer)) return; // Race guard

			APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld());
			// Input may have disappeared or the controller changed while loading.
			// Pending was cleared above, so the normal refresh ticker can retry.
			if (!IsPlayerInputReadyForLayout(LocalPlayer, PC)) return;

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

bool UGorgeousUIPolicy::RefreshPlayerLayouts(float DeltaTime)
{
	UGameInstance* Instance = GetGameInstance();
	if (!Instance) return true;
	for (ULocalPlayer* Player : Instance->GetLocalPlayers())
	{
		UGorgeousPrimaryGameLayout* Layout = GetRootLayout(Player);
		APlayerController* Controller = Player ? Player->GetPlayerController(GetWorld()) : nullptr;
		if (Layout && (!Controller || Layout->GetWorld() != GetWorld() || Layout->GetOwningPlayer() != Controller))
			RemoveLayoutForPlayer(Player);
		if (Controller && !GetRootLayout(Player)) CreateLayoutForPlayer(Player);
	}
	return true;
}
