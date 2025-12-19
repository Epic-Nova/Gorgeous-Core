#include "GorgeousHarnessGameMode.h"

#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/WorldSettings.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "QualityOfLife/GorgeousPlayerState.h"
#include "QualityOfLife/GorgeousGameState.h"

AGorgeousHarnessGameMode::AGorgeousHarnessGameMode()
{
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
	SpectatorClass = nullptr;
	ReplaySpectatorPlayerControllerClass = APlayerController::StaticClass();
	PlayerControllerClass = AGorgeousPlayerController::StaticClass();
	PlayerStateClass = AGorgeousPlayerState::StaticClass();
	GameStateClass = AGorgeousGameState::StaticClass();
}

void AGorgeousHarnessGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	ErrorMessage.Empty();
}

void AGorgeousHarnessGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

void AGorgeousHarnessGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

void AGorgeousHarnessGameMode::InitGameState()
{
	if (!GameSession)
	{
		GameSession = NewObject<AGameSession>(this, AGameSession::StaticClass());
		if (GameSession)
		{
			GameSession->InitOptions(TEXT(""));
		}
	}

	Super::InitGameState();
}

void AGorgeousHarnessGameMode::HandleMatchHasStarted()
{
	// Notify game session
	if (GameSession)
	{
		GameSession->HandleMatchHasStarted();
	}

	// Start human players first
	UWorld* World = GetWorld();
	if (World)
	{
		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = Iterator->Get();
			if (PlayerController && (PlayerController->GetPawn() == nullptr) && PlayerCanRestart(PlayerController))
			{
				RestartPlayer(PlayerController);
			}
		}
	}

	// IMPORTANT: We intentionally skip GEngine->BlockTillLevelStreamingCompleted(GetWorld()) here.
	// In headless automation contexts (e.g., server/client harness tests), calling this function
	// triggers Slate focus changes via the movie player's WaitForMovieToFinish(). CommonUI's
	// UCommonUIActionRouterBase::HandleSlateFocusChanging() then calls GetLocalPlayerIndex(),
	// which crashes because the headless LocalPlayer has no ViewportClient, so GetGameInstance()
	// returns nullptr and GetLocalPlayers() dereferences a null pointer.
	//
	// Since the harness tests don't need level streaming or movie players, skipping this is safe.

	// Fire BeginPlay if we haven't already in waiting to start match
	if (AWorldSettings* WorldSettings = GetWorldSettings())
	{
		WorldSettings->NotifyBeginPlay();

		// Then fire off match started
		WorldSettings->NotifyMatchStarted();
	}

	// Skip replay recording - harness tests don't need it
}
