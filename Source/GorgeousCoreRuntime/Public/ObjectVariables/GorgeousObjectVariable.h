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
#include "GorgeousCoreUtilitiesMinimalShared.h"
#include "Interfaces/GorgeousObjectVariableInteraction_I.h"
#include "ObjectVariables/Helpers/GorgeousObjectVariableHelperMacros.h"
#include "ObjectVariables/Interfaces/GorgeousSingleObjectVariablesGetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousSingleObjectVariablesSetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousArrayObjectVariablesGetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousArrayObjectVariablesSetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousMapObjectVariablesGetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousMapObjectVariablesSetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousSetObjectVariablesGetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousSetObjectVariablesSetter_I.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousObjectVariable.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Base class for defining variables as objects within the Gorgeous Things ecosystem.
 *
 * This class provides a flexible and extensible way to represent variables as UObjects,
 * allowing for dynamic data storage and manipulation. Child classes can leverage this
 * foundation to create specialized variable types with network replication capabilities.
 *
 * Key features include:
 * - Support for single, array, map, and set variable types.
 * - Integration with interfaces for getter and setter operations.
 * - Dynamic property management using templates.
 * - Registry for tracking object variables.
 * - Persistence options for level transitions.
 * - Unique identifier generation for each object variable.
 *
 * @note This class serves as the cornerstone for managing variables in the Gorgeous Things system,
 * enabling a more object-oriented approach to variable handling.
 *
 */
UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, ClassGroup = "Gorgeous Core|Gorgeous Object Variables", DisplayName = "Gorgeous Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables",
	meta = (ToolTip = "Used for providing a more interactive way to define variables in object form.", ShortTooltip = "The base class for all object variables."))
