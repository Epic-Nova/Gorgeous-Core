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
 * Shorthand for object variable array used in multi-map definitions.
 */
#define UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE FObjectVariableMultiMapValue

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
 * Declares the getter interface for a multi-map-based variable container.
 * 
 * @param VariableKeyType     The key type of the map (e.g., FString, int32).
 * @param VariableTypeName    The name used in the function.
 */
#define UE_DEFINE_OBJECT_VARIABLE_GET_MULTIMAP_INTERFACE(VariableKeyType, VariableTypeName) \
virtual TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Get##VariableTypeName##MultiMapObjectVariable_Implementation(const FName OptionalVariableName) const override

/**
 * Declares the setter interface for a multi-map-based variable container, passed by reference.
 * 
 * @param VariableKeyType     The key type of the map (e.g., FString, int32).
 * @param VariableTypeName    The name used in the function.
 */
#define UE_DEFINE_OBJECT_VARIABLE_SET_MULTIMAP_REFERENCE_INTERFACE(VariableKeyType, VariableTypeName) \
	virtual TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Set##VariableTypeName##MultiMapObjectVariable_Implementation(const FName OptionalVariableName, UPARAM(ref) TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE>& NewValue) override

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
 * Declares both the getter and setter interface methods for a multi-map-based variable container.
 * 
 * @param VariableKeyType     The key type of the map (e.g., FString, int32).
 * @param VariableTypeName    The name used in the function (e.g., Object, Float).
 */
#define UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_REFERENCE_INTERFACE(VariableKeyType, VariableTypeName) \
	UE_DEFINE_OBJECT_VARIABLE_GET_MULTIMAP_INTERFACE(VariableKeyType, VariableTypeName); \
	UE_DEFINE_OBJECT_VARIABLE_SET_MULTIMAP_REFERENCE_INTERFACE(VariableKeyType, VariableTypeName);

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
 * Returns a dynamically stored multi-map if a name is specified, otherwise returns the member multi-map variable.
 * 
 * @param VariableType      The key type of the map (e.g., FString, int32).
 * @param VariableName      The actual member multi-map to use if no dynamic map is found.
 */
#define UE_DEFINE_OBJECT_MULTIMAP_VARIABLE_GET(VariableType, VariableName) \
	{ \
		if (OptionalVariableName != FName("None")) \
		{ \
			TMap<VariableType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> OutValue; \
			if (GetDynamicProperty<TMap<VariableType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE>, FProperty>(OptionalVariableName, OutValue)) \
			{ \
				return OutValue; \
			} \
		} \
		return VariableName; \
	}

/**
 * Sets a dynamically stored multi-map if a name is specified, otherwise modifies the member multi-map variable.
 * 
 * @param VariableType      The key type of the map (e.g., FString, int32).
 * @param VariableName      The actual member multi-map to modify if no dynamic map is found.
 * @param NewValue          The new map value to set.
 */
