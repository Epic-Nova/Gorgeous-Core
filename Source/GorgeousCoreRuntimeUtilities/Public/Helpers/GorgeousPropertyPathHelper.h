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
//<--------------------------=== Engine Includes ===------------------------->
#include "UObject/UnrealType.h"
//<-------------------------------------------------------------------------->

namespace GorgeousPropertyPath
{
	FORCEINLINE bool IsAllDigits(const FString& Value)
	{
		if (Value.IsEmpty())
		{
			return false;
		}

		for (const TCHAR Character : Value)
		{
			if (!FChar::IsDigit(Character))
			{
				return false;
			}
		}

		return true;
	}

	FORCEINLINE bool IsAllHex(const FString& Value)
	{
		if (Value.IsEmpty())
		{
			return false;
		}

		for (const TCHAR Character : Value)
		{
			if (!FChar::IsHexDigit(Character))
			{
				return false;
			}
		}

		return true;
	}

	// Strips UE-generated struct suffix pattern: <Base>_<Numeric>_<32HexChars>
	FORCEINLINE FString StripGeneratedStructSuffix(const FString& Name)
	{
		int32 HexUnderscoreIndex = INDEX_NONE;
		if (!Name.FindLastChar(TEXT('_'), HexUnderscoreIndex))
		{
			return Name;
		}

		const FString TrailingHexSegment = Name.Mid(HexUnderscoreIndex + 1);
		if (TrailingHexSegment.Len() != 32 || !IsAllHex(TrailingHexSegment))
		{
			return Name;
		}

		const FString PrefixBeforeHex = Name.Left(HexUnderscoreIndex);
		int32 NumberUnderscoreIndex = INDEX_NONE;
		if (!PrefixBeforeHex.FindLastChar(TEXT('_'), NumberUnderscoreIndex))
		{
			return Name;
		}

		const FString NumericSegment = PrefixBeforeHex.Mid(NumberUnderscoreIndex + 1);
		if (!IsAllDigits(NumericSegment))
		{
			return Name;
		}

		const FString BaseName = PrefixBeforeHex.Left(NumberUnderscoreIndex);
		return BaseName.IsEmpty() ? Name : BaseName;
	}

	FORCEINLINE FString GetPreferredPropertySegment(const FProperty* Property)
	{
		if (!Property)
		{
			return FString();
		}

		FString Segment = Property->GetAuthoredName();
		if (Segment.IsEmpty())
		{
			Segment = Property->GetName();
		}

		return StripGeneratedStructSuffix(Segment);
	}

	FORCEINLINE bool DoesPropertySegmentMatch(const FProperty* Property, const FString& Segment)
	{
		if (!Property || Segment.IsEmpty())
		{
			return false;
		}

		if (Property->GetName().Equals(Segment, ESearchCase::IgnoreCase)
			|| Property->GetAuthoredName().Equals(Segment, ESearchCase::IgnoreCase))
		{
			return true;
		}

		const FString CanonicalSegment = StripGeneratedStructSuffix(Segment);
		if (StripGeneratedStructSuffix(Property->GetName()).Equals(CanonicalSegment, ESearchCase::IgnoreCase)
			|| StripGeneratedStructSuffix(Property->GetAuthoredName()).Equals(CanonicalSegment, ESearchCase::IgnoreCase))
		{
			return true;
		}

		return false;
	}

	FORCEINLINE const FProperty* FindConstPropertyBySegment(const UStruct* CurrentStruct, const FString& Segment)
	{
		if (!CurrentStruct || Segment.IsEmpty())
		{
			return nullptr;
		}

		if (const FProperty* ExactProperty = CurrentStruct->FindPropertyByName(*Segment))
		{
			return ExactProperty;
		}

		for (TFieldIterator<FProperty> PropertyIt(CurrentStruct, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
		{
			if (DoesPropertySegmentMatch(*PropertyIt, Segment))
			{
				return *PropertyIt;
			}
		}

		return nullptr;
	}

	FORCEINLINE FProperty* FindPropertyBySegment(UStruct* CurrentStruct, const FString& Segment)
	{
		return const_cast<FProperty*>(FindConstPropertyBySegment(CurrentStruct, Segment));
	}

	FORCEINLINE bool ArePathStringsEquivalent(const FString& LeftPath, const FString& RightPath)
	{
		if (LeftPath.Equals(RightPath, ESearchCase::IgnoreCase))
		{
			return true;
		}

		TArray<FString> LeftSegments;
		TArray<FString> RightSegments;
		LeftPath.ParseIntoArray(LeftSegments, TEXT("."), true);
		RightPath.ParseIntoArray(RightSegments, TEXT("."), true);

		if (LeftSegments.Num() != RightSegments.Num())
		{
			return false;
		}

		for (int32 SegmentIndex = 0; SegmentIndex < LeftSegments.Num(); ++SegmentIndex)
		{
			const FString LeftCanonical = StripGeneratedStructSuffix(LeftSegments[SegmentIndex]);
			const FString RightCanonical = StripGeneratedStructSuffix(RightSegments[SegmentIndex]);
			if (!LeftCanonical.Equals(RightCanonical, ESearchCase::IgnoreCase))
			{
				return false;
			}
		}

		return true;
	}
}
