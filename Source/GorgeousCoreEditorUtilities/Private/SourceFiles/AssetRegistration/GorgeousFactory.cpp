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
#include "AssetRegistration/GorgeousFactory.h"

//<=============================--- Includes ---=============================>
//<-------------------------=== Engine Includes ===-------------------------->
#include "Kismet2/KismetEditorUtilities.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousFactory Implementation
//=============================================================================

UObject* UGorgeousFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FKismetEditorUtilities::CreateBlueprint(
		FactoryInfo.SupportedClass,
		InParent,
		InName,
		BPTYPE_Normal,
		FactoryInfo.BlueprintClass ? FactoryInfo.BlueprintClass.Get() : UBlueprint::StaticClass(),
		FactoryInfo.BlueprintGeneratedClass ? FactoryInfo.BlueprintGeneratedClass.Get() : UBlueprintGeneratedClass::StaticClass(),
		NAME_None
	);
}

void UGorgeousFactory::SetFactoryInformation(const FGorgeousFactoryInfo_S& NewFactoryInfo)
{
	FactoryInfo = NewFactoryInfo;
	SupportedClass = NewFactoryInfo.BlueprintClass ? NewFactoryInfo.BlueprintClass.Get() : UBlueprint::StaticClass();
	bEditAfterNew = NewFactoryInfo.bEditAfterNew;
	bEditorImport = NewFactoryInfo.bEditorImport;
	bCreateNew = NewFactoryInfo.bCreateNew;
	bText = NewFactoryInfo.bText;
}