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
//<--------------------------=== Engine Includes ===------------------------->
#include "EditorValidatorBase.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousExtensionResourceGuardValidator.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Validator for UGorgeousExtensionResourceGuard assets.
 *
 * This validator performs the following checks on each guard asset:
 *  1. Verifies that all plugins listed in RequiredPlugins are currently enabled.
 *     If any required plugin is disabled, an error is logged with a hyperlink
 *     that allows the user to enable the plugin directly from the log message.
 *  2. Ensures that no two guard assets share the same SystemIdentifier, which
 *     would indicate a configuration error since each system should have exactly
 *     one guard instance.
 */
UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousExtensionResourceGuardValidator : public UEditorValidatorBase
{
	GENERATED_BODY()
	
	// Constructor
	UGorgeousExtensionResourceGuardValidator();
	
	// Destructor
	~UGorgeousExtensionResourceGuardValidator();

	//<============================--- Overrides ---============================>
public:

	// Override of CanValidateAsset to specify that this validator only applies to UGorgeousExtensionResourceGuard assets.
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	
	// Override of ValidateLoadedAsset to perform the actual validation logic on loaded guard assets.
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
	//<------------------------------------------------------------------------->

	//<====================--- UAT/UBT Exposed Variables ---====================>

	/**
	 * Hyperlink action handler — enables the specified plugin in the
	 * project descriptor and prompts the user to restart the editor.
	 *
	 * @param PluginName The name of the plugin to enable (passed as action payload).
	 */
	UFUNCTION()
	void HandleEnableRequiredPlugin(const FString& PluginName);
	//<------------------------------------------------------------------------->
};