// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GorgeousUIMessageStructures.generated.h"

class UGorgeousUIMessageConfig_DA;

/**
 * Data passed when requesting a new Gorgeous Message/Dialog.
 */
USTRUCT(BlueprintType)
struct FGorgeousUIMessageRequest
{
	GENERATED_BODY()

	/** Unique ID for this specific message instance to track the result. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FGuid RequestID;

	/** The title text for the dialog. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FText Title;

	/** The main body/message text. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FText Message;

	/** The configuration asset defining the look and feel. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	TObjectPtr<const UGorgeousUIMessageConfig_DA> Config;

	/** Optional custom data to pass to the message widget. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	TMap<FName, FString> Metadata;

	FGorgeousUIMessageRequest()
		: RequestID(FGuid::NewGuid()), Config(nullptr)
	{}
};

/**
 * Data passed back when a message is resolved (button clicked).
 */
USTRUCT(BlueprintType)
struct FGorgeousUIMessageResult
{
	GENERATED_BODY()

	/** The ID of the request this result belongs to. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FGuid RequestID;

	/** The tag of the button that was clicked (e.g., "Confirm", "Cancel", "Repair"). */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FName ResultTag;

	FGorgeousUIMessageResult()
		: ResultTag(NAME_None)
	{}
};