class GORGEOUSCORERUNTIME_API UGorgeousObjectVariable : public UGorgeousBaseWorldContextUObject,
public IGorgeousObjectVariableInteraction_I,
public IGorgeousSingleObjectVariablesGetter_I, public IGorgeousSingleObjectVariablesSetter_I,
public IGorgeousArrayObjectVariablesGetter_I, public IGorgeousArrayObjectVariablesSetter_I,
public IGorgeousMapObjectVariablesGetter_I, public IGorgeousMapObjectVariablesSetter_I,
public IGorgeousSetObjectVariablesGetter_I, public IGorgeousSetObjectVariablesSetter_I
{
	GENERATED_BODY()

protected:
	
	/** The Class Constructor for the Base Object Variable is used to set Default Values. */
	UGorgeousObjectVariable();

	// Sets up this object variable to be supported for networking.
	virtual bool IsSupportedForNetworking() const override { return true; }

public:

	/**
	 * Constructs a new object variable and registers it within the given registry depending on the parent given.
	 *
	 * @param Class The class that the object variable should derive from.
	 * @param Identifier The unique identifier of the object variable.
	 * @param Parent The parent of this object variable. The chain can be followed up to the root object variable.
	 * @param bShouldPersist Weather this object variable should be persistent across level switches.
	 * @return A new variable in object format.
	 *
	 * //@TODO: UGorgeousEvent is appearing here as it is also a object variable, we need to filter it out as the construction is handled differently
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables", meta = (DeterminesOutputType = "Class"))
	UGorgeousObjectVariable* NewObjectVariable(TSubclassOf<UGorgeousObjectVariable> Class, FGuid& Identifier, UGorgeousObjectVariable* Parent = nullptr, bool bShouldPersist = false);

	/**
	 * Instantiates a new object variable of the specified class as transactional and registers it as a child of the given Parent for persistence across editor sessions.
	 *
	 * If no parent is specified, the root object variable will be used as the default parent.
	 * The created object is marked as transactional and assigned a unique identifier.
	 *
	 * @param Class The class type to instantiate. Must be a subclass of UGorgeousObjectVariable.
	 * @param Parent The optional parent object variable. If null, the root object variable is used instead.
	 * @return A pointer to the newly instantiated UGorgeousObjectVariable, or nullptr if instantiation failed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables", meta = (DeterminesOutputType = "Class"))
	UGorgeousObjectVariable* InstantiateTransactionalObjectVariable(TSubclassOf<UGorgeousObjectVariable> Class, UGorgeousObjectVariable* Parent = nullptr);

	/**
	 * Invokes the instanced functionality for when the ObjectVariable is contained inside a UPROPERTY with the Instanced meta specifier.
	 * 
	 * @param NewUniqueIdentifier The new unique identifier.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables")
	virtual void InvokeInstancedFunctionality(FGuid NewUniqueIdentifier);
	
    /**
     * Registers the object variable with the registry.
     *
     * @param NewObjectVariable The object variable to register.
     */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables")
    virtual void RegisterWithRegistry(UGorgeousObjectVariable* NewObjectVariable);

	/**
	 * Sets the new parent oft this object variable.
	 * 
	 * @param NewParent The new parent of the object variable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables")
	void SetParent(UGorgeousObjectVariable* NewParent);
	
    /**
     * Sets a dynamic property of the object variable.
     *
     * @tparam InTCppType The C++ type of the property.
     * @tparam TInPropertyBaseClass The base class of the property.
     * @param PropertyName The name of the property.
     * @param Value The value to set.
     */
//grepper cpp unreal wildcard
	//Gorgeous-Core on [GitHub](https://github.com/Epic-Nova/Gorgeous-Core)
    template<typename InTCppType, typename TInPropertyBaseClass>
    void SetDynamicProperty(const FName PropertyName, const InTCppType& Value)
    {
    	if (FProperty* Property = FindFProperty<FProperty>(GetClass(), PropertyName); PropertyName.IsValid())
    	{
    		// Handle UObject* properties separately
    		if constexpr (std::is_base_of_v<UObject, InTCppType>)
    		{
    			if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
    			{
    				ObjectProperty->SetObjectPropertyValue_InContainer(this, Value);
    			}
    		}
    		else if (Property && Property->IsA<TProperty<InTCppType, TInPropertyBaseClass>>())
    		{
    			if (TProperty<InTCppType, TInPropertyBaseClass>* TypedProperty = CastField<TProperty<InTCppType, TInPropertyBaseClass>>(Property))
    			{
    				TypedProperty->SetPropertyValue_InContainer(this, Value);
    			}
    		}
    	}
    }
//end grepper
	
    /**
     * Gets a dynamic property of the object variable.
     *
     * @tparam InTCppType The C++ type of the property.
     * @tparam TInPropertyBaseClass The base class of the property.
     * @param PropertyName The name of the property.
     * @param OutValue The output value of the property.
     * @return True if the property was successfully retrieved, false otherwise.
     */
//grepper cpp unreal wildcard
	//Gorgeous-Core on [GitHub](https://github.com/Epic-Nova/Gorgeous-Core)
    template<typename InTCppType, typename TInPropertyBaseClass>
    bool GetDynamicProperty(const FName PropertyName, InTCppType& OutValue) const
    {
    	if (FProperty* Property = FindFProperty<FProperty>(GetClass(), PropertyName); Property && PropertyName.IsValid())
    	{
    		// Handle UObject* properties separately
    		if constexpr (std::is_base_of_v<UObject, InTCppType>)
    		{
    			if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
    			{
    				OutValue = Cast<InTCppType>(ObjectProperty->GetObjectPropertyValue_InContainer(this));
    				return true;
    			}
    		}
    		else if (Property && Property->IsA<TProperty<InTCppType, TInPropertyBaseClass>>())
    		{
    			if (const TProperty<InTCppType, TInPropertyBaseClass>* TypedProperty = CastField<TProperty<InTCppType, TInPropertyBaseClass>>(Property))
    			{
    				OutValue = TypedProperty->GetPropertyValue_InContainer(this);
    				return true;
    			}
    		}
    	}
    	return false;
    }
//end grepper
	
	/**
	 * The unique identifier of the object variable.
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Object Variable")
	FGuid UniqueIdentifier;

	/**
	 * The registry of object variables.
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Object Variable")
	TArray<TObjectPtr<UGorgeousObjectVariable>> VariableRegistry;

	/**
	 * Whether the object variable is persistent across level switches.
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Object Variable")
	bool bPersistent;


protected:

	/**
	 * The parent of the object variable.
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gorgeous Object Variable")
	UGorgeousObjectVariable* Parent;
	
private:
	
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(UGorgeousObjectVariable*, ObjectVariable, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(UObject*, ObjectObject, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(UClass*, ObjectClass, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSoftObjectPtr<UObject>, SoftObjectObject, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSoftClassPtr<UObject>, SoftObjectClass, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(bool, Boolean, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(uint8, Byte, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(double, Float, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(int64, Integer64, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(int32, Integer, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FName, Name, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FRotator, Rotator, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FString, String, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FText, Text, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FTransform, Transform, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FVector, Vector, Single)

	
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<UGorgeousObjectVariable*>, ObjectVariable, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<UObject*>, ObjectObject, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<UClass*>, ObjectClass, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<TSoftObjectPtr<UObject>>, SoftObjectObject, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<TSoftClassPtr<UObject>>, SoftObjectClass, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<bool>, Boolean, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<uint8>, Byte, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<double>, Float, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<int64>, Integer64, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<int32>, Integer, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FName>, Name, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FRotator>, Rotator, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FString>, String, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FText>, Text, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FTransform>, Transform, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FVector>, Vector, Array)

	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(UGorgeousObjectVariable*, ObjectVariable)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(UObject*, ObjectObject)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(UClass*, ObjectClass)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(TSoftObjectPtr<UObject>, SoftObjectObject)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(TSoftClassPtr<UObject>, SoftObjectClass)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(uint8, Byte)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(double, Float)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(int64, Integer64)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(int32, Integer)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(FName, Name)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(FString, String)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(FTransform, Transform)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(FVector, Vector)

	
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<UGorgeousObjectVariable*>, ObjectVariable, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<UObject*>, ObjectObject, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<UClass*>, ObjectClass, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<TSoftObjectPtr<UObject>>, SoftObjectObject, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<TSoftClassPtr<UObject>>, SoftObjectClass, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<uint8>, Byte, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<double>, Float, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<int64>, Integer64, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<int32>, Integer, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<FName>, Name, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<FString>, String, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<FTransform>, Transform, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<FVector>, Vector, Set)

	/**
	 * Gets the unique identifier for an object variable.
	 *
	 * @return The unique identifier as an FGuid.
	 */
	virtual FGuid GetUniqueIdentifierForObjectVariable_Implementation() override;
};


