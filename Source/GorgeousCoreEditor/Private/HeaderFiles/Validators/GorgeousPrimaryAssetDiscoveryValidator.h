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
#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousPrimaryAssetDiscoveryValidator.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Validator that ensures Gorgeous Primary Data Assets are correctly registered in the Asset Manager.
 * If missing, it provides automated "Fix" actions to add the necessary scan rules.
 */
UCLASS()
class GORGEOUSCOREEDITOR_API UGorgeousPrimaryAssetDiscoveryValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

public:
	UGorgeousPrimaryAssetDiscoveryValidator();

	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;

protected:
	/** Internal handler for the automated "Fix" hyperlink. */
	UFUNCTION()
	void HandleRegisterAssetType(const FString& Payload);

private:
	/** Checks if a specific primary asset type is registered in Asset Manager settings. */
	bool IsTypeRegistered(const FPrimaryAssetType& Type, UClass* BaseClass) const;
};