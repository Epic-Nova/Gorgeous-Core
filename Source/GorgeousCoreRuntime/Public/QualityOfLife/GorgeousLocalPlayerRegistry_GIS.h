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
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousLocalPlayerRegistry_GIS.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class APlayerController;
class AGameModeBase;
class AController;
//<------------------------------------------------------------->//<-------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Local Player Registry
| Functional Name: UGorgeousLocalPlayerRegistry_GIS
| Parent Class: UGameInstanceSubsystem
| Class Suffix: _GIS
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Game Instance Subsystem that maintains a **stable string ID ↔
| PlayerController** registry. Every PlayerController that participates in
| QoL self-reference logic is automatically assigned a default stable ID of
| the form "LocalPlayer_N" (where N equals the local player index) the first
| time EnsureSelfReference is called for it. Callers can rename these IDs at
| any time via RenameLocalPlayer(). The registry is kept up to date by
| binding to FGameModeEvents, so entries are added on PostLogin and removed
| on Logout, no manual cleanup required between level transitions.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/QualityOfLife/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/QualityOfLife/GorgeousLocalPlayerRegistry_GIS",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/QualityOfLife/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousLocalPlayerRegistry_GIS : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	// ── Lifecycle ──────────────────────────────────────────────────────────

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ── Registry API ──────────────────────────────────────────────────────

	/**
	 * Registers a PlayerController under the given stable string ID.
	 * If the PC is already registered the existing entry is updated in-place.
	 * If StableId is already taken by a different PC the call is a no-op and returns false.
	 */
	bool RegisterPC(APlayerController* PC, const FString& StableId);

	/**
	 * Removes the entry for the given PlayerController (both directions).
	 * Safe to call when the PC is already unregistered.
	 */
	void UnregisterPC(APlayerController* PC);

	/**
	 * Renames the stable ID of an already-registered PlayerController.
	 * Returns false if NewStableId is taken by a different PC or PC is not registered.
	 */
	bool RenameLocalPlayer(APlayerController* PC, const FString& NewStableId);

	/** Returns the stable string ID for a PlayerController, or an empty string if not registered. */
	FString GetStableId(const APlayerController* PC) const;

	/** Returns the PlayerController associated with a stable ID, or nullptr if unknown/stale. */
	APlayerController* GetPCForStableId(const FString& StableId) const;

	/**
	 * Returns all current entries as two parallel arrays (stable IDs and their assigned
	 * local-player indices). Stale weak pointers are pruned during this call.
	 */
	void GetAllEntries(TArray<FString>& OutStableIds, TArray<int32>& OutPlayerIndices);

	/**
	 * Returns the next integer that should be used as the N in "LocalPlayer_N" for a new
	 * registration, max existing auto-index + 1, or 0 if none are registered.
	 */
	int32 GetNextAutoIndex() const;

	/** Helper: get (or create) the subsystem for the game instance that owns WorldContextObject. */
	static UGorgeousLocalPlayerRegistry_GIS* Get(const UObject* WorldContextObject);

	/** Helper: get (or create) the subsystem from a PlayerController's world. */
	static UGorgeousLocalPlayerRegistry_GIS* GetFromPC(const APlayerController* PC);

private:

	// ── Delegate callbacks ─────────────────────────────────────────────────

	void OnGameModePostLogin(AGameModeBase* GameMode, APlayerController* NewPC);
	void OnGameModeLogout(AGameModeBase* GameMode, AController* Exiting);

	// ── State ──────────────────────────────────────────────────────────────

	/** PC → stable ID. Weak pointer so the entry doesn't prevent GC. */
	TMap<TWeakObjectPtr<APlayerController>, FString> PCToStableId;

	/** Stable ID → PC. */
	TMap<FString, TWeakObjectPtr<APlayerController>> StableIdToPC;

	/** PC → auto-assigned local player index (used to build default IDs and for Blueprint queries). */
	TMap<TWeakObjectPtr<APlayerController>, int32> PCToIndex;

	FDelegateHandle PostLoginHandle;
	FDelegateHandle LogoutHandle;
};