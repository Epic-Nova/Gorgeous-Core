#pragma once

#include "CoreMinimal.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"

class UObject_AOTOV;
class APlayerController;

namespace FGorgeousQualityOfLifeStatics
{
	GORGEOUSCORERUNTIME_API UObject_AOTOV* EnsureSelfReference(UObject* Owner, TMap<FName, FGorgeousObjectVariableEntry>& AdditionalData, bool bExposeThroughNetworkStack);
	GORGEOUSCORERUNTIME_API void ClearSelfReference(const UObject* Owner, TMap<FName, FGorgeousObjectVariableEntry>& AdditionalData);
	/**
	 * Removes Owner from the SelfReference OV's object array without destroying the
	 * shared OV itself.  Used when a single player leaves a splitscreen session or
	 * disconnects from the server — the OV must persist for the remaining players.
	 * Also prunes any other stale (invalid) entries from the array at the same time.
	 * If the array becomes empty after removal the entire OV entry is cleaned up.
	 */
	GORGEOUSCORERUNTIME_API void RemoveOwnerFromSelfReference(const UObject* Owner, TMap<FName, FGorgeousObjectVariableEntry>& AdditionalData);	
	GORGEOUSCORERUNTIME_API UObject* ResolveSelfReference(TSubclassOf<UObject> QualityOfLifeClass);
	GORGEOUSCORERUNTIME_API FString ResolveSelfReferenceName(TSubclassOf<UObject> QualityOfLifeClass);

	/**
	 * Collects every object stored inside registered SelfReference OVs that is an
	 * instance of QualityOfLifeClass.  If StablePlayerId is non-empty only the object
	 * whose owning PlayerController is registered under that ID is returned.
	 * WorldContextObject is used to filter out OV entries that belong to other worlds
	 * (prevents cross-world bleeding in PIE where multiple worlds share the same process).
	 */
	GORGEOUSCORERUNTIME_API TArray<UObject*> ResolveSelfReferences(const UObject* WorldContextObject, TSubclassOf<UObject> QualityOfLifeClass, const FString& StablePlayerId = FString());

	/** Strips any default object variables from a CDO so they don't end up serialized into cooked assets. */
	GORGEOUSCORERUNTIME_API void SanitizeCDOAdditionalData(UObject* Owner, TMap<FName, FGorgeousObjectVariableEntry>& AdditionalData);

	// ── Local Player Stable ID helpers ────────────────────────────────────
	// These delegate to UGorgeousLocalPlayerRegistry_GIS; they are thin
	// convenience wrappers so non-Blueprint code doesn't need to look up the
	// subsystem directly.

	/**
	 * Auto-registers a PlayerController with a default "LocalPlayer_N" ID if it is
	 * not already in the registry.  Called automatically by EnsureSelfReference.
	 */
	GORGEOUSCORERUNTIME_API void AutoRegisterLocalPlayerStableId(APlayerController* PC);

	/** Explicitly registers (or updates) a PlayerController under a custom stable ID. */
	GORGEOUSCORERUNTIME_API bool RegisterLocalPlayerStableId(APlayerController* PC, const FString& StableId);

	/** Removes a PlayerController from the stable-ID registry. */
	GORGEOUSCORERUNTIME_API void UnregisterLocalPlayerStableId(APlayerController* PC);

	/** Returns the registered stable ID for a PC, or an empty string if not registered. */
	GORGEOUSCORERUNTIME_API FString GetLocalPlayerStableId(const APlayerController* PC);

	/** Returns the PlayerController mapped to a stable ID, or nullptr. */
	GORGEOUSCORERUNTIME_API APlayerController* GetPlayerControllerForStableId(const UObject* WorldContextObject, const FString& StableId);

	/** Renames the stable ID of an already-registered PlayerController. Returns false on conflict. */
	GORGEOUSCORERUNTIME_API bool RenameLocalPlayerStableId(APlayerController* PC, const FString& NewStableId);

	/** Returns all registered stable IDs and their corresponding player indices. */
	GORGEOUSCORERUNTIME_API void GetAllRegisteredLocalPlayers(const UObject* WorldContextObject, TArray<FString>& OutStableIds, TArray<int32>& OutPlayerIndices);
}
