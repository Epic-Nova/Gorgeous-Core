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
#include "ModuleCore/GorgeousCoreDeveloperSettings.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousDetailExtensionSettings.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Detail Extensions
| Functional Name: UGorgeousDetailExtensionSettings
| Parent Class: UGorgeousCoreDeveloperSettings
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Per-user settings that enable registered property customizations or
| restore the native editor for selected extension identifiers.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Config = EditorPerProjectUserSettings, DisplayName = "Detail Extensions", meta = (GorgeousSettingsPage = "General"))
class GORGEOUSCOREEDITOR_API UGorgeousDetailExtensionSettings : public UGorgeousCoreDeveloperSettings
{
	GENERATED_BODY()

#if WITH_EDITOR
	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Saves opt-out changes and refreshes open property editors.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& Event) override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides
#endif


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

	// Enables registered extensions for properties that select them through metadata.
	UPROPERTY(EditAnywhere, Config, Category = "Detail Extensions")
	bool bEnableDetailExtensions = true;

	// Extension identifiers that use the native property editor instead.
	UPROPERTY(EditAnywhere, Config, Category = "Detail Extensions")
	TSet<FName> DisabledExtensions;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};