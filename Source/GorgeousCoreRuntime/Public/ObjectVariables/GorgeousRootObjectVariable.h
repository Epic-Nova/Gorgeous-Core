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
#include "GorgeousObjectVariable.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousRootObjectVariable.generated.h"
//<-------------------------------------------------------------------------->

UENUM(BlueprintType)
enum class EGorgeousObjectVariableOrphanResolution : uint8
{
   ReparentToRoot UMETA(DisplayName = "Reparent Orphans To Root"),
   DestroyOrphans UMETA(DisplayName = "Destroy Orphans")
};

USTRUCT(BlueprintType)
struct FGorgeousRootRegistryOwnerHandle
{
   GENERATED_BODY()

   FGorgeousRootRegistryOwnerHandle()
      : RootName(NAME_None)
      , OwnerToken()
   {
   }

   // Root registry this handle refers to.
   UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
   FName RootName;

   // Stable token assigned when ownership was first granted.
   UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
   FGuid OwnerToken;

   // Stable identifier provided by the original owner, typically a connection key.
   UPROPERTY(BlueprintReadOnly, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
   FString StableIdentifier;

   bool IsValid() const { return OwnerToken.IsValid(); }
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Root Object Variable
| Functional Name: UGorgeousRootObjectVariable
| Parent Class: UGorgeousObjectVariable
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| The root object variable is the central registry for all object variables.
| It provides global access, hierarchy management, property access, and
| registry cleanup.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ObjectVariables/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ObjectVariables/GorgeousRootObjectVariable",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ObjectVariables/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousRootObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()

public:
   /** Broadcast when root registry membership or ownership changes (root name provided, may be None). */
   static FSimpleMulticastDelegate OnRootRegistryChanged;

    /**
     * Constructor for the root object variable.
     */
    UGorgeousRootObjectVariable();

   /**
    * Gets (or lazily creates) the named root object variable.
    *
    * @param RootName Optional name resolved against the root settings. When omitted, the default root is returned.
    */
   UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables")
   static UGorgeousRootObjectVariable* GetRootObjectVariable(FName RootName = NAME_None);

   /** Returns an already instantiated root if it exists without creating a new one. */
   static UGorgeousRootObjectVariable* TryGetExistingRoot(FName RootName = NAME_None);

   /** Returns the configured default root name. */
   UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables")
   static FName GetDefaultRootName();

   /** Returns all registered root names for dropdowns/console usage. */
   UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables")
   static TArray<FName> GetRegisteredRootNames();

    /**
     * Gets the hierarchy registry of all object variables.
     *
     * @return An array of all registered object variables.
     */
   UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables")
   static TArray<UGorgeousObjectVariable*> GetVariableHierarchyRegistry(FName RootName = NAME_None);

    /**
     * Gets the root variable registry.
     *
     * @return An array of root-level object variables.
     */
   UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables")
   static TArray<UGorgeousObjectVariable*> GetRootVariableRegistry(FName RootName = NAME_None);

   /** Returns true when the root entry is configured to handle parentless replicated variables (shared-root behavior). */
    static bool IsSharedNetworkingRoot(FName RootName);

   /** Returns true when the root entry enforces networking for every registered variable. */
    static bool IsEnforcedNetworkingRoot(FName RootName);

    /**
     * Removes a variable from the registry.
     *
     * @param VariableToRemove The variable to remove.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables")
    static void RemoveVariableFromRegistry(UGorgeousObjectVariable* VariableToRemove);

    /**
     * Checks if a given object variable is already registered with the registry
     *
     * @param Variable The variable to check for existance in the registry
     * @return True if the variable is contained either in the root registry ot the registry of any other variable, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables")
   static bool IsVariableRegistered(UGorgeousObjectVariable* Variable);

   /** Sets the default orphan resolution policy for future registry operations. */
   UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables")
   static void SetDefaultOrphanResolution(EGorgeousObjectVariableOrphanResolution InResolution);

   /** Returns the default orphan resolution policy. */
   UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables")
   static EGorgeousObjectVariableOrphanResolution GetDefaultOrphanResolution();

   /** Fast lookup helper by identifier. */
   UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables")
   static UGorgeousObjectVariable* FindVariableByIdentifier(FGuid Identifier);

   /** Lookup helper that resolves a variable by its registered display name. */
   UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables")
   static UGorgeousObjectVariable* FindVariableByDisplayName(FName InDisplayName);

   template<typename VariableType>
   static VariableType* FindVariableByDisplayNameTyped(const FName InDisplayName)
   {
      return Cast<VariableType>(FindVariableByDisplayName(InDisplayName));
   }

   /** Claims ownership of the specified root so replication has a stable authority context. */
   UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
   static FGorgeousRootRegistryOwnerHandle ClaimRootRegistryOwnership(FName RootName, const FString& StableIdentifier, UObject* OwningContext);

   /** Attempts to reattach a returning owner using the identifier that was recorded previously. */
   UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
   static bool RestoreRootRegistryOwnership(const FString& StableIdentifier, UObject* ReturningOwner, FGorgeousRootRegistryOwnerHandle& OutHandle);

   /** Releases a previously claimed ownership handle and optionally promotes a fallback owner. */
   UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
   static void ReleaseRootRegistryOwnership(const FGorgeousRootRegistryOwnerHandle& Handle, UObject* CachedOwner = nullptr);

   /** Ensures the specified root keeps a valid network owner by assigning the provided fallback. */
   UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Networking")
   static void PromoteRootRegistryOwner(FName RootName, UObject* CachedOwner);

   /** Enables/disables creation of new root instances (used to block late teardown re-entry). */
   static void SetRootCreationAllowed(bool bAllowed);

   /** Returns whether new root instances are currently allowed to be created. */
   static bool IsRootCreationAllowed();

	/** Resolves an explicit root name against the developer settings to produce a canonical entry key. */
	static FName ResolveRootName(FName RequestedRootName);

    /**
     * Cleans up the registry, optionally performing a full cleanup.
     *
     * @param bFullCleanup Whether to perform a full cleanup.
     */
    static void CleanupRegistry(bool bFullCleanup = false);

    /**
     * Removes all registry entries whose owning world matches DyingWorld from every root
     * object-variable registry.  This must be called before GC sweeps the dying world so
     * the AddToRoot'd root OV no longer holds a UPROPERTY reference chain that reaches
     * the world object, which would otherwise block PIE world collection and trigger the
     * "Object from PIE level still referenced" hard check in EndPlayMap.
     *
     * @param DyingWorld     The world that is about to be garbage-collected.
     * @param bSessionEnded  true when the game session is ending (PIE close, game exit)
     *                      , performs a full purge including root OVs outered to the
     *                       GameInstance and the GI itself.  false during a level switch
     *                      , only purges non-persistent OVs directly outered to the
     *                       dying world, preserving persistent root OVs and the GI.
     */
    static void PurgeWorldOwnedRegistryEntries(UWorld* DyingWorld, bool bSessionEnded);


    /**
     * Sets the value of a property with any type for an object variable identified by its unique identifier.
     *
     * This function is not intended to be called from C++, it should only be used in Blueprint.
     * For a C++ version if this function, check out the equivalent function in UGorgeousObjectVariable.
     *
     * @param Identifier The unique identifier of the object variable.
     * @param OptionalPropertyName The name of the property to set.
     * @param Value The value to set.
     *
     * @todo A dropdown that lists the available variables inside OptionalPropertyName with a corresponding TSubclassOf (meta specifier)
     * @todo dynamic pin for the variable type (right click, change to variable type => like the math operations) or automatically set the thunk type with the OptionalPropertyName or default value "Value"
     */
    UFUNCTION(BlueprintCallable, CustomThunk, Category = "Gorgeous Core|Gorgeous Object Variables", meta = (CustomStructureParam = "Value"))
    static void SetUniversalVariable(FGuid Identifier, FName OptionalPropertyName, const int32& Value);

    DECLARE_FUNCTION(execSetUniversalVariable);

    /**
     * Gets the value of a property with any type from an object variable identified by its unique identifier.
     *
     * This function is not intended to be called from C++, it should only be used in Blueprint.
     * For a C++ version if this function, check out the equivalent function in UGorgeousObjectVariable.
     *
     * @param Identifier The unique identifier of the object variable.
     * @param OptionalPropertyName The name of the property to get.
     * @param OutValue The output value.
     *
     * @todo A dropdown that lists the available variables inside OptionalPropertyName with a corresponding TSubclassOf (meta specifier)
     * @todo dynamic pin for the variable type (right click, change to variable type => like the math operations) or automatically set the thunk type with the OptionalPropertyName or default value "Value"
     */
    UFUNCTION(BlueprintPure, CustomThunk, Category = "Gorgeous Core|Gorgeous Object Variables", meta = (CustomStructureParam = "OutValue"))
    static void GetUniversalVariable(FGuid Identifier, FName OptionalPropertyName, int32& OutValue);

    DECLARE_FUNCTION(execGetUniversalVariable);

    /**
     * Registers a new object variable with the registry.
     *
     * @param NewObjectVariable The object variable to register.
     * @param RegistryKey       Optional explicit key; derived from DisplayName / GUID when NAME_None.
     */
    virtual void RegisterWithRegistry(UGorgeousObjectVariable* NewObjectVariable, FName RegistryKey = NAME_None) override;

   static FString ReserveDisplayName(UGorgeousObjectVariable* Variable, const FString& CandidateLabel);
   static void ReleaseDisplayName(UGorgeousObjectVariable* Variable);
   static void TrackRegisteredVariable(UGorgeousObjectVariable* Variable);

	/** Returns the key this root instance registered under. */
	FName GetRegisteredRootName() const { return RegisteredRootName; }

private:
   static void HandleOrphanedChildren(UGorgeousObjectVariable* RemovedParent, EGorgeousObjectVariableOrphanResolution Resolution);
   static UGorgeousRootObjectVariable* GetOrCreateRootInternal(FName RootName);

public:
   /** Named root instances keyed by the identifier configured in settings. */
   static TMap<FName, TObjectPtr<UGorgeousRootObjectVariable>> NamedRootInstances;

protected:
   // Name key this root was registered with (None for legacy fallback).
   UPROPERTY(VisibleInstanceOnly, Category = "Gorgeous Core|Gorgeous Object Variables")
   FName RegisteredRootName;

   static FName CachedDefaultRootName;

	static TMap<FGuid, TWeakObjectPtr<UGorgeousObjectVariable>> IdentifierLookup;
	static TMap<FName, FGuid> DisplayNameToIdentifier;
	static TMap<FGuid, FName> IdentifierToDisplayName;
	static EGorgeousObjectVariableOrphanResolution DefaultOrphanResolution;
};
