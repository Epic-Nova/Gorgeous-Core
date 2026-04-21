// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<-------------------------=== Engine Includes ===-------------------------->
#include "Factories/Factory.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousDataSchemaMappingFactory.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Factory used to create Data Schema Mapping assets directly from the Content Browser.
 */
UCLASS()
class UGorgeousDataSchemaMappingFactory : public UFactory
{
	GENERATED_BODY()

public:
	UGorgeousDataSchemaMappingFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual uint32 GetMenuCategories() const override;
	virtual FText GetDisplayName() const override;
	virtual bool ShouldShowInNewMenu() const override { return true; }
};
