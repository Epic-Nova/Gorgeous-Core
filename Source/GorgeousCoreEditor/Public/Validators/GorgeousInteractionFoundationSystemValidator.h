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
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousInteractionFoundationSystemValidator.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Validator for Gorgeous systems and assets.
 * Checks for required project settings and data integrity.
 *
 * [MIGRATION NOTE]: Moved from GorgeousInventory to GorgeousCore as this
 * provides baseline project-wide validation for interaction-enabled systems.
 */
UCLASS()
class GORGEOUSCOREEDITOR_API UGorgeousInteractionFoundationSystemValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

	UGorgeousInteractionFoundationSystemValidator();

public:
	//<============================--- Overrides ---============================>

	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;

	//<------------------------------------------------------------------------->

	//<=======================--- Hyperlink Handlers ---=======================>

	UFUNCTION()
	void HandleFixCollisionChannel(const FString& Payload);

	UFUNCTION()
	bool HandleCanFixCollisionChannel(const FString& Payload);

	//<------------------------------------------------------------------------->

    /** Static helper to run project-wide validation checks */
    static void ValidateProjectState(FDataValidationContext& InContext);
};