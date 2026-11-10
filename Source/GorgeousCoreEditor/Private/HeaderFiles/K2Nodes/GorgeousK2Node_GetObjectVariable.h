#pragma once
#include "CoreMinimal.h"
#include "K2Node.h"
#include "GorgeousK2Node_GetObjectVariable.generated.h"

/** A typed, pure read of the concrete Object Variable's reflected Value property. */
UCLASS()
class GORGEOUSCOREEDITOR_API UGorgeousK2Node_GetObjectVariable : public UK2Node
{
    GENERATED_BODY()
public:
    virtual void AllocateDefaultPins() override;
    virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;
    virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
    virtual void PostReconstructNode() override;
    virtual bool IsNodePure() const override { return true; }
    virtual FText GetNodeTitle(ENodeTitleType::Type) const override;
    virtual FText GetMenuCategory() const override;
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& Registrar) const override;
    virtual void ExpandNode(FKismetCompilerContext& Context, UEdGraph* Graph) override;
private:
    UClass* ResolveTargetClass() const;
    void RefreshValueType();
};
