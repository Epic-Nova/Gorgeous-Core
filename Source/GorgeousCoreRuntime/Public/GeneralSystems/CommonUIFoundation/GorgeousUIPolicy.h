// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GorgeousUIPolicy.generated.h"

class UGorgeousPrimaryGameLayout;
class ULocalPlayer;
class APlayerController;

/**
<<<<<<< HEAD
 * The Grand Architect of the Gorgeous Core|Common UI Foundation system.
=======
 * The Grand Architect of the Gorgeous UI system.
>>>>>>> b4c134c (Some other changes i dont remember)
 *
 * A self-contained GameInstanceSubsystem (no Lyra dependency) that:
 *   - Automatically creates a UGorgeousPrimaryGameLayout for every local player.
 *   - Tracks created layouts for O(1) lookup.
 *   - Cleans up automatically when a player leaves.
 *
 * --- SETUP ---
 * 1. Create a Blueprint subclass of your UGorgeousPrimaryGameLayout.
<<<<<<< HEAD
 * 2. Set DefaultLayoutClass in Project Settings -> Gorgeous Core|Common UI Foundation Foundation.
=======
 * 2. Set DefaultLayoutClass in Project Settings -> Gorgeous UI Foundation.
>>>>>>> b4c134c (Some other changes i dont remember)
 * 3. Done. The system handles the rest automatically.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLayoutCreatedDelegate, ULocalPlayer*, LocalPlayer, UGorgeousPrimaryGameLayout*, Layout);

UCLASS(config = Game, DefaultConfig)
class GORGEOUSCORERUNTIME_API UGorgeousUIPolicy : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UGorgeousUIPolicy(const FObjectInitializer& ObjectInitializer);
	
public:
	/** Fired when a primary game layout is created and added to the viewport for a player. */
<<<<<<< HEAD
	UPROPERTY(BlueprintAssignable, Category = "Gorgeous Core|Common UI Foundation|Policy")
=======
	UPROPERTY(BlueprintAssignable, Category = "Gorgeous UI|Policy")
>>>>>>> b4c134c (Some other changes i dont remember)
	FOnLayoutCreatedDelegate OnLayoutCreatedDelegate;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Returns the policy for the given world context. O(1). */
<<<<<<< HEAD
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Policy", meta = (WorldContext = "WorldContextObject"))
	static UGorgeousUIPolicy* GetCurrent(const UObject* WorldContextObject);

	/** Returns the root layout for a specific local player. O(1). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Policy")
	UGorgeousPrimaryGameLayout* GetRootLayout(const ULocalPlayer* LocalPlayer) const;

	/** Returns the root layout for a player controller. O(1). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Policy")
	UGorgeousPrimaryGameLayout* GetRootLayoutForController(const APlayerController* PC) const;

	/** Returns the root layout for the primary (first) player. O(1). */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Policy", meta = (WorldContext = "WorldContextObject"))
=======
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
>>>>>>> b4c134c (Some other changes i dont remember)
	static UGorgeousPrimaryGameLayout* GetPrimaryPlayerLayout(const UObject* WorldContextObject);

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
