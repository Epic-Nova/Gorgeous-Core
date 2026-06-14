#if 0
#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "GorgeousK2Node_GetObjectVariable.generated.h"

//@TODO: when directly placing a variable on top of a already connected one, the logic that recalculates the pin type does not fire
//@TODO: Also it is not firing on recompile
//@TODO: Structures are not appearing correct
//@TODO: Support for custom structs created in blueprint
UCLASS()
class GORGEOUSCOREEDITOR_API UGorgeousK2Node_GetObjectVariable : public UK2Node
{
	GENERATED_BODY()

public:
	// Node metadata
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool ShouldDrawCompact() const override { return true; }
	virtual FText GetCompactNodeTitle() const override { return INVTEXT("Get Value @TODO: for v1.1"); }
	virtual FLinearColor GetNodeTitleColor() const override { return FLinearColor(0.066f, 0.46f, 0.63f); }
	virtual bool IsNodePure() const override { return true; }
	
	virtual void AllocateDefaultPins() override;
	virtual void ReconstructNode() override;

	// Registration info
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;

	virtual bool IsCompatibleWithGraph(const UEdGraph* TargetGraph) const override;
	
	virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;

private:
	
	UEdGraphPin* BuildTargetPin();
	UEdGraphPin* BuildReturnValuePin();
	
	UEdGraphPin* GetTargetPin() const {  return FindPin(TEXT("Target")); }
	UEdGraphPin* GetReturnValuePin() const {  return FindPin(TEXT("ReturnValue")); }

	UClass* ResolveTargetClass() const;
	//static void CustomizeNodeForContext(UEdGraphNode* NewNode, bool bIsTemplateNode);

	
	virtual void PostPlacedNewNode() override;
	virtual void PostLoad() override;
	virtual void BeginDestroy() override;
	
	FDelegateHandle BlueprintCompiledHandle;
	FDelegateHandle ObjectModifiedHandle;

	void HandleBlueprintCompiled(UBlueprint* Blueprint);
	void HandleObjectModified(UObject* Object);

	void BindListeners();
	void UnbindListeners();
	
	bool bRefreshQueued = false;

	void QueueRefresh();
	void PerformDeferredRefresh();
};

#endif