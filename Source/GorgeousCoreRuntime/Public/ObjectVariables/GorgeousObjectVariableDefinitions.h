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


UCLASS(Abstract, NotBlueprintType)
class GORGEOUSCORERUNTIME_API USingleObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
};

UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API USingleObjectTypeObjectVariable : public USingleObjectVariable
{
    GENERATED_BODY()
};

UCLASS(DisplayName = "Object Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UObject_SOTOV : public USingleObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(UObject*, Value, ObjectObject)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    UObject* Value;
};

UCLASS(DisplayName = "Class Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UClass_SOTOV : public USingleObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(UClass*, Value, ObjectClass)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    UClass* Value;
};

UCLASS(DisplayName = "Soft Object Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftObject_SOTOV : public USingleObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(TSoftObjectPtr<UObject>, Value, SoftObjectObject)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TSoftObjectPtr<UObject> Value;
};

UCLASS(DisplayName = "Soft Class Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftClass_SOTOV : public USingleObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(TSoftClassPtr<UObject>, Value, SoftObjectClass)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TSoftClassPtr<UObject> Value;
};

UCLASS(DisplayName = "Boolean Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UBoolean_SOV  : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(bool, Value, Boolean)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    bool Value;
};

UCLASS(DisplayName = "Byte Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UByte_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(uint8, Value, Byte)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    uint8 Value;
};

UCLASS(DisplayName = "Float Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UFloat_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(double, Value, Float)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    double Value;
};

UCLASS(DisplayName = "Integer64 Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger64_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(int64, Value, Integer64)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    int64 Value;
};

UCLASS(DisplayName = "Integer Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(int32, Value, Integer)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    int32 Value;
};

UCLASS(DisplayName = "Name Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UName_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FName, Value, Name)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    FName Value;
};

UCLASS(DisplayName = "Rotator Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API URotator_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FRotator, Value, Rotator)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    FRotator Value;
};

UCLASS(DisplayName = "String Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UString_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FString, Value, String)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    FString Value;
};

UCLASS(DisplayName = "Text Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UText_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FText, Value, Text)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    FText Value;
};

UCLASS(DisplayName = "Transform Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UTransform_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FTransform, Value, Transform)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    FTransform Value;
};

UCLASS(DisplayName = "Vector Single OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UVector_SOV : public USingleObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(FVector, Value, Vector)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    FVector Value;
};

// Array Object Variables
UCLASS(Abstract, NotBlueprintType)
class GORGEOUSCORERUNTIME_API UArrayObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
};

UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API UArrayObjectTypeObjectVariable : public UArrayObjectVariable
{
    GENERATED_BODY()
};


UCLASS(DisplayName = "Object Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UObject_AOTOV : public UArrayObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<UObject*>, Value, ObjectObject)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TArray<UObject*> Value;
};

UCLASS(DisplayName = "Class Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UClass_AOTOV : public UArrayObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<UClass*>, Value, ObjectClass)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TArray<UClass*> Value;
};

UCLASS(DisplayName = "Soft Object Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftObject_AOTOV : public UArrayObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<TSoftObjectPtr<UObject>>, Value, SoftObjectObject)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TArray<TSoftObjectPtr<UObject>> Value;
};

UCLASS(DisplayName = "Soft Class Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftClass_AOTOV : public UArrayObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<TSoftClassPtr<UObject>>, Value, SoftObjectClass)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TArray<TSoftClassPtr<UObject>> Value;
};

UCLASS(DisplayName = "Boolean Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UBoolean_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array,  TArray<bool>, Value, Boolean)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TArray<bool> Value;
};

UCLASS(DisplayName = "Byte Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UByte_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<uint8>, Value, Byte)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TArray<uint8> Value;
};

UCLASS(DisplayName = "Float Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UFloat_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array,  TArray<double>, Value, Float)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TArray<double> Value;
};

UCLASS(DisplayName = "Integer64 Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger64_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<int64>, Value, Integer64)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TArray<int64> Value;
};

UCLASS(DisplayName = "Integer Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<int32>, Value, Integer)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TArray<int32> Value;
};

UCLASS(DisplayName = "Name Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UName_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FName>, Value, Name)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TArray<FName> Value;
};

UCLASS(DisplayName = "Rotator Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API URotator_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FRotator>, Value, Rotator)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TArray<FRotator> Value;
};

UCLASS(DisplayName = "String Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UString_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FString>, Value, String)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TArray<FString> Value;
};

UCLASS(DisplayName = "Text Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UText_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FText>, Value, Text)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TArray<FText> Value;
};

UCLASS(DisplayName = "Transform Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UTransform_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FTransform>, Value, Transform)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TArray<FTransform> Value;
};

UCLASS(DisplayName = "Vector Array OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UVector_AOV : public UArrayObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, TArray<FVector>, Value, Vector)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TArray<FVector> Value;
};

