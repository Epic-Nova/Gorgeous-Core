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

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "DataSchemaMapping/GorgeousDataSchemaMappingTypes.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class FProperty;
class UObject;
class UGorgeousDataSchemaMapping_DA;
class UStruct;
//<-------------------------------------------------->

/**
 * Runtime reflection mapper that applies source snapshots to target objects.
 * This class provides functionality to map data from a source snapshot to a target UObject based on a defined data schema mapping asset.
 */
class GORGEOUSCORERUNTIME_API FGorgeousDataSchemaMapper
{
public:
	
	/**
	 * Applies a source snapshot to a target object using the provided schema map.
	 * 
	 * @param SourceSnapshot The source snapshot containing the data to apply.
	 * @param TargetObject The object to which the data should be applied.
	 * @param SchemaMap The data schema mapping asset that defines how to map the source data to the target object.
	 * @param OutWarnings An array to collect any warnings that occur during the mapping process.
	 * @param OutErrors An array to collect any errors that occur during the mapping process.
	 * 
	 * @return true if the mapping was successful, false otherwise.
	 */
	static bool ApplySnapshotToObject(
		const FGorgeousDataSchemaSourceSnapshot_S& SourceSnapshot,
		UObject* TargetObject,
		const UGorgeousDataSchemaMapping_DA* SchemaMap,
		TArray<FString>& OutWarnings,
		TArray<FString>& OutErrors);

	/**
	 * Applies a single transform definition to a source value, producing an output value.
	 * 
	 * @param TransformDefinition The transform definition to apply.
	 * @param SourceValue The input value to transform.
	 * @param OutValue The output value after applying the transform.
	 * @param OutError An output parameter to receive any error message if the transform fails.
	 * 
	 * @return true if the transform was successfully applied, false otherwise.
	 */
	static bool ApplyTransformDefinition(
		const FGorgeousDataSchemaTransformDefinition_S& TransformDefinition,
		const FString& SourceValue,
		FString& OutValue,
		FString& OutError);

private:

	/**
	 * Resolves a property path on a given root object, returning the leaf property and its container pointer.
	 * 
	 * @param RootObject The root object from which to start resolving the property path.
	 * @param PropertyPath The dot-separated property path to resolve (e.g., "MeshComponent.Material").
	 * @param OutLeafProperty An output parameter to receive the resolved leaf property.
	 * @param OutLeafContainerPtr An output parameter to receive the pointer to the container of the leaf property.
	 * @param OutError An output parameter to receive any error message if the resolution fails.
	 * 
	 * @return true if the property path was successfully resolved, false otherwise.
	 */
	static bool ResolvePropertyPath(
		UObject* RootObject,
		const FString& PropertyPath,
		FProperty*& OutLeafProperty,
		void*& OutLeafContainerPtr,
		FString& OutError);

	/**
	 * Helper function to resolve a property path on a struct, used internally by ResolvePropertyPath.
	 * 
	 * @param CurrentStruct The current struct being traversed.
	 * @param CurrentContainerPtr The pointer to the current struct instance.
	 * @param Segments The segments of the property path being resolved.
	 * @param SegmentIndex The current index in the segments array.
	 * @param OutLeafProperty An output parameter to receive the resolved leaf property.
	 * @param OutLeafContainerPtr An output parameter to receive the pointer to the container of the leaf property.
	 * @param OutError An output parameter to receive any error message if the resolution fails.
	 */
	static bool ResolveStructPropertyPath(
		UStruct* CurrentStruct,
		void* CurrentContainerPtr,
		const TArray<FString>& Segments,
		int32 SegmentIndex,
		FProperty*& OutLeafProperty,
		void*& OutLeafContainerPtr,
		FString& OutError);
};