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
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousObjectVariableRegistry_GIS.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class ULevel;
class UWorld;
//<------------------------------------------------------------->//<-------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Object Variable Registry
| Functional Name: UGorgeousObjectVariableRegistry_GIS
| Parent Class: UGameInstanceSubsystem
| Class Suffix: _GIS
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Game Instance Subsystem for managing the Gorgeous Object Variable
| Registry. Key features include: - Initialization and deinitialization of
| the registry. - Handling level removal events to clean up persistent
| variables.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(NotBlueprintable, NotBlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ObjectVariables/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ObjectVariables/GorgeousObjectVariableRegistry_GIS",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ObjectVariables/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousObjectVariableRegistry_GIS : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/**
	 * Initializes the subsystem.
	 *
	 * @param Collection The collection of subsystems.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Deinitializes the subsystem.
	 */
	virtual void Deinitialize() override;

private:

	/**
	 * Handles the event when a level is removed.
	 *
	 * @param Level The level that was removed.
	 * @param World The world the level belonged to.
	 */
	void OnLevelRemoved(ULevel* Level, UWorld* World);
};