// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousObjectVariable.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Containers/Queue.h"
#include "Containers/Deque.h"
#include "Containers/Map.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "NativeObjectVariableDefinitions.generated.h"
//<-------------------------------------------------------------------------->

//@TODO: Own TStack class
template <typename ElementType, typename AllocatorType = FDefaultAllocator>
using TStack = TArray<ElementType, AllocatorType>;

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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = ( AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
    FText Value;

    UText_SOV()
    {
        Value = FText::GetEmpty();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Text, NAME_None, nullptr, EObjectVariableContainerType_E::ESingle, false);
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
    TArray<FText> Value;

    UText_AOV()
    {
        Value = TArray<FText>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Text, NAME_None, nullptr, EObjectVariableContainerType_E::EArray, false);
    }
};

/**
 * Abstract base class for queue object variables.
 *
 * @author Nils Bergemann
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API UQueueObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, SaveGame, Category = "Queue")
    EObjectVariableQueueMode_E QueueMode;

protected:
    FORCEINLINE EObjectVariableQueueMode_E GetQueueMode() const { return QueueMode; }

    UQueueObjectVariable()
    {
        QueueMode = EObjectVariableQueueMode_E::ESpsc;
    }
};

/**
 * Abstract base class for queue object variables that hold object type values.
 *
 * @author Nils Bergemann
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API UQueueObjectTypeObjectVariable : public UQueueObjectVariable
{
    GENERATED_BODY()
};

/**
 * Object Queue Object Variable. Holds a queue of UObject pointers.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Object Queue Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UObject_QOTOV : public UQueueObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_INTERFACE(UObject*, ObjectObject)
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_STORAGE(UObject*)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Enqueue"))
    void EnqueueQueue(UObject* InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<UObject*> Values = GetObjectObjectQueueObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetObjectObjectQueueObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Dequeue"))
    bool DequeueQueue(UPARAM(ref) UObject*& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<UObject*> Values = GetObjectObjectQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetObjectObjectQueueObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Peek"))
    bool PeekQueue(UPARAM(ref) UObject*& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<UObject*> Values = GetObjectObjectQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Count"))
    int32 GetQueueCount(FName OptionalVariableName = NAME_None) const
    {
        return GetObjectObjectQueueObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Clear"))
    void ClearQueue(FName OptionalVariableName = NAME_None)
    {
        TArray<UObject*> Values;
        SetObjectObjectQueueObjectVariable(OptionalVariableName, Values);
    }

    UObject_QOTOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Object, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EQueue, false);
    }
};

/**
 * Class Queue Object Variable. Holds a queue of UClass pointers.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Class Queue Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UClass_QOTOV : public UQueueObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_INTERFACE(UClass*, ObjectClass)
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_STORAGE(UClass*)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Enqueue"))
    void EnqueueQueue(UClass* InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<UClass*> Values = GetObjectClassQueueObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetObjectClassQueueObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Dequeue"))
    bool DequeueQueue(UPARAM(ref) UClass*& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<UClass*> Values = GetObjectClassQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetObjectClassQueueObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Peek"))
    bool PeekQueue(UPARAM(ref) UClass*& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<UClass*> Values = GetObjectClassQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Count"))
    int32 GetQueueCount(FName OptionalVariableName = NAME_None) const
    {
        return GetObjectClassQueueObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Clear"))
    void ClearQueue(FName OptionalVariableName = NAME_None)
    {
        TArray<UClass*> Values;
        SetObjectClassQueueObjectVariable(OptionalVariableName, Values);
    }

    UClass_QOTOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Class, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EQueue, false);
    }
};

/**
 * Soft Object Queue Object Variable. Holds a queue of TSoftObjectPtr to UObjects.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Object Queue Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftObject_QOTOV : public UQueueObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_INTERFACE(TSoftObjectPtr<UObject>, SoftObjectObject)
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_STORAGE(TSoftObjectPtr<UObject>)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Enqueue"))
    void EnqueueQueue(TSoftObjectPtr<UObject> InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftObjectPtr<UObject>> Values = GetSoftObjectObjectQueueObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetSoftObjectObjectQueueObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Dequeue"))
    bool DequeueQueue(UPARAM(ref) TSoftObjectPtr<UObject>& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftObjectPtr<UObject>> Values = GetSoftObjectObjectQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetSoftObjectObjectQueueObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Peek"))
    bool PeekQueue(UPARAM(ref) TSoftObjectPtr<UObject>& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<TSoftObjectPtr<UObject>> Values = GetSoftObjectObjectQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Count"))
    int32 GetQueueCount(FName OptionalVariableName = NAME_None) const
    {
        return GetSoftObjectObjectQueueObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Clear"))
    void ClearQueue(FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftObjectPtr<UObject>> Values;
        SetSoftObjectObjectQueueObjectVariable(OptionalVariableName, Values);
    }

    USoftObject_QOTOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_SoftObject, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EQueue, false);
    }
};

/**
 * Soft Class Queue Object Variable. Holds a queue of TSoftClassPtr to UObjects.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Class Queue Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftClass_QOTOV : public UQueueObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_INTERFACE(TSoftClassPtr<UObject>, SoftObjectClass)
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_STORAGE(TSoftClassPtr<UObject>)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Enqueue"))
    void EnqueueQueue(TSoftClassPtr<UObject> InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftClassPtr<UObject>> Values = GetSoftObjectClassQueueObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetSoftObjectClassQueueObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Dequeue"))
    bool DequeueQueue(UPARAM(ref) TSoftClassPtr<UObject>& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftClassPtr<UObject>> Values = GetSoftObjectClassQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetSoftObjectClassQueueObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Peek"))
    bool PeekQueue(UPARAM(ref) TSoftClassPtr<UObject>& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<TSoftClassPtr<UObject>> Values = GetSoftObjectClassQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Count"))
    int32 GetQueueCount(FName OptionalVariableName = NAME_None) const
    {
        return GetSoftObjectClassQueueObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Clear"))
    void ClearQueue(FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftClassPtr<UObject>> Values;
        SetSoftObjectClassQueueObjectVariable(OptionalVariableName, Values);
    }

    USoftClass_QOTOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_SoftClass, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EQueue, false);
    }
};


/**
 * Boolean Queue Object Variable. Holds a queue of boolean values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Boolean Queue Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UBoolean_QOV : public UQueueObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_INTERFACE(bool, Boolean)
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_STORAGE(bool)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Enqueue"))
    void EnqueueQueue(bool InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<bool> Values = GetBooleanQueueObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetBooleanQueueObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Dequeue"))
    bool DequeueQueue(UPARAM(ref) bool& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<bool> Values = GetBooleanQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetBooleanQueueObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Peek"))
    bool PeekQueue(UPARAM(ref) bool& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<bool> Values = GetBooleanQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Count"))
    int32 GetQueueCount(FName OptionalVariableName = NAME_None) const
    {
        return GetBooleanQueueObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Clear"))
    void ClearQueue(FName OptionalVariableName = NAME_None)
    {
        TArray<bool> Values;
        SetBooleanQueueObjectVariable(OptionalVariableName, Values);
    }

    UBoolean_QOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr, EObjectVariableContainerType_E::EQueue, false);
    }
};

/**
 * Byte Queue Object Variable. Holds a queue of byte values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Byte Queue Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UByte_QOV : public UQueueObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_INTERFACE(uint8, Byte)
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_STORAGE(uint8)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Enqueue"))
    void EnqueueQueue(uint8 InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<uint8> Values = GetByteQueueObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetByteQueueObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Dequeue"))
    bool DequeueQueue(UPARAM(ref) uint8& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<uint8> Values = GetByteQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetByteQueueObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Peek"))
    bool PeekQueue(UPARAM(ref) uint8& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<uint8> Values = GetByteQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Count"))
    int32 GetQueueCount(FName OptionalVariableName = NAME_None) const
    {
        return GetByteQueueObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Clear"))
    void ClearQueue(FName OptionalVariableName = NAME_None)
    {
        TArray<uint8> Values;
        SetByteQueueObjectVariable(OptionalVariableName, Values);
    }

    UByte_QOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Byte, NAME_None, nullptr, EObjectVariableContainerType_E::EQueue, false);
    }
};

/**
 * Float Queue Object Variable. Holds a queue of float values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Float Queue Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UFloat_QOV : public UQueueObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_INTERFACE(float, Float)
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_STORAGE(float)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Enqueue"))
    void EnqueueQueue(float InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<float> Values = GetFloatQueueObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetFloatQueueObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Dequeue"))
    bool DequeueQueue(UPARAM(ref) float& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<float> Values = GetFloatQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetFloatQueueObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Peek"))
    bool PeekQueue(UPARAM(ref) float& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<float> Values = GetFloatQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Count"))
    int32 GetQueueCount(FName OptionalVariableName = NAME_None) const
    {
        return GetFloatQueueObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Clear"))
    void ClearQueue(FName OptionalVariableName = NAME_None)
    {
        TArray<float> Values;
        SetFloatQueueObjectVariable(OptionalVariableName, Values);
    }

    UFloat_QOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Float, NAME_None, nullptr, EObjectVariableContainerType_E::EQueue, false);
    }
};

/**
 * Double Queue Object Variable. Holds a queue of float values (double precision).
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Double Queue Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UDouble_QOV : public UQueueObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_INTERFACE(double, Double)
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_STORAGE(double)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Enqueue"))
    void EnqueueQueue(double InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<double> Values = GetDoubleQueueObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetDoubleQueueObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Dequeue"))
    bool DequeueQueue(UPARAM(ref) double& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<double> Values = GetDoubleQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetDoubleQueueObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Peek"))
    bool PeekQueue(UPARAM(ref) double& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<double> Values = GetDoubleQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Count"))
    int32 GetQueueCount(FName OptionalVariableName = NAME_None) const
    {
        return GetDoubleQueueObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Clear"))
    void ClearQueue(FName OptionalVariableName = NAME_None)
    {
        TArray<double> Values;
        SetDoubleQueueObjectVariable(OptionalVariableName, Values);
    }

    UDouble_QOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Double, NAME_None, nullptr, EObjectVariableContainerType_E::EQueue, false);
    }
};

/**
 * Integer64 Queue Object Variable. Holds a queue of 64-bit integer values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer64 Queue Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger64_QOV : public UQueueObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_INTERFACE(int64, Integer64)
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_STORAGE(int64)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Enqueue"))
    void EnqueueQueue(int64 InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<int64> Values = GetInteger64QueueObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetInteger64QueueObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Dequeue"))
    bool DequeueQueue(UPARAM(ref) int64& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<int64> Values = GetInteger64QueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetInteger64QueueObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Peek"))
    bool PeekQueue(UPARAM(ref) int64& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<int64> Values = GetInteger64QueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Count"))
    int32 GetQueueCount(FName OptionalVariableName = NAME_None) const
    {
        return GetInteger64QueueObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Clear"))
    void ClearQueue(FName OptionalVariableName = NAME_None)
    {
        TArray<int64> Values;
        SetInteger64QueueObjectVariable(OptionalVariableName, Values);
    }

    UInteger64_QOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Int64, NAME_None, nullptr, EObjectVariableContainerType_E::EQueue, false);
    }
};

/**
 * Integer Queue Object Variable. Holds a queue of 32-bit integer values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer Queue Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger_QOV : public UQueueObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_INTERFACE(int32, Integer)
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_STORAGE(int32)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Enqueue"))
    void EnqueueQueue(int32 InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<int32> Values = GetIntegerQueueObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetIntegerQueueObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Dequeue"))
    bool DequeueQueue(UPARAM(ref) int32& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<int32> Values = GetIntegerQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetIntegerQueueObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Peek"))
    bool PeekQueue(UPARAM(ref) int32& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<int32> Values = GetIntegerQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Count"))
    int32 GetQueueCount(FName OptionalVariableName = NAME_None) const
    {
        return GetIntegerQueueObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Clear"))
    void ClearQueue(FName OptionalVariableName = NAME_None)
    {
        TArray<int32> Values;
        SetIntegerQueueObjectVariable(OptionalVariableName, Values);
    }

    UInteger_QOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Int, NAME_None, nullptr, EObjectVariableContainerType_E::EQueue, false);
    }
};

/**
 * Name Queue Object Variable. Holds a queue of FName values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Name Queue Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UName_QOV : public UQueueObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_INTERFACE(FName, Name)
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_STORAGE(FName)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Enqueue"))
    void EnqueueQueue(FName InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FName> Values = GetNameQueueObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetNameQueueObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Dequeue"))
    bool DequeueQueue(UPARAM(ref) FName& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FName> Values = GetNameQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetNameQueueObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Peek"))
    bool PeekQueue(UPARAM(ref) FName& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<FName> Values = GetNameQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Count"))
    int32 GetQueueCount(FName OptionalVariableName = NAME_None) const
    {
        return GetNameQueueObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Clear"))
    void ClearQueue(FName OptionalVariableName = NAME_None)
    {
        TArray<FName> Values;
        SetNameQueueObjectVariable(OptionalVariableName, Values);
    }

    UName_QOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Name, NAME_None, nullptr, EObjectVariableContainerType_E::EQueue, false);
    }
};

/**
 * Rotator Queue Object Variable. Holds a queue of FRotator values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Rotator Queue Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API URotator_QOV : public UQueueObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_INTERFACE(FRotator, Rotator)
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_STORAGE(FRotator)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Enqueue"))
    void EnqueueQueue(FRotator InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FRotator> Values = GetRotatorQueueObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetRotatorQueueObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Dequeue"))
    bool DequeueQueue(UPARAM(ref) FRotator& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FRotator> Values = GetRotatorQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetRotatorQueueObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Peek"))
    bool PeekQueue(UPARAM(ref) FRotator& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<FRotator> Values = GetRotatorQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Count"))
    int32 GetQueueCount(FName OptionalVariableName = NAME_None) const
    {
        return GetRotatorQueueObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Clear"))
    void ClearQueue(FName OptionalVariableName = NAME_None)
    {
        TArray<FRotator> Values;
        SetRotatorQueueObjectVariable(OptionalVariableName, Values);
    }

    URotator_QOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FRotator>::Get(), EObjectVariableContainerType_E::EQueue, false);
    }
};

/**
 * String Queue Object Variable. Holds a queue of FString values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "String Queue Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UString_QOV : public UQueueObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_INTERFACE(FString, String)
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_STORAGE(FString)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Enqueue"))
    void EnqueueQueue(FString InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FString> Values = GetStringQueueObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetStringQueueObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Dequeue"))
    bool DequeueQueue(UPARAM(ref) FString& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FString> Values = GetStringQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetStringQueueObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Peek"))
    bool PeekQueue(UPARAM(ref) FString& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<FString> Values = GetStringQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Count"))
    int32 GetQueueCount(FName OptionalVariableName = NAME_None) const
    {
        return GetStringQueueObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Clear"))
    void ClearQueue(FName OptionalVariableName = NAME_None)
    {
        TArray<FString> Values;
        SetStringQueueObjectVariable(OptionalVariableName, Values);
    }

    UString_QOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_String, NAME_None, nullptr, EObjectVariableContainerType_E::EQueue, false);
    }
};

/**
 * Text Queue Object Variable. Holds a queue of FText values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Text Queue Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UText_QOV : public UQueueObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_INTERFACE(FText, Text)
    UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_STORAGE(FText)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Enqueue"))
    void EnqueueQueue(FText InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FText> Values = GetTextQueueObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetTextQueueObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Dequeue"))
    bool DequeueQueue(UPARAM(ref) FText& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FText> Values = GetTextQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetTextQueueObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Peek"))
    bool PeekQueue(UPARAM(ref) FText& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<FText> Values = GetTextQueueObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Count"))
    int32 GetQueueCount(FName OptionalVariableName = NAME_None) const
    {
        return GetTextQueueObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue", meta=(CompactNodeTitle="Clear"))
    void ClearQueue(FName OptionalVariableName = NAME_None)
    {
        TArray<FText> Values;
        SetTextQueueObjectVariable(OptionalVariableName, Values);
    }

    UText_QOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Text, NAME_None, nullptr, EObjectVariableContainerType_E::EQueue, false);
    }
};


/**
 * Abstract base class for stack object variables.
 *
 * @author Nils Bergemann
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API UStackObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
};

/**
 * Abstract base class for stack object variables that hold object type values.
 *
 * @author Nils Bergemann
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API UStackObjectTypeObjectVariable : public UStackObjectVariable
{
    GENERATED_BODY()
};

/**
 * Object Stack Object Variable. Holds a stack of UObject pointers.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Object Stack Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UObject_SKOTOV : public UStackObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_STACK_NATIVE_INTERFACE(UObject*, ObjectObject)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Push"))
    void PushStack(UObject* InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<UObject*> Values = GetObjectObjectStackObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetObjectObjectStackObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Pop"))
    bool PopStack(UPARAM(ref) UObject*& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<UObject*> Values = GetObjectObjectStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetObjectObjectStackObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Peek"))
    bool PeekStack(UPARAM(ref) UObject*& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<UObject*> Values = GetObjectObjectStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Count"))
    int32 GetStackCount(FName OptionalVariableName = NAME_None) const
    {
        return GetObjectObjectStackObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Clear"))
    void ClearStack(FName OptionalVariableName = NAME_None)
    {
        TArray<UObject*> Values;
        SetObjectObjectStackObjectVariable(OptionalVariableName, Values);
    }

    UObject_SKOTOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Object, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EStack, false);
    }
};

/**
 * Class Stack Object Variable. Holds a stack of UClass pointers.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Class Stack Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UClass_SKOTOV : public UStackObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_STACK_NATIVE_INTERFACE(UClass*, ObjectClass)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Push"))
    void PushStack(UClass* InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<UClass*> Values = GetObjectClassStackObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetObjectClassStackObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Pop"))
    bool PopStack(UPARAM(ref) UClass*& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<UClass*> Values = GetObjectClassStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetObjectClassStackObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Peek"))
    bool PeekStack(UPARAM(ref) UClass*& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<UClass*> Values = GetObjectClassStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Count"))
    int32 GetStackCount(FName OptionalVariableName = NAME_None) const
    {
        return GetObjectClassStackObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Clear"))
    void ClearStack(FName OptionalVariableName = NAME_None)
    {
        TArray<UClass*> Values;
        SetObjectClassStackObjectVariable(OptionalVariableName, Values);
    }

    UClass_SKOTOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Class, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EStack, false);
    }
};

/**
 * Soft Object Stack Object Variable. Holds a stack of TSoftObjectPtr to UObjects.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Object Stack Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftObject_SKOTOV : public UStackObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_STACK_NATIVE_INTERFACE(TSoftObjectPtr<UObject>, SoftObjectObject)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Push"))
    void PushStack(TSoftObjectPtr<UObject> InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftObjectPtr<UObject>> Values = GetSoftObjectObjectStackObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetSoftObjectObjectStackObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Pop"))
    bool PopStack(UPARAM(ref) TSoftObjectPtr<UObject>& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftObjectPtr<UObject>> Values = GetSoftObjectObjectStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetSoftObjectObjectStackObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Peek"))
    bool PeekStack(UPARAM(ref) TSoftObjectPtr<UObject>& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<TSoftObjectPtr<UObject>> Values = GetSoftObjectObjectStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Count"))
    int32 GetStackCount(FName OptionalVariableName = NAME_None) const
    {
        return GetSoftObjectObjectStackObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Clear"))
    void ClearStack(FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftObjectPtr<UObject>> Values;
        SetSoftObjectObjectStackObjectVariable(OptionalVariableName, Values);
    }

    USoftObject_SKOTOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_SoftObject, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EStack, false);
    }
};

/**
 * Soft Class Stack Object Variable. Holds a stack of TSoftClassPtr to UObjects.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Class Stack Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftClass_SKOTOV : public UStackObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_STACK_NATIVE_INTERFACE(TSoftClassPtr<UObject>, SoftObjectClass)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Push"))
    void PushStack(TSoftClassPtr<UObject> InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftClassPtr<UObject>> Values = GetSoftObjectClassStackObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetSoftObjectClassStackObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Pop"))
    bool PopStack(UPARAM(ref) TSoftClassPtr<UObject>& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftClassPtr<UObject>> Values = GetSoftObjectClassStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetSoftObjectClassStackObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Peek"))
    bool PeekStack(UPARAM(ref) TSoftClassPtr<UObject>& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<TSoftClassPtr<UObject>> Values = GetSoftObjectClassStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Count"))
    int32 GetStackCount(FName OptionalVariableName = NAME_None) const
    {
        return GetSoftObjectClassStackObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Clear"))
    void ClearStack(FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftClassPtr<UObject>> Values;
        SetSoftObjectClassStackObjectVariable(OptionalVariableName, Values);
    }

    USoftClass_SKOTOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_SoftClass, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EStack, false);
    }
};


    /**
     * Boolean Stack Object Variable. Holds a stack of boolean values.
     *
     * @author Nils Bergemann
     */
    UCLASS(DisplayName = "Boolean Stack Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
    class GORGEOUSCORERUNTIME_API UBoolean_SKOV : public UStackObjectVariable
    {
        GENERATED_BODY()
        UE_DEFINE_OBJECT_VARIABLE_STACK_NATIVE_INTERFACE(bool, Boolean)
    public:
        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Push"))
        void PushStack(bool InValue, FName OptionalVariableName = NAME_None)
        {
            TArray<bool> Values = GetBooleanStackObjectVariable(OptionalVariableName);
            Values.Add(InValue);
            SetBooleanStackObjectVariable(OptionalVariableName, Values);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Pop"))
        bool PopStack(UPARAM(ref) bool& OutValue, FName OptionalVariableName = NAME_None)
        {
            TArray<bool> Values = GetBooleanStackObjectVariable(OptionalVariableName);
            if (Values.Num() == 0)
            {
                return false;
            }
            OutValue = Values.Last();
            Values.RemoveAt(Values.Num() - 1);
            SetBooleanStackObjectVariable(OptionalVariableName, Values);
            return true;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Peek"))
        bool PeekStack(UPARAM(ref) bool& OutValue, FName OptionalVariableName = NAME_None) const
        {
            TArray<bool> Values = GetBooleanStackObjectVariable(OptionalVariableName);
            if (Values.Num() == 0)
            {
                return false;
            }
            OutValue = Values.Last();
            return true;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Count"))
        int32 GetStackCount(FName OptionalVariableName = NAME_None) const
        {
            return GetBooleanStackObjectVariable(OptionalVariableName).Num();
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Clear"))
        void ClearStack(FName OptionalVariableName = NAME_None)
        {
            TArray<bool> Values;
            SetBooleanStackObjectVariable(OptionalVariableName, Values);
        }

        UBoolean_SKOV()
        {
            PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr, EObjectVariableContainerType_E::EStack, false);
        }
    };


/**
 * Byte Stack Object Variable. Holds a stack of byte values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Byte Stack Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UByte_SKOV : public UStackObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_STACK_NATIVE_INTERFACE(uint8, Byte)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Push"))
    void PushStack(uint8 InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<uint8> Values = GetByteStackObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetByteStackObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Pop"))
    bool PopStack(UPARAM(ref) uint8& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<uint8> Values = GetByteStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetByteStackObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Peek"))
    bool PeekStack(UPARAM(ref) uint8& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<uint8> Values = GetByteStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Count"))
    int32 GetStackCount(FName OptionalVariableName = NAME_None) const
    {
        return GetByteStackObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Clear"))
    void ClearStack(FName OptionalVariableName = NAME_None)
    {
        TArray<uint8> Values;
        SetByteStackObjectVariable(OptionalVariableName, Values);
    }

    UByte_SKOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Byte, NAME_None, nullptr, EObjectVariableContainerType_E::EStack, false);
    }
};

/**
 * Float Stack Object Variable. Holds a stack of float values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Float Stack Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UFloat_SKOV : public UStackObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_STACK_NATIVE_INTERFACE(float, Float)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Push"))
    void PushStack(float InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<float> Values = GetFloatStackObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetFloatStackObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Pop"))
    bool PopStack(UPARAM(ref) float& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<float> Values = GetFloatStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetFloatStackObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Peek"))
    bool PeekStack(UPARAM(ref) float& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<float> Values = GetFloatStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Count"))
    int32 GetStackCount(FName OptionalVariableName = NAME_None) const
    {
        return GetFloatStackObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Clear"))
    void ClearStack(FName OptionalVariableName = NAME_None)
    {
        TArray<float> Values;
        SetFloatStackObjectVariable(OptionalVariableName, Values);
    }

    UFloat_SKOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Float, NAME_None, nullptr, EObjectVariableContainerType_E::EStack, false);
    }
};

/**
 * Double Stack Object Variable. Holds a stack of float values (double precision).
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Double Stack Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UDouble_SKOV : public UStackObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_STACK_NATIVE_INTERFACE(double, Double)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Push"))
    void PushStack(double InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<double> Values = GetDoubleStackObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetDoubleStackObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Pop"))
    bool PopStack(UPARAM(ref) double& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<double> Values = GetDoubleStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetDoubleStackObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Peek"))
    bool PeekStack(UPARAM(ref) double& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<double> Values = GetDoubleStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Count"))
    int32 GetStackCount(FName OptionalVariableName = NAME_None) const
    {
        return GetDoubleStackObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Clear"))
    void ClearStack(FName OptionalVariableName = NAME_None)
    {
        TArray<double> Values;
        SetDoubleStackObjectVariable(OptionalVariableName, Values);
    }

    UDouble_SKOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Double, NAME_None, nullptr, EObjectVariableContainerType_E::EStack, false);
    }
};

/**
 * Integer64 Stack Object Variable. Holds a stack of 64-bit integer values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer64 Stack Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger64_SKOV : public UStackObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_STACK_NATIVE_INTERFACE(int64, Integer64)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Push"))
    void PushStack(int64 InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<int64> Values = GetInteger64StackObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetInteger64StackObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Pop"))
    bool PopStack(UPARAM(ref) int64& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<int64> Values = GetInteger64StackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetInteger64StackObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Peek"))
    bool PeekStack(UPARAM(ref) int64& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<int64> Values = GetInteger64StackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Count"))
    int32 GetStackCount(FName OptionalVariableName = NAME_None) const
    {
        return GetInteger64StackObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Clear"))
    void ClearStack(FName OptionalVariableName = NAME_None)
    {
        TArray<int64> Values;
        SetInteger64StackObjectVariable(OptionalVariableName, Values);
    }

    UInteger64_SKOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Int64, NAME_None, nullptr, EObjectVariableContainerType_E::EStack, false);
    }
};

/**
 * Integer Stack Object Variable. Holds a stack of 32-bit integer values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer Stack Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger_SKOV : public UStackObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_STACK_NATIVE_INTERFACE(int32, Integer)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Push"))
    void PushStack(int32 InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<int32> Values = GetIntegerStackObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetIntegerStackObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Pop"))
    bool PopStack(UPARAM(ref) int32& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<int32> Values = GetIntegerStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetIntegerStackObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Peek"))
    bool PeekStack(UPARAM(ref) int32& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<int32> Values = GetIntegerStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Count"))
    int32 GetStackCount(FName OptionalVariableName = NAME_None) const
    {
        return GetIntegerStackObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Clear"))
    void ClearStack(FName OptionalVariableName = NAME_None)
    {
        TArray<int32> Values;
        SetIntegerStackObjectVariable(OptionalVariableName, Values);
    }

    UInteger_SKOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Int, NAME_None, nullptr, EObjectVariableContainerType_E::EStack, false);
    }
};

/**
 * Name Stack Object Variable. Holds a stack of FName values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Name Stack Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UName_SKOV : public UStackObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_STACK_NATIVE_INTERFACE(FName, Name)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Push"))
    void PushStack(FName InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FName> Values = GetNameStackObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetNameStackObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Pop"))
    bool PopStack(UPARAM(ref) FName& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FName> Values = GetNameStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetNameStackObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Peek"))
    bool PeekStack(UPARAM(ref) FName& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<FName> Values = GetNameStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Count"))
    int32 GetStackCount(FName OptionalVariableName = NAME_None) const
    {
        return GetNameStackObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Clear"))
    void ClearStack(FName OptionalVariableName = NAME_None)
    {
        TArray<FName> Values;
        SetNameStackObjectVariable(OptionalVariableName, Values);
    }

    UName_SKOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Name, NAME_None, nullptr, EObjectVariableContainerType_E::EStack, false);
    }
};

/**
 * Rotator Stack Object Variable. Holds a stack of FRotator values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Rotator Stack Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API URotator_SKOV : public UStackObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_STACK_NATIVE_INTERFACE(FRotator, Rotator)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Push"))
    void PushStack(FRotator InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FRotator> Values = GetRotatorStackObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetRotatorStackObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Pop"))
    bool PopStack(UPARAM(ref) FRotator& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FRotator> Values = GetRotatorStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetRotatorStackObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Peek"))
    bool PeekStack(UPARAM(ref) FRotator& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<FRotator> Values = GetRotatorStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Count"))
    int32 GetStackCount(FName OptionalVariableName = NAME_None) const
    {
        return GetRotatorStackObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Clear"))
    void ClearStack(FName OptionalVariableName = NAME_None)
    {
        TArray<FRotator> Values;
        SetRotatorStackObjectVariable(OptionalVariableName, Values);
    }

    URotator_SKOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FRotator>::Get(), EObjectVariableContainerType_E::EStack, false);
    }
};

/**
 * String Stack Object Variable. Holds a stack of FString values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "String Stack Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UString_SKOV : public UStackObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_STACK_NATIVE_INTERFACE(FString, String)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Push"))
    void PushStack(FString InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FString> Values = GetStringStackObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetStringStackObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Pop"))
    bool PopStack(UPARAM(ref) FString& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FString> Values = GetStringStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetStringStackObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Peek"))
    bool PeekStack(UPARAM(ref) FString& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<FString> Values = GetStringStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Count"))
    int32 GetStackCount(FName OptionalVariableName = NAME_None) const
    {
        return GetStringStackObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Clear"))
    void ClearStack(FName OptionalVariableName = NAME_None)
    {
        TArray<FString> Values;
        SetStringStackObjectVariable(OptionalVariableName, Values);
    }

    UString_SKOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_String, NAME_None, nullptr, EObjectVariableContainerType_E::EStack, false);
    }
};

/**
 * Text Stack Object Variable. Holds a stack of FText values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Text Stack Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UText_SKOV : public UStackObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_STACK_NATIVE_INTERFACE(FText, Text)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Push"))
    void PushStack(FText InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FText> Values = GetTextStackObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetTextStackObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Pop"))
    bool PopStack(UPARAM(ref) FText& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FText> Values = GetTextStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetTextStackObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Peek"))
    bool PeekStack(UPARAM(ref) FText& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<FText> Values = GetTextStackObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Count"))
    int32 GetStackCount(FName OptionalVariableName = NAME_None) const
    {
        return GetTextStackObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack", meta=(CompactNodeTitle="Clear"))
    void ClearStack(FName OptionalVariableName = NAME_None)
    {
        TArray<FText> Values;
        SetTextStackObjectVariable(OptionalVariableName, Values);
    }

    UText_SKOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Text, NAME_None, nullptr, EObjectVariableContainerType_E::EStack, false);
    }
};


/**
 * Abstract base class for deque object variables.
 *
 * @author Nils Bergemann
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API UDequeObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
};

/**
 * Abstract base class for deque object variables that hold object type values.
 *
 * @author Nils Bergemann
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API UDequeObjectTypeObjectVariable : public UDequeObjectVariable
{
    GENERATED_BODY()
};

/**
 * Object Deque Object Variable. Holds a deque of UObject pointers.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Object Deque Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UObject_DQOTOV : public UDequeObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_DEQUE_NATIVE_INTERFACE(UObject*, ObjectObject)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushBack"))
    void PushBackDeque(UObject* InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<UObject*> Values = GetObjectObjectDequeObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetObjectObjectDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushFront"))
    void PushFrontDeque(UObject* InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<UObject*> Values = GetObjectObjectDequeObjectVariable(OptionalVariableName);
        Values.Insert(InValue, 0);
        SetObjectObjectDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopFront"))
    bool PopFrontDeque(UPARAM(ref) UObject*& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<UObject*> Values = GetObjectObjectDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetObjectObjectDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopBack"))
    bool PopBackDeque(UPARAM(ref) UObject*& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<UObject*> Values = GetObjectObjectDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetObjectObjectDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekFront"))
    bool PeekFrontDeque(UPARAM(ref) UObject*& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<UObject*> Values = GetObjectObjectDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekBack"))
    bool PeekBackDeque(UPARAM(ref) UObject*& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<UObject*> Values = GetObjectObjectDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Count"))
    int32 GetDequeCount(FName OptionalVariableName = NAME_None) const
    {
        return GetObjectObjectDequeObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Clear"))
    void ClearDeque(FName OptionalVariableName = NAME_None)
    {
        TArray<UObject*> Values;
        SetObjectObjectDequeObjectVariable(OptionalVariableName, Values);
    }

    UObject_DQOTOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Object, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EDeque, false);
    }
};

/**
 * Class Deque Object Variable. Holds a deque of UClass pointers.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Class Deque Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UClass_DQOTOV : public UDequeObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_DEQUE_NATIVE_INTERFACE(UClass*, ObjectClass)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushBack"))
    void PushBackDeque(UClass* InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<UClass*> Values = GetObjectClassDequeObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetObjectClassDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushFront"))
    void PushFrontDeque(UClass* InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<UClass*> Values = GetObjectClassDequeObjectVariable(OptionalVariableName);
        Values.Insert(InValue, 0);
        SetObjectClassDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopFront"))
    bool PopFrontDeque(UPARAM(ref) UClass*& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<UClass*> Values = GetObjectClassDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetObjectClassDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopBack"))
    bool PopBackDeque(UPARAM(ref) UClass*& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<UClass*> Values = GetObjectClassDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetObjectClassDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekFront"))
    bool PeekFrontDeque(UPARAM(ref) UClass*& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<UClass*> Values = GetObjectClassDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekBack"))
    bool PeekBackDeque(UPARAM(ref) UClass*& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<UClass*> Values = GetObjectClassDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Count"))
    int32 GetDequeCount(FName OptionalVariableName = NAME_None) const
    {
        return GetObjectClassDequeObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Clear"))
    void ClearDeque(FName OptionalVariableName = NAME_None)
    {
        TArray<UClass*> Values;
        SetObjectClassDequeObjectVariable(OptionalVariableName, Values);
    }

    UClass_DQOTOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Class, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EDeque, false);
    }
};

/**
 * Soft Object Deque Object Variable. Holds a deque of TSoftObjectPtr to UObjects.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Object Deque Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftObject_DQOTOV : public UDequeObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_DEQUE_NATIVE_INTERFACE(TSoftObjectPtr<UObject>, SoftObjectObject)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushBack"))
    void PushBackDeque(TSoftObjectPtr<UObject> InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftObjectPtr<UObject>> Values = GetSoftObjectObjectDequeObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetSoftObjectObjectDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushFront"))
    void PushFrontDeque(TSoftObjectPtr<UObject> InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftObjectPtr<UObject>> Values = GetSoftObjectObjectDequeObjectVariable(OptionalVariableName);
        Values.Insert(InValue, 0);
        SetSoftObjectObjectDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopFront"))
    bool PopFrontDeque(UPARAM(ref) TSoftObjectPtr<UObject>& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftObjectPtr<UObject>> Values = GetSoftObjectObjectDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetSoftObjectObjectDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopBack"))
    bool PopBackDeque(UPARAM(ref) TSoftObjectPtr<UObject>& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftObjectPtr<UObject>> Values = GetSoftObjectObjectDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetSoftObjectObjectDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekFront"))
    bool PeekFrontDeque(UPARAM(ref) TSoftObjectPtr<UObject>& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<TSoftObjectPtr<UObject>> Values = GetSoftObjectObjectDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekBack"))
    bool PeekBackDeque(UPARAM(ref) TSoftObjectPtr<UObject>& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<TSoftObjectPtr<UObject>> Values = GetSoftObjectObjectDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Count"))
    int32 GetDequeCount(FName OptionalVariableName = NAME_None) const
    {
        return GetSoftObjectObjectDequeObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Clear"))
    void ClearDeque(FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftObjectPtr<UObject>> Values;
        SetSoftObjectObjectDequeObjectVariable(OptionalVariableName, Values);
    }

    USoftObject_DQOTOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_SoftObject, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EDeque, false);
    }
};

/**
 * Soft Class Deque Object Variable. Holds a deque of TSoftClassPtr to UObjects.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Class Deque Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftClass_DQOTOV : public UDequeObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_DEQUE_NATIVE_INTERFACE(TSoftClassPtr<UObject>, SoftObjectClass)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushBack"))
    void PushBackDeque(TSoftClassPtr<UObject> InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftClassPtr<UObject>> Values = GetSoftObjectClassDequeObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetSoftObjectClassDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushFront"))
    void PushFrontDeque(TSoftClassPtr<UObject> InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftClassPtr<UObject>> Values = GetSoftObjectClassDequeObjectVariable(OptionalVariableName);
        Values.Insert(InValue, 0);
        SetSoftObjectClassDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopFront"))
    bool PopFrontDeque(UPARAM(ref) TSoftClassPtr<UObject>& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftClassPtr<UObject>> Values = GetSoftObjectClassDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetSoftObjectClassDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopBack"))
    bool PopBackDeque(UPARAM(ref) TSoftClassPtr<UObject>& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftClassPtr<UObject>> Values = GetSoftObjectClassDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetSoftObjectClassDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekFront"))
    bool PeekFrontDeque(UPARAM(ref) TSoftClassPtr<UObject>& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<TSoftClassPtr<UObject>> Values = GetSoftObjectClassDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekBack"))
    bool PeekBackDeque(UPARAM(ref) TSoftClassPtr<UObject>& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<TSoftClassPtr<UObject>> Values = GetSoftObjectClassDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Count"))
    int32 GetDequeCount(FName OptionalVariableName = NAME_None) const
    {
        return GetSoftObjectClassDequeObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Clear"))
    void ClearDeque(FName OptionalVariableName = NAME_None)
    {
        TArray<TSoftClassPtr<UObject>> Values;
        SetSoftObjectClassDequeObjectVariable(OptionalVariableName, Values);
    }

    USoftClass_DQOTOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_SoftClass, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EDeque, false);
    }
};


    /**
     * Boolean Deque Object Variable. Holds a deque of boolean values.
     *
     * @author Nils Bergemann
     */
    UCLASS(DisplayName = "Boolean Deque Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
    class GORGEOUSCORERUNTIME_API UBoolean_DQOV : public UDequeObjectVariable
    {
        GENERATED_BODY()
        UE_DEFINE_OBJECT_VARIABLE_DEQUE_NATIVE_INTERFACE(bool, Boolean)
    public:
        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushBack"))
        void PushBackDeque(bool InValue, FName OptionalVariableName = NAME_None)
        {
            TArray<bool> Values = GetBooleanDequeObjectVariable(OptionalVariableName);
            Values.Add(InValue);
            SetBooleanDequeObjectVariable(OptionalVariableName, Values);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushFront"))
        void PushFrontDeque(bool InValue, FName OptionalVariableName = NAME_None)
        {
            TArray<bool> Values = GetBooleanDequeObjectVariable(OptionalVariableName);
            Values.Insert(InValue, 0);
            SetBooleanDequeObjectVariable(OptionalVariableName, Values);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopFront"))
        bool PopFrontDeque(UPARAM(ref) bool& OutValue, FName OptionalVariableName = NAME_None)
        {
            TArray<bool> Values = GetBooleanDequeObjectVariable(OptionalVariableName);
            if (Values.Num() == 0)
            {
                return false;
            }
            OutValue = Values[0];
            Values.RemoveAt(0);
            SetBooleanDequeObjectVariable(OptionalVariableName, Values);
            return true;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopBack"))
        bool PopBackDeque(UPARAM(ref) bool& OutValue, FName OptionalVariableName = NAME_None)
        {
            TArray<bool> Values = GetBooleanDequeObjectVariable(OptionalVariableName);
            if (Values.Num() == 0)
            {
                return false;
            }
            OutValue = Values.Last();
            Values.RemoveAt(Values.Num() - 1);
            SetBooleanDequeObjectVariable(OptionalVariableName, Values);
            return true;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekFront"))
        bool PeekFrontDeque(UPARAM(ref) bool& OutValue, FName OptionalVariableName = NAME_None) const
        {
            TArray<bool> Values = GetBooleanDequeObjectVariable(OptionalVariableName);
            if (Values.Num() == 0)
            {
                return false;
            }
            OutValue = Values[0];
            return true;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekBack"))
        bool PeekBackDeque(UPARAM(ref) bool& OutValue, FName OptionalVariableName = NAME_None) const
        {
            TArray<bool> Values = GetBooleanDequeObjectVariable(OptionalVariableName);
            if (Values.Num() == 0)
            {
                return false;
            }
            OutValue = Values.Last();
            return true;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Count"))
        int32 GetDequeCount(FName OptionalVariableName = NAME_None) const
        {
            return GetBooleanDequeObjectVariable(OptionalVariableName).Num();
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Clear"))
        void ClearDeque(FName OptionalVariableName = NAME_None)
        {
            TArray<bool> Values;
            SetBooleanDequeObjectVariable(OptionalVariableName, Values);
        }

        UBoolean_DQOV()
        {
            PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr, EObjectVariableContainerType_E::EDeque, false);
        }
    };


/**
 * Byte Deque Object Variable. Holds a deque of byte values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Byte Deque Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UByte_DQOV : public UDequeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_DEQUE_NATIVE_INTERFACE(uint8, Byte)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushBack"))
    void PushBackDeque(uint8 InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<uint8> Values = GetByteDequeObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetByteDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushFront"))
    void PushFrontDeque(uint8 InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<uint8> Values = GetByteDequeObjectVariable(OptionalVariableName);
        Values.Insert(InValue, 0);
        SetByteDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopFront"))
    bool PopFrontDeque(UPARAM(ref) uint8& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<uint8> Values = GetByteDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetByteDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopBack"))
    bool PopBackDeque(UPARAM(ref) uint8& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<uint8> Values = GetByteDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetByteDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekFront"))
    bool PeekFrontDeque(UPARAM(ref) uint8& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<uint8> Values = GetByteDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekBack"))
    bool PeekBackDeque(UPARAM(ref) uint8& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<uint8> Values = GetByteDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Count"))
    int32 GetDequeCount(FName OptionalVariableName = NAME_None) const
    {
        return GetByteDequeObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Clear"))
    void ClearDeque(FName OptionalVariableName = NAME_None)
    {
        TArray<uint8> Values;
        SetByteDequeObjectVariable(OptionalVariableName, Values);
    }

    UByte_DQOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Byte, NAME_None, nullptr, EObjectVariableContainerType_E::EDeque, false);
    }
};

/**
 * Float Deque Object Variable. Holds a deque of float values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Float Deque Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UFloat_DQOV : public UDequeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_DEQUE_NATIVE_INTERFACE(float, Float)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushBack"))
    void PushBackDeque(float InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<float> Values = GetFloatDequeObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetFloatDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushFront"))
    void PushFrontDeque(float InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<float> Values = GetFloatDequeObjectVariable(OptionalVariableName);
        Values.Insert(InValue, 0);
        SetFloatDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopFront"))
    bool PopFrontDeque(UPARAM(ref) float& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<float> Values = GetFloatDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetFloatDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopBack"))
    bool PopBackDeque(UPARAM(ref) float& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<float> Values = GetFloatDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetFloatDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekFront"))
    bool PeekFrontDeque(UPARAM(ref) float& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<float> Values = GetFloatDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekBack"))
    bool PeekBackDeque(UPARAM(ref) float& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<float> Values = GetFloatDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Count"))
    int32 GetDequeCount(FName OptionalVariableName = NAME_None) const
    {
        return GetFloatDequeObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Clear"))
    void ClearDeque(FName OptionalVariableName = NAME_None)
    {
        TArray<float> Values;
        SetFloatDequeObjectVariable(OptionalVariableName, Values);
    }

    UFloat_DQOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Float, NAME_None, nullptr, EObjectVariableContainerType_E::EDeque, false);
    }
};

/**
 * Double Deque Object Variable. Holds a deque of float values (double precision).
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Double Deque Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UDouble_DQOV : public UDequeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_DEQUE_NATIVE_INTERFACE(double, Double)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushBack"))
    void PushBackDeque(double InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<double> Values = GetDoubleDequeObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetDoubleDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushFront"))
    void PushFrontDeque(double InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<double> Values = GetDoubleDequeObjectVariable(OptionalVariableName);
        Values.Insert(InValue, 0);
        SetDoubleDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopFront"))
    bool PopFrontDeque(UPARAM(ref) double& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<double> Values = GetDoubleDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetDoubleDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopBack"))
    bool PopBackDeque(UPARAM(ref) double& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<double> Values = GetDoubleDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetDoubleDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekFront"))
    bool PeekFrontDeque(UPARAM(ref) double& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<double> Values = GetDoubleDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekBack"))
    bool PeekBackDeque(UPARAM(ref) double& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<double> Values = GetDoubleDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Count"))
    int32 GetDequeCount(FName OptionalVariableName = NAME_None) const
    {
        return GetDoubleDequeObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Clear"))
    void ClearDeque(FName OptionalVariableName = NAME_None)
    {
        TArray<double> Values;
        SetDoubleDequeObjectVariable(OptionalVariableName, Values);
    }

    UDouble_DQOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Double, NAME_None, nullptr, EObjectVariableContainerType_E::EDeque, false);
    }
};

/**
 * Integer64 Deque Object Variable. Holds a deque of 64-bit integer values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer64 Deque Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger64_DQOV : public UDequeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_DEQUE_NATIVE_INTERFACE(int64, Integer64)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushBack"))
    void PushBackDeque(int64 InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<int64> Values = GetInteger64DequeObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetInteger64DequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushFront"))
    void PushFrontDeque(int64 InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<int64> Values = GetInteger64DequeObjectVariable(OptionalVariableName);
        Values.Insert(InValue, 0);
        SetInteger64DequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopFront"))
    bool PopFrontDeque(UPARAM(ref) int64& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<int64> Values = GetInteger64DequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetInteger64DequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopBack"))
    bool PopBackDeque(UPARAM(ref) int64& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<int64> Values = GetInteger64DequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetInteger64DequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekFront"))
    bool PeekFrontDeque(UPARAM(ref) int64& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<int64> Values = GetInteger64DequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekBack"))
    bool PeekBackDeque(UPARAM(ref) int64& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<int64> Values = GetInteger64DequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Count"))
    int32 GetDequeCount(FName OptionalVariableName = NAME_None) const
    {
        return GetInteger64DequeObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Clear"))
    void ClearDeque(FName OptionalVariableName = NAME_None)
    {
        TArray<int64> Values;
        SetInteger64DequeObjectVariable(OptionalVariableName, Values);
    }

    UInteger64_DQOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Int64, NAME_None, nullptr, EObjectVariableContainerType_E::EDeque, false);
    }
};

/**
 * Integer Deque Object Variable. Holds a deque of 32-bit integer values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer Deque Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger_DQOV : public UDequeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_DEQUE_NATIVE_INTERFACE(int32, Integer)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushBack"))
    void PushBackDeque(int32 InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<int32> Values = GetIntegerDequeObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetIntegerDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushFront"))
    void PushFrontDeque(int32 InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<int32> Values = GetIntegerDequeObjectVariable(OptionalVariableName);
        Values.Insert(InValue, 0);
        SetIntegerDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopFront"))
    bool PopFrontDeque(UPARAM(ref) int32& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<int32> Values = GetIntegerDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetIntegerDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopBack"))
    bool PopBackDeque(UPARAM(ref) int32& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<int32> Values = GetIntegerDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetIntegerDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekFront"))
    bool PeekFrontDeque(UPARAM(ref) int32& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<int32> Values = GetIntegerDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekBack"))
    bool PeekBackDeque(UPARAM(ref) int32& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<int32> Values = GetIntegerDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Count"))
    int32 GetDequeCount(FName OptionalVariableName = NAME_None) const
    {
        return GetIntegerDequeObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Clear"))
    void ClearDeque(FName OptionalVariableName = NAME_None)
    {
        TArray<int32> Values;
        SetIntegerDequeObjectVariable(OptionalVariableName, Values);
    }

    UInteger_DQOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Int, NAME_None, nullptr, EObjectVariableContainerType_E::EDeque, false);
    }
};

/**
 * Name Deque Object Variable. Holds a deque of FName values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Name Deque Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UName_DQOV : public UDequeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_DEQUE_NATIVE_INTERFACE(FName, Name)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushBack"))
    void PushBackDeque(FName InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FName> Values = GetNameDequeObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetNameDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushFront"))
    void PushFrontDeque(FName InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FName> Values = GetNameDequeObjectVariable(OptionalVariableName);
        Values.Insert(InValue, 0);
        SetNameDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopFront"))
    bool PopFrontDeque(UPARAM(ref) FName& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FName> Values = GetNameDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetNameDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopBack"))
    bool PopBackDeque(UPARAM(ref) FName& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FName> Values = GetNameDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetNameDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekFront"))
    bool PeekFrontDeque(UPARAM(ref) FName& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<FName> Values = GetNameDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekBack"))
    bool PeekBackDeque(UPARAM(ref) FName& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<FName> Values = GetNameDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Count"))
    int32 GetDequeCount(FName OptionalVariableName = NAME_None) const
    {
        return GetNameDequeObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Clear"))
    void ClearDeque(FName OptionalVariableName = NAME_None)
    {
        TArray<FName> Values;
        SetNameDequeObjectVariable(OptionalVariableName, Values);
    }

    UName_DQOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Name, NAME_None, nullptr, EObjectVariableContainerType_E::EDeque, false);
    }
};

/**
 * Rotator Deque Object Variable. Holds a deque of FRotator values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Rotator Deque Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API URotator_DQOV : public UDequeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_DEQUE_NATIVE_INTERFACE(FRotator, Rotator)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushBack"))
    void PushBackDeque(FRotator InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FRotator> Values = GetRotatorDequeObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetRotatorDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushFront"))
    void PushFrontDeque(FRotator InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FRotator> Values = GetRotatorDequeObjectVariable(OptionalVariableName);
        Values.Insert(InValue, 0);
        SetRotatorDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopFront"))
    bool PopFrontDeque(UPARAM(ref) FRotator& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FRotator> Values = GetRotatorDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetRotatorDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopBack"))
    bool PopBackDeque(UPARAM(ref) FRotator& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FRotator> Values = GetRotatorDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetRotatorDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekFront"))
    bool PeekFrontDeque(UPARAM(ref) FRotator& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<FRotator> Values = GetRotatorDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekBack"))
    bool PeekBackDeque(UPARAM(ref) FRotator& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<FRotator> Values = GetRotatorDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Count"))
    int32 GetDequeCount(FName OptionalVariableName = NAME_None) const
    {
        return GetRotatorDequeObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Clear"))
    void ClearDeque(FName OptionalVariableName = NAME_None)
    {
        TArray<FRotator> Values;
        SetRotatorDequeObjectVariable(OptionalVariableName, Values);
    }

    URotator_DQOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FRotator>::Get(), EObjectVariableContainerType_E::EDeque, false);
    }
};

/**
 * String Deque Object Variable. Holds a deque of FString values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "String Deque Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UString_DQOV : public UDequeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_DEQUE_NATIVE_INTERFACE(FString, String)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushBack"))
    void PushBackDeque(FString InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FString> Values = GetStringDequeObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetStringDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushFront"))
    void PushFrontDeque(FString InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FString> Values = GetStringDequeObjectVariable(OptionalVariableName);
        Values.Insert(InValue, 0);
        SetStringDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopFront"))
    bool PopFrontDeque(UPARAM(ref) FString& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FString> Values = GetStringDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetStringDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopBack"))
    bool PopBackDeque(UPARAM(ref) FString& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FString> Values = GetStringDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetStringDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekFront"))
    bool PeekFrontDeque(UPARAM(ref) FString& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<FString> Values = GetStringDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekBack"))
    bool PeekBackDeque(UPARAM(ref) FString& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<FString> Values = GetStringDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Count"))
    int32 GetDequeCount(FName OptionalVariableName = NAME_None) const
    {
        return GetStringDequeObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Clear"))
    void ClearDeque(FName OptionalVariableName = NAME_None)
    {
        TArray<FString> Values;
        SetStringDequeObjectVariable(OptionalVariableName, Values);
    }

    UString_DQOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_String, NAME_None, nullptr, EObjectVariableContainerType_E::EDeque, false);
    }
};

/**
 * Text Deque Object Variable. Holds a deque of FText values.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Text Deque Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UText_DQOV : public UDequeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_DEQUE_NATIVE_INTERFACE(FText, Text)
public:
    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushBack"))
    void PushBackDeque(FText InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FText> Values = GetTextDequeObjectVariable(OptionalVariableName);
        Values.Add(InValue);
        SetTextDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PushFront"))
    void PushFrontDeque(FText InValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FText> Values = GetTextDequeObjectVariable(OptionalVariableName);
        Values.Insert(InValue, 0);
        SetTextDequeObjectVariable(OptionalVariableName, Values);
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopFront"))
    bool PopFrontDeque(UPARAM(ref) FText& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FText> Values = GetTextDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        Values.RemoveAt(0);
        SetTextDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PopBack"))
    bool PopBackDeque(UPARAM(ref) FText& OutValue, FName OptionalVariableName = NAME_None)
    {
        TArray<FText> Values = GetTextDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        Values.RemoveAt(Values.Num() - 1);
        SetTextDequeObjectVariable(OptionalVariableName, Values);
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekFront"))
    bool PeekFrontDeque(UPARAM(ref) FText& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<FText> Values = GetTextDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values[0];
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="PeekBack"))
    bool PeekBackDeque(UPARAM(ref) FText& OutValue, FName OptionalVariableName = NAME_None) const
    {
        TArray<FText> Values = GetTextDequeObjectVariable(OptionalVariableName);
        if (Values.Num() == 0)
        {
            return false;
        }
        OutValue = Values.Last();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Count"))
    int32 GetDequeCount(FName OptionalVariableName = NAME_None) const
    {
        return GetTextDequeObjectVariable(OptionalVariableName).Num();
    }

    UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque", meta=(CompactNodeTitle="Clear"))
    void ClearDeque(FName OptionalVariableName = NAME_None)
    {
        TArray<FText> Values;
        SetTextDequeObjectVariable(OptionalVariableName, Values);
    }

    UText_DQOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Text, NAME_None, nullptr, EObjectVariableContainerType_E::EDeque, false);
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
    TMap<FString, UGorgeousObjectVariable*> Value;

    UString_MOV()
    {
        Value = TMap<FString, UGorgeousObjectVariable*>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_String, NAME_None, nullptr, EObjectVariableContainerType_E::EMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};


/**
 * Abstract base class for multi-map object variables.
 *
 * @author Nils Bergemann
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API UMultiMapObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
};

/**
 * Abstract base class for multi-map object variables that hold object type keys.
 *
 * @author Nils Bergemann
 */
UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API UMultiMapObjectTypeObjectVariable : public UMultiMapObjectVariable
{
    GENERATED_BODY()
};

/**
 * Object Multi Map Object Variable. Holds a multi-map where the key is a UObject pointer.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Object Multi Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UObject_MMOTOV : public UMultiMapObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_NATIVE_INTERFACE(UObject*, ObjectObject)

    UObject_MMOTOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Object, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EMultiMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * Class Multi Map Object Variable. Holds a multi-map where the key is a UClass pointer.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Class Multi Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UClass_MMOTOV : public UMultiMapObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_NATIVE_INTERFACE(UClass*, ObjectClass)

    UClass_MMOTOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Class, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EMultiMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * Soft Object Multi Map Object Variable. Holds a multi-map where the key is a TSoftObjectPtr to a UObject.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Object Multi Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftObject_MMOTOV : public UMultiMapObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_NATIVE_INTERFACE(TSoftObjectPtr<UObject>, SoftObjectObject)

    USoftObject_MMOTOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_SoftObject, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EMultiMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass(), false, true);
    }
};

/**
 * Soft Class Multi Map Object Variable. Holds a multi-map where the key is a TSoftClassPtr to a UObject.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Class Multi Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftClass_MMOTOV : public UMultiMapObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_NATIVE_INTERFACE(TSoftClassPtr<UObject>, SoftObjectClass)

    USoftClass_MMOTOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_SoftClass, NAME_None, UObject::StaticClass(), EObjectVariableContainerType_E::EMultiMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass(), false, true);
    }
};

/**
 * Byte Multi Map Object Variable. Holds a multi-map where the key is a byte.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Byte Multi Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UByte_MMOV : public UMultiMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_NATIVE_INTERFACE(uint8, Byte)
    
    public:
        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Add"))
        void AddMultiMap(uint8 Key, UGorgeousObjectVariable* InValue, FName OptionalVariableName = NAME_None)
        {
            TMap<uint8, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetByteMultiMapObjectVariable(OptionalVariableName);
            Values.FindOrAdd(Key).Values.Add(InValue);
            SetByteMultiMapObjectVariable(OptionalVariableName, Values);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="RemoveKey"))
        bool RemoveMultiMapKey(uint8 Key, FName OptionalVariableName = NAME_None)
        {
            TMap<uint8, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetByteMultiMapObjectVariable(OptionalVariableName);
            const bool bRemoved = Values.Remove(Key) > 0;
            SetByteMultiMapObjectVariable(OptionalVariableName, Values);
            return bRemoved;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="RemoveEntry"))
        bool RemoveMultiMapEntry(uint8 Key, UGorgeousObjectVariable* InValue, FName OptionalVariableName = NAME_None)
        {
            TMap<uint8, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetByteMultiMapObjectVariable(OptionalVariableName);
            if (UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE* Found = Values.Find(Key))
            {
                const int32 Removed = Found->Values.Remove(InValue);
                if (Found->Values.Num() == 0)
                {
                    Values.Remove(Key);
                }
                SetByteMultiMapObjectVariable(OptionalVariableName, Values);
                return Removed > 0;
            }
            return false;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Find"))
        FObjectVariableMultiMapValue FindMultiMapValues(uint8 Key, FName OptionalVariableName = NAME_None) const
        {
            if (const UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE* Found = GetByteMultiMapObjectVariable(OptionalVariableName).Find(Key))
            {
                return *Found;
            }
            return UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE();
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Contains"))
        bool ContainsMultiMapKey(uint8 Key, FName OptionalVariableName = NAME_None) const
        {
            return GetByteMultiMapObjectVariable(OptionalVariableName).Contains(Key);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Count"))
        int32 GetMultiMapKeyCount(FName OptionalVariableName = NAME_None) const
        {
            return GetByteMultiMapObjectVariable(OptionalVariableName).Num();
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Clear"))
        void ClearMultiMap(FName OptionalVariableName = NAME_None)
        {
            TMap<uint8, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values;
            SetByteMultiMapObjectVariable(OptionalVariableName, Values);
        }

        UByte_MMOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Byte, NAME_None, nullptr, EObjectVariableContainerType_E::EMultiMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * Float Multi Map Object Variable. Holds a multi-map where the key is a float.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Float Multi Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UFloat_MMOV : public UMultiMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_NATIVE_INTERFACE(float, Float)
    public:
        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Add"))
        void AddMultiMap(float Key, UGorgeousObjectVariable* InValue, FName OptionalVariableName = NAME_None)
        {
            TMap<float, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetFloatMultiMapObjectVariable(OptionalVariableName);
            Values.FindOrAdd(Key).Values.Add(InValue);
            SetFloatMultiMapObjectVariable(OptionalVariableName, Values);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="RemoveKey"))
        bool RemoveMultiMapKey(float Key, FName OptionalVariableName = NAME_None)
        {
            TMap<float, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetFloatMultiMapObjectVariable(OptionalVariableName);
            const bool bRemoved = Values.Remove(Key) > 0;
            SetFloatMultiMapObjectVariable(OptionalVariableName, Values);
            return bRemoved;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="RemoveEntry"))
        bool RemoveMultiMapEntry(float Key, UGorgeousObjectVariable* InValue, FName OptionalVariableName = NAME_None)
        {
            TMap<float, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetFloatMultiMapObjectVariable(OptionalVariableName);
            if (UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE* Found = Values.Find(Key))
            {
                const int32 Removed = Found->Values.Remove(InValue);
                if (Found->Values.Num() == 0)
                {
                    Values.Remove(Key);
                }
                SetFloatMultiMapObjectVariable(OptionalVariableName, Values);
                return Removed > 0;
            }
            return false;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Find"))
        FObjectVariableMultiMapValue FindMultiMapValues(float Key, FName OptionalVariableName = NAME_None) const
        {
            if (const UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE* Found = GetFloatMultiMapObjectVariable(OptionalVariableName).Find(Key))
            {
                return *Found;
            }
            return UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE();
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Contains"))
        bool ContainsMultiMapKey(float Key, FName OptionalVariableName = NAME_None) const
        {
            return GetFloatMultiMapObjectVariable(OptionalVariableName).Contains(Key);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Count"))
        int32 GetMultiMapKeyCount(FName OptionalVariableName = NAME_None) const
        {
            return GetFloatMultiMapObjectVariable(OptionalVariableName).Num();
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Clear"))
        void ClearMultiMap(FName OptionalVariableName = NAME_None)
        {
            TMap<float, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values;
            SetFloatMultiMapObjectVariable(OptionalVariableName, Values);
        }

        UFloat_MMOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Float, NAME_None, nullptr, EObjectVariableContainerType_E::EMultiMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * Double Multi Map Object Variable. Holds a multi-map where the key is a float (double precision).
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Double Multi Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UDouble_MMOV : public UMultiMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_NATIVE_INTERFACE(double, Double)
    public:
        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Add"))
        void AddMultiMap(double Key, UGorgeousObjectVariable* InValue, FName OptionalVariableName = NAME_None)
        {
            TMap<double, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetDoubleMultiMapObjectVariable(OptionalVariableName);
            Values.FindOrAdd(Key).Values.Add(InValue);
            SetDoubleMultiMapObjectVariable(OptionalVariableName, Values);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="RemoveKey"))
        bool RemoveMultiMapKey(double Key, FName OptionalVariableName = NAME_None)
        {
            TMap<double, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetDoubleMultiMapObjectVariable(OptionalVariableName);
            const bool bRemoved = Values.Remove(Key) > 0;
            SetDoubleMultiMapObjectVariable(OptionalVariableName, Values);
            return bRemoved;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="RemoveEntry"))
        bool RemoveMultiMapEntry(double Key, UGorgeousObjectVariable* InValue, FName OptionalVariableName = NAME_None)
        {
            TMap<double, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetDoubleMultiMapObjectVariable(OptionalVariableName);
            if (UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE* Found = Values.Find(Key))
            {
                const int32 Removed = Found->Values.Remove(InValue);
                if (Found->Values.Num() == 0)
                {
                    Values.Remove(Key);
                }
                SetDoubleMultiMapObjectVariable(OptionalVariableName, Values);
                return Removed > 0;
            }
            return false;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Find"))
        FObjectVariableMultiMapValue FindMultiMapValues(double Key, FName OptionalVariableName = NAME_None) const
        {
            if (const UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE* Found = GetDoubleMultiMapObjectVariable(OptionalVariableName).Find(Key))
            {
                return *Found;
            }
            return UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE();
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Contains"))
        bool ContainsMultiMapKey(double Key, FName OptionalVariableName = NAME_None) const
        {
            return GetDoubleMultiMapObjectVariable(OptionalVariableName).Contains(Key);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Count"))
        int32 GetMultiMapKeyCount(FName OptionalVariableName = NAME_None) const
        {
            return GetDoubleMultiMapObjectVariable(OptionalVariableName).Num();
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Clear"))
        void ClearMultiMap(FName OptionalVariableName = NAME_None)
        {
            TMap<double, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values;
            SetDoubleMultiMapObjectVariable(OptionalVariableName, Values);
        }

        UDouble_MMOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Double, NAME_None, nullptr, EObjectVariableContainerType_E::EMultiMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * Integer64 Multi Map Object Variable. Holds a multi-map where the key is a 64-bit integer.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer64 Multi Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger64_MMOV : public UMultiMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_NATIVE_INTERFACE(int64, Integer64)
    public:
        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Add"))
        void AddMultiMap(int64 Key, UGorgeousObjectVariable* InValue, FName OptionalVariableName = NAME_None)
        {
            TMap<int64, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetInteger64MultiMapObjectVariable(OptionalVariableName);
            Values.FindOrAdd(Key).Values.Add(InValue);
            SetInteger64MultiMapObjectVariable(OptionalVariableName, Values);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="RemoveKey"))
        bool RemoveMultiMapKey(int64 Key, FName OptionalVariableName = NAME_None)
        {
            TMap<int64, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetInteger64MultiMapObjectVariable(OptionalVariableName);
            const bool bRemoved = Values.Remove(Key) > 0;
            SetInteger64MultiMapObjectVariable(OptionalVariableName, Values);
            return bRemoved;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="RemoveEntry"))
        bool RemoveMultiMapEntry(int64 Key, UGorgeousObjectVariable* InValue, FName OptionalVariableName = NAME_None)
        {
            TMap<int64, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetInteger64MultiMapObjectVariable(OptionalVariableName);
            if (UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE* Found = Values.Find(Key))
            {
                const int32 Removed = Found->Values.Remove(InValue);
                if (Found->Values.Num() == 0)
                {
                    Values.Remove(Key);
                }
                SetInteger64MultiMapObjectVariable(OptionalVariableName, Values);
                return Removed > 0;
            }
            return false;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Find"))
        FObjectVariableMultiMapValue FindMultiMapValues(int64 Key, FName OptionalVariableName = NAME_None) const
        {
            if (const UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE* Found = GetInteger64MultiMapObjectVariable(OptionalVariableName).Find(Key))
            {
                return *Found;
            }
            return UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE();
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Contains"))
        bool ContainsMultiMapKey(int64 Key, FName OptionalVariableName = NAME_None) const
        {
            return GetInteger64MultiMapObjectVariable(OptionalVariableName).Contains(Key);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Count"))
        int32 GetMultiMapKeyCount(FName OptionalVariableName = NAME_None) const
        {
            return GetInteger64MultiMapObjectVariable(OptionalVariableName).Num();
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Clear"))
        void ClearMultiMap(FName OptionalVariableName = NAME_None)
        {
            TMap<int64, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values;
            SetInteger64MultiMapObjectVariable(OptionalVariableName, Values);
        }

        UInteger64_MMOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Int64, NAME_None, nullptr, EObjectVariableContainerType_E::EMultiMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * Integer Multi Map Object Variable. Holds a multi-map where the key is a 32-bit integer.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer Multi Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger_MMOV : public UMultiMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_NATIVE_INTERFACE(int32, Integer)
    public:
        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Add"))
        void AddMultiMap(int32 Key, UGorgeousObjectVariable* InValue, FName OptionalVariableName = NAME_None)
        {
            TMap<int32, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetIntegerMultiMapObjectVariable(OptionalVariableName);
            Values.FindOrAdd(Key).Values.Add(InValue);
            SetIntegerMultiMapObjectVariable(OptionalVariableName, Values);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="RemoveKey"))
        bool RemoveMultiMapKey(int32 Key, FName OptionalVariableName = NAME_None)
        {
            TMap<int32, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetIntegerMultiMapObjectVariable(OptionalVariableName);
            const bool bRemoved = Values.Remove(Key) > 0;
            SetIntegerMultiMapObjectVariable(OptionalVariableName, Values);
            return bRemoved;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="RemoveEntry"))
        bool RemoveMultiMapEntry(int32 Key, UGorgeousObjectVariable* InValue, FName OptionalVariableName = NAME_None)
        {
            TMap<int32, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetIntegerMultiMapObjectVariable(OptionalVariableName);
            if (UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE* Found = Values.Find(Key))
            {
                const int32 Removed = Found->Values.Remove(InValue);
                if (Found->Values.Num() == 0)
                {
                    Values.Remove(Key);
                }
                SetIntegerMultiMapObjectVariable(OptionalVariableName, Values);
                return Removed > 0;
            }
            return false;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Find"))
        FObjectVariableMultiMapValue FindMultiMapValues(int32 Key, FName OptionalVariableName = NAME_None) const
        {
            if (const UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE* Found = GetIntegerMultiMapObjectVariable(OptionalVariableName).Find(Key))
            {
                return *Found;
            }
            return UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE();
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Contains"))
        bool ContainsMultiMapKey(int32 Key, FName OptionalVariableName = NAME_None) const
        {
            return GetIntegerMultiMapObjectVariable(OptionalVariableName).Contains(Key);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Count"))
        int32 GetMultiMapKeyCount(FName OptionalVariableName = NAME_None) const
        {
            return GetIntegerMultiMapObjectVariable(OptionalVariableName).Num();
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Clear"))
        void ClearMultiMap(FName OptionalVariableName = NAME_None)
        {
            TMap<int32, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values;
            SetIntegerMultiMapObjectVariable(OptionalVariableName, Values);
        }

        UInteger_MMOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Int, NAME_None, nullptr, EObjectVariableContainerType_E::EMultiMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * Name Multi Map Object Variable. Holds a multi-map where the key is an FName.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Name Multi Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UName_MMOV : public UMultiMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_NATIVE_INTERFACE(FName, Name)
    public:
        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Add"))
        void AddMultiMap(FName Key, UGorgeousObjectVariable* InValue, FName OptionalVariableName = NAME_None)
        {
            TMap<FName, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetNameMultiMapObjectVariable(OptionalVariableName);
            Values.FindOrAdd(Key).Values.Add(InValue);
            SetNameMultiMapObjectVariable(OptionalVariableName, Values);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="RemoveKey"))
        bool RemoveMultiMapKey(FName Key, FName OptionalVariableName = NAME_None)
        {
            TMap<FName, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetNameMultiMapObjectVariable(OptionalVariableName);
            const bool bRemoved = Values.Remove(Key) > 0;
            SetNameMultiMapObjectVariable(OptionalVariableName, Values);
            return bRemoved;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="RemoveEntry"))
        bool RemoveMultiMapEntry(FName Key, UGorgeousObjectVariable* InValue, FName OptionalVariableName = NAME_None)
        {
            TMap<FName, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetNameMultiMapObjectVariable(OptionalVariableName);
            if (UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE* Found = Values.Find(Key))
            {
                const int32 Removed = Found->Values.Remove(InValue);
                if (Found->Values.Num() == 0)
                {
                    Values.Remove(Key);
                }
                SetNameMultiMapObjectVariable(OptionalVariableName, Values);
                return Removed > 0;
            }
            return false;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Find"))
        FObjectVariableMultiMapValue FindMultiMapValues(FName Key, FName OptionalVariableName = NAME_None) const
        {
            if (const UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE* Found = GetNameMultiMapObjectVariable(OptionalVariableName).Find(Key))
            {
                return *Found;
            }
            return UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE();
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Contains"))
        bool ContainsMultiMapKey(FName Key, FName OptionalVariableName = NAME_None) const
        {
            return GetNameMultiMapObjectVariable(OptionalVariableName).Contains(Key);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Count"))
        int32 GetMultiMapKeyCount(FName OptionalVariableName = NAME_None) const
        {
            return GetNameMultiMapObjectVariable(OptionalVariableName).Num();
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Clear"))
        void ClearMultiMap(FName OptionalVariableName = NAME_None)
        {
            TMap<FName, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values;
            SetNameMultiMapObjectVariable(OptionalVariableName, Values);
        }

        UName_MMOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_Name, NAME_None, nullptr, EObjectVariableContainerType_E::EMultiMap, false,
            true, UEdGraphSchema_K2::PC_Object, NAME_None, UGorgeousObjectVariable::StaticClass());
    }
};

/**
 * String Multi Map Object Variable. Holds a multi-map where the key is an FString.
 *
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "String Multi Map Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UString_MMOV : public UMultiMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_NATIVE_INTERFACE(FString, String)
    public:
        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Add"))
        void AddMultiMap(FString Key, UGorgeousObjectVariable* InValue, FName OptionalVariableName = NAME_None)
        {
            TMap<FString, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetStringMultiMapObjectVariable(OptionalVariableName);
            Values.FindOrAdd(Key).Values.Add(InValue);
            SetStringMultiMapObjectVariable(OptionalVariableName, Values);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="RemoveKey"))
        bool RemoveMultiMapKey(FString Key, FName OptionalVariableName = NAME_None)
        {
            TMap<FString, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetStringMultiMapObjectVariable(OptionalVariableName);
            const bool bRemoved = Values.Remove(Key) > 0;
            SetStringMultiMapObjectVariable(OptionalVariableName, Values);
            return bRemoved;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="RemoveEntry"))
        bool RemoveMultiMapEntry(FString Key, UGorgeousObjectVariable* InValue, FName OptionalVariableName = NAME_None)
        {
            TMap<FString, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values = GetStringMultiMapObjectVariable(OptionalVariableName);
            if (UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE* Found = Values.Find(Key))
            {
                const int32 Removed = Found->Values.Remove(InValue);
                if (Found->Values.Num() == 0)
                {
                    Values.Remove(Key);
                }
                SetStringMultiMapObjectVariable(OptionalVariableName, Values);
                return Removed > 0;
            }
            return false;
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Find"))
        FObjectVariableMultiMapValue FindMultiMapValues(FString Key, FName OptionalVariableName = NAME_None) const
        {
            if (const UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE* Found = GetStringMultiMapObjectVariable(OptionalVariableName).Find(Key))
            {
                return *Found;
            }
            return UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE();
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Contains"))
        bool ContainsMultiMapKey(FString Key, FName OptionalVariableName = NAME_None) const
        {
            return GetStringMultiMapObjectVariable(OptionalVariableName).Contains(Key);
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Count"))
        int32 GetMultiMapKeyCount(FName OptionalVariableName = NAME_None) const
        {
            return GetStringMultiMapObjectVariable(OptionalVariableName).Num();
        }

        UFUNCTION(BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map", meta=(CompactNodeTitle="Clear"))
        void ClearMultiMap(FName OptionalVariableName = NAME_None)
        {
            TMap<FString, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Values;
            SetStringMultiMapObjectVariable(OptionalVariableName, Values);
        }

        UString_MMOV()
    {
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_String, NAME_None, nullptr, EObjectVariableContainerType_E::EMultiMap, false,
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Default", meta = (ExposeOnSpawn = true, AllowPrivateAccess = "true"))
    TSet<FString> Value;

    UString_STOV()
    {
        Value = TSet<FString>();
        PinConfiguration = FObjectVariablePinConfiguration_S(UEdGraphSchema_K2::PC_String, NAME_None, nullptr, EObjectVariableContainerType_E::ESet, false);
    }
};



