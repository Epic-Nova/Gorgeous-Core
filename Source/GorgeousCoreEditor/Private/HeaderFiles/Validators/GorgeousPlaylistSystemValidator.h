// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|              Gorgeous Events - Events functionality provider              |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "EditorValidatorBase.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousPlaylistSystemValidator.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Validator for DataRegistry assets used in the Gorgeous Playlist System.
 */
UCLASS()
class UGorgeousPlaylistSystemValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

	// Constructor
	UGorgeousPlaylistSystemValidator();
	
	// Destructor
	~UGorgeousPlaylistSystemValidator();
	
	//<============================--- Overrides ---============================>
public:

	// Override of CanValidateAsset to specify that this validator only applies to UDataRegistry assets within 'Systems/Playlist/Data/AdvancedData'.
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	
	// Override of ValidateLoadedAsset to perform the actual validation logic on data registry assets.
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
	//<------------------------------------------------------------------------->
	
	//<=======================--- Blueprint Functions ---=======================>
	
	/// Hyperlink action handler — registers the correct directory in the DataRegistry settings.
	UFUNCTION()
	void HandleRegisterDirectoryHyperlink(const FString& Payload);
	
	//<------------------------------------------------------------------------->
	
	//<============================--- Variables ---============================>
private: 
	
	//<------------------------------------------------------------------------->
};
