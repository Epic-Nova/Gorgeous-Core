// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
#include "GorgeousUIInstancedValueUtils.generated.h"

/**
 * Small wrapper USTRUCTs used to store primitive and non-UStruct values inside FInstancedStruct
 * so that the engine's InstancedStruct API can be used consistently.
 */
USTRUCT(BlueprintType)
struct FGorgeousInstancedText
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	FText Value;
};

USTRUCT(BlueprintType)
struct FGorgeousInstancedFloat
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	float Value = 0.0f;
};

USTRUCT(BlueprintType)
struct FGorgeousInstancedInt
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FGorgeousInstancedBool
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	bool Value = false;
};

USTRUCT(BlueprintType)
struct FGorgeousInstancedObject
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	TObjectPtr<UObject> Value = nullptr;
};

USTRUCT(BlueprintType)
struct FGorgeousInstancedName
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	FName Value;
};

/** Helper extractors: attempt to read wrapper types from an FInstancedStruct. */
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
