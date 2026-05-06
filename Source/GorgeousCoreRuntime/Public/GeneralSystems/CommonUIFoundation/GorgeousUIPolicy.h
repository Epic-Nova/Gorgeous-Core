// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GorgeousUIPolicy.generated.h"

class UGorgeousPrimaryGameLayout;
class ULocalPlayer;
class APlayerController;

/**
 * The Grand Architect of the Gorgeous UI system.
 *
 * A self-contained GameInstanceSubsystem (no Lyra dependency) that:
 *   - Automatically creates a UGorgeousPrimaryGameLayout for every local player.
 *   - Tracks created layouts for O(1) lookup.
 *   - Cleans up automatically when a player leaves.
 *
 * --- SETUP ---
 * 1. Create a Blueprint subclass of your UGorgeousPrimaryGameLayout.
 * 2. Set DefaultLayoutClass in DefaultGame.ini:
 *      [/Script/GorgeousCoreRuntime.GorgeousUIPolicy]
 *      DefaultLayoutClass=/Game/UI/WBP_PrimaryLayout.WBP_PrimaryLayout_C
 * 3. Done. The system handles the rest automatically.
 */
UCLASS(config = Game, DefaultConfig)
class GORGEOUSCORERUNTIME_API UGorgeousUIPolicy : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UGorgeousUIPolicy(const FObjectInitializer& ObjectInitializer);
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Returns the policy for the given world context. O(1). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI|Policy", meta = (WorldContext = "WorldContextObject"))
	static UGorgeousUIPolicy* GetCurrent(const UObject* WorldContextObject);

	/** Returns the root layout for a specific local player. O(1). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI|Policy")
	UGorgeousPrimaryGameLayout* GetRootLayout(const ULocalPlayer* LocalPlayer) const;

	/** Returns the root layout for a player controller. O(1). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI|Policy")
	UGorgeousPrimaryGameLayout* GetRootLayoutForController(const APlayerController* PC) const;

	/** Returns the root layout for the primary (first) player. O(1). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI|Policy", meta = (WorldContext = "WorldContextObject"))
	static UGorgeousPrimaryGameLayout* GetPrimaryPlayerLayout(const UObject* WorldContextObject);

	/**
	 * Blueprint class to create for each local player.
	 * Configure via DefaultGame.ini or in Project Settings.
	 */
	UPROPERTY(config, EditAnywhere, NoClear, Category = "Gorgeous UI")
	TSoftClassPtr<UGorgeousPrimaryGameLayout> DefaultLayoutClass;

protected:
	/** Override to run custom logic after a layout is created for a player. */
	virtual void OnLayoutCreated(ULocalPlayer* LocalPlayer, UGorgeousPrimaryGameLayout* Layout) {}
	/** Override to run custom logic before a layout is destroyed. */
	virtual void OnLayoutRemoving(ULocalPlayer* LocalPlayer, UGorgeousPrimaryGameLayout* Layout) {}

private:
	void CreateLayoutForPlayer(ULocalPlayer* LocalPlayer);
	void RemoveLayoutForPlayer(ULocalPlayer* LocalPlayer);

	void HandleLocalPlayerAdded(ULocalPlayer* LocalPlayer);
	void HandleLocalPlayerRemoved(ULocalPlayer* LocalPlayer);

	/** Map of LocalPlayer -> their PrimaryGameLayout. O(1) lookups. */
	UPROPERTY(Transient)
	TMap<TObjectPtr<ULocalPlayer>, TObjectPtr<UGorgeousPrimaryGameLayout>> Layouts;
};
