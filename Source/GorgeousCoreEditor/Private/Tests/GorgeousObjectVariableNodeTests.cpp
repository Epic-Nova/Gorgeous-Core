#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "K2Nodes/GorgeousK2Node_GetObjectVariable.h"
#include "K2Nodes/GorgeousK2Node_NewObjectVariable.h"
#include "ObjectVariables/NativeObjectVariableDefinitions.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EdGraphSchema_K2.h"
#include "EdGraph/EdGraph.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousTypedObjectVariableNodeRegression,"GorgeousCore.LimitationAudit.TypedK2Nodes",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousTypedObjectVariableNodeRegression::RunTest(const FString&)
{
    UBlueprint* BP=FKismetEditorUtilities::CreateBlueprint(UObject::StaticClass(),GetTransientPackage(),MakeUniqueObjectName(GetTransientPackage(),UBlueprint::StaticClass(),TEXT("GorgeousTypedNodeAudit")),BPTYPE_Normal);
    TestNotNull(TEXT("Transient Blueprint created"),BP); if (!BP) return false;
    UEdGraph* Graph=FBlueprintEditorUtils::CreateNewGraph(BP,TEXT("AuditValue"),UEdGraph::StaticClass(),UEdGraphSchema_K2::StaticClass());
    FBlueprintEditorUtils::AddFunctionGraph<UClass>(BP,Graph,true,nullptr);
    UK2Node_FunctionEntry* Entry=nullptr;
    for (UEdGraphNode* Node:Graph->Nodes) if (auto* Candidate=Cast<UK2Node_FunctionEntry>(Node)) Entry=Candidate;
    TestNotNull(TEXT("Function entry created"),Entry); if (!Entry) return false;
    auto* NewNode=NewObject<UK2Node_GorgeousNewObjectVariable>(Graph); Graph->AddNode(NewNode,false,false); NewNode->CreateNewGuid(); NewNode->AllocateDefaultPins();
    UEdGraphPin* ClassPin=NewNode->GetClassPin(); ClassPin->DefaultObject=UInteger_SOV::StaticClass(); NewNode->PinDefaultValueChanged(ClassPin);
    UEdGraphPin* ValuePin=NewNode->FindPin(TEXT("Value")); TestNotNull(TEXT("Exposed Value spawn pin exists"),ValuePin); if (!ValuePin) return false; ValuePin->DefaultValue=TEXT("271");
    auto* Get=NewObject<UGorgeousK2Node_GetObjectVariable>(Graph); Graph->AddNode(Get,false,false); Get->CreateNewGuid(); Get->AllocateDefaultPins();
    const auto* Schema=GetDefault<UEdGraphSchema_K2>();
    TestTrue(TEXT("Create output connects to typed getter"),Schema->TryCreateConnection(NewNode->GetResultPin(),Get->FindPinChecked(TEXT("Target"))));
    TestEqual(TEXT("Getter resolves int pin"),Get->FindPinChecked(TEXT("ReturnValue"))->PinType.PinCategory,UEdGraphSchema_K2::PC_Int);
    auto* Result=NewObject<UK2Node_FunctionResult>(Graph); Graph->AddNode(Result,false,false); Result->CreateNewGuid(); Result->FunctionReference=Entry->FunctionReference; Result->AllocateDefaultPins();
    FEdGraphPinType ReturnType; ReturnType.PinCategory=UEdGraphSchema_K2::PC_Int;
    UEdGraphPin* ReturnPin=Result->CreateUserDefinedPin(TEXT("ReturnValue"),ReturnType,EGPD_Input);
    TestTrue(TEXT("Entry connects to factory"),Schema->TryCreateConnection(Entry->FindPinChecked(UEdGraphSchema_K2::PN_Then),NewNode->GetExecPin()));
    TestTrue(TEXT("Factory then connects to function return"),Schema->TryCreateConnection(NewNode->GetThenPin(),Result->FindPinChecked(UEdGraphSchema_K2::PN_Execute)));
    TestTrue(TEXT("Typed value connects to return"),Schema->TryCreateConnection(Get->FindPinChecked(TEXT("ReturnValue")),ReturnPin));
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
    FKismetEditorUtilities::CompileBlueprint(BP);
    TestFalse(TEXT("Typed New/Get graph compiles"),BP->Status==BS_Error);
    if (BP->Status==BS_Error || !BP->GeneratedClass) return false;
    UObject* Instance=NewObject<UObject>(GetTransientPackage(),BP->GeneratedClass);
    UFunction* Function=Instance->FindFunction(TEXT("AuditValue")); TestNotNull(TEXT("Compiled function exists"),Function); if (!Function) return false;
    struct { int32 ReturnValue=0; } Parameters;
    Instance->ProcessEvent(Function,&Parameters);
    TestEqual(TEXT("Factory registration, spawn assignment and typed read execute in Blueprint bytecode"),Parameters.ReturnValue,271);
    return true;
}
#endif