#define UE_DEFINE_OBJECT_MULTIMAP_VARIABLE_SET(VariableType, VariableName, NewValue) \
	{ \
		if (OptionalVariableName != FName("None")) \
		{ \
			SetDynamicProperty<TMap<VariableType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE>, FProperty>(OptionalVariableName, NewValue); \
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
 * Implements getter and setter methods for multi-map-based variable containers.
 * 
 * @param VariableType      The key type of the map (e.g., FString, int32).
 * @param VariableName      The actual member multi-map variable.
 * @param VariableTypeName  The name used in the function (e.g., Object, Float).
 */
#define UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_INTERFACE_IMPLEMENTATION(VariableType, VariableName, VariableTypeName) \
	public: \
		UE_DEFINE_OBJECT_VARIABLE_GET_MULTIMAP_INTERFACE(VariableType, VariableTypeName) \
			UE_DEFINE_OBJECT_MULTIMAP_VARIABLE_GET(VariableType, VariableName) \
		UE_DEFINE_OBJECT_VARIABLE_SET_MULTIMAP_REFERENCE_INTERFACE(VariableType, VariableTypeName) \
			UE_DEFINE_OBJECT_MULTIMAP_VARIABLE_SET(VariableType, VariableName, NewValue)

/**
 * Implements queue object variable interfaces using native TQueue storage.
 */
#define UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_STORAGE(VariableType) \
	private: \
		using FQueueElementType = VariableType; \
		mutable TQueue<FQueueElementType, EQueueMode::Spsc> ValueSpsc; \
		mutable TQueue<FQueueElementType, EQueueMode::Mpsc> ValueMpsc; \
		mutable int32 ValueSpscCount = 0; \
		mutable int32 ValueMpscCount = 0; \
		FORCEINLINE bool UsesMpscQueue() const { return GetQueueMode() == EObjectVariableQueueMode_E::EMpsc; } \
		FORCEINLINE void ResetQueue(TQueue<FQueueElementType, EQueueMode::Spsc>& Queue, int32& Count) const \
		{ \
			FQueueElementType Temp; \
			while (Queue.Dequeue(Temp)) {} \
			Count = 0; \
		} \
		FORCEINLINE void ResetQueue(TQueue<FQueueElementType, EQueueMode::Mpsc>& Queue, int32& Count) const \
		{ \
			FQueueElementType Temp; \
			while (Queue.Dequeue(Temp)) {} \
			Count = 0; \
		} \
		FORCEINLINE TArray<VariableType> GetQueueAsArray() const \
		{ \
			TArray<VariableType> Result; \
			if (UsesMpscQueue()) \
			{ \
				Result.Reserve(ValueMpscCount); \
				FQueueElementType Temp; \
				for (int32 Index = 0; Index < ValueMpscCount; ++Index) \
				{ \
					if (ValueMpsc.Dequeue(Temp)) \
					{ \
						Result.Add(Temp); \
						ValueMpsc.Enqueue(Temp); \
					} \
				} \
			} \
			else \
			{ \
				Result.Reserve(ValueSpscCount); \
				FQueueElementType Temp; \
				for (int32 Index = 0; Index < ValueSpscCount; ++Index) \
				{ \
					if (ValueSpsc.Dequeue(Temp)) \
					{ \
						Result.Add(Temp); \
						ValueSpsc.Enqueue(Temp); \
					} \
				} \
			} \
			return Result; \
		} \
		FORCEINLINE void SetQueueFromArray(const TArray<VariableType>& NewValue) const \
		{ \
			if (UsesMpscQueue()) \
			{ \
				ResetQueue(ValueMpsc, ValueMpscCount); \
				for (int32 i = 0; i < NewValue.Num(); ++i) \
				{ \
					FQueueElementType EntryCopy = (FQueueElementType)NewValue[i]; \
					if (ValueMpsc.Enqueue(EntryCopy)) \
					{ \
						++ValueMpscCount; \
					} \
				} \
			} \
			else \
			{ \
				ResetQueue(ValueSpsc, ValueSpscCount); \
				for (int32 i = 0; i < NewValue.Num(); ++i) \
				{ \
					FQueueElementType EntryCopy = (FQueueElementType)NewValue[i]; \
					if (ValueSpsc.Enqueue(EntryCopy)) \
					{ \
						++ValueSpscCount; \
					} \
				} \
			} \
		}

#define UE_DEFINE_OBJECT_VARIABLE_QUEUE_NATIVE_INTERFACE(VariableType, VariableTypeName) \
	public: \
		virtual TArray<VariableType> Get##VariableTypeName##QueueObjectVariable_Implementation(const FName OptionalVariableName) const override \
		{ \
			if (OptionalVariableName != NAME_None) \
			{ \
				TArray<VariableType> OutValue; \
				if (GetDynamicProperty<TArray<VariableType>, FProperty>(OptionalVariableName, OutValue)) \
				{ \
					return OutValue; \
				} \
			} \
			return GetQueueAsArray(); \
		} \
		virtual TArray<VariableType> Set##VariableTypeName##QueueObjectVariable_Implementation(const FName OptionalVariableName, TArray<VariableType>& NewValue) override \
		{ \
			if (OptionalVariableName != NAME_None) \
			{ \
				SetDynamicProperty<TArray<VariableType>, FProperty>(OptionalVariableName, NewValue); \
				return NewValue; \
			} \
			SetQueueFromArray(NewValue); \
			return NewValue; \
		}

/**
 * Implements stack object variable interfaces using native TStack storage.
 */
#define UE_DEFINE_OBJECT_VARIABLE_STACK_NATIVE_INTERFACE(VariableType, VariableTypeName) \
	public: \
		virtual TArray<VariableType> Get##VariableTypeName##StackObjectVariable_Implementation(const FName OptionalVariableName) const override \
		{ \
			if (OptionalVariableName != NAME_None) \
			{ \
				TArray<VariableType> OutValue; \
				if (GetDynamicProperty<TArray<VariableType>, FProperty>(OptionalVariableName, OutValue)) \
				{ \
					return OutValue; \
				} \
			} \
			return Value; \
		} \
		virtual TArray<VariableType> Set##VariableTypeName##StackObjectVariable_Implementation(const FName OptionalVariableName, TArray<VariableType>& NewValue) override \
		{ \
			if (OptionalVariableName != NAME_None) \
			{ \
				SetDynamicProperty<TArray<VariableType>, FProperty>(OptionalVariableName, NewValue); \
				return NewValue; \
			} \
			Value = NewValue; \
			return Value; \
		} \
	private: \
		TStack<VariableType> Value;

/**
 * Implements deque object variable interfaces using native TDeque storage.
 */
#define UE_DEFINE_OBJECT_VARIABLE_DEQUE_NATIVE_INTERFACE(VariableType, VariableTypeName) \
	public: \
		using FDequeElementType = VariableType; \
		virtual TArray<VariableType> Get##VariableTypeName##DequeObjectVariable_Implementation(const FName OptionalVariableName) const override \
		{ \
			if (OptionalVariableName != NAME_None) \
			{ \
				TArray<VariableType> OutValue; \
				if (GetDynamicProperty<TArray<VariableType>, FProperty>(OptionalVariableName, OutValue)) \
				{ \
					return OutValue; \
				} \
			} \
			TArray<VariableType> Result; \
			Result.Reserve(Value.Num()); \
			for (int32 i = 0; i < Value.Num(); ++i) \
			{ \
				Result.Add((VariableType)Value[i]); \
			} \
			return Result; \
		} \
		virtual TArray<VariableType> Set##VariableTypeName##DequeObjectVariable_Implementation(const FName OptionalVariableName, TArray<VariableType>& NewValue) override \
		{ \
			if (OptionalVariableName != NAME_None) \
			{ \
				SetDynamicProperty<TArray<VariableType>, FProperty>(OptionalVariableName, NewValue); \
				return NewValue; \
			} \
			Value.Empty(); \
			for (int32 i = 0; i < NewValue.Num(); ++i) \
			{ \
				FDequeElementType EntryCopy = (FDequeElementType)NewValue[i]; \
				Value.PushLast(EntryCopy); \
			} \
			return NewValue; \
		} \
	private: \
		TDeque<FDequeElementType> Value;

/**
 * Implements multi-map object variable interfaces using native TMultiMap storage.
 */
#define UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_NATIVE_INTERFACE(VariableKeyType, VariableTypeName) \
	public: \
		virtual TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Get##VariableTypeName##MultiMapObjectVariable_Implementation(const FName OptionalVariableName) const override \
		{ \
			if (OptionalVariableName != NAME_None) \
			{ \
				TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> OutValue; \
				if (GetDynamicProperty<TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE>, FProperty>(OptionalVariableName, OutValue)) \
				{ \
					return OutValue; \
				} \
			} \
			TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Result; \
			for (const TPair<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE>& Pair : Value) \
			{ \
				Result.FindOrAdd(Pair.Key).Values.Add(Pair.Value); \
			} \
			return Result; \
		} \
		virtual TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> Set##VariableTypeName##MultiMapObjectVariable_Implementation(const FName OptionalVariableName, TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE>& NewValue) override \
		{ \
			if (OptionalVariableName != NAME_None) \
			{ \
				SetDynamicProperty<TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE>, FProperty>(OptionalVariableName, NewValue); \
				return NewValue; \
			} \
			Value.Empty(); \
			for (const TPair<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE>& Pair : NewValue) \
			{ \
				for (UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE Entry : Pair.Value.Values) \
				{ \
					Value.Add(Pair.Key, Entry); \
				} \
			} \
			return NewValue; \
		} \
	private: \
		TMultiMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MAP_VALUE> Value;

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
 * Declares default multiple variable interface implementations for custom container return types.
 * 
 * @param ContainerType      The container type name suffix (e.g., Queue, Stack).
 * @param VariableType       The full container type (e.g., TArray<int32>).
 * @param VariableTypeName   The function name segment.
 */
#define UE_DECLARE_OBJECT_VARIABLE_DEFAULT_CUSTOM_MULTIPLE_INTERFACE_IMPLEMENTATION(ContainerType, VariableType, VariableTypeName) \
	VariableType UGorgeousObjectVariable::Get##VariableTypeName##ContainerType##ObjectVariable_Implementation(const FName OptionalVariableName) const \
	{ \
		return VariableType(); \
	} \
	VariableType UGorgeousObjectVariable::Set##VariableTypeName##ContainerType##ObjectVariable_Implementation(const FName OptionalVariableName, VariableType& NewValue) \
	{ \
		return VariableType(); \
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
 * Declares default multi-map variable interface implementations that always return an empty map.
 * 
 * @param VariableKeyType      The key type of the map (e.g., FString, int32).
 * @param VariableTypeName     The function name segment.
 */
#define UE_DECLARE_OBJECT_VARIABLE_DEFAULT_MULTIMAP_INTERFACE_IMPLEMENTATION(VariableKeyType, VariableTypeName) \
	TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> UGorgeousObjectVariable::Get##VariableTypeName##MultiMapObjectVariable_Implementation(const FName OptionalVariableName) const \
	{ \
		return TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE>(); \
	} \
	TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE> UGorgeousObjectVariable::Set##VariableTypeName##MultiMapObjectVariable_Implementation(const FName OptionalVariableName, TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE>& NewValue) \
	{ \
		return TMap<VariableKeyType, UE_DEFINE_OBJECT_VARIABLE_MULTIMAP_VALUE>(); \
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
	UE_DECLARE_OBJECT_VARIABLE_DEFAULT_CUSTOM_MULTIPLE_INTERFACE_IMPLEMENTATION(Queue, TArray<VariableType>, VariableTypeName) \
	UE_DECLARE_OBJECT_VARIABLE_DEFAULT_CUSTOM_MULTIPLE_INTERFACE_IMPLEMENTATION(Stack, TArray<VariableType>, VariableTypeName) \
	UE_DECLARE_OBJECT_VARIABLE_DEFAULT_CUSTOM_MULTIPLE_INTERFACE_IMPLEMENTATION(Deque, TArray<VariableType>, VariableTypeName) \
	UE_DECLARE_OBJECT_VARIABLE_DEFAULT_MAP_INTERFACE_IMPLEMENTATION(VariableType, VariableTypeName) \
	UE_DECLARE_OBJECT_VARIABLE_DEFAULT_MULTIMAP_INTERFACE_IMPLEMENTATION(VariableType, VariableTypeName) \
	UE_DECLARE_OBJECT_VARIABLE_DEFAULT_MULTIPLE_INTERFACE_IMPLEMENTATION(Set, VariableType, VariableTypeName)

/**
 * Registers the "Value" property for auto-replication when the mixin activates replication.
 * Overrides OnReplicationActivated_Implementation so the registration happens at the correct
 * time, after ActivateReplication has set up the mixin context, rather than at object
 * construction time (PostInitProperties) when bReplicates is not yet true.
 */
#define UE_SETUP_OBJECT_VARIABLE_AUTO_REPLICATION \
	virtual void OnReplicationActivated_Implementation(const FGorgeousAutoReplicationContext& Context) override \
	{ \
		Super::OnReplicationActivated_Implementation(Context); \
		RegisterReplicatedProperty("Value"); \
	}