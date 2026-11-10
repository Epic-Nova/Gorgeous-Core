// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once
#include "CoreMinimal.h"
enum class EGorgeousConfigOperation : uint8;

namespace GorgeousConfiguration
{
/** Edits one source INI section without replacing unrelated array entries or comments. */
GORGEOUSCORERUNTIME_API bool ApplyTextEdit(FString& Contents, const FString& Section, const FString& Key,
	const FString& Value, EGorgeousConfigOperation Operation);
}
