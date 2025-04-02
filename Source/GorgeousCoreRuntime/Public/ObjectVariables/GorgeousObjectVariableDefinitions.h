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
 * @brief Abstract base class for single object variables.
 * @author Nils Bergemann
 */
UCLASS(Abstract, NotBlueprintType)
class GORGEOUSCORERUNTIME_API USingleObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
};

/**
 * Abstract base class for single object variables that hold object type values.
 *
 * @brief Abstract base class for single object variables that hold object type values.
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
 * @brief Object Single Object Variable. Holds a single UObject pointer.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Object Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UObject_SOTOV : public USingleObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(UObject*, Value, ObjectObject)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    UObject* Value;
};

/**
 * Class Single Object Variable. Holds a single UClass pointer.
 *
 * @brief Class Single Object Variable. Holds a single UClass pointer.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Class Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UClass_SOTOV : public USingleObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(UClass*, Value, ObjectClass)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    UClass* Value;
};

/**
 * Soft Object Single Object Variable. Holds a single TSoftObjectPtr to a UObject.
 *
 * @brief Soft Object Single Object Variable. Holds a single TSoftObjectPtr to a UObject.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Object Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftObject_SOTOV : public USingleObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(TSoftObjectPtr<UObject>, Value, SoftObjectObject)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSoftObjectPtr<UObject> Value;
};

/**
 * Soft Class Single Object Variable. Holds a single TSoftClassPtr to a UObject.
 *
 * @brief Soft Class Single Object Variable. Holds a single TSoftClassPtr to a UObject.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Class Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftClass_SOTOV : public USingleObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(TSoftClassPtr<UObject>, Value, SoftObjectClass)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSoftClassPtr<UObject> Value;
};

/**
 * Boolean Single Object Variable. Holds a single boolean value.
 *
 * @brief Boolean Single Object Variable. Holds a single boolean value.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Boolean Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UBoolean_SOV  : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(bool, Value, Boolean)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    bool Value;
};

/**
 * Byte Single Object Variable. Holds a single byte value.
 *
 * @brief Byte Single Object Variable. Holds a single byte value.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Byte Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UByte_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(uint8, Value, Byte)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    uint8 Value;
};

/**
 * Float Single Object Variable. Holds a single float value (double precision).
 *
 * @brief Float Single Object Variable. Holds a single float value (double precision).
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Float Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UFloat_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(double, Value, Float)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    double Value;
};

/**
 * Integer64 Single Object Variable. Holds a single 64-bit integer value.
 *
 * @brief Integer64 Single Object Variable. Holds a single 64-bit integer value.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer64 Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger64_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(int64, Value, Integer64)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    int64 Value;
};

/**
 * Integer Single Object Variable. Holds a single 32-bit integer value.
 *
 * @brief Integer Single Object Variable. Holds a single 32-bit integer value.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(int32, Value, Integer)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    int32 Value;
};

/**
 * Name Single Object Variable. Holds a single FName value.
 *
 * @brief Name Single Object Variable. Holds a single FName value.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Name Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UName_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FName, Value, Name)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    FName Value;
};

/**
 * Rotator Single Object Variable. Holds a single FRotator value.
 *
 * @brief Rotator Single Object Variable. Holds a single FRotator value.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Rotator Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API URotator_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FRotator, Value, Rotator)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    FRotator Value;
};

/**
 * String Single Object Variable. Holds a single FString value.
 *
 * @brief String Single Object Variable. Holds a single FString value.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "String Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UString_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FString, Value, String)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    FString Value;
};

/**
 * Text Single Object Variable. Holds a single FText value.
 *
 * @brief Text Single Object Variable. Holds a single FText value.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Text Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UText_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FText, Value, Text)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    FText Value;
};

/**
 * Transform Single Object Variable. Holds a single FTransform value.
 *
 * @brief Transform Single Object Variable. Holds a single FTransform value.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Transform Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UTransform_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FTransform, Value, Transform)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    FTransform Value;
};

/**
 * Vector Single Object Variable. Holds a single FVector value.
 *
 * @brief Vector Single Object Variable. Holds a single FVector value.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Vector Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UVector_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FVector, Value, Vector)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    FVector Value;
};


/**
 * Abstract base class for array object variables.
 *
 * @brief Abstract base class for array object variables.
 * @author Nils Bergemann
 */