// Map Object Variables
UCLASS(Abstract, NotBlueprintType)
class GORGEOUSCORERUNTIME_API UMapObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
};

UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API UMapObjectTypeObjectVariable : public UMapObjectVariable
{
    GENERATED_BODY()
};

UCLASS(DisplayName = "Object Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UObject_MOTOV : public UMapObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(UObject*, Value, ObjectObject)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TMap<UObject*, UGorgeousObjectVariable*> Value;
};

UCLASS(DisplayName = "Class Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UClass_MOTOV : public UMapObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(UClass*, Value, ObjectClass)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TMap<UClass*, UGorgeousObjectVariable*> Value;
};

UCLASS(DisplayName = "Soft Object Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftObject_MOTOV : public UMapObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(TSoftObjectPtr<UObject>, Value, SoftObjectObject)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TMap<TSoftObjectPtr<UObject>, UGorgeousObjectVariable*> Value;
};

UCLASS(DisplayName = "Soft Class Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftClass_MOTOV : public UMapObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(TSoftClassPtr<UObject>, Value, SoftObjectClass)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TMap<TSoftClassPtr<UObject>, UGorgeousObjectVariable*> Value;
};

UCLASS(DisplayName = "Byte Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UByte_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(uint8, Value, Byte)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TMap<uint8, UGorgeousObjectVariable*> Value;
};

UCLASS(DisplayName = "Float Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UFloat_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(double, Value, Float)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TMap<double, UGorgeousObjectVariable*> Value;
};

UCLASS(DisplayName = "Integer64 Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger64_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(int64, Value, Integer64)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TMap<int64, UGorgeousObjectVariable*> Value;
};

UCLASS(DisplayName = "Integer Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(int32, Value, Integer)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TMap<int32, UGorgeousObjectVariable*> Value;
};

UCLASS(DisplayName = "Name Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UName_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(FName, Value, Name)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TMap<FName, UGorgeousObjectVariable*> Value;
};

UCLASS(DisplayName = "String Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UString_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(FString, Value, String)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TMap<FString, UGorgeousObjectVariable*> Value;
};

UCLASS(DisplayName = "Transform Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UTransform_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(FTransform, Value, Transform)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TMap<FTransform, UGorgeousObjectVariable*> Value;
};

UCLASS(DisplayName = "Vector Map OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UVector_MOV : public UMapObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(FVector, Value, Vector)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TMap<FVector, UGorgeousObjectVariable*> Value;
};

// Set Object Variables
UCLASS(Abstract, NotBlueprintType)
class GORGEOUSCORERUNTIME_API USetObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
};

UCLASS(Abstract)
class GORGEOUSCORERUNTIME_API USetObjectTypeObjectVariable : public USetObjectVariable
{
    GENERATED_BODY()
};

UCLASS(DisplayName = "Object Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UObject_STOTOV : public USetObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<UObject*>, Value, ObjectObject)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TSet<UObject*> Value;
};

UCLASS(DisplayName = "Class Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UClass_STOTOV : public USetObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<UClass*>, Value, ObjectClass)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TSet<UClass*> Value;
};

UCLASS(DisplayName = "Soft Object Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftObject_STOTOV : public USetObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<TSoftObjectPtr<UObject>>, Value, SoftObjectObject)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TSet<TSoftObjectPtr<UObject>> Value;
};

UCLASS(DisplayName = "Soft Class Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API USoftClass_STOTOV : public USetObjectTypeObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<TSoftClassPtr<UObject>>, Value, SoftObjectClass)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TSet<TSoftClassPtr<UObject>> Value;
};


UCLASS(DisplayName = "Byte Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UByte_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<uint8>, Value, Byte)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TSet<uint8> Value;
};

UCLASS(DisplayName = "Float Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UFloat_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<double>, Value, Float)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TSet<double> Value;
};

UCLASS(DisplayName = "Integer64 Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger64_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<int64>, Value, Integer64)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TSet<int64> Value;
};

UCLASS(DisplayName = "Integer Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UInteger_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<int32>, Value, Integer)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TSet<int32> Value;
};

UCLASS(DisplayName = "Name Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UName_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<FName>, Value, Name)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TSet<FName> Value;
};

UCLASS(DisplayName = "String Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UString_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<FString>, Value, String)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TSet<FString> Value;
};

UCLASS(DisplayName = "Transform Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UTransform_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<FTransform>, Value, Transform)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TSet<FTransform> Value;
};

UCLASS(DisplayName = "Vector Set OV", Category = "Gorgeous Core|Gorgeous Object Variables")
class GORGEOUSCORERUNTIME_API UVector_STOV : public USetObjectVariable
{
    GENERATED_BODY()
    UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, TSet<FVector>, Value, Vector)
    UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
    TSet<FVector> Value;
};