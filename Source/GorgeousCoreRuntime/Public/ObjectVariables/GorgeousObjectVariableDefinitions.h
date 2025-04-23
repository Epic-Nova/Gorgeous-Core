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
#include "GorgeousObjectVariable.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousObjectVariableDefinitions.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Abstract base class for single object variables.
 *
 * @author Nils Bergemann
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API USingleObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
};

/**
 * Abstract base class for single object variables that hold object type values.
 *
 * @author Nils Bergemann
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API USingleObjectTypeObjectVariable : public USingleObjectVariable
{
    GENERATED_BODY()
};

/**
 * Object Single Object Variable. Holds a single UObject pointer.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Object Single Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UObject_SOTOV : public USingleObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(UObject*, Value, ObjectObject)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    UObject* Value;

    UObject_SOTOV()
    {
        Value = nullptr;
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Object, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::ESingle, false);
    }
};

/**
 * Class Single Object Variable. Holds a single UClass pointer.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Class Single Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UClass_SOTOV : public USingleObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(UClass*, Value, ObjectClass)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    UClass* Value;

    UClass_SOTOV()
    {
        Value = nullptr;
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Class, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::ESingle, false);
    }
};

/**
 * Soft Object Single Object Variable. Holds a single TSoftObjectPtr to a UObject.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Object Single Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftObject_SOTOV : public USingleObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(TSoftObjectPtr<UObject>, Value, SoftObjectObject)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSoftObjectPtr<UObject> Value;

    USoftObject_SOTOV()
    {
        Value = nullptr;
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_SoftObject, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::ESingle, false);
    }
};

/**
 * Soft Class Single Object Variable. Holds a single TSoftClassPtr to a UObject.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Class Single Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftClass_SOTOV : public USingleObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(TSoftClassPtr<UObject>, Value, SoftObjectClass)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSoftClassPtr<UObject> Value;

    USoftClass_SOTOV()
    {
        Value = nullptr;
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_SoftClass, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::ESingle, false);
    }
};

/**
 * Boolean Single Object Variable. Holds a single boolean value.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Boolean Single Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UBoolean_SOV  : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(bool, Value, Boolean)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    bool Value;

    UBoolean_SOV()
    {
        Value = false;
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr, EObjectVariableContainerType_E::ESingle, false);
    }
};

/**
 * Byte Single Object Variable. Holds a single byte value.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Byte Single Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UByte_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(uint8, Value, Byte)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    uint8 Value;

    UByte_SOV()
    {
        Value = 0;
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Byte, NAME_None, nullptr, EObjectVariableContainerType_E::ESingle, false);
    }
};

/**
 * Float Single Object Variable. Holds a single float value.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Float Single Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UFloat_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(float, Value, Float)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    float Value;

    UFloat_SOV()
    {
        Value = 0;
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Float, NAME_None, nullptr, EObjectVariableContainerType_E::ESingle, false);
    }
};

/**
 * Double Single Object Variable. Holds a single float value (double precision).
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Double Single Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UDouble_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(double, Value, Double)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    double Value;

    UDouble_SOV()
    {
        Value = 0;
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Double, NAME_None, nullptr, EObjectVariableContainerType_E::ESingle, false);
    }
};

/**
 * Integer64 Single Object Variable. Holds a single 64-bit integer value.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer64 Single Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger64_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(int64, Value, Integer64)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    int64 Value;

    UInteger64_SOV()
    {
        Value = 0;
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Int64, NAME_None, nullptr, EObjectVariableContainerType_E::ESingle, false);
    }
};

/**
 * Integer Single Object Variable. Holds a single 32-bit integer value.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer Single Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(int32, Value, Integer)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    int32 Value;

    UInteger_SOV()
    {
        Value = 0;
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Int, NAME_None, nullptr, EObjectVariableContainerType_E::ESingle, false);
    }
};

/**
 * Name Single Object Variable. Holds a single FName value.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Name Single Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UName_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FName, Value, Name)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    FName Value;

    UName_SOV()
    {
        Value = NAME_None;
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Name, NAME_None, nullptr, EObjectVariableContainerType_E::ESingle, false);
    }
};

/**
 * Rotator Single Object Variable. Holds a single FRotator value.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Rotator Single Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API URotator_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FRotator, Value, Rotator)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    FRotator Value;

    URotator_SOV()
    {
        Value = FRotator::ZeroRotator;
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FRotator>::Get(), EObjectVariableContainerType_E::ESingle, false);
    }
};

/**
 * String Single Object Variable. Holds a single FString value.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "String Single Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UString_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FString, Value, String)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    FString Value;

    UString_SOV()
    {
        Value = FString();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_String, NAME_None, nullptr, EObjectVariableContainerType_E::ESingle, false);
    }
};

/**
 * Text Single Object Variable. Holds a single FText value.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Text Single Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UText_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FText, Value, Text)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    FText Value;

    UText_SOV()
    {
        Value = FText::GetEmpty();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Text, NAME_None, nullptr, EObjectVariableContainerType_E::ESingle, false);
    }
};

/**
 * Transform Single Object Variable. Holds a single FTransform value.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Transform Single Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UTransform_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FTransform, Value, Transform)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    FTransform Value;

    UTransform_SOV()
    {
        Value = FTransform();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FTransform>::Get(), EObjectVariableContainerType_E::ESingle, false);
    }
};

/**
 * Vector Single Object Variable. Holds a single FVector value.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Vector Single Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UVector_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FVector, Value, Vector)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    FVector Value;

    UVector_SOV()
    {
        Value = FVector::Zero();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FVector>::Get(), EObjectVariableContainerType_E::ESingle, false);
    }
};


/**
 * Abstract base class for array object variables.
 *
 * @author Nils Bergemann
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API UArrayObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
};

/**
 * Abstract base class for array object variables that hold object type values.
 *
 * @author Nils Bergemann
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API UArrayObjectTypeObjectVariable : public UArrayObjectVariable
{
    GENERATED_BODY()
};

/**
 * Object Array Object Variable. Holds an array of UObject pointers.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Object Array Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UObject_AOTOV : public UArrayObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<UObject*>, Value, ObjectObject)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<UObject*> Value;

    UObject_AOTOV()
    {
        Value = TArray<UObject*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Object, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EArray, false);
    }
};

/**
 * Class Array Object Variable. Holds an array of UClass pointers.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Class Array Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UClass_AOTOV : public UArrayObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<UClass*>, Value, ObjectClass)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<UClass*> Value;

    UClass_AOTOV()
    {
        Value = TArray<UClass*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Class, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EArray, false);
    }
};

/**
 * Soft Object Array Object Variable. Holds an array of TSoftObjectPtr to UObjects.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Object Array Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftObject_AOTOV : public UArrayObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<TSoftObjectPtr<UObject>>, Value, SoftObjectObject)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<TSoftObjectPtr<UObject>> Value;

    USoftObject_AOTOV()
    {
        Value = TArray<TSoftObjectPtr<UObject>>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_SoftObject, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EArray, false);
    }
};

/**
 * Soft Class Array Object Variable. Holds an array of TSoftClassPtr to UObjects.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Class Array Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftClass_AOTOV : public UArrayObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<TSoftClassPtr<UObject>>, Value, SoftObjectClass)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<TSoftClassPtr<UObject>> Value;

    USoftClass_AOTOV()
    {
        Value = TArray<TSoftClassPtr<UObject>>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_SoftClass, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EArray, false);
    }
};

/**
 * Boolean Array Object Variable. Holds an array of boolean values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Boolean Array Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UBoolean_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array,  TArray<bool>, Value, Boolean)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<bool> Value;

    UBoolean_AOV()
    {
        Value = TArray<bool>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr, EObjectVariableContainerType_E::EArray, false);
    }
};

/**
 * Byte Array Object Variable. Holds an array of byte values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Byte Array Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UByte_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<uint8>, Value, Byte)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<uint8> Value;

    UByte_AOV()
    {
        Value = TArray<uint8>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Byte, NAME_None, nullptr, EObjectVariableContainerType_E::EArray, false);
    }
};

/**
 * Float Array Object Variable. Holds an array of float values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Float Array Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UFloat_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<float>, Value, Float)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<float> Value;

    UFloat_AOV()
    {
        Value = TArray<float>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Float, NAME_None, nullptr, EObjectVariableContainerType_E::EArray, false);
    }
};

/**
 * Double Array Object Variable. Holds an array of float values (double precision).
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Double Array Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UDouble_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array,  TArray<double>, Value, Double)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<double> Value;

    UDouble_AOV()
    {
        Value = TArray<double>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Double, NAME_None, nullptr, EObjectVariableContainerType_E::EArray, false);
    }
};

/**
 * Integer64 Array Object Variable. Holds an array of 64-bit integer values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer64 Array Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger64_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<int64>, Value, Integer64)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<int64> Value;

    UInteger64_AOV()
    {
        Value = TArray<int64>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Int64, NAME_None, nullptr, EObjectVariableContainerType_E::EArray, false);
    }
};

/**
 * Integer Array Object Variable. Holds an array of 32-bit integer values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer Array Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<int32>, Value, Integer)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<int32> Value;

    UInteger_AOV()
    {
        Value = TArray<int32>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Int, NAME_None, nullptr, EObjectVariableContainerType_E::EArray, false);
    }
};

/**
 * Name Array Object Variable. Holds an array of FName values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Name Array Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UName_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FName>, Value, Name)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<FName> Value;
    
    UName_AOV()
    {
        Value = TArray<FName>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Name, NAME_None, nullptr, EObjectVariableContainerType_E::EArray, false);
    }
};

/**
 * Rotator Array Object Variable. Holds an array of FRotator values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Rotator Array Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API URotator_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FRotator>, Value, Rotator)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<FRotator> Value;

    URotator_AOV()
    {
        Value = TArray<FRotator>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FRotator>::Get(), EObjectVariableContainerType_E::EArray, false);
    }
};

/**
 * String Array Object Variable. Holds an array of FString values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "String Array Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UString_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FString>, Value, String)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<FString> Value;

    UString_AOV()
    {
        Value = TArray<FString>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_String, NAME_None, nullptr, EObjectVariableContainerType_E::EArray, false);
    }
};

/**
 * Text Array Object Variable. Holds an array of FText values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Text Array Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UText_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FText>, Value, Text)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<FText> Value;

    UText_AOV()
    {
        Value = TArray<FText>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Text, NAME_None, nullptr, EObjectVariableContainerType_E::EArray, false);
    }
};

/**
 * Transform Array Object Variable. Holds an array of FTransform values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Transform Array Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UTransform_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FTransform>, Value, Transform)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<FTransform> Value;

    UTransform_AOV()
    {
        Value = TArray<FTransform>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FTransform>::Get(), EObjectVariableContainerType_E::EArray, false);
    }
};

/**
 * Vector Array Object Variable. Holds an array of FVector values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Vector Array Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UVector_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FVector>, Value, Vector)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<FVector> Value;

    UVector_AOV()
    {
        Value = TArray<FVector>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FVector>::Get(), EObjectVariableContainerType_E::EArray, false);
    }
};


/**
 * Abstract base class for map object variables.
 *
 * @author Nils Bergemann
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API UMapObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
};

/**
 * Abstract base class for map object variables that hold object type keys.
 *
 * @author Nils Bergemann
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API UMapObjectTypeObjectVariable : public UMapObjectVariable
{
    GENERATED_BODY()
};

/**
 * Object Map Object Variable. Holds a map where the key is a UObject pointer.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Object Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UObject_MOTOV : public UMapObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(UObject*, Value, ObjectObject)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<UObject*, UGorgeousObjectVariable*> Value;

    UObject_MOTOV()
    {
        Value = TMap<UObject*, UGorgeousObjectVariable*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Object, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * Class Map Object Variable. Holds a map where the key is a UClass pointer.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Class Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UClass_MOTOV : public UMapObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(UClass*, Value, ObjectClass)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<UClass*, UGorgeousObjectVariable*> Value;

    UClass_MOTOV()
    {
        Value = TMap<UClass*, UGorgeousObjectVariable*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Class, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * Soft Object Map Object Variable. Holds a map where the key is a TSoftObjectPtr to a UObject.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Object Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftObject_MOTOV : public UMapObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(TSoftObjectPtr<UObject>, Value, SoftObjectObject)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<TSoftObjectPtr<UObject>, UGorgeousObjectVariable*> Value;

    USoftObject_MOTOV()
    {
        Value = TMap<TSoftObjectPtr<UObject>, UGorgeousObjectVariable*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_SoftObject, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass(), false, true);
    }
};

/**
 * Soft Class Map Object Variable. Holds a map where the key is a TSoftClassPtr to a UObject.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Class Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftClass_MOTOV : public UMapObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(TSoftClassPtr<UObject>, Value, SoftObjectClass)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<TSoftClassPtr<UObject>, UGorgeousObjectVariable*> Value;
    
    USoftClass_MOTOV()
    {
        Value = TMap<TSoftClassPtr<UObject>, UGorgeousObjectVariable*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_SoftClass, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass(), false, true);
    }
};

/**
 * Byte Map Object Variable. Holds a map where the key is a byte.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Byte Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UByte_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(uint8, Value, Byte)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<uint8, UGorgeousObjectVariable*> Value;
    
    UByte_MOV()
    {
        Value = TMap<uint8, UGorgeousObjectVariable*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Byte, NAME_None, nullptr, EObjectVariableContainerType_E::EMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * Float Map Object Variable. Holds a map where the key is a float.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Float Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UFloat_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(float, Value, Float)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<float, UGorgeousObjectVariable*> Value;

    UFloat_MOV()
    {
        Value = TMap<float, UGorgeousObjectVariable*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Float, NAME_None, nullptr, EObjectVariableContainerType_E::EMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * Double Map Object Variable. Holds a map where the key is a float (double precision).
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Double Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UDouble_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(double, Value, Double)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<double, UGorgeousObjectVariable*> Value;

    UDouble_MOV()
    {
        Value = TMap<double, UGorgeousObjectVariable*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Double, NAME_None, nullptr, EObjectVariableContainerType_E::EMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * Integer64 Map Object Variable. Holds a map where the key is a 64-bit integer.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer64 Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger64_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(int64, Value, Integer64)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<int64, UGorgeousObjectVariable*> Value;

    UInteger64_MOV()
    {
        Value = TMap<int64, UGorgeousObjectVariable*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Int64, NAME_None, nullptr, EObjectVariableContainerType_E::EMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * Integer Map Object Variable. Holds a map where the key is a 32-bit integer.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(int32, Value, Integer)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<int32, UGorgeousObjectVariable*> Value;

    UInteger_MOV()
    {
        Value = TMap<int32, UGorgeousObjectVariable*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Int, NAME_None, nullptr, EObjectVariableContainerType_E::EMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * Name Map Object Variable. Holds a map where the key is an FName.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Name Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UName_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(FName, Value, Name)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<FName, UGorgeousObjectVariable*> Value;

    UName_MOV()
    {
        Value = TMap<FName, UGorgeousObjectVariable*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Name, NAME_None, nullptr, EObjectVariableContainerType_E::EMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * String Map Object Variable. Holds a map where the key is an FString.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "String Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UString_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(FString, Value, String)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<FString, UGorgeousObjectVariable*> Value;

    UString_MOV()
    {
        Value = TMap<FString, UGorgeousObjectVariable*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_String, NAME_None, nullptr, EObjectVariableContainerType_E::EMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * Transform Map Object Variable. Holds a map where the key is an FTransform.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Transform Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UTransform_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(FTransform, Value, Transform)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<FTransform, UGorgeousObjectVariable*> Value;

    UTransform_MOV()
    {
        Value = TMap<FTransform, UGorgeousObjectVariable*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FTransform>::Get(), EObjectVariableContainerType_E::EMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * Vector Map Object Variable. Holds a map where the key is an FVector.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Vector Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UVector_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(FVector, Value, Vector)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<FVector, UGorgeousObjectVariable*> Value;

    UVector_MOV()
    {
        Value = TMap<FVector, UGorgeousObjectVariable*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FVector>::Get(), EObjectVariableContainerType_E::EMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};


/**
 * Abstract base class for set object variables.
 *
 * @author Nils Bergemann
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API USetObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
};

/**
 * Abstract base class for set object variables that hold object type values.
 *
 * @author Nils Bergemann
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API USetObjectTypeObjectVariable : public USetObjectVariable
{
    GENERATED_BODY()
};

/**
 * Object Set Object Variable. Holds a set of UObject pointers.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Object Set Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UObject_STOTOV : public USetObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<UObject*>, Value, ObjectObject)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<UObject*> Value;

    UObject_STOTOV()
    {
        Value = TSet<UObject*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Object, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::ESet, false);
    }
};

/**
 * Class Set Object Variable. Holds a set of UClass pointers.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Class Set Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UClass_STOTOV : public USetObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<UClass*>, Value, ObjectClass)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<UClass*> Value;

    UClass_STOTOV()
    {
        Value = TSet<UClass*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Class, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::ESet, false);
    }
};

/**
 * Soft Object Set Object Variable. Holds a set of TSoftObjectPtr to UObjects.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Object Set Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftObject_STOTOV : public USetObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<TSoftObjectPtr<UObject>>, Value, SoftObjectObject)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<TSoftObjectPtr<UObject>> Value;

    USoftObject_STOTOV()
    {
        Value = TSet<TSoftObjectPtr<UObject>>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_SoftObject, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::ESet, false);
    }
};

/**
 * Soft Class Set Object Variable. Holds a set of TSoftClassPtr to UObjects.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Class Set Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftClass_STOTOV : public USetObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<TSoftClassPtr<UObject>>, Value, SoftObjectClass)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<TSoftClassPtr<UObject>> Value;

    USoftClass_STOTOV()
    {
        Value = TSet<TSoftClassPtr<UObject>>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_SoftClass, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::ESet, false);
    }
};


/**
 * Byte Set Object Variable. Holds a set of byte values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Byte Set Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UByte_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<uint8>, Value, Byte)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<uint8> Value;

    UByte_STOV()
    {
        Value = TSet<uint8>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Byte, NAME_None, nullptr, EObjectVariableContainerType_E::ESet, false);
    }
};

/**
 * Float Set Object Variable. Holds a set of float values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Float Set Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UFloat_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<float>, Value, Float)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<float> Value;

    UFloat_STOV()
    {
        Value = TSet<float>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Float, NAME_None, nullptr, EObjectVariableContainerType_E::ESet, false);
    }
};

/**
 * Double Set Object Variable. Holds a set of float values (double precision).
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Double Set Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UDouble_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<double>, Value, Double)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<double> Value;

    UDouble_STOV()
    {
        Value = TSet<double>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Double, NAME_None, nullptr, EObjectVariableContainerType_E::ESet, false);
    }
};

/**
 * Integer64 Set Object Variable. Holds a set of 64-bit integer values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer64 Set Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger64_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<int64>, Value, Integer64)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<int64> Value;

    UInteger64_STOV()
    {
        Value = TSet<int64>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Int64, NAME_None, nullptr, EObjectVariableContainerType_E::ESet, false);
    }
};

/**
 * Integer Set Object Variable. Holds a set of 32-bit integer values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer Set Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<int32>, Value, Integer)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<int32> Value;

    UInteger_STOV()
    {
        Value = TSet<int32>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Int, NAME_None, nullptr, EObjectVariableContainerType_E::ESet, false);
    }
};

/**
 * Name Set Object Variable. Holds a set of FName values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Name Set Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UName_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<FName>, Value, Name)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<FName> Value;

    UName_STOV()
    {
        Value = TSet<FName>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Name, NAME_None, nullptr, EObjectVariableContainerType_E::ESet, false);
    }
};

/**
 * String Set Object Variable. Holds a set of FString values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "String Set Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UString_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<FString>, Value, String)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<FString> Value;

    UString_STOV()
    {
        Value = TSet<FString>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_String, NAME_None, nullptr, EObjectVariableContainerType_E::ESet, false);
    }
};

/**
 * Transform Set Object Variable. Holds a set of FTransform values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Transform Set Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UTransform_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<FTransform>, Value, Transform)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<FTransform> Value;

    UTransform_STOV()
    {
        Value = TSet<FTransform>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FTransform>::Get(), EObjectVariableContainerType_E::ESet, false);
    }
};

/**
 * Vector Set Object Variable. Holds a set of FVector values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Vector Set Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UVector_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<FVector>, Value, Vector)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<FVector> Value;

    UVector_STOV()
    {
        Value = TSet<FVector>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FVector>::Get(), EObjectVariableContainerType_E::ESet, false);
    }
};