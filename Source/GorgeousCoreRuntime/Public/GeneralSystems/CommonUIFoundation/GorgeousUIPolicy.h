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
#include "Subsystems/GameInstanceSubsystem.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousUIPolicy.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGorgeousPrimaryGameLayout;
class ULocalPlayer;
class APlayerController;
//<------------------------------------------------------------->
/**
 * The Grand Architect of the Gorgeous Core|Common UI Foundation system.
 *
 * A self-contained GameInstanceSubsystem (no Lyra dependency) that:
 *   - Automatically creates a UGorgeousPrimaryGameLayout for every local player.
 *   - Tracks created layouts for O(1) lookup.
 *   - Cleans up automatically when a player leaves.
 *
 * --- SETUP ---
 * 1. Create a Blueprint subclass of your UGorgeousPrimaryGameLayout.
 * 2. Set DefaultLayoutClass in Project Settings -> Gorgeous Core|Common UI Foundation Foundation.
 * 3. Done. The system handles the rest automatically.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLayoutCreatedDelegate, ULocalPlayer*, LocalPlayer, UGorgeousPrimaryGameLayout*, Layout);

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous UIPolicy
| Functional Name: UGorgeousUIPolicy
| Parent Class: UGameInstanceSubsystem
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Provides runtime functionality for Gorgeous UIPolicy.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(config = Game, DefaultConfig,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/GorgeousUIPolicy",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousUIPolicy : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	// Initializes the policy subsystem with the configured layout class.
	UGorgeousUIPolicy(const FObjectInitializer& ObjectInitializer);


	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Initializes layouts for existing and future local players.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Removes layouts and player event bindings during subsystem shutdown.
	virtual void Deinitialize() override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<============================--- Callbacks ---============================>
	#pragma region Callbacks
public:

	// Fired when a primary game layout is created and added to the viewport for a player.
	UPROPERTY(BlueprintAssignable, Category = "Gorgeous Core|Common UI Foundation|Policy")
	FOnLayoutCreatedDelegate OnLayoutCreatedDelegate;
	//<------------------------------------------------------------------------->
	#pragma endregion Callbacks


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Resolves the UI policy for a world context.
	 *
	 * @param WorldContextObject The object that supplies the target world.
	 * @return The active UI policy, or null when it is unavailable.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Policy", meta = (WorldContext = "WorldContextObject"))
	static UGorgeousUIPolicy* GetCurrent(const UObject* WorldContextObject);

	/**
	 * Resolves the root layout for a local player.
	 *
	 * @param LocalPlayer The player that owns the layout.
	 * @return The player's root layout, or null when it is unavailable.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Policy")
	UGorgeousPrimaryGameLayout* GetRootLayout(const ULocalPlayer* LocalPlayer) const;

	/**
	 * Resolves the root layout for a player controller.
	 *
	 * @param PC The controller that owns the target local player.
	 * @return The player's root layout, or null when it is unavailable.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Policy")
	UGorgeousPrimaryGameLayout* GetRootLayoutForController(const APlayerController* PC) const;

	/**
	 * Resolves the root layout for the primary local player.
	 *
	 * @param WorldContextObject The object that supplies the target world.
	 * @return The primary player's root layout, or null when it is unavailable.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Policy", meta = (WorldContext = "WorldContextObject"))
	static UGorgeousPrimaryGameLayout* GetPrimaryPlayerLayout(const UObject* WorldContextObject);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
protected:
	/**
	 * Runs custom logic after a layout is created for a player.
	 *
	 * @param LocalPlayer The player that owns the created layout.
	 * @param Layout The created layout.
	 */
	virtual void OnLayoutCreated(ULocalPlayer* LocalPlayer, UGorgeousPrimaryGameLayout* Layout) {}
	/**
	 * Runs custom logic before a player's layout is destroyed.
	 *
	 * @param LocalPlayer The player that owns the layout being removed.
	 * @param Layout The layout being removed.
	 */
	virtual void OnLayoutRemoving(ULocalPlayer* LocalPlayer, UGorgeousPrimaryGameLayout* Layout) {}

	/**
	 * Creates the configured layout for a local player.
	 *
	 * @param LocalPlayer The player that receives the layout.
	 */
	void CreateLayoutForPlayer(ULocalPlayer* LocalPlayer);
	/**
	 * Removes the layout owned by a local player.
	 *
	 * @param LocalPlayer The player whose layout is removed.
	 */
	void RemoveLayoutForPlayer(ULocalPlayer* LocalPlayer);

	/**
	 * Handles a local player joining the game instance.
	 *
	 * @param LocalPlayer The player that joined.
	 */
	void HandleLocalPlayerAdded(ULocalPlayer* LocalPlayer);
	/**
	 * Handles a local player leaving the game instance.
	 *
	 * @param LocalPlayer The player that left.
	 */
	void HandleLocalPlayerRemoved(ULocalPlayer* LocalPlayer);
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only


	//<============================--- Variables ---============================>
	#pragma region Variables
private:

	// Map of LocalPlayer -> their PrimaryGameLayout. O(1) lookups.
	UPROPERTY(Transient)
	TMap<TObjectPtr<ULocalPlayer>, TObjectPtr<UGorgeousPrimaryGameLayout>> Layouts;
	//<------------------------------------------------------------------------->
	#pragma endregion Variables
};