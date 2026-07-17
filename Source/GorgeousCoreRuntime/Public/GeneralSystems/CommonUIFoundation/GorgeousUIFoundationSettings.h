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
#include "Engine/DeveloperSettings.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousUIFoundationSettings.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGorgeousPrimaryGameLayout;
class UGorgeousInputBinding_DA;
//<------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous UIFoundation Settings
| Functional Name: UGorgeousUIFoundationSettings
| Parent Class: UDeveloperSettings
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Global settings for the Gorgeous UI Foundation.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(config = Game, defaultconfig,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSettings",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousUIFoundationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	// Initializes the configurable defaults for the UI Foundation.
	UGorgeousUIFoundationSettings(const FObjectInitializer& ObjectInitializer);


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

	// The Blueprint class automatically created as the primary layout for each local player.
	UPROPERTY(EditAnywhere, Config, Category = "Layout", meta = (MetaClass = "/Script/GorgeousCoreRuntime.GorgeousPrimaryGameLayout"))
	TSoftClassPtr<UGorgeousPrimaryGameLayout> DefaultLayoutClass;

	// Fallback input bindings used when a UI state does not provide its own.
	UPROPERTY(EditAnywhere, Config, Category = "Input")
	TSoftObjectPtr<UGorgeousInputBinding_DA> DefaultInputBindings;

	// Default theme to apply at startup if no other theme is active.
	UPROPERTY(EditAnywhere, Config, Category = "Visuals")
	TSoftObjectPtr<class UGorgeousUITheme_DA> DefaultTheme;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables


	//<============================--- Overrides ---============================>
	#pragma region Overrides
#if WITH_EDITOR
	public:

	// Returns the project settings category used by this settings object.
	virtual FName GetCategoryName() const override { return TEXT("Gorgeous Things"); }

	// Returns the project settings section identifier.
	virtual FName GetSectionName() const override { return TEXT("Core|UI Foundation"); }

	// Returns the display name shown for this settings section.
	virtual FText GetSectionText() const override { return NSLOCTEXT("GorgeousCoreUIFoundationSettings", "SectionName", "Core ↪ UI Foundation"); }

	// Returns the descriptive text shown for this settings section.
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("GorgeousCoreUIFoundationSettings", "SectionDescription", "Global settings for the Gorgeous UI Foundation, including default layout, theme, and input bindings."); }
#endif
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides
};