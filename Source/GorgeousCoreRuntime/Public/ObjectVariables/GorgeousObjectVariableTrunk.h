// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "GorgeousObjectVariableEnums.h"

#if WITH_EDITORONLY_DATA
#include "GorgeousObjectVariableStructures.h"
#endif

#include "GorgeousObjectVariableTrunk.generated.h"

class FProperty;
class FStructProperty;

class UObject;
class UGorgeousObjectVariable;

namespace GorgeousObjectVariableTrunkTags
{
GORGEOUSCORERUNTIME_API extern const FName PropertyAccessor;
}

namespace GorgeousObjectVariableTrunkUtils
{
GORGEOUSCORERUNTIME_API FGorgeousObjectVariableTrunk* ResolveTrunk(UObject* Outer, const FName TrunkPropertyName);
GORGEOUSCORERUNTIME_API const FGorgeousObjectVariableTrunk* ResolveTrunk(const UObject* Outer, const FName TrunkPropertyName);
}

/**
 * Serialized payload that captures the authoritative default-value state for an object variable.
 * This payload is agnostic of the editor/runtime context and can be hydrated on demand.
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousObjectVariableSerializedPayload
{
	GENERATED_BODY()

public:
	FGorgeousObjectVariableSerializedPayload();

	/** Identifier that callers use to reference this payload inside a trunk. */
	UPROPERTY(EditAnywhere, Category = "Gorgeous Object Variable")
	FGuid VariableIdentifier;

	/** Concrete object-variable class that produced this payload. */
	UPROPERTY(EditAnywhere, Category = "Gorgeous Object Variable")
	TSubclassOf<UGorgeousObjectVariable> VariableClass;

	/** Container selection mirrored into graph pins (Single, Array, etc.). */
	UPROPERTY(EditAnywhere, Category = "Gorgeous Object Variable")
	EObjectVariableContainerType_E SelectedContainerType;

	/** When true, the property customization displays all editable properties of the object variable class. */
	UPROPERTY(EditAnywhere, Category = "Gorgeous Object Variable")
	bool bShowInnerProperties;

#if WITH_EDITORONLY_DATA
	/** Cached pin configuration exposed to editor-only tooling. */
	UPROPERTY(EditAnywhere, Category = "Gorgeous Object Variable")
	FObjectVariablePinConfiguration_S CachedPinConfiguration;
#endif

	/** Raw serialized bytes captured from the variable instance. */
	UPROPERTY(VisibleAnywhere, Category = "Gorgeous Object Variable")
	TArray<uint8> SerializedBytes;

	/** Rolling revision that increments whenever the serialized bytes change. */
	UPROPERTY(VisibleAnywhere, Category = "Gorgeous Object Variable")
	uint32 PayloadRevision;

	/** Hash that helps to detect external tampering and cache invalidation. */
	UPROPERTY(VisibleAnywhere, Category = "Gorgeous Object Variable")
	uint32 PayloadHash;

	/** Returns true when the payload stores a class reference and serialized bytes. */
	bool IsValid() const;

	/** Bumps the revision and recomputes the hash. */
	void Touch();

	/** Clears the serialized bytes while preserving metadata. */
	void ResetSerializedState();

	/** Copies editor metadata from the supplied class (if available). */
	void ApplyMetadataFromClass(TSubclassOf<UGorgeousObjectVariable> InClass);
};

/** Lightweight cache entry that mirrors a hydrated runtime instance. */
struct FGorgeousObjectVariableRuntimeCacheEntry
{
	FGorgeousObjectVariableRuntimeCacheEntry()
		: PayloadRevision(0)
	{
	}

	TWeakObjectPtr<UGorgeousObjectVariable> Instance;
	uint32 PayloadRevision;
};

/** Helper responsible for serializing/deserializing object-variable instances. */
struct GORGEOUSCORERUNTIME_API FGorgeousObjectVariableSerialization
{
	static bool WriteObjectToPayload(UGorgeousObjectVariable& Source, FGorgeousObjectVariableSerializedPayload& OutPayload);
	static bool LoadObjectFromPayload(UGorgeousObjectVariable& Destination, const FGorgeousObjectVariableSerializedPayload& InPayload);
	static uint32 ComputePayloadHash(const TArray<uint8>& Bytes);
};

/**
 * Instanced trunk that stores serialized payloads and exposes lazy runtime caches.
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousObjectVariableTrunk
{
	GENERATED_BODY()

public:
	FGorgeousObjectVariableTrunk();

	/** Map of serialized payloads keyed by stable variable identifiers. */
	UPROPERTY(EditAnywhere, Category = "Gorgeous Object Variable")
	TMap<FGuid, FGorgeousObjectVariableSerializedPayload> SerializedEntries;

	const FGorgeousObjectVariableSerializedPayload* FindPayload(const FGuid& Identifier) const;
	FGorgeousObjectVariableSerializedPayload* FindPayload(const FGuid& Identifier);

	FGorgeousObjectVariableSerializedPayload& UpsertPayload(const FGuid& Identifier);

	void SetPayload(const FGorgeousObjectVariableSerializedPayload& Payload);
	bool RemovePayload(const FGuid& Identifier);

	UGorgeousObjectVariable* GetOrCreateRuntimeInstance(const FGuid& Identifier, UObject* Outer) const;

	void InvalidateCache(const FGuid& Identifier) const;
	void InvalidateAll() const;

private:
	UGorgeousObjectVariable* BuildRuntimeInstance(const FGorgeousObjectVariableSerializedPayload& Payload, UObject* Outer) const;

	mutable TMap<FGuid, FGorgeousObjectVariableRuntimeCacheEntry> RuntimeCache;
};
