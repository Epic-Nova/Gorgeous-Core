// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GorgeousPermissionProvider.generated.h"

/**
 * Base UObject representation of a permission or configuration provider.
 * Exposes runtime evaluation, cheat manager overrides, and registration.
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class GORGEOUSCORERUNTIME_API UGorgeousPermissionProvider : public UObject
{
	GENERATED_BODY()

public:
	UGorgeousPermissionProvider();

	/** Unique system permission identifier (e.g. "com.gorgeous.simsalabim.chest.open"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Permission")
	FString PermissionName;

	/** Human-readable name. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Permission")
	FText DisplayLabel;

	/** Detailed description of what this permission grants. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Permission")
	FText Description;

	/** Default allowance if no override or team rule matches. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Permission")
	bool bDefaultAllowed = true;

	/** If true, the cheat manager override will take precedence. */
	UPROPERTY(BlueprintReadOnly, Category = "Permission|Cheat")
	bool bCheatOverrideActive = false;

	/** Cheat-forced permission value. */
	UPROPERTY(BlueprintReadOnly, Category = "Permission|Cheat")
	bool bCheatOverrideValue = false;

	/** Sets cheat-forced status. Easy hook for Cheat Manager extensions. */
	UFUNCTION(BlueprintCallable, Category = "Permission|Cheat")
	void SetCheatOverride(bool bInOverrideActive, bool bInOverrideValue);

	/** Evaluates whether the given interactor is authorized to interact with the target object. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Permission")
	bool EvaluatePermission(AActor* Interactor, AActor* TargetObject) const;
	virtual bool EvaluatePermission_Implementation(AActor* Interactor, AActor* TargetObject) const;

public:
	/** Registers a provider with the global registry. */
	static void RegisterProvider(UGorgeousPermissionProvider* Provider);

	/** Unregisters a provider. */
	static void UnregisterProvider(UGorgeousPermissionProvider* Provider);

	/** Retrieves all registered providers. */
	static TArray<UGorgeousPermissionProvider*> GetRegisteredProviders();

	/** Finds a registered provider by name. */
	static UGorgeousPermissionProvider* FindProvider(const FString& InPermissionName);

private:
	static TMap<FString, TWeakObjectPtr<UGorgeousPermissionProvider>> RegisteredProviders;
};
