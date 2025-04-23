#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "UK2Node_MyCustomNode.generated.h"

//@TODO: when directly placing a variable on top of a already connected one, the logic that recalculates the pin type does not fire
//@TODO: Also it is not firing on recompile
//@TODO: Structures are not appearing correct
//@TODO: Support for custom structs created in blueprint
UCLASS()
class GORGEOUSCORERUNTIME_API UK2Node_MyCustomNode : public UK2Node
{
	GENERATED_BODY()

public:
	// Node metadata
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool ShouldDrawCompact() const override { return true; }
	virtual FText GetCompactNodeTitle() const override { return INVTEXT("Get"); }
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual FLinearColor GetNodeTitleColor() const override { return FLinearColor(0.066f, 0.46f, 0.63f); }
	virtual bool IsNodePure() const override { return true; }


	virtual void ReconstructNode() override;

	// Registration info
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;

	virtual bool IsCompatibleWithGraph(const UEdGraph* TargetGraph) const override;
	
	virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;

private:
	
	UEdGraphPin* GetTargetPin() const {  return FindPin(TEXT("Target")); }
	UEdGraphPin* GetReturnValuePin() const {  return FindPin(TEXT("ReturnValue")); }

	static void CustomizeNodeForContext(UEdGraphNode* NewNode, bool bIsTemplateNode);

	UEdGraphPin* BuildReturnValuePin();

	/**
	 * Attempts to retrieve the editor-time UObject instance connected to a specific input pin.
	 * This function traverses through basic Reroute nodes (UK2Node_Knot).
	 *
	 * @param TargetPin The input pin on your node for which you want to find the connected source object.
	 * @return The connected UObject* if found and determinable at editor time, otherwise nullptr.
	 *
	 * NOTE: This function relies on editor-time data (like DefaultObject) and reflection.
	 * It may not accurately reflect runtime values, especially for function calls.
	 * It only handles a limited set of node types (VariableGet, CallFunction, Literal, Knot).
	 */
	UClass* GetConnectedUObjectInstance(const UEdGraphPin* TargetPin);
};