UCLASS(Abstract, NotBlueprintType)
class GORGEOUSCORERUNTIME_API UArrayObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
};

/**
 * Abstract base class for array object variables that hold object type values.
 *
 * @brief Abstract base class for array object variables that hold object type values.
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
 * @brief Object Array Object Variable. Holds an array of UObject pointers.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Object Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UObject_AOTOV : public UArrayObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<UObject*>, Value, ObjectObject)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<UObject*> Value;
};

/**
 * Class Array Object Variable. Holds an array of UClass pointers.
 *
 * @brief Class Array Object Variable. Holds an array of UClass pointers.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Class Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UClass_AOTOV : public UArrayObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<UClass*>, Value, ObjectClass)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<UClass*> Value;
};

/**
 * Soft Object Array Object Variable. Holds an array of TSoftObjectPtr to UObjects.
 *
 * @brief Soft Object Array Object Variable. Holds an array of TSoftObjectPtr to UObjects.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Object Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftObject_AOTOV : public UArrayObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<TSoftObjectPtr<UObject>>, Value, SoftObjectObject)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<TSoftObjectPtr<UObject>> Value;
};

/**
 * Soft Class Array Object Variable. Holds an array of TSoftClassPtr to UObjects.
 *
 * @brief Soft Class Array Object Variable. Holds an array of TSoftClassPtr to UObjects.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Class Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftClass_AOTOV : public UArrayObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<TSoftClassPtr<UObject>>, Value, SoftObjectClass)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<TSoftClassPtr<UObject>> Value;
};

/**
 * Boolean Array Object Variable. Holds an array of boolean values.
 *
 * @brief Boolean Array Object Variable. Holds an array of boolean values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Boolean Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UBoolean_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array,  TArray<bool>, Value, Boolean)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<bool> Value;
};

/**
 * Byte Array Object Variable. Holds an array of byte values.
 *
 * @brief Byte Array Object Variable. Holds an array of byte values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Byte Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UByte_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<uint8>, Value, Byte)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<uint8> Value;
};

/**
 * Float Array Object Variable. Holds an array of float values (double precision).
 *
 * @brief Float Array Object Variable. Holds an array of float values (double precision).
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Float Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UFloat_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array,  TArray<double>, Value, Float)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<double> Value;
};

/**
 * Integer64 Array Object Variable. Holds an array of 64-bit integer values.
 *
 * @brief Integer64 Array Object Variable. Holds an array of 64-bit integer values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer64 Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger64_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<int64>, Value, Integer64)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<int64> Value;
};

/**
 * Integer Array Object Variable. Holds an array of 32-bit integer values.
 *
 * @brief Integer Array Object Variable. Holds an array of 32-bit integer values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<int32>, Value, Integer)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<int32> Value;
};

/**
 * Name Array Object Variable. Holds an array of FName values.
 *
 * @brief Name Array Object Variable. Holds an array of FName values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Name Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UName_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FName>, Value, Name)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<FName> Value;
};

/**
 * Rotator Array Object Variable. Holds an array of FRotator values.
 *
 * @brief Rotator Array Object Variable. Holds an array of FRotator values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Rotator Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API URotator_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FRotator>, Value, Rotator)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<FRotator> Value;
};

/**
 * String Array Object Variable. Holds an array of FString values.
 *
 * @brief String Array Object Variable. Holds an array of FString values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "String Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UString_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FString>, Value, String)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<FString> Value;
};

/**
 * Text Array Object Variable. Holds an array of FText values.
 *
 * @brief Text Array Object Variable. Holds an array of FText values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Text Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UText_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FText>, Value, Text)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<FText> Value;
};

/**
 * Transform Array Object Variable. Holds an array of FTransform values.
 *
 * @brief Transform Array Object Variable. Holds an array of FTransform values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Transform Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UTransform_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FTransform>, Value, Transform)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<FTransform> Value;
};

/**
 * Vector Array Object Variable. Holds an array of FVector values.
 *
 * @brief Vector Array Object Variable. Holds an array of FVector values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Vector Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UVector_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FVector>, Value, Vector)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TArray<FVector> Value;
};


/**
 * Abstract base class for map object variables.
 *
 * @brief Abstract base class for map object variables.
 * @author Nils Bergemann
 */
