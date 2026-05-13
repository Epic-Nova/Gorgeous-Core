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
#include "DataSchemaMapping/GorgeousDataSchemaMapper.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "DataSchemaMapping/GorgeousDataSchemaMapping_DA.h"
#include "Helpers/GorgeousPropertyPathHelper.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "String/LexFromString.h"
#include "UObject/UnrealType.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// FGorgeousDataSchemaMapper Implementation
//=============================================================================

bool FGorgeousDataSchemaMapper::ApplySnapshotToObject(
	const FGorgeousDataSchemaSourceSnapshot_S& SourceSnapshot,
	UObject* TargetObject,
	const UGorgeousDataSchemaMapping_DA* SchemaMap,
	TArray<FString>& OutWarnings,
	TArray<FString>& OutErrors)
{
	if (!TargetObject)
	{
		OutErrors.Add(TEXT("Target object is null."));
		return false;
	}

	if (!SchemaMap)
	{
		OutErrors.Add(TEXT("Schema map is null."));
		return false;
	}

	for (const FGorgeousDataSchemaFieldMapping_S& FieldMapping : SchemaMap->FieldMappings)
	{
		if (FieldMapping.SourcePath.IsEmpty() || FieldMapping.TargetPath.IsEmpty())
		{
			OutWarnings.Add(TEXT("Skipped a mapping row with an empty source or target path."));
			continue;
		}

		FString SourceValue;
		if (!SourceSnapshot.TryGetValueByPath(FieldMapping.SourcePath, SourceValue))
		{
			if (FieldMapping.bRequired)
			{
				OutErrors.Add(FString::Printf(
					TEXT("Required source path '%s' was not found in snapshot '%s'."),
					*FieldMapping.SourcePath,
					*SourceSnapshot.SourceLabel));
			}
			else
			{
				OutWarnings.Add(FString::Printf(
					TEXT("Optional source path '%s' was not found in snapshot '%s'."),
					*FieldMapping.SourcePath,
					*SourceSnapshot.SourceLabel));
			}
			continue;
		}

		FProperty* LeafProperty = nullptr;
		void* LeafContainerPtr = nullptr;
		FString ResolveError;
		if (!ResolvePropertyPath(TargetObject, FieldMapping.TargetPath, LeafProperty, LeafContainerPtr, ResolveError))
		{
			OutErrors.Add(FString::Printf(
				TEXT("Target path '%s' failed to resolve: %s"),
				*FieldMapping.TargetPath,
				*ResolveError));
			continue;
		}

		void* LeafValuePtr = LeafProperty->ContainerPtrToValuePtr<void>(LeafContainerPtr);
		if (!LeafValuePtr)
		{
			OutErrors.Add(FString::Printf(
				TEXT("Target path '%s' resolved, but value pointer was null."),
				*FieldMapping.TargetPath));
			continue;
		}

		FString MappedValue;
		FString TransformError;
		if (!ApplyTransformDefinition(FieldMapping.TransformDefinition, SourceValue, MappedValue, TransformError))
		{
			OutErrors.Add(FString::Printf(
				TEXT("Transform failed for source '%s' -> target '%s': %s"),
				*FieldMapping.SourcePath,
				*FieldMapping.TargetPath,
				*TransformError));
			continue;
		}

		if (!LeafProperty->ImportText_Direct(*MappedValue, LeafValuePtr, TargetObject, PPF_None))
		{
			OutErrors.Add(FString::Printf(
				TEXT("Failed to import value from source '%s' into target '%s'. Value was '%s'."),
				*FieldMapping.SourcePath,
				*FieldMapping.TargetPath,
				*MappedValue));
		}
	}

	return OutErrors.IsEmpty();
}

bool FGorgeousDataSchemaMapper::ResolvePropertyPath(
	UObject* RootObject,
	const FString& PropertyPath,
	FProperty*& OutLeafProperty,
	void*& OutLeafContainerPtr,
	FString& OutError)
{
	OutLeafProperty = nullptr;
	OutLeafContainerPtr = nullptr;

	if (!RootObject)
	{
		OutError = TEXT("Root object is null.");
		return false;
	}

	TArray<FString> Segments;
	PropertyPath.ParseIntoArray(Segments, TEXT("."), true);
	if (Segments.IsEmpty())
	{
		OutError = TEXT("Property path was empty.");
		return false;
	}

	return ResolveStructPropertyPath(
		RootObject->GetClass(),
		RootObject,
		Segments,
		0,
		OutLeafProperty,
		OutLeafContainerPtr,
		OutError);
}

bool FGorgeousDataSchemaMapper::ResolveStructPropertyPath(
	UStruct* CurrentStruct,
	void* CurrentContainerPtr,
	const TArray<FString>& Segments,
	const int32 SegmentIndex,
	FProperty*& OutLeafProperty,
	void*& OutLeafContainerPtr,
	FString& OutError)
{
	if (!CurrentStruct || !CurrentContainerPtr)
	{
		OutError = TEXT("Current struct or container pointer is invalid.");
		return false;
	}

	if (!Segments.IsValidIndex(SegmentIndex))
	{
		OutError = TEXT("Segment index exceeded path bounds.");
		return false;
	}

	FProperty* ResolvedProperty = GorgeousPropertyPath::FindPropertyBySegment(CurrentStruct, Segments[SegmentIndex]);
	if (!ResolvedProperty)
	{
		OutError = FString::Printf(
			TEXT("Property '%s' was not found on struct '%s'."),
			*Segments[SegmentIndex],
			*CurrentStruct->GetName());
		return false;
	}

	const bool bIsLeaf = SegmentIndex == Segments.Num() - 1;
	if (bIsLeaf)
	{
		OutLeafProperty = ResolvedProperty;
		OutLeafContainerPtr = CurrentContainerPtr;
		return true;
	}

	if (FStructProperty* StructProperty = CastField<FStructProperty>(ResolvedProperty))
	{
		void* NextContainerPtr = StructProperty->ContainerPtrToValuePtr<void>(CurrentContainerPtr);
		if (!NextContainerPtr)
		{
			OutError = FString::Printf(
				TEXT("Struct path '%s' had a null intermediate container."),
				*StructProperty->GetName());
			return false;
		}

		return ResolveStructPropertyPath(
			StructProperty->Struct,
			NextContainerPtr,
			Segments,
			SegmentIndex + 1,
			OutLeafProperty,
			OutLeafContainerPtr,
			OutError);
	}

	if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(ResolvedProperty))
	{
		UObject* NextObject = ObjectProperty->GetObjectPropertyValue_InContainer(CurrentContainerPtr);
		if (!NextObject)
		{
			OutError = FString::Printf(
				TEXT("Object path '%s' is null and cannot be traversed."),
				*ObjectProperty->GetName());
			return false;
		}

		return ResolveStructPropertyPath(
			NextObject->GetClass(),
			NextObject,
			Segments,
			SegmentIndex + 1,
			OutLeafProperty,
			OutLeafContainerPtr,
			OutError);
	}

	OutError = FString::Printf(
		TEXT("Property '%s' is not a struct/object bridge but path traversal continues."),
		*ResolvedProperty->GetName());
	return false;
}

bool FGorgeousDataSchemaMapper::ApplyTransformDefinition(
	const FGorgeousDataSchemaTransformDefinition_S& TransformDefinition,
	const FString& SourceValue,
	FString& OutValue,
	FString& OutError)
{
	OutValue = SourceValue;
	OutError.Reset();

	switch (TransformDefinition.TransformKind)
	{
	case EGorgeousDataSchemaTransformKind_E::Transform_None:
		return true;

	case EGorgeousDataSchemaTransformKind_E::Transform_SetLiteral:
		OutValue = TransformDefinition.ParameterA;
		return true;

	case EGorgeousDataSchemaTransformKind_E::Transform_UseFallbackLiteralIfEmpty:
		OutValue = SourceValue.TrimStartAndEnd().IsEmpty()
			? TransformDefinition.ParameterA
			: SourceValue;
		return true;

	case EGorgeousDataSchemaTransformKind_E::Transform_TrimWhitespace:
		OutValue = SourceValue.TrimStartAndEnd();
		return true;

	case EGorgeousDataSchemaTransformKind_E::Transform_ToLower:
		OutValue = SourceValue.ToLower();
		return true;

	case EGorgeousDataSchemaTransformKind_E::Transform_ToUpper:
		OutValue = SourceValue.ToUpper();
		return true;

	case EGorgeousDataSchemaTransformKind_E::Transform_AddPrefix:
		OutValue = TransformDefinition.ParameterA + SourceValue;
		return true;

	case EGorgeousDataSchemaTransformKind_E::Transform_AddSuffix:
		OutValue = SourceValue + TransformDefinition.ParameterA;
		return true;

	case EGorgeousDataSchemaTransformKind_E::Transform_ReplaceText:
		if (TransformDefinition.ParameterA.IsEmpty())
		{
			OutError = TEXT("Replace Text transform requires ParameterA (search text).");
			return false;
		}

		OutValue = SourceValue.Replace(*TransformDefinition.ParameterA, *TransformDefinition.ParameterB, ESearchCase::CaseSensitive);
		return true;

	case EGorgeousDataSchemaTransformKind_E::Transform_MultiplyNumeric:
	case EGorgeousDataSchemaTransformKind_E::Transform_AddNumericOffset:
		{
			double SourceNumericValue = 0.0;
			if (!LexTryParseString(SourceNumericValue, *SourceValue))
			{
				OutError = FString::Printf(TEXT("Source value '%s' is not numeric."), *SourceValue);
				return false;
			}

			double NumericParameter = 0.0;
			if (!LexTryParseString(NumericParameter, *TransformDefinition.ParameterA))
			{
				OutError = FString::Printf(TEXT("ParameterA '%s' is not numeric."), *TransformDefinition.ParameterA);
				return false;
			}

			const double ResultValue = TransformDefinition.TransformKind == EGorgeousDataSchemaTransformKind_E::Transform_MultiplyNumeric
				? (SourceNumericValue * NumericParameter)
				: (SourceNumericValue + NumericParameter);

			OutValue = FString::SanitizeFloat(ResultValue);
			return true;
		}

	case EGorgeousDataSchemaTransformKind_E::Transform_InvertBoolean:
		{
			const FString NormalizedSourceValue = SourceValue.TrimStartAndEnd();
			bool bSourceBooleanValue = false;

			if (NormalizedSourceValue == TEXT("1"))
			{
				bSourceBooleanValue = true;
			}
			else if (NormalizedSourceValue == TEXT("0"))
			{
				bSourceBooleanValue = false;
			}
			else if (!LexTryParseString(bSourceBooleanValue, *NormalizedSourceValue))
			{
				OutError = FString::Printf(TEXT("Source value '%s' is not a valid boolean."), *SourceValue);
				return false;
			}

			OutValue = bSourceBooleanValue ? TEXT("False") : TEXT("True");
			return true;
		}

	default:
		OutError = TEXT("Unsupported transform kind.");
		return false;
	}
}