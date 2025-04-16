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

/*======================================================================================================================>
| --------------------------------------------------------------------------------------------------------------------- |
|													MACRO HELL															|
| --------------------------------------------------------------------------------------------------------------------- |
<======================================================================================================================*/

/**
 * Shorthand for object variable pointer used in map definitions.
 */
#define UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE UGorgeousObjectVariable*

/**
 * Declares the getter interface method for a variable with a dynamic name and multiple instances.
 * 
 * @param VariableType       The C++ type of the variable.
 * @param VariableTypeName   The name used in the function (e.g., Bool, Float).
 * @param VariableNumberName The suffix indicating single/multiple (e.g., Single, Array).
 */
#define UE_DEFINE_OBJECT_VARIABLE_GET_MULTIPLE_INTERFACE(VariableType, VariableTypeName, VariableNumberName) \
		virtual VariableType Get##VariableTypeName##VariableNumberName##ObjectVariable_Implementation(const FName OptionalVariableName) const override

/**
 * Declares the setter interface method for a variable with a dynamic name and multiple instances, passed by reference.
 * 
 * @param VariableType       The C++ type of the variable.
 * @param VariableTypeName   The name used in the function (e.g., Bool, Float).
 * @param VariableNumberName The suffix indicating single/multiple (e.g., Single, Array).
 */
#define UE_DEFINE_OBJECT_VARIABLE_SET_MULTIPLE_REFERENCE_INTERFACE(VariableType, VariableTypeName, VariableNumberName) \
		virtual VariableType Set##VariableTypeName##VariableNumberName##ObjectVariable_Implementation(const FName OptionalVariableName, UPARAM(ref) VariableType& NewValue) override

/**
 * Declares the getter interface for a map-based variable container.
 * 
 * @param VariableKeyType     The key type of the map (e.g., FString, int32).
 * @param VariableTypeName    The name used in the function.
 */
#define UE_DEFINE_OBJECT_VARIABLE_GET_MAP_INTERFACE(VariableKeyType, VariableTypeName) \
virtual TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE> Get##VariableTypeName##MapObjectVariable_Implementation(const FName OptionalVariableName) const override

/**
 * Declares the setter interface for a map-based variable container, passed by reference.
 * 
 * @param VariableKeyType     The key type of the map (e.g., FString, int32).
 * @param VariableTypeName    The name used in the function.
 */
#define UE_DEFINE_OBJECT_VARIABLE_SET_MAP_REFERENCE_INTERFACE(VariableKeyType, VariableTypeName) \
	virtual TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE> Set##VariableTypeName##MapObjectVariable_Implementation(const FName OptionalVariableName, UPARAM(ref) TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE>& NewValue) override

/**
 * Declares both the getter and setter interface methods for a variable with multiple values (e.g., arrays or sets).
 * 
 * @param VariableType       The C++ type of the variable (e.g., bool, float).
 * @param VariableTypeName   The name used in the function (e.g., Bool, Float).
 * @param VariableNumberName The suffix indicating single/multiple (e.g., Array, Set).
 */
#define UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(VariableType, VariableTypeName, VariableNumberName) \
		UE_DEFINE_OBJECT_VARIABLE_GET_MULTIPLE_INTERFACE(VariableType, VariableTypeName, VariableNumberName); \
		UE_DEFINE_OBJECT_VARIABLE_SET_MULTIPLE_REFERENCE_INTERFACE(VariableType, VariableTypeName, VariableNumberName);

/**
 * Declares both the getter and setter interface methods for a map-based variable container.
 * 
 * @param VariableKeyType     The key type of the map (e.g., FString, int32).
 * @param VariableTypeName    The name used in the function (e.g., Float, Object).
 */
#define UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(VariableKeyType, VariableTypeName) \
	UE_DEFINE_OBJECT_VARIABLE_GET_MAP_INTERFACE(VariableKeyType, VariableTypeName); \
	UE_DEFINE_OBJECT_VARIABLE_SET_MAP_REFERENCE_INTERFACE(VariableKeyType, VariableTypeName);

/**
 * Returns a dynamically stored variable if a name is specified, otherwise returns the member variable.
 * 
 * @param VariableType      The C++ type of the variable.
 * @param VariableName      The actual member variable to use if no dynamic variable is found.
 */
#define UE_DEFINE_OBJECT_MULTIPLE_VARIABLE_GET(VariableType, VariableName) \
	{ \
		if (OptionalVariableName != FName("None")) \
		{ \
			VariableType OutValue; \
			if (GetDynamicProperty<VariableType, FProperty>(OptionalVariableName, OutValue)) \
			{ \
				return OutValue; \
			} \
		} \
		return VariableName; \
	}

/**
 * Sets a dynamically stored variable if a name is specified, otherwise modifies the member variable.
 * 
 * @param VariableType      The C++ type of the variable.
 * @param VariableName      The actual member variable to modify if no dynamic variable is found.
 * @param NewValue          The new value to set.
 */
#define UE_DEFINE_OBJECT_MULTIPLE_VARIABLE_SET(VariableType, VariableName, NewValue) \
	{ \
		if (OptionalVariableName != FName("None")) \
		{ \
			SetDynamicProperty<VariableType, FProperty>(OptionalVariableName, NewValue); \
		} \
		else \
		{ \
			VariableName = NewValue; \
		} \
		return VariableName; \
	}

/**
 * Returns a dynamically stored map if a name is specified, otherwise returns the member map variable.
 * 
 * @param VariableType      The key type of the map (e.g., FString, int32).
 * @param VariableName      The actual member map to use if no dynamic map is found.
 */
#define UE_DEFINE_OBJECT_MAP_VARIABLE_GET(VariableType, VariableName) \
	{ \
		if (OptionalVariableName != FName("None")) \
		{ \
			TMap<VariableType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE> OutValue; \
			if (GetDynamicProperty<TMap<VariableType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE>, FProperty>(OptionalVariableName, OutValue)) \
			{ \
				return OutValue; \
			} \
		} \
		return VariableName; \
	}

/**
 * Sets a dynamically stored map if a name is specified, otherwise modifies the member map variable.
 * 
 * @param VariableType      The key type of the map (e.g., FString, int32).
 * @param VariableName      The actual member map to modify if no dynamic map is found.
 * @param NewValue          The new map value to set.
 */
#define UE_DEFINE_OBJECT_MAP_VARIABLE_SET(VariableType, VariableName, NewValue) \
	{ \
		if (OptionalVariableName != FName("None")) \
		{ \
			SetDynamicProperty<TMap<VariableType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE>, FProperty>(OptionalVariableName, NewValue); \
		} \
		else \
		{ \
			VariableName = NewValue; \
		} \
		return VariableName; \
	}

/**
 * Implements getter and setter methods for a single instance of a variable.
 * 
 * @param VariableType      The C++ type of the variable.
 * @param VariableName      The actual member variable.
 * @param VariableTypeName  The name used in the function (e.g., Float, Bool).
 */
#define UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(VariableType, VariableName, VariableTypeName) \
	public: \
		UE_DEFINE_OBJECT_VARIABLE_GET_MULTIPLE_INTERFACE(VariableType, VariableTypeName, Single) \
			UE_DEFINE_OBJECT_MULTIPLE_VARIABLE_GET(VariableType, VariableName) \
		UE_DEFINE_OBJECT_VARIABLE_SET_MULTIPLE_REFERENCE_INTERFACE(VariableType, VariableTypeName, Single) \
			UE_DEFINE_OBJECT_MULTIPLE_VARIABLE_SET(VariableType, VariableName, NewValue)

/**
 * Implements getter and setter methods for multiple instances of a variable (e.g., arrays, sets).
 * 
 * @param ContainerType     The type of container (e.g., Array, Set).
 * @param VariableType      The C++ type of the variable.
 * @param VariableName      The actual member container variable.
 * @param VariableTypeName  The name used in the function (e.g., Float, Bool).
 */
#define UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(ContainerType, VariableType, VariableName, VariableTypeName) \
	public: \
		UE_DEFINE_OBJECT_VARIABLE_GET_MULTIPLE_INTERFACE(VariableType, VariableTypeName, ContainerType) \
			UE_DEFINE_OBJECT_MULTIPLE_VARIABLE_GET(VariableType, VariableName) \
		UE_DEFINE_OBJECT_VARIABLE_SET_MULTIPLE_REFERENCE_INTERFACE(VariableType, VariableTypeName, ContainerType) \
			UE_DEFINE_OBJECT_MULTIPLE_VARIABLE_SET(VariableType, VariableName, NewValue)

/**
 * Implements getter and setter methods for map-based variable containers.
 * 
 * @param VariableType      The key type of the map (e.g., FString, int32).
 * @param VariableName      The actual member map variable.
 * @param VariableTypeName  The name used in the function (e.g., Object, Float).
 */
#define UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(VariableType, VariableName, VariableTypeName) \
	public: \
		UE_DEFINE_OBJECT_VARIABLE_GET_MAP_INTERFACE(VariableType, VariableTypeName) \
			UE_DEFINE_OBJECT_MAP_VARIABLE_GET(VariableType, VariableName) \
		UE_DEFINE_OBJECT_VARIABLE_SET_MAP_REFERENCE_INTERFACE(VariableType, VariableTypeName) \
			UE_DEFINE_OBJECT_MAP_VARIABLE_SET(VariableType, VariableName, NewValue)

/**
 * Declares default single variable interface implementations that always return a fallback value.
 * 
 * @param VariableType         The C++ type of the variable.
 * @param VariableTypeName     The function name segment.
 * @param ShouldReturn         The value to return in both getter and setter.
 */
#define UE_DECLARE_OBJECT_VARIABLE_DEFAULT_SINGLE_INTERFACE_IMPLEMENTATION(VariableType, VariableTypeName, ShouldReturn) \
	VariableType UGorgeousObjectVariable::Get##VariableTypeName##SingleObjectVariable_Implementation(const FName OptionalVariableName) const \
	{ \
		return ShouldReturn; \
	} \
	VariableType UGorgeousObjectVariable::Set##VariableTypeName##SingleObjectVariable_Implementation(const FName OptionalVariableName, VariableType& NewValue) \
	{ \
		return ShouldReturn; \
	}

/**
 * Declares default multiple variable interface implementations that always return an empty container.
 * 
 * @param ContainerType        The container type (e.g., Array, Set).
 * @param VariableType         The C++ type of the variable.
 * @param VariableTypeName     The function name segment.
 */
#define UE_DECLARE_OBJECT_VARIABLE_DEFAULT_MULTIPLE_INTERFACE_IMPLEMENTATION(ContainerType, VariableType, VariableTypeName) \
	T##ContainerType<VariableType> UGorgeousObjectVariable::Get##VariableTypeName##ContainerType##ObjectVariable_Implementation(const FName OptionalVariableName) const \
	{ \
		return T##ContainerType<VariableType>(); \
	} \
	T##ContainerType<VariableType> UGorgeousObjectVariable::Set##VariableTypeName##ContainerType##ObjectVariable_Implementation(const FName OptionalVariableName, T##ContainerType<VariableType>& NewValue) \
	{ \
		return T##ContainerType<VariableType>(); \
	}

/**
 * Declares default map variable interface implementations that always return an empty map.
 * 
 * @param VariableKeyType      The key type of the map (e.g., FString, int32).
 * @param VariableTypeName     The function name segment.
 */
#define UE_DECLARE_OBJECT_VARIABLE_DEFAULT_MAP_INTERFACE_IMPLEMENTATION(VariableKeyType, VariableTypeName) \
	TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE> UGorgeousObjectVariable::Get##VariableTypeName##MapObjectVariable_Implementation(const FName OptionalVariableName) const \
	{ \
		return TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE>(); \
	} \
	TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE> UGorgeousObjectVariable::Set##VariableTypeName##MapObjectVariable_Implementation(const FName OptionalVariableName, TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE>& NewValue) \
	{ \
		return TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE>(); \
	}

/**
 * Declares default interface implementations for single, multiple (array, set), and map object variables.
 * 
 * @param VariableType             The C++ type of the variable.
 * @param VariableTypeName         The function name segment.
 * @param ShouldReturnForSingle    The fallback value returned by the single getter/setter.
 */
#define UE_DECLARE_OBJECT_VARIABLE_DEFAULT_GENERAL_INTERFACE_IMPLEMENTATION(VariableType, VariableTypeName, ShouldReturnForSingle) \
	UE_DECLARE_OBJECT_VARIABLE_DEFAULT_SINGLE_INTERFACE_IMPLEMENTATION(VariableType, VariableTypeName, ShouldReturnForSingle) \
	UE_DECLARE_OBJECT_VARIABLE_DEFAULT_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, VariableType, VariableTypeName) \
	UE_DECLARE_OBJECT_VARIABLE_DEFAULT_MAP_INTERFACE_IMPLEMENTATION(VariableType, VariableTypeName) \
	UE_DECLARE_OBJECT_VARIABLE_DEFAULT_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, VariableType, VariableTypeName)

