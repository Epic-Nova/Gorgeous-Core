// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Asset Definitions                           |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

#pragma once

#include "AssetDefinitionDefault.h"
#include "Containers/ArrayView.h"
#include "GorgeousCoreMinimalShared.h"
#include "GorgeousCoreAssetDefinitions.generated.h"

namespace GorgeousCoreAssetCategories
{
	inline const FAssetCategoryPath& Root()
	{
		static const FAssetCategoryPath RootCategory(NSLOCTEXT("GorgeousCore", "Category_GorgeousThings", "Gorgeous Things"));
		return RootCategory;
	}

	inline const FAssetCategoryPath& Core()
	{
		static const FAssetCategoryPath CoreCategory(Root(), NSLOCTEXT("GorgeousCore", "Category_GorgeousCore", "Gorgeous Core"));
		return CoreCategory;
	}

	inline const FAssetCategoryPath& ConditionalChoosers()
	{
		static const FAssetCategoryPath Category(Core(), NSLOCTEXT("GorgeousCore", "Category_ConditionalChoosers", "Conditional Object Choosers"));
		return Category;
	}

	inline const FAssetCategoryPath& Conditions()
	{
		static const FAssetCategoryPath Category(ConditionalChoosers(), NSLOCTEXT("GorgeousCore", "Category_Conditions", "Conditions"));
		return Category;
	}

	inline const FAssetCategoryPath& QualityOfLife()
	{
		static const FAssetCategoryPath Category(Core(), NSLOCTEXT("GorgeousCore", "Category_QualityOfLife", "Quality of Life"));
		return Category;
	}
}

/**
 * Asset definition for Gorgeous Object Variable.
 */
UCLASS()
class UAssetDefinition_GorgeousObjectVariable : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override { return NSLOCTEXT("GorgeousCore", "ObjectVariable", "Gorgeous Object Variable"); }
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor::Blue); }
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UGorgeousObjectVariable::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		static const FAssetCategoryPath Categories[] = { GorgeousCoreAssetCategories::Core() };
		return MakeArrayView(Categories);
	}
};

/**
 * Asset definition for Gorgeous Conditional Object Chooser.
 */
UCLASS()
class UAssetDefinition_GorgeousConditionalObjectChooser : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override { return NSLOCTEXT("GorgeousCore", "ConditionalObjectChooser", "Gorgeous Conditional Object Chooser"); }
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor::Cyan); }
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UGorgeousConditionalObjectChooser::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		static const FAssetCategoryPath Categories[] = { GorgeousCoreAssetCategories::ConditionalChoosers() };
		return MakeArrayView(Categories);
	}
};

/**
 * Asset definition for Gorgeous Boolean Condition.
 */
UCLASS()
class UAssetDefinition_GorgeousBooleanCondition : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override { return NSLOCTEXT("GorgeousCore", "BooleanCondition", "Gorgeous Boolean Condition"); }
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor::Turquoise); }
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UGorgeousBooleanCondition::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		static const FAssetCategoryPath Categories[] = { GorgeousCoreAssetCategories::Conditions() };
		return MakeArrayView(Categories);
	}
};

/**
 * Asset definition for Gorgeous IsValid Condition.
 */
UCLASS()
class UAssetDefinition_GorgeousIsValidCondition : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override { return NSLOCTEXT("GorgeousCore", "IsValidCondition", "Gorgeous IsValid Condition"); }
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor::Turquoise); }
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UGorgeousIsValidCondition::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		static const FAssetCategoryPath Categories[] = { GorgeousCoreAssetCategories::Conditions() };
		return MakeArrayView(Categories);
	}
};

/**
 * Asset definition for Gorgeous Gameplay Tag Condition.
 */
UCLASS()
class UAssetDefinition_GorgeousGameplayTagCondition : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override { return NSLOCTEXT("GorgeousCore", "GameplayTagCondition", "Gorgeous Gameplay Tag Condition"); }
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor::Turquoise); }
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UGorgeousGameplayTagCondition::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		static const FAssetCategoryPath Categories[] = { GorgeousCoreAssetCategories::Conditions() };
		return MakeArrayView(Categories);
	}
};

/**
 * Asset definition for Gorgeous Game Instance.
 */
UCLASS()
class UAssetDefinition_GorgeousGameInstance : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override { return NSLOCTEXT("GorgeousCore", "GameInstance", "Gorgeous Game Instance"); }
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor::Blue); }
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UGorgeousGameInstance::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		static const FAssetCategoryPath Categories[] = { GorgeousCoreAssetCategories::QualityOfLife() };
		return MakeArrayView(Categories);
	}
};

/**
 * Asset definition for Gorgeous Game Mode.
 */
UCLASS()
class UAssetDefinition_GorgeousGameMode : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override { return NSLOCTEXT("GorgeousCore", "GameMode", "Gorgeous Game Mode"); }
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor::Blue); }
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return AGorgeousGameMode::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		static const FAssetCategoryPath Categories[] = { GorgeousCoreAssetCategories::QualityOfLife() };
		return MakeArrayView(Categories);
	}
};

/**
 * Asset definition for Gorgeous Game State.
 */
UCLASS()
class UAssetDefinition_GorgeousGameState : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override { return NSLOCTEXT("GorgeousCore", "GameState", "Gorgeous Game State"); }
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor::Blue); }
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return AGorgeousGameState::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		static const FAssetCategoryPath Categories[] = { GorgeousCoreAssetCategories::QualityOfLife() };
		return MakeArrayView(Categories);
	}
};

/**
 * Asset definition for Gorgeous Player Controller.
 */
UCLASS()
class UAssetDefinition_GorgeousPlayerController : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override { return NSLOCTEXT("GorgeousCore", "PlayerController", "Gorgeous Player Controller"); }
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor::Blue); }
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return AGorgeousPlayerController::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		static const FAssetCategoryPath Categories[] = { GorgeousCoreAssetCategories::QualityOfLife() };
		return MakeArrayView(Categories);
	}
};

/**
 * Asset definition for Gorgeous Player State.
 */
UCLASS()
class UAssetDefinition_GorgeousPlayerState : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override { return NSLOCTEXT("GorgeousCore", "PlayerState", "Gorgeous Player State"); }
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor::Blue); }
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return AGorgeousPlayerState::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		static const FAssetCategoryPath Categories[] = { GorgeousCoreAssetCategories::QualityOfLife() };
		return MakeArrayView(Categories);
	}
};

/**
 * Asset definition for Gorgeous World Settings.
 */
UCLASS()
class UAssetDefinition_GorgeousWorldSettings : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override { return NSLOCTEXT("GorgeousCore", "WorldSettings", "Gorgeous World Settings"); }
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor::Blue); }
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return AGorgeousWorldSettings::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		static const FAssetCategoryPath Categories[] = { GorgeousCoreAssetCategories::QualityOfLife() };
		return MakeArrayView(Categories);
	}
};
