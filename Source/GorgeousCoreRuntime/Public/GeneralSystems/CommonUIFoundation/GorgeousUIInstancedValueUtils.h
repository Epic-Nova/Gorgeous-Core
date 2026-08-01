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
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousUIInstancedValueUtils.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Wraps text values for storage in an FInstancedStruct.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousInstancedText
{
	GENERATED_BODY()

	// Stores the wrapped text value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	FText Value;
};

/**
 * Wraps floating-point values for storage in an FInstancedStruct.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousInstancedFloat
{
	GENERATED_BODY()

	// Stores the wrapped floating-point value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	float Value = 0.0f;
};

/**
 * Wraps integer values for storage in an FInstancedStruct.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousInstancedInt
{
	GENERATED_BODY()

	// Stores the wrapped integer value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	int32 Value = 0;
};

/**
 * Wraps boolean values for storage in an FInstancedStruct.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousInstancedBool
{
	GENERATED_BODY()

	// Stores the wrapped boolean value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	bool Value = false;
};

/**
 * Wraps object references for storage in an FInstancedStruct.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousInstancedObject
{
	GENERATED_BODY()

	// Stores the wrapped object reference.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	TObjectPtr<UObject> Value = nullptr;
};

/**
 * Wraps name values for storage in an FInstancedStruct.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousInstancedName
{
	GENERATED_BODY()

	// Stores the wrapped name value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	FName Value;
};

// Attempts to read typed wrapper values from an FInstancedStruct.
namespace GorgeousUIInstanced
{
	static inline bool TryGetText(const FInstancedStruct& S, FText& Out)
	{
		if (const FGorgeousInstancedText* W = S.GetPtr<FGorgeousInstancedText>())
		{
			Out = W->Value;
			return true;
		}
		return false;
	}

	static inline bool TryGetFloat(const FInstancedStruct& S, float& Out)
	{
		if (const FGorgeousInstancedFloat* W = S.GetPtr<FGorgeousInstancedFloat>())
		{
			Out = W->Value;
			return true;
		}
		return false;
	}

	static inline bool TryGetInt(const FInstancedStruct& S, int32& Out)
	{
		if (const FGorgeousInstancedInt* W = S.GetPtr<FGorgeousInstancedInt>())
		{
			Out = W->Value;
			return true;
		}
		return false;
	}

	static inline bool TryGetBool(const FInstancedStruct& S, bool& Out)
	{
		if (const FGorgeousInstancedBool* W = S.GetPtr<FGorgeousInstancedBool>())
		{
			Out = W->Value;
			return true;
		}
		return false;
	}

	static inline bool TryGetObject(const FInstancedStruct& S, UObject*& Out)
	{
		if (const FGorgeousInstancedObject* W = S.GetPtr<FGorgeousInstancedObject>())
		{
			Out = W->Value.Get();
			return true;
		}
		return false;
	}

	static inline bool TryGetName(const FInstancedStruct& S, FName& Out)
	{
		if (const FGorgeousInstancedName* W = S.GetPtr<FGorgeousInstancedName>())
		{
			Out = W->Value;
			return true;
		}
		return false;
	}
}