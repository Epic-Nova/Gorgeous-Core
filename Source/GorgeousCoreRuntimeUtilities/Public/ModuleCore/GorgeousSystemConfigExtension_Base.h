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
#include "GorgeousSystemConfigExtension_Base.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Base abstract class for polymorphic data injection into System Configurations.
 *
 * Used by external implementations (like UI Adapters, Audio Adapters, or Net Sync Adapters)
 * to define their own data requirements (e.g., Widget Classes, Sound Bases).
 *
 * At setup wizard time, these extensions are instantiated inside the template and
 * natively exposed to the user via IDetailsView.
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable)
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousSystemConfigExtension_Base : public UObject
{
	GENERATED_BODY()

public:

	//<=====--- Variables ---=====>

	/** Human-readable label displayed in the Setup Wizard for this extension block. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extension|Identity")
	FText ExtensionDisplayName;

	/**
	 * The adapter pack key this extension belongs to.
	 * Used by the system runtime to route the extension back to its owning adapter.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extension|Identity")
	FName OwningAdapterKey;
	//<-------------------------->
};