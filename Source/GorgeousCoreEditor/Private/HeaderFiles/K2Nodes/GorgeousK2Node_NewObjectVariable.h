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

#if 0

#pragma once

#include "CoreMinimal.h"
#include "K2Node_ConstructObjectFromClass.h"
#include "GorgeousK2Node_NewObjectVariable.generated.h"

class UGorgeousObjectVariable;

/**
 * TODO: Under construction: Planned to be available in v1.1
 * K2 node that constructs a Gorgeous Object Variable and honors ExposeOnSpawn properties.
 * Mirrors the NewObjectVariable helper while surfacing pins for exposed properties.
 */
UCLASS()
class GORGEOUSCOREEDITOR_API UK2Node_GorgeousNewObjectVariable : public UK2Node_ConstructObjectFromClass
{
	GENERATED_BODY()

public:
	UK2Node_GorgeousNewObjectVariable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ End UEdGraphNode Interface

	//~ Begin UK2Node Interface
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	//~ End UK2Node Interface

protected:
	virtual UClass* GetClassPinBaseClass() const override;
	virtual FText GetBaseNodeTitle() const override;
	virtual FText GetDefaultNodeTitle() const override;
	virtual FText GetNodeTitleFormat() const override;
	virtual bool IsSpawnVarPin(UEdGraphPin* Pin) const override;

private:
	UEdGraphPin* GetParentPin() const;
	UEdGraphPin* GetPersistPin() const;
	UEdGraphPin* GetDisplayNamePin() const;
	UEdGraphPin* GetIdentifierPin() const;

	static const FName ParentPinName;
	static const FName PersistPinName;
	static const FName DisplayNamePinName;
	static const FName IdentifierPinName;
};


#endif