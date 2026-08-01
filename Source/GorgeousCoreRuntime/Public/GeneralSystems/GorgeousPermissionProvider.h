// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "UObject/NoExportTypes.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousPermissionProvider.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Permission Provider
| Functional Name: UGorgeousPermissionProvider
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Base UObject representation of a permission or configuration provider.
| Exposes runtime evaluation, cheat manager overrides, and registration.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/GorgeousPermissionProvider",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousPermissionProvider : public UObject
{
	GENERATED_BODY()

	// Constructs a permission provider.
public:
	UGorgeousPermissionProvider();

	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

	// Unique system permission identifier (e.g. "com.gorgeous.simsalabim.chest.open").
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Permission")
	FString PermissionName;

	// Human-readable name.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Permission")
	FText DisplayLabel;

	// Detailed description of what this permission grants.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Permission")
	FText Description;

	// Default allowance if no override or team rule matches.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Permission")
	bool bDefaultAllowed = true;

	// If true, the cheat manager override will take precedence.
	UPROPERTY(BlueprintReadOnly, Category = "Permission|Cheat")
	bool bCheatOverrideActive = false;

	// Cheat-forced permission value.
	UPROPERTY(BlueprintReadOnly, Category = "Permission|Cheat")
	bool bCheatOverrideValue = false;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Sets the cheat-forced permission status.
	 *
	 * @param bInOverrideActive Whether the cheat override is enabled.
	 * @param bInOverrideValue The permission value forced by the cheat override.
	 */
	UFUNCTION(BlueprintCallable, Category = "Permission|Cheat")
	void SetCheatOverride(bool bInOverrideActive, bool bInOverrideValue);

	/**
	 * Evaluates whether an interactor is authorized to access a target object.
	 *
	 * @param Interactor The actor attempting access.
	 * @param TargetObject The object being accessed.
	 * @return True when access is granted, false otherwise.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Permission")
	bool EvaluatePermission(AActor* Interactor, AActor* TargetObject) const;
	virtual bool EvaluatePermission_Implementation(AActor* Interactor, AActor* TargetObject) const;
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
public:
	/**
	 * Registers a provider with the global registry.
	 *
	 * @param Provider The provider to register.
	 */
	static void RegisterProvider(UGorgeousPermissionProvider* Provider);

	/**
	 * Unregisters a provider from the global registry.
	 *
	 * @param Provider The provider to unregister.
	 */
	static void UnregisterProvider(UGorgeousPermissionProvider* Provider);

	/**
	 * Returns every registered permission provider.
	 *
	 * @return The registered permission providers.
	 */
	static TArray<UGorgeousPermissionProvider*> GetRegisteredProviders();

	/**
	 * Finds a registered provider by permission name.
	 *
	 * @param InPermissionName The permission name to find.
	 * @return The registered provider, or nullptr when none matches.
	 */
	static UGorgeousPermissionProvider* FindProvider(const FString& InPermissionName);
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only


	//<============================--- Variables ---============================>
	#pragma region Variables
private:
	static TMap<FString, TWeakObjectPtr<UGorgeousPermissionProvider>> RegisteredProviders;
	//<------------------------------------------------------------------------->
	#pragma endregion Variables
};