UCLASS(Abstract, NotBlueprintType)
class GORGEOUSCORERUNTIME_API UMapObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
};

/**
 * Abstract base class for map object variables that hold object type keys.
 *
 * @brief Abstract base class for map object variables that hold object type keys.
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
 * @brief Object Map Object Variable. Holds a map where the key is a UObject pointer.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Object Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UObject_MOTOV : public UMapObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(UObject*, Value, ObjectObject)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<UObject*, UGorgeousObjectVariable*> Value;
};

/**
 * Class Map Object Variable. Holds a map where the key is a UClass pointer.
 *
 * @brief Class Map Object Variable. Holds a map where the key is a UClass pointer.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Class Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UClass_MOTOV : public UMapObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(UClass*, Value, ObjectClass)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<UClass*, UGorgeousObjectVariable*> Value;
};

/**
 * Soft Object Map Object Variable. Holds a map where the key is a TSoftObjectPtr to a UObject.
 *
 * @brief Soft Object Map Object Variable. Holds a map where the key is a TSoftObjectPtr to a UObject.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Object Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftObject_MOTOV : public UMapObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(TSoftObjectPtr<UObject>, Value, SoftObjectObject)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<TSoftObjectPtr<UObject>, UGorgeousObjectVariable*> Value;
};

/**
 * Soft Class Map Object Variable. Holds a map where the key is a TSoftClassPtr to a UObject.
 *
 * @brief Soft Class Map Object Variable. Holds a map where the key is a TSoftClassPtr to a UObject.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Class Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftClass_MOTOV : public UMapObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(TSoftClassPtr<UObject>, Value, SoftObjectClass)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<TSoftClassPtr<UObject>, UGorgeousObjectVariable*> Value;
};

/**
 * Byte Map Object Variable. Holds a map where the key is a byte.
 *
 * @brief Byte Map Object Variable. Holds a map where the key is a byte.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Byte Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UByte_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(uint8, Value, Byte)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<uint8, UGorgeousObjectVariable*> Value;
};

/**
 * Float Map Object Variable. Holds a map where the key is a float (double precision).
 *
 * @brief Float Map Object Variable. Holds a map where the key is a float (double precision).
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Float Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UFloat_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(double, Value, Float)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<double, UGorgeousObjectVariable*> Value;
};

/**
 * Integer64 Map Object Variable. Holds a map where the key is a 64-bit integer.
 *
 * @brief Integer64 Map Object Variable. Holds a map where the key is a 64-bit integer.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer64 Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger64_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(int64, Value, Integer64)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<int64, UGorgeousObjectVariable*> Value;
};

/**
 * Integer Map Object Variable. Holds a map where the key is a 32-bit integer.
 *
 * @brief Integer Map Object Variable. Holds a map where the key is a 32-bit integer.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(int32, Value, Integer)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<int32, UGorgeousObjectVariable*> Value;
};

/**
 * Name Map Object Variable. Holds a map where the key is an FName.
 *
 * @brief Name Map Object Variable. Holds a map where the key is an FName.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Name Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UName_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(FName, Value, Name)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<FName, UGorgeousObjectVariable*> Value;
};

/**
 * String Map Object Variable. Holds a map where the key is an FString.
 *
 * @brief String Map Object Variable. Holds a map where the key is an FString.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "String Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UString_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(FString, Value, String)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<FString, UGorgeousObjectVariable*> Value;
};

/**
 * Transform Map Object Variable. Holds a map where the key is an FTransform.
 *
 * @brief Transform Map Object Variable. Holds a map where the key is an FTransform.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Transform Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UTransform_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(FTransform, Value, Transform)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<FTransform, UGorgeousObjectVariable*> Value;
};

/**
 * Vector Map Object Variable. Holds a map where the key is an FVector.
 *
 * @brief Vector Map Object Variable. Holds a map where the key is an FVector.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Vector Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UVector_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(FVector, Value, Vector)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TMap<FVector, UGorgeousObjectVariable*> Value;
};


/**
 * Abstract base class for set object variables.
 *
 * @brief Abstract base class for set object variables.
 * @author Nils Bergemann
 */
UCLASS(Abstract, NotBlueprintType)
class GORGEOUSCORERUNTIME_API USetObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
};

/**
 * Abstract base class for set object variables that hold object type values.
 *
 * @brief Abstract base class for set object variables that hold object type values.
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
 * @brief Object Set Object Variable. Holds a set of UObject pointers.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Object Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UObject_STOTOV : public USetObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<UObject*>, Value, ObjectObject)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<UObject*> Value;
};

/**
 * Class Set Object Variable. Holds a set of UClass pointers.
 *
 * @brief Class Set Object Variable. Holds a set of UClass pointers.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Class Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UClass_STOTOV : public USetObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<UClass*>, Value, ObjectClass)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<UClass*> Value;
};

/**
 * Soft Object Set Object Variable. Holds a set of TSoftObjectPtr to UObjects.
 *
 * @brief Soft Object Set Object Variable. Holds a set of TSoftObjectPtr to UObjects.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Object Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftObject_STOTOV : public USetObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<TSoftObjectPtr<UObject>>, Value, SoftObjectObject)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<TSoftObjectPtr<UObject>> Value;
};

/**
 * Soft Class Set Object Variable. Holds a set of TSoftClassPtr to UObjects.
 *
 * @brief Soft Class Set Object Variable. Holds a set of TSoftClassPtr to UObjects.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Soft Class Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftClass_STOTOV : public USetObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<TSoftClassPtr<UObject>>, Value, SoftObjectClass)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<TSoftClassPtr<UObject>> Value;
};


/**
 * Byte Set Object Variable. Holds a set of byte values.
 *
 * @brief Byte Set Object Variable. Holds a set of byte values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Byte Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UByte_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<uint8>, Value, Byte)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<uint8> Value;
};

/**
 * Float Set Object Variable. Holds a set of float values (double precision).
 *
 * @brief Float Set Object Variable. Holds a set of float values (double precision).
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Float Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UFloat_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<double>, Value, Float)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<double> Value;
};

/**
 * Integer64 Set Object Variable. Holds a set of 64-bit integer values.
 *
 * @brief Integer64 Set Object Variable. Holds a set of 64-bit integer values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer64 Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger64_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<int64>, Value, Integer64)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<int64> Value;
};

/**
 * Integer Set Object Variable. Holds a set of 32-bit integer values.
 *
 * @brief Integer Set Object Variable. Holds a set of 32-bit integer values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Integer Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<int32>, Value, Integer)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<int32> Value;
};

/**
 * Name Set Object Variable. Holds a set of FName values.
 *
 * @brief Name Set Object Variable. Holds a set of FName values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Name Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UName_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<FName>, Value, Name)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<FName> Value;
};

/**
 * String Set Object Variable. Holds a set of FString values.
 *
 * @brief String Set Object Variable. Holds a set of FString values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "String Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UString_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<FString>, Value, String)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<FString> Value;
};

/**
 * Transform Set Object Variable. Holds a set of FTransform values.
 *
 * @brief Transform Set Object Variable. Holds a set of FTransform values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Transform Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UTransform_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<FTransform>, Value, Transform)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<FTransform> Value;
};

/**
 * Vector Set Object Variable. Holds a set of FVector values.
 *
 * @brief Vector Set Object Variable. Holds a set of FVector values.
 * @author Nils Bergemann
 */
UCLASS(DisplayName = "Vector Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UVector_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<FVector>, Value, Vector)
    UPROPERTY(EditAnywhere, SaveGame, Category = "Default")
    TSet<FVector> Value;
};