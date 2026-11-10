#pragma once
#include "CoreMinimal.h"

/** Per-project, per-user network consent. All GT editor requests use this gate. */
namespace GorgeousNetworkConsent
{
    GORGEOUSCOREEDITOR_API bool IsAllowed(bool bPrompt = false);
    GORGEOUSCOREEDITOR_API bool IsSafeURL(const FString& URL);
}
