// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/Blueprint.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "Templates/Function.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousAssetRegistrationStructures.generated.h"
//<-------------------------------------------------------------------------->

class IToolkitHost;

/**
 * Defines which asset type action implementation should be used.
 */
enum class EGorgeousAssetActionType : uint8
{
	Auto,
	Blueprint,
	DataAsset,
	Generic
};

/**
 * Metadata describing how a custom asset should appear and behave in the editor.
 * Used by asset type actions and editor registration helpers.
 */
struct GORGEOUSCOREEDITORUTILITIES_API FGorgeousAssetTypeActionInfo_S
{
	FText DisplayName = FText::GetEmpty();
	TWeakObjectPtr<UClass> SupportedClass = nullptr;
	FColor TypeColor = FColor::White;
	uint32 Categories = 0;
	TArray<FText> SubMenus;
	const FSlateBrush* ThumbnailBrush = nullptr;
	const FSlateBrush* IconBrush = nullptr;
	EGorgeousAssetActionType ActionType = EGorgeousAssetActionType::Auto;
	TFunction<void(const TArray<UObject*>&, TSharedPtr<IToolkitHost>)> OpenAssetEditor;
};

/**
 * Structure that defines how the factory should behave within the editor.
 * Used primarily in UGorgeousFactory-based classes to set up the object creation behavior.
 *
 * @author Nils Bergemann
 */
USTRUCT()
struct GORGEOUSCOREEDITORUTILITIES_API FGorgeousFactoryInfo_S
{
	GENERATED_BODY()

	// Default constructor with safe initialization.
	FGorgeousFactoryInfo_S()
		: SupportedClass(nullptr)
		  , BlueprintClass(UBlueprint::StaticClass())
		  , BlueprintGeneratedClass(UBlueprintGeneratedClass::StaticClass())
		  , bEditAfterNew(false)
		  , bEditorImport(false)
		  , bCreateNew(false)
		  , bText(false)
		  , bDataOnly(false)
	{
	}

	// Parameterized constructor for custom behavior setup.
	FGorgeousFactoryInfo_S(const TSubclassOf<UObject>& NewSupportedClass, const bool NewEditAfterNew, const bool NewEditorImport,
	                       const bool NewCreateNew, const bool NewText,
	                       const TSubclassOf<UBlueprint>& NewBlueprintClass = UBlueprint::StaticClass(),
	                       const TSubclassOf<UBlueprintGeneratedClass>& NewBlueprintGeneratedClass = UBlueprintGeneratedClass::StaticClass(), const bool bNewDataOnly = false)
		: SupportedClass(NewSupportedClass)
		  , BlueprintClass(NewBlueprintClass)
		  , BlueprintGeneratedClass(NewBlueprintGeneratedClass)
		  , bEditAfterNew(NewEditAfterNew)
		  , bEditorImport(NewEditorImport)
		  , bCreateNew(NewCreateNew)
		  , bText(NewText)
		  , bDataOnly(bNewDataOnly)
	{
	}

	// The class manufactured by this factory.
	TSubclassOf<UObject> SupportedClass;

	/// Blueprint asset class to create for this factory.
	TSubclassOf<UBlueprint> BlueprintClass;
	
	// Generated class to use for this blueprint asset type.
	TSubclassOf<UBlueprintGeneratedClass> BlueprintGeneratedClass;

	// True if the associated editor should be opened after creating a new object.
	bool bEditAfterNew;
	
	// True if the factory imports objects from files.
	bool bEditorImport;

	// True if the factory supports object creation from scratch.
	bool bCreateNew;

	// True if the factory supports importing from text.
	bool bText;
	
	// Trie if the factory should use NewObject instead of CreateBlueprint.
	bool bDataOnly;
};

/**
 * Hashing function for FGorgeousFactoryInfo_S to allow use in hashed containers.
 */
FORCEINLINE uint32 GetTypeHash(const FGorgeousFactoryInfo_S& Info)
{
	uint32 Hash = 0;
	Hash = HashCombine(Hash, GetTypeHash(Info.SupportedClass.Get()));
	Hash = HashCombine(Hash, GetTypeHash(Info.BlueprintClass.Get()));
	Hash = HashCombine(Hash, GetTypeHash(Info.BlueprintGeneratedClass.Get()));
	Hash = HashCombine(Hash, GetTypeHash(Info.bEditAfterNew));
	Hash = HashCombine(Hash, GetTypeHash(Info.bEditorImport));
	Hash = HashCombine(Hash, GetTypeHash(Info.bCreateNew));
	Hash = HashCombine(Hash, GetTypeHash(Info.bText));
	Hash = HashCombine(Hash, GetTypeHash(Info.bDataOnly));
	return Hash;
}

/**
 * Equality operator for FGorgeousFactoryInfo_S.
 */
FORCEINLINE bool operator==(const FGorgeousFactoryInfo_S& A, const FGorgeousFactoryInfo_S& B)
{
	return A.SupportedClass == B.SupportedClass
		&& A.BlueprintClass == B.BlueprintClass
		&& A.BlueprintGeneratedClass == B.BlueprintGeneratedClass
		&& A.bEditAfterNew == B.bEditAfterNew
		&& A.bEditorImport == B.bEditorImport
		&& A.bCreateNew == B.bCreateNew
		&& A.bText == B.bText
		&& A.bDataOnly == B.bDataOnly;
}