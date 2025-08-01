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

/**
 * Invokes the instanced functionality of each element in the AdditionalGorgeousData container.
 * 
 * This macro iterates over the AdditionalGorgeousData container and invokes the function 
 * `InvokeInstancedFunctionality` for each value, passing the unique identifier of that value.
 * 
 * @note This macro is useful for initializing or triggering functionality for each data element 
 * in the AdditionalGorgeousData array.
 */
#define UE_DECLARE_QOF_CLASS_INIT_INVOKE_ADDITIONAL_DATA \
	for (const auto GorgeousSetting : AdditionalGorgeousData) \
	{ \
		GorgeousSetting.Value->InvokeInstancedFunctionality(GorgeousSetting.Value->UniqueIdentifier); \
	}

/**
 * Handles property changes for the class during the editor post-edit phase.
 * 
 * This macro implements the `PostEditChangeProperty` function. It checks if the property that was changed
 * is `AdditionalGorgeousData`, and if so, handles array additions by updating the `UniqueIdentifier` of
 * the last added element to a new GUID.
 * 
 * @param Class The class for which the property changes are handled.
 * 
 * @note This macro ensures that when new items are added to the `AdditionalGorgeousData` array, each new item
 * receives a unique identifier, which is critical for maintaining individual identity during further processing.
 */
#define UE_DECLARE_QOF_CLASS_POST_EDIT_CHANGE_PROPERTY(Class) \
	void Class::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) \
	{ \
		Super::PostEditChangeProperty(PropertyChangedEvent); \
		const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None; \
		{ \
			if (PropertyName == GET_MEMBER_NAME_CHECKED(Class, AdditionalGorgeousData) && PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet) \
			{ \
				TArray<UGorgeousObjectVariable*> ObjectVariables; \
				AdditionalGorgeousData.GenerateValueArray(ObjectVariables); \
				if (ObjectVariables.Last()) \
				{ \
					ObjectVariables.Last()->UniqueIdentifier = FGuid::NewGuid(); \
				} \
			} \
		} \
	}
