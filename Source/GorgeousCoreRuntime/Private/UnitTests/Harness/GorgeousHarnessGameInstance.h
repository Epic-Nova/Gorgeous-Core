#pragma once

#include "Engine/GameInstance.h"
#include "GorgeousHarnessGameInstance.generated.h"

struct FWorldContext;

/**
 * Minimal transient game instance to back the automation harness worlds.
 * Provides a way to bind a specific world context since UGameInstance keeps it protected.
 */
UCLASS(Transient)
class UGorgeousHarnessGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	void AttachWorldContext(FWorldContext* InWorldContext)
	{
		WorldContext = InWorldContext;
		if (WorldContext)
		{
			WorldContext->OwningGameInstance = this;
		}
	}
};
