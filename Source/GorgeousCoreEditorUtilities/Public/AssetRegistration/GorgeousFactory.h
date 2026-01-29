// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

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
 * This abstract factory class extends Unreal Engine's UFactory to provide a consistent,
 * metadata-driven system for creating custom Gorgeous Core assets in the editor.
 * It serves as the foundational layer for specialized asset factories, such as
 * UGorgeousBooleanConditionFactory, enabling standardized object creation workflows
 * and simplifying the process of registering and managing new asset types.
 * 
 * Key features include:
 * - A metadata structure (FGorgeousFactoryInfo_S) to define behavior for subclasses.
 * - An overridden FactoryCreateNew function for dynamic object instantiation.
 *
 * This class is intended to be extended and not used directly.
 * Each subclass should configure the FactoryInfos structure in its constructor.
 *
 * @author Nils Bergemann
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
	 * @param NewFactoryInfos The new factory information
	 */
	void SetFactoryInformation(const FGorgeousFactoryInfo_S& NewFactoryInfos);

private:

	/**
	 * The factory configuration used to determine the creation behavior of this factory instance.
	 * This structure must be set by subclasses using SetFactoryInformation to ensure correct behavior.
	 */
	FGorgeousFactoryInfo_S FactoryInfos;
};