// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Blueprint/UserWidget.h"
#include "GorgeousInsightActionConfig_DA.generated.h"

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousBlueprintInsightAction
{
	GENERATED_BODY()

	/** Unique Action ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FName ActionId;

	/** Display Name in the Insight Matrix */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FText DisplayName;

	/** Detailed description of the action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FText Description;

	/** The category under which this action appears */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FName Category;

	/** The Editor Utility Widget class to spawn in Editor mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets", meta=(MetaClass="/Script/Blutility.EditorUtilityWidget"))
	TSoftClassPtr<class UUserWidget> EditorWidgetClass;

	/** The Runtime User Widget class to spawn in PIE/Standalone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSoftClassPtr<class UUserWidget> RuntimeWidgetClass;
};

/**
 * Data asset used to register Insight Matrix actions and bind them to
 * Editor Utility Widgets and Runtime Debug Widgets.
 */
UCLASS(BlueprintType)
class GORGEOUSCORERUNTIME_API UGorgeousInsightActionConfig_DA : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Which provider/system this action config belongs to (e.g., "TeamSystem", "Inventory"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FName ProviderName;

	/** The list of actions defined by this configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TArray<FGorgeousBlueprintInsightAction> Actions;
};
