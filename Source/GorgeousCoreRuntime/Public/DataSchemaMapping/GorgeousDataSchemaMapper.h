// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "DataSchemaMapping/GorgeousDataSchemaMappingStructures.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class FProperty;
class UObject;
class UGorgeousDataSchemaMapping_DA;
class UStruct;
//<-------------------------------------------------->

/**
 * Runtime reflection mapper that applies source snapshots to target objects.
 */
class GORGEOUSCORERUNTIME_API FGorgeousDataSchemaMapper
{
public:
	static bool ApplySnapshotToObject(
		const FGorgeousDataSchemaSourceSnapshot_S& SourceSnapshot,
		UObject* TargetObject,
		const UGorgeousDataSchemaMapping_DA* SchemaMap,
		TArray<FString>& OutWarnings,
		TArray<FString>& OutErrors);

	static bool ApplyTransformDefinition(
		const FGorgeousDataSchemaTransformDefinition_S& TransformDefinition,
		const FString& SourceValue,
		FString& OutValue,
		FString& OutError);

private:
	static bool ResolvePropertyPath(
		UObject* RootObject,
		const FString& PropertyPath,
		FProperty*& OutLeafProperty,
		void*& OutLeafContainerPtr,
		FString& OutError);

	static bool ResolveStructPropertyPath(
		UStruct* CurrentStruct,
		void* CurrentContainerPtr,
		const TArray<FString>& Segments,
		int32 SegmentIndex,
		FProperty*& OutLeafProperty,
		void*& OutLeafContainerPtr,
		FString& OutError);
};
