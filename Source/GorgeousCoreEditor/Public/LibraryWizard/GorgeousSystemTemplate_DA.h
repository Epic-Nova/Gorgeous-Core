// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|         Gorgeous Core - Abstract System Template Configuration             |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
<=========================================================================*/

#pragma once

//<=====--- Includes ---=====>
//<----- Engine Includes ----->
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
//--- Miscellaneous Includes -->
#include "GorgeousSystemTemplate_DA.generated.h"

class UTexture2D;

//<-------------------------->

class UGorgeousSetupWizardPayload;

/**
 * Base data asset for any Editor System Setup Template.
 *
 * Used by SGorgeousLibraryView and SGorgeousSetupWizard to dynamically spin up
 * workflows for creating complex configuration objects (e.g., Inventory setups,
 * Skill Trees).
 *
 * Templates are editor-only assets — they define *how* to create runtime data,
 * but are never cooked into shipping builds.
 *
 * Wizard flow:
 *   1. User clicks a template tile in the Gorgeous Library.
 *   2. SGorgeousSetupWizard opens a window, instantiates ConfigurationPayloadClass,
 *      and lets the user fill it out via IDetailsView.
 *   3. User provides a save path + asset name, clicks "Finish".
 *   4. SGorgeousSetupWizard creates the asset of AssetClassToCreate via IAssetTools.
 *   5. ExecuteTemplateGeneration() is called on the new asset to wire up defaults.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class GORGEOUSCOREEDITOR_API UGorgeousSystemTemplate_DA : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	//<=====--- Overrides ---=====>

	/**
	 * Returns true so that UE ensures this asset is never cooked into shipping builds.
	 * All template types inherit this behaviour.
	 */
	virtual bool IsEditorOnly() const override { return true; }
	//<-------------------------->

	//<=====--- Variables ---=====>

	/** Human-readable title of this template shown in the Library tile. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Meta")
	FText TemplateTitle;

	/** Detailed description visible when hovering or selecting the template tile. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Meta", meta = (MultiLine = "true"))
	FText TemplateDescription;

	/** Texture to use as an icon in the library tile. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Meta")
	TObjectPtr<UTexture2D> TemplateIcon;

	/**
	 * The UPrimaryDataAsset-derived class that the Setup Wizard will physically
	 * create and save to disk when the user clicks "Finish".
	 *
	 * Example: UGorgeousInventoryConfig_DA::StaticClass()
	 *
	 * The wizard creates the asset first, then calls ExecuteTemplateGeneration()
	 * on it so derived templates can populate it with their default values.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Generation")
	TSubclassOf<UPrimaryDataAsset> AssetClassToCreate;

	/**
	 * Specialized Payload classes whose UPROPERTIES are rendered as wizard pages.
	 * 
	 * [AUTO-MAPPING]: If a payload property name and type match a property in the 
	 * generated Config asset, the Wizard will automatically copy the value 
	 * UNLESS PostCreate() is overridden in the payload class.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Generation")
	TArray<TSubclassOf<UGorgeousSetupWizardPayload>> ConfigurationPayloadClasses;
	//<-------------------------->

	//<=====--- BlueprintNativeEvent ---=====>

	/**
	 * Called by the wizard after the final asset has been created and saved.
	 * Override to populate the newly created asset with template-specific defaults.
	 *
	 * @param GeneratedAsset  The newly created and saved UPrimaryDataAsset instance.
	 * @param PayloadData     The instantiated payload objects containing user configuration.
	 * @return True if generation was successful and the wizard can close.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous Setup Wizard")
	bool ExecuteTemplateGeneration(UObject* GeneratedAsset, const TArray<UObject*>& PayloadData) const;
	virtual bool ExecuteTemplateGeneration_Implementation(UObject* GeneratedAsset, const TArray<UObject*>& PayloadData) const { return false; }
	//<-------------------------->
};
