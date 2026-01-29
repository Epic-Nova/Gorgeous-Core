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

//<=============================--- Includes ---=============================>
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousAssetRegistrationStructures.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Metadata describing how a custom asset should appear and behave in the editor.
 * Used by deprecated asset type actions and editor registration helpers.
 */
struct FGorgeousAssetTypeActionInfo_S
{
	FText DisplayName = FText::GetEmpty();
	TWeakObjectPtr<UClass> SupportedClass = nullptr;
	FColor TypeColor = FColor::White;
	uint32 Categories = 0;
	TArray<FText> SubMenus;
	const FSlateBrush* ThumbnailBrush = nullptr;
	const FSlateBrush* IconBrush = nullptr;
};

/**
 * Structure that defines how the factory should behave within the editor.
 *
 * This struct provides the necessary metadata to configure object factories, such as
 * whether they support creating new objects, importing files, or handling text-based assets.
 * 
 * Used primarily in UGorgeousFactory-based classes to set up the object creation behavior.
 *
 * @author Nils Bergemann
 */
USTRUCT()
struct FGorgeousFactoryInfo_S
{
	GENERATED_BODY()

	// Default constructor with safe initialization.
	FGorgeousFactoryInfo_S()
		: SupportedClass(nullptr)
		, bEditAfterNew(false)
		, bEditorImport(false)
		, bCreateNew(false)
		, bText(false)
	{}

	// Parameterized constructor for custom behavior setup.
	FGorgeousFactoryInfo_S(const TSubclassOf<UObject>& NewSupportedClass, const bool NewEditAfterNew, const bool NewEditorImport,
	                       const bool NewbCreateNew, const bool NewText)
		: SupportedClass(NewSupportedClass)
		, bEditAfterNew(NewEditAfterNew)
		, bEditorImport(NewEditorImport)
		, bCreateNew(NewbCreateNew)
		, bText(NewText)
	{}

	/** The class manufactured by this factory. */
	UPROPERTY()
	TSubclassOf<UObject> SupportedClass;

	/** True if the associated editor should be opened after creating a new object. */
	UPROPERTY()
	bool bEditAfterNew;

	/** True if the factory imports objects from files. */
	UPROPERTY()
	bool bEditorImport;

	/** True if the factory supports object creation from scratch. */
	UPROPERTY()
	bool bCreateNew;

	/** True if the factory supports importing from text. */
	UPROPERTY()
	bool bText;
};

/**
 * Hashing function for FGorgeousFactoryInfo_S to allow use in hashed containers.
 */
FORCEINLINE uint32 GetTypeHash(const FGorgeousFactoryInfo_S& Info)
{
	uint32 Hash = 0;
	Hash = HashCombine(Hash, GetTypeHash(Info.SupportedClass.Get()));
	Hash = HashCombine(Hash, GetTypeHash(Info.bEditAfterNew));
	Hash = HashCombine(Hash, GetTypeHash(Info.bEditorImport));
	Hash = HashCombine(Hash, GetTypeHash(Info.bCreateNew));
	Hash = HashCombine(Hash, GetTypeHash(Info.bText));
	return Hash;
}

/**
 * Equality operator for FGorgeousFactoryInfo_S.
 */
FORCEINLINE bool operator==(const FGorgeousFactoryInfo_S& A, const FGorgeousFactoryInfo_S& B)
{
	return A.SupportedClass == B.SupportedClass
		&& A.bEditAfterNew == B.bEditAfterNew
		&& A.bEditorImport == B.bEditorImport
		&& A.bCreateNew == B.bCreateNew
		&& A.bText == B.bText;
}