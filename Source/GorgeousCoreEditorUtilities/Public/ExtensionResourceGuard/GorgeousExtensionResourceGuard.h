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
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousExtensionResourceGuard.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Extension Resource Guard
| Functional Name: UGorgeousExtensionResourceGuard
| Parent Class: UDataAsset
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Base definition for Extension Resource Guards.
| 
| An Extension Resource Guard is a Data Asset that declares the plugin
| dependencies required by a specific extension content pack (e.g. Playlist,
| Team). When the guard asset is validated, a companion validator checks
| that all RequiredPlugins are enabled. If a plugin is missing, a
| hyperlink log message is dispatched through the Gorgeous Things logging
| system, offering the user the option to enable the plugin and restart
| the editor.
|
| Each system must have exactly ONE guard instance  - duplicates are flagged
| as validation errors.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(BlueprintType)
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousExtensionResourceGuard : public UDataAsset
{
	GENERATED_BODY()
	
	//<============================--- Overrides ---=============================>
	
	virtual bool IsEditorOnly() const override { return true; }
	
	//<-------------------------------------------------------------------------->

	
	//<====================--- UAT/UBT Exposed Variables ---====================>
public:

	/**
	 * Unique identifier for the system this guard protects.
	 * Must be unique across the entire project  - no two guard instances
	 * may share the same SystemIdentifier.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Extension Resource Guard")
	FName SystemIdentifier;

	/**
	 * Human-readable display name for the system.
	 * Used in log messages and validation output.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Extension Resource Guard")
	FText SystemDisplayName;

	/**
	 * Plugin names that MUST be enabled for this system's content to function.
	 * The validator will check each entry against IPluginManager and report
	 * any missing plugins with a hyperlink to enable them.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Extension Resource Guard", meta = (GetOptions = "GetPluginOptions"))
	TArray<FName> RequiredPlugins;

	/**
	 * Blueprint extension packs (referenced by system identifier) that must be
	 * present on disk for this guard to function.
	 * If the referenced pack cannot be found, the validator will toast a user
	 * message instructing the user to download it from the Gorgeous plugin
	 * website resource section.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Extension Resource Guard")
	TArray<FName> RequiredBlueprintPacks;

	/**
	 * The name of the plugin whose Content directory owns this system's
	 * content pack, e.g. "GorgeousCore".
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Extension Resource Guard", meta = (GetOptions = "GetPluginOptions"))
	FName OwningPluginName;

	/**
	 * When true, this guard protects a content pack on disk and
	 * ContentSubPath is used to verify its presence.
	 *
	 * When false, this is a content-less ("empty") guard  - useful for
	 * declaring plugin dependencies that only C++ classes rely on.
	 * ContentSubPath is ignored and the guard is always considered active.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Extension Resource Guard")
	bool bIsContentPackGuard = true;

	/**
	 * Relative sub-path within the owning plugin's Content directory,
	 * e.g. "Systems/Playlist".
	 *
	 * Resolved at runtime to <PluginBaseDir>/Content/<ContentSubPath>.
	 * If the directory does not exist on disk the entire guard is
	 * considered inactive  - validation, enforcement, and all warnings
	 * are skipped. This allows the content pack to be optional / removable.
	 *
	 * Only used when bIsContentPackGuard is true.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Extension Resource Guard", meta = (EditCondition = "bIsContentPackGuard", EditConditionHides))
	FString ContentSubPath;

	/**
	 * (Optional) Blueprint assets that are part of this system and should be
	 * automatically loaded and executed at editor startup if the guard is active.
	 * Only used for content packs that include editor utility blueprints.
	 */
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Extension Resource Guard")
	//TArray<TSoftClassPtr<class UEditorUtilityObject>> UtilityBlueprintsToExecute;
	
	//<------------------------------------------------------------------------->

	
	//<============================--- C++ Only ---=============================>

	/**
	 * Returns true if the content pack directory actually exists on disk.
	 * If OwningPluginName or ContentSubPath is empty, the guard is assumed
	 * to always be present (backward-compatible default).
	 *
	 * Implemented in GorgeousExtensionResourceGuard.cpp (requires Projects module).
	 */
	bool IsContentPresent() const;

	// ── Plugin Descriptor Management ──────────────────────────────────────

	/**
	 * Ensures that the owning plugin's .uplugin lists PluginName as an
	 * enabled dependency. If it is already present and enabled, this is
	 * a no-op. Preserves all other JSON fields in the .uplugin file.
	 *
	 * @return true on success, false if the descriptor could not be updated.
	 */
	static bool AddPluginDependencyToDescriptor(
		const FName& OwningPlugin, const FString& PluginName);

	/**
	 * Removes PluginName from the owning plugin's .uplugin dependency list.
	 * If it is not present, this is a no-op.
	 *
	 * @return true on success, false if the descriptor could not be updated.
	 */
	static bool RemovePluginDependencyFromDescriptor(
		const FName& OwningPlugin, const FString& PluginName);

	/**
	 * Runs on editor startup: reconciles plugin descriptors against all
	 * registered guards.
	 *
	 * - Removes stale dependencies when a content pack is no longer present.
	 * - Adds missing dependencies when an active guard's required plugins
	 *  are not yet listed in the owning plugin's .uplugin descriptor.
	 */
	static void ReconcilePluginDependencies();

	UFUNCTION()
	TArray<FString> GetPluginOptions() const;
	//<------------------------------------------------------------------------->
};