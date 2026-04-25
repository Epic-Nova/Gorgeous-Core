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
#include "GorgeousCommonUIFoundationSystemValidator.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Validator for DataRegistry assets used in the Gorgeous CommonUIFoundation System.
 */
UCLASS()
class UGorgeousCommonUIFoundationSystemValidator : public UEditorValidatorBase
{
	GENERATED_BODY()
	
	// Constructor
	UGorgeousCommonUIFoundationSystemValidator();
	
	// Destructor
	~UGorgeousCommonUIFoundationSystemValidator();
	
	//<============================--- Overrides ---============================>
public:

	// Override of CanValidateAsset to specify that this validator only applies to UDataRegistry assets within 'Systems/CommonUIFoundation/Data/AdvancedData'.
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	
	// Override of ValidateLoadedAsset to perform the actual validation logic on data registry assets.
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
	
	static void ValidateViewportClient();
	//<------------------------------------------------------------------------->
	
	//<=======================--- Blueprint Functions ---=======================>
	
	/// Hyperlink action handler — registers the correct directory in the DataRegistry settings.
	UFUNCTION()
	void HandleRegisterDirectoryHyperlink(const FString& Payload);

	/// Hyperlink action handler — fixes the Game Viewport Client class in project settings.
	UFUNCTION()
	void HandleFixViewportClientHyperlink(const FString& Payload);

	/// Hyperlink condition handler — checks if the Game Viewport Client can be fixed.
	UFUNCTION()
	bool HandleCanFixViewportClientHyperlink(const FString& Payload);
	
	//<------------------------------------------------------------------------->
	
	//<============================--- Variables ---============================>
private: 
	
	//<------------------------------------------------------------------------->
};
