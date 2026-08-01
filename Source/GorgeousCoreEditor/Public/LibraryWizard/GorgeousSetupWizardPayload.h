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
#include "UObject/NoExportTypes.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousSetupWizardPayload.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Base class for all Gorgeous Setup Wizard payload pages.
 * Each subclass represents a single 'Page' in the setup wizard.
 *
 * Provides lifecycle hooks to interact with the asset generation process.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class GORGEOUSCOREEDITOR_API UGorgeousSetupWizardPayload : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * The title shown at the top of the wizard page.
	 * If empty, the wizard will display the default page count.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gorgeous Setup Wizard")
	FString PayloadTitle;

	/**
	 * A detailed description or purpose of this configuration step.
	 * Shown as middle-aligned text at the top of the wizard window.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gorgeous Setup Wizard", meta = (MultiLine = true))
	FString PayloadDescription;

public:
	/**
	 * Called whenever a property is changed in the wizard OR when the user
	 * attempts to proceed to the next page.
	 *
	 * [UI BEHAVIOR]: If this returns False, the 'Next' or 'Finish' button in
	 * the wizard will be DISABLED.
	 *
	 * @param OutFailureReason  Human-readable message shown as a tooltip on the disabled button.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous Setup Wizard")
	bool PreCreate(FString& OutFailureReason);
	virtual bool PreCreate_Implementation(FString& OutFailureReason) { return true; }

	/**
	 * Called AFTER the target asset has been created and saved to disk.
	 *
	 * [UI BEHAVIOR]: If this returns False, a Message Dialogue Box will pop up
	 * with the OutFailureReason, and the Wizard will remain open.
	 *
	 * @param CreatedAsset      The newly created PrimaryDataAsset.
	 * @param OutFailureReason  The error message shown in the pop-up dialogue.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous Setup Wizard")
	bool PostCreate(UObject* CreatedAsset, FString& OutFailureReason);
	virtual bool PostCreate_Implementation(UObject* CreatedAsset, FString& OutFailureReason) { return true; }

	/**
	 * Utility function to mark the created asset as dirty, ensuring it is resaved
	 * after the wizard completes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Setup Wizard")
	void MarkAssetDirty(UObject* TargetAsset)
	{
		if (IsValid(TargetAsset))
		{
			TargetAsset->Modify();
		}
	}

	/**
	 * Returns true so that this object is never cooked into shipping builds.
	 */
	virtual bool IsEditorOnly() const override { return true; }
};