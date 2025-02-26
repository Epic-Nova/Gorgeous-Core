// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|      that is has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/
#pragma once

#define UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE UGorgeousObjectVariable*

#define UE_DEFINE_OBJECT_VARIABLE_GET_MULTIPLE_INTERFACE(VariableType, VariableTypeName, VariableNumberName) \
		virtual VariableType Get##VariableTypeName##VariableNumberName##ObjectVariable_Implementation() const override

#define UE_DEFINE_OBJECT_VARIABLE_SET_MULTIPLE_REFERENCE_INTERFACE(VariableType, VariableTypeName, VariableNumberName) \
		virtual VariableType Set##VariableTypeName##VariableNumberName##ObjectVariable_Implementation(UPARAM(ref) VariableType& NewValue) override

#define UE_DEFINE_OBJECT_VARIABLE_GET_MAP_INTERFACE(VariableKeyType, VariableTypeName) \
virtual TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE> Get##VariableTypeName##MapObjectVariable_Implementation() const override

#define UE_DEFINE_OBJECT_VARIABLE_SET_MAP_REFERENCE_INTERFACE(VariableKeyType, VariableTypeName) \
	virtual TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE> Set##VariableTypeName##MapObjectVariable_Implementation(UPARAM(ref) TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE>& NewValue) override

#define UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(VariableType, VariableTypeName, VariableNumberName) \
		UE_DEFINE_OBJECT_VARIABLE_GET_MULTIPLE_INTERFACE(VariableType, VariableTypeName, VariableNumberName); \
		UE_DEFINE_OBJECT_VARIABLE_SET_MULTIPLE_REFERENCE_INTERFACE(VariableType, VariableTypeName, VariableNumberName);

#define UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(VariableKeyType, VariableTypeName) \
	UE_DEFINE_OBJECT_VARIABLE_GET_MAP_INTERFACE(VariableKeyType, VariableTypeName); \
	UE_DEFINE_OBJECT_VARIABLE_SET_MAP_REFERENCE_INTERFACE(VariableKeyType, VariableTypeName);

#define UE_DEFINE_OBJECT_SINGLE_VARIABLE_GET(VariableName) \
	{ \
		return VariableName; \
	}

#define UE_DEFINE_OBJECT_SINGLE_VARIABLE_SET(VariableName, NewValue) \
	{ \
		VariableName = NewValue; \
		return VariableName; \
	}

#define UE_DEFINE_OBJECT_MULTIPLE_VARIABLE_GET(VariableName) \
	{ \
		return VariableName; \
	}

#define UE_DEFINE_OBJECT_MULTIPLE_VARIABLE_SET(VariableName, NewValue) \
	{ \
		VariableName = NewValue; \
		return VariableName; \
	}

#define UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION(VariableType, VariableName, VariableTypeName) \
	public: \
		UE_DEFINE_OBJECT_VARIABLE_GET_MULTIPLE_INTERFACE(VariableType, VariableTypeName, Single) \
			UE_DEFINE_OBJECT_SINGLE_VARIABLE_GET(VariableName) \
		UE_DEFINE_OBJECT_VARIABLE_SET_MULTIPLE_REFERENCE_INTERFACE(VariableType, VariableTypeName, Single) \
			UE_DEFINE_OBJECT_SINGLE_VARIABLE_SET(VariableName, NewValue)

#define UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION(ContainerType, VariableType, VariableName, VariableTypeName) \
	public: \
		UE_DEFINE_OBJECT_VARIABLE_GET_MULTIPLE_INTERFACE(T##ContainerType<VariableType>, VariableTypeName, ContainerType) \
			UE_DEFINE_OBJECT_MULTIPLE_VARIABLE_GET(VariableName) \
		UE_DEFINE_OBJECT_VARIABLE_SET_MULTIPLE_REFERENCE_INTERFACE(T##ContainerType<VariableType>, VariableTypeName, ContainerType) \
			UE_DEFINE_OBJECT_MULTIPLE_VARIABLE_SET(VariableName, NewValue)

#define UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION(VariableType, VariableName, VariableTypeName) \
	public: \
		UE_DEFINE_OBJECT_VARIABLE_GET_MAP_INTERFACE(VariableType, VariableTypeName) \
			UE_DEFINE_OBJECT_MULTIPLE_VARIABLE_GET(VariableName) \
		UE_DEFINE_OBJECT_VARIABLE_SET_MAP_REFERENCE_INTERFACE(VariableType, VariableTypeName) \
			UE_DEFINE_OBJECT_MULTIPLE_VARIABLE_SET(VariableName, NewValue)




#define UE_DECLARE_OBJECT_VARIABLE_DEFAULT_SINGLE_INTERFACE_IMPLEMENTATION(VariableType, VariableTypeName, ShouldReturn) \
	VariableType UGorgeousObjectVariable::Get##VariableTypeName##SingleObjectVariable_Implementation() const \
	{ \
		return ShouldReturn; \
	} \
	VariableType UGorgeousObjectVariable::Set##VariableTypeName##SingleObjectVariable_Implementation(VariableType& NewValue) \
	{ \
		return ShouldReturn; \
	}

#define UE_DECLARE_OBJECT_VARIABLE_DEFAULT_MULTIPLE_INTERFACE_IMPLEMENTATION(ContainerType, VariableType, VariableTypeName) \
	T##ContainerType<VariableType> UGorgeousObjectVariable::Get##VariableTypeName##ContainerType##ObjectVariable_Implementation() const \
	{ \
	return T##ContainerType<VariableType>(); \
	} \
	T##ContainerType<VariableType> UGorgeousObjectVariable::Set##VariableTypeName##ContainerType##ObjectVariable_Implementation(T##ContainerType<VariableType>& NewValue) \
	{ \
	return T##ContainerType<VariableType>(); \
	}

#define UE_DEFINE_OBJECT_VARIABLE_DEFAULT_MAP_INTERFACE_IMPLEMENTATION(VariableKeyType, VariableTypeName) \
	TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE> UGorgeousObjectVariable::Get##VariableTypeName##MapObjectVariable_Implementation() const \
	{ \
		return TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE>(); \
	} \
	TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE> UGorgeousObjectVariable::Set##VariableTypeName##MapObjectVariable_Implementation(TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE>& NewValue) \
	{ \
		return TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE>(); \
	}

#define UE_DECLARE_OBJECT_VARIABLE_DEFAULT_GENERAL_INTERFACE_IMPLEMENTATION(VariableType, VariableTypeName, ShouldReturnForSingle) \
	UE_DECLARE_OBJECT_VARIABLE_DEFAULT_SINGLE_INTERFACE_IMPLEMENTATION(VariableType, VariableTypeName, ShouldReturnForSingle) \
	UE_DECLARE_OBJECT_VARIABLE_DEFAULT_MULTIPLE_INTERFACE_IMPLEMENTATION(Array, VariableType, VariableTypeName) \
	UE_DEFINE_OBJECT_VARIABLE_DEFAULT_MAP_INTERFACE_IMPLEMENTATION(VariableType, VariableTypeName) \
	UE_DECLARE_OBJECT_VARIABLE_DEFAULT_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, VariableType, VariableTypeName)

