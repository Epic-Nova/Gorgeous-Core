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
#include "GorgeousObjectVariableEnums.h"
//<-------------------------------------------------------------------------->

#if WITH_EDITOR
#include "EdGraphSchema_K2.h"
#endif
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousObjectVariableStructures.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UGorgeousObjectVariable;
//<-------------------------------------------------->

#if WITH_EDITORONLY_DATA

/**
 * Configuration struct for defining the pin type of object variable nodes in Blueprints.
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

	// Category found in the UEdGraphSchema_K2::PC_* constants or custom categories defined by plugins.
	UPROPERTY(EditAnywhere)
	FName PinCategory;

	// Sub-category for more specific type information, such as the struct name for a struct pin or the enum name for an enum pin.
	UPROPERTY(EditAnywhere)
	FName PinSubCategory;

	// Sub-category object for even more specific type information, such as a reference to the UScriptStruct for a struct pin or the UEnum for an enum pin.
	UPROPERTY(EditAnywhere)
	UObject* PinSubCategoryObject;

	// Container type for the pin, indicating whether it's a single value, array, map, set, etc.
	UPROPERTY(EditAnywhere)
	EObjectVariableContainerType_E ContainerType;

	// Whether the pin is a reference (e.g., for output parameters) or a value pin.
	UPROPERTY(EditAnywhere)
	bool bIsReference;

	/**
	 * Whether to use a custom graph terminal type instead of deriving it from the pin type.
	 * This allows for more control over how the pin is treated in the graph, such as constness and weak pointer semantics.
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	bool bCustomGraphTerminalType;

	/**
	 * Custom graph terminal type information.
	 * This allows for specifying a different terminal type than what would be derived from the pin type, which can affect how the pin is treated in the graph (e.g., constness, weak pointer semantics).
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, meta = (EditCondition = "bCustomGraphTerminalType", EditConditionHides))
	FName TerminalCategory;

	// Sub-category for the custom graph terminal type, providing more specific type information if needed (e.g., struct name for a struct pin).
	UPROPERTY(EditAnywhere, AdvancedDisplay, meta = (EditCondition = "bCustomGraphTerminalType", EditConditionHides))
	FName TerminalSubCategory;

	// Sub-category object for the custom graph terminal type, allowing for even more specific type information (e.g., reference to UScriptStruct for a struct pin).
	UPROPERTY(EditAnywhere, AdvancedDisplay, meta = (EditCondition = "bCustomGraphTerminalType", EditConditionHides))
	TWeakObjectPtr<UObject> TerminalSubCategoryObject;

	// Whether the custom graph terminal type should be treated as const, which can affect how the pin is used in the graph (e.g., preventing modification of the value).
	UPROPERTY(EditAnywhere, AdvancedDisplay, meta = (EditCondition = "bCustomGraphTerminalType", EditConditionHides))
	bool bTerminalIsConst;

	// Whether the custom graph terminal type should be treated as a weak pointer, which can affect how the pin is used in the graph (e.g., allowing for null references without keeping the object alive).
	UPROPERTY(EditAnywhere, AdvancedDisplay, meta = (EditCondition = "bCustomGraphTerminalType", EditConditionHides))
	bool bTerminalIsWeakPointer;

	// Whether the custom graph terminal type should be treated as a UObject wrapper, which can affect how the pin is used in the graph (e.g., allowing for special handling of UObject references).
	UPROPERTY(EditAnywhere, AdvancedDisplay, meta = (EditCondition = "bCustomGraphTerminalType", EditConditionHides))
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
			case EObjectVariableContainerType_E::EQueue:
			case EObjectVariableContainerType_E::EStack:
			case EObjectVariableContainerType_E::EDeque:
				UnrealContainerType = EPinContainerType::Array;
				break;
			case EObjectVariableContainerType_E::EMultiMap:
				UnrealContainerType = EPinContainerType::Map;
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

#endif

/**
 * Multi-map value wrapper for object variables.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FObjectVariableMultiMapValue
{
	GENERATED_BODY()

	// Value array for the multi-map entry, allowing for multiple values to be associated with a single key.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Gorgeous Object Variables", meta = (ShowOnlyInnerProperties))
	TArray<UGorgeousObjectVariable*> Values;
};