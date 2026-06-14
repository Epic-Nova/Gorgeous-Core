// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<-------------------------=== Engine Includes ===-------------------------->
#include "Factories/Factory.h"
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousAssetRegistrationStructures.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousFactory.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Base class for all Gorgeous Core asset factories.
 *
 * @author Nils Bergemann
 * 
 * @todo: Make factory creation chained with function calls
 */
UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousFactory : public UFactory
{
	GENERATED_BODY()

public:

	//<============================--- Overrides ---=============================>

	/**
	 * Creates a new UObject instance based on the factory's configured class and flags.
	 *
	 * @param InClass The class of object to create.
	 * @param InParent The parent object for the new instance.
	 * @param InName The name to assign to the new object.
	 * @param Flags Creation flags to apply to the new object.
	 * @param Context Optional context object (unused in this implementation).
	 * @param Warn Feedback context for logging or warning purposes.
	 * @return A pointer to the newly created UObject.
	 */
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	//<-------------------------------------------------------------------------->

	/**
	 * Updates the factory information necessary for this class to execute its overwritten functions properly.
	 * 
	 * @param NewFactoryInfo The new factory information
	 */
	void SetFactoryInformation(const FGorgeousFactoryInfo_S& NewFactoryInfo);

	/**
	 * Weather this Factory should participate in the Content Browser context menu. 
	 *
	 * @return True if the handled asset should show up in the create context menu, false otherwise.
	 */
	virtual bool ShouldShowInNewMenu() const override;
	
	//<============================--- Variables ---============================>
private:

	/**
	 * The factory configuration used to determine the creation behavior of this factory instance.
	 * This structure must be set by subclasses using SetFactoryInformation to ensure correct behavior.
	 */
	FGorgeousFactoryInfo_S FactoryInfo;
	//<------------------------------------------------------------------------->
};