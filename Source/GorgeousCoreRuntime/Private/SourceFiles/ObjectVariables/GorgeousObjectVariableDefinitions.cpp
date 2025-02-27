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

#include "ObjectVariables/GorgeousObjectVariableDefinitions.h"

//=============================================================================
// GorgeousObjectVariable Definitions Implementations
//=============================================================================

UE_DECLARE_OBJECT_VARIABLE_DEFAULT_SINGLE_INTERFACE_IMPLEMENTATION(bool, Boolean, false)
UE_DECLARE_OBJECT_VARIABLE_DEFAULT_SINGLE_INTERFACE_IMPLEMENTATION(FRotator, Rotator, FRotator::ZeroRotator)
UE_DECLARE_OBJECT_VARIABLE_DEFAULT_SINGLE_INTERFACE_IMPLEMENTATION(FText, Text, FText::GetEmpty())

UE_DECLARE_OBJECT_VARIABLE_DEFAULT_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, bool, Boolean)
UE_DECLARE_OBJECT_VARIABLE_DEFAULT_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, FRotator, Rotator)
UE_DECLARE_OBJECT_VARIABLE_DEFAULT_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, FText, Text)

UE_DECLARE_OBJECT_VARIABLE_DEFAULT_GENERAL_INTERFACE_IMPLEMENTATION(UGorgeousObjectVariable*, ObjectVariable, nullptr)
UE_DECLARE_OBJECT_VARIABLE_DEFAULT_GENERAL_INTERFACE_IMPLEMENTATION(UObject*, ObjectObject, nullptr)
UE_DECLARE_OBJECT_VARIABLE_DEFAULT_GENERAL_INTERFACE_IMPLEMENTATION(UClass*, ObjectClass, nullptr)
UE_DECLARE_OBJECT_VARIABLE_DEFAULT_GENERAL_INTERFACE_IMPLEMENTATION(TSoftObjectPtr<UObject>, SoftObjectObject, TSoftObjectPtr<UObject>())
UE_DECLARE_OBJECT_VARIABLE_DEFAULT_GENERAL_INTERFACE_IMPLEMENTATION(TSoftClassPtr<UObject>, SoftObjectClass, TSoftClassPtr<UObject>())
UE_DECLARE_OBJECT_VARIABLE_DEFAULT_GENERAL_INTERFACE_IMPLEMENTATION(uint8, Byte, 0)
UE_DECLARE_OBJECT_VARIABLE_DEFAULT_GENERAL_INTERFACE_IMPLEMENTATION(double, Float, 0.0)
UE_DECLARE_OBJECT_VARIABLE_DEFAULT_GENERAL_INTERFACE_IMPLEMENTATION(int64, Integer64, 0)
UE_DECLARE_OBJECT_VARIABLE_DEFAULT_GENERAL_INTERFACE_IMPLEMENTATION(int32, Integer, 0)
UE_DECLARE_OBJECT_VARIABLE_DEFAULT_GENERAL_INTERFACE_IMPLEMENTATION(FName, Name, NAME_None)
UE_DECLARE_OBJECT_VARIABLE_DEFAULT_GENERAL_INTERFACE_IMPLEMENTATION(FString, String, "")
UE_DECLARE_OBJECT_VARIABLE_DEFAULT_GENERAL_INTERFACE_IMPLEMENTATION(FTransform, Transform, FTransform::Identity)
UE_DECLARE_OBJECT_VARIABLE_DEFAULT_GENERAL_INTERFACE_IMPLEMENTATION(FVector, Vector, FVector::ZeroVector)