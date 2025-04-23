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
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousObjectVariableEnums.h"
//<-------------------------=== Engine Includes ===-------------------------->
#if WITH_EDITOR
#include "EdGraphSchema_K2.h"
#endif
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousObjectVariableStructures.generated.h"
//<-------------------------------------------------------------------------->

#if WITH_EDITORONLY_DATA

/**
 * ...
 *
 * ...
 *
 * @author Nils Bergemann
 */
USTRUCT(Blueprintable)
struct FObjectVariablePinConfiguration_S
{
	GENERATED_BODY()

	// Default constructor with safe initial values.
	FObjectVariablePinConfiguration_S()
		: PinCategory(UEdGraphSchema_K2::PC_Wildcard)
		  , PinSubCategory(NAME_None)
		  , PinSubCategoryObject(nullptr)
		  , ContainerType(EObjectVariableContainerType_E::ESingle)
		  , bIsReference(false)
		  , bCustomGraphTerminalType(false), TerminalCategory(NAME_None)
		  , TerminalSubCategory(NAME_None)
		  , TerminalSubCategoryObject(nullptr)
		  , bTerminalIsConst(false)
		  , bTerminalIsWeakPointer(false)
		  , bTerminalIsUObjectWrapper(false)
	{
	}

	// Parameterized constructor for easy setup.
	explicit FObjectVariablePinConfiguration_S(const FName& NewPinCategory, const FName& NewPinSubCategory, UObject* NewPinSubCategoryObject,
												const EObjectVariableContainerType_E& NewContainerType, const bool& bNewIsReference,
												const bool& bNewCustomGraphTerminalType = false, const FName& NewTerminalCategory = NAME_None, const FName& NewTerminalSubCategory = NAME_None, const TWeakObjectPtr<UObject>& NewTerminalSubCategoryObject = nullptr,
												const bool& bNewTerminalIsConst = false, const bool& bNewTerminalIsWeakPointer = false, const bool& bNewTerminalIsUObjectWrapper = false)
		: PinCategory(NewPinCategory)
		  , PinSubCategory(NewPinSubCategory)
		  , PinSubCategoryObject(NewPinSubCategoryObject)
		  , ContainerType(NewContainerType)
		  , bIsReference(bNewIsReference)
		  , bCustomGraphTerminalType(bNewCustomGraphTerminalType), TerminalCategory(NewTerminalCategory)
		  , TerminalSubCategory(NewTerminalSubCategory)
		  , TerminalSubCategoryObject(NewTerminalSubCategoryObject)
		  , bTerminalIsConst(bNewTerminalIsConst)
		  , bTerminalIsWeakPointer(bNewTerminalIsWeakPointer)
		  , bTerminalIsUObjectWrapper(bNewTerminalIsUObjectWrapper)
	{
	}

	/** Category */
	UPROPERTY(EditAnywhere)
	FName PinCategory;

	/** Sub-category */
	UPROPERTY(EditAnywhere)
	FName PinSubCategory;

	/** Sub-category object */
	UPROPERTY(EditAnywhere)
	UObject* PinSubCategoryObject;

	UPROPERTY(EditAnywhere)
	EObjectVariableContainerType_E ContainerType;

	UPROPERTY(EditAnywhere)
	bool bIsReference;


	UPROPERTY(EditAnywhere, AdvancedDisplay)
	bool bCustomGraphTerminalType;
	
	/** Category */
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	FName TerminalCategory;

	/** Sub-category */
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	FName TerminalSubCategory;

	/** Sub-category object */
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	TWeakObjectPtr<UObject> TerminalSubCategoryObject;

	/** Whether this pin is a immutable const value */
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	bool bTerminalIsConst;

	/** Whether this is a weak reference */
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	bool bTerminalIsWeakPointer;

	/** Whether this is a "wrapped" Unreal object ptr type (e.g. TSubclassOf<T> instead of UClass*) */
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	bool bTerminalIsUObjectWrapper;

	FEdGraphPinType GetMappedPinType() const
	{
		EPinContainerType UnrealContainerType;
		switch (ContainerType)
		{
			case EObjectVariableContainerType_E::ESingle:
				UnrealContainerType = EPinContainerType::None;
				break;
			case EObjectVariableContainerType_E::EArray:
				UnrealContainerType = EPinContainerType::Array;
				break;
			case EObjectVariableContainerType_E::EMap:
				UnrealContainerType = EPinContainerType::Map;
				break;
			case EObjectVariableContainerType_E::ESet:
				UnrealContainerType = EPinContainerType::Set;
				break;
			default: UnrealContainerType = EPinContainerType::None;
		}

		FEdGraphPinType PinType = FEdGraphPinType(PinCategory, PinSubCategory, PinSubCategoryObject, UnrealContainerType, bIsReference, FEdGraphTerminalType());
		
		FEdGraphTerminalType GraphTerminalType;
		
		if (bCustomGraphTerminalType)
		{
			GraphTerminalType.TerminalCategory = TerminalCategory;
			GraphTerminalType.TerminalSubCategory = TerminalSubCategory;
			GraphTerminalType.TerminalSubCategoryObject = TerminalSubCategoryObject;
			GraphTerminalType.bTerminalIsConst = bTerminalIsConst;
			GraphTerminalType.bTerminalIsWeakPointer = bTerminalIsWeakPointer;
			GraphTerminalType.bTerminalIsUObjectWrapper = bTerminalIsUObjectWrapper;
		}
		else
		{
			GraphTerminalType = FEdGraphTerminalType::FromPinType(PinType);
		}

		PinType.PinValueType = GraphTerminalType;

		return PinType;
	}
};

#endif WITH_EDITORONLY_DATA