#pragma once

#include "GameFramework/GameMode.h"
#include "GorgeousHarnessGameMode.generated.h"

/**
 * Minimal transient GameMode used exclusively by the automation harness worlds.
 * Avoids any project-specific blueprint logic so we can stand up server/client
 * contexts without triggering gameplay constructors.
 */
UCLASS(Transient)
class AGorgeousHarnessGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AGorgeousHarnessGameMode();

protected:
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void InitGameState() override;

	/**
	 * Override to skip BlockTillLevelStreamingCompleted which can trigger Slate focus changes.
	 * In headless automation contexts, CommonUI's action router may crash when handling focus
	 * events because the local player lacks a ViewportClient (so GetGameInstance() returns null).
	 */
	virtual void HandleMatchHasStarted() override;
};
