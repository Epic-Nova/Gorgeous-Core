#pragma once
#include "GeneralSystems/VisualDataGathering/GorgeousVDGConsent.h"
#include "K2Node_AsyncAction.h"
#include "K2Node_CustomEvent.h"
#include "EdGraphSchema_K2.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "KismetCompiler.h"
#include "Misc/SecureHash.h"
#include "Misc/Parse.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "HAL/FileManager.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/SavePackage.h"
#include "GeneralSystems/VisualDataGathering/GorgeousVDGStoreRouting.h"
#include "K2Node_CallFunction.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_Select.h"
#include "K2Node_VariableGet.h"
#include "K2Node_Knot.h"

/** Preserve the existing setter, but pass the selected persistent map by reference. */
static int32 RunGorgeousVDGStoreCommitPatch(const FString& Params)
{
    const bool bApply=FParse::Param(*Params,TEXT("PatchVDGConsent")) && FParse::Param(*Params,TEXT("Apply"));
    const FString AssetPath=TEXT("/GorgeousCore/Systems/VisualDataGeathering/Classes/ActorComponents/BP_VisualDataGeatheringManager_AC");
    auto Report=MakeShared<FJsonObject>();
    Report->SetStringField(TEXT("Asset"),AssetPath); Report->SetBoolField(TEXT("Apply"),bApply); Report->SetBoolField(TEXT("Saved"),false);
    auto Finish=[&](int32 Code,const FString& Message)
    {
        Report->SetNumberField(TEXT("ExitCode"),Code); Report->SetStringField(TEXT("Message"),Message);
        FString Text; auto Writer=TJsonWriterFactory<>::Create(&Text); FJsonSerializer::Serialize(Report,Writer);
        const FString Filename=FPaths::ProjectSavedDir()/TEXT("GorgeousThings/VDGStoreCommitPatch.json");
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename),true);
        if (!FFileHelper::SaveStringToFile(Text,*Filename)) return 1;
        UE_LOG(LogTemp,Display,TEXT("VDG_STORE_COMMIT: %s; code=%d"),*Message,Code); return Code;
    };
    auto* Blueprint=LoadObject<UBlueprint>(nullptr,*AssetPath);
    if (!Blueprint) return Finish(1,TEXT("Manager Blueprint did not load"));
    TArray<UEdGraph*> Graphs; Blueprint->GetAllGraphs(Graphs);
    UEdGraph* SetterGraph=nullptr; UEdGraph* WriteGraph=nullptr; int32 SetterGraphs=0,WriteGraphs=0;
    for (auto* Graph:Graphs)
    {
        if (Graph->GetFName()==TEXT("SetContainerValue")) { SetterGraph=Graph; ++SetterGraphs; }
        if (Graph->GetFName()==TEXT("Sync VDGProprtyWrite")) { WriteGraph=Graph; ++WriteGraphs; }
    }
    if (SetterGraphs!=1 || WriteGraphs!=1) return Finish(1,TEXT("Expected exactly one setter and existing write graph"));
    UK2Node_FunctionEntry* Entry=nullptr; int32 Entries=0;
    for (UEdGraphNode* Node:SetterGraph->Nodes) if (auto* Function=Cast<UK2Node_FunctionEntry>(Node)) { Entry=Function; ++Entries; }
    TSharedPtr<FUserPinInfo> UserMap; int32 UserMaps=0;
    if (Entry) for (const auto& Pin:Entry->UserDefinedPins) if (Pin.IsValid() && Pin->PinName==TEXT("TargetContainer")) { UserMap=Pin; ++UserMaps; }
    auto* EntryMap=Entry ? Entry->FindPin(TEXT("TargetContainer")) : nullptr;
    if (Entries!=1 || UserMaps!=1 || !EntryMap || !EntryMap->PinType.IsMap()) return Finish(1,TEXT("Unexpected existing setter map signature"));
    UK2Node_Select* Select=nullptr; UK2Node_VariableGet* Global=nullptr; UK2Node_VariableGet* Member=nullptr;
    int32 Selects=0,Globals=0,Members=0;
    TArray<UK2Node_CallFunction*> Calls;
    for (UEdGraphNode* Node:WriteGraph->Nodes)
    {
        if (auto* Call=Cast<UK2Node_CallFunction>(Node)) if (Call->FunctionReference.GetMemberName()==TEXT("SetContainerValue")) Calls.Add(Call);
        if (auto* Candidate=Cast<UK2Node_Select>(Node)) if (Candidate->FindPin(TEXT("Option 0")) && Candidate->FindPin(TEXT("Option 1"))) { Select=Candidate; ++Selects; }
        if (auto* Variable=Cast<UK2Node_VariableGet>(Node))
        {
            if (Variable->GetVarName()==TEXT("GlobalProperties")) { Global=Variable; ++Globals; }
            if (Variable->GetVarName()==TEXT("MemberProperties")) { Member=Variable; ++Members; }
        }
    }
    auto* GlobalValue=Global ? Global->FindPin(TEXT("GlobalProperties")) : nullptr;
    auto* MemberValue=Member ? Member->FindPin(TEXT("MemberProperties")) : nullptr;
    auto* Index=Select ? Select->FindPin(TEXT("Index")) : nullptr;
    auto* OptionFalse=Select ? Select->FindPin(TEXT("Option 0")) : nullptr;
    auto* OptionTrue=Select ? Select->FindPin(TEXT("Option 1")) : nullptr;
    if (Selects!=1 || Globals!=1 || Members!=1 || !GlobalValue || !MemberValue || !Index || Index->LinkedTo.Num()!=1 ||
        !OptionFalse || OptionFalse->LinkedTo.Num()!=1 || OptionFalse->LinkedTo[0]!=MemberValue ||
        !OptionTrue || OptionTrue->LinkedTo.Num()!=1 || OptionTrue->LinkedTo[0]!=GlobalValue)
        return Finish(1,TEXT("Unexpected original scope selector; no graph change"));
    auto Verify=[&]()
    {
        UFunction* Function=Blueprint->GeneratedClass ? Blueprint->GeneratedClass->FindFunctionByName(TEXT("SetContainerValue")) : nullptr;
        auto* Parameter=Function ? FindFProperty<FMapProperty>(Function,TEXT("TargetContainer")) : nullptr;
        const bool bReference=UserMap->PinType.bIsReference && !UserMap->PinType.bIsConst && Entry->FindPin(TEXT("TargetContainer"))->PinType.bIsReference &&
            Parameter && Parameter->HasAnyPropertyFlags(CPF_ReferenceParm) && !Parameter->HasAnyPropertyFlags(CPF_ConstParm);
        Report->SetBoolField(TEXT("SetterReferenceVerified"),bReference);
        UK2Node_CallFunction* GlobalCall=nullptr; UK2Node_CallFunction* MemberCall=nullptr;
        for (auto* Call:Calls)
        {
            auto* Map=Call->FindPin(TEXT("TargetContainer"));
            if (!Map || !Map->PinType.bIsReference || Map->LinkedTo.Num()!=1) continue;
            if (Map->LinkedTo[0]==GlobalValue) GlobalCall=Call;
            if (Map->LinkedTo[0]==MemberValue) MemberCall=Call;
        }
        bool bDirect=Calls.Num()==2 && GlobalCall && MemberCall;
        if (bDirect)
        {
            auto* GlobalExec=GlobalCall->FindPin(UEdGraphSchema_K2::PN_Execute);
            auto* MemberExec=MemberCall->FindPin(UEdGraphSchema_K2::PN_Execute);
            auto* GlobalThen=GlobalCall->FindPin(UEdGraphSchema_K2::PN_Then);
            auto* MemberThen=MemberCall->FindPin(UEdGraphSchema_K2::PN_Then);
            auto* Branch=GlobalExec && GlobalExec->LinkedTo.Num()==1 ? Cast<UK2Node_IfThenElse>(GlobalExec->LinkedTo[0]->GetOwningNode()) : nullptr;
            bDirect=Branch && GlobalExec->LinkedTo[0]==Branch->FindPin(UEdGraphSchema_K2::PN_Then) &&
                MemberExec && MemberExec->LinkedTo.Num()==1 && MemberExec->LinkedTo[0]==Branch->GetElsePin() &&
                Branch->GetConditionPin()->LinkedTo.Num()==1 && Branch->GetConditionPin()->LinkedTo[0]==Index->LinkedTo[0] &&
                GlobalThen && MemberThen && GlobalThen->LinkedTo.Num()==1 && MemberThen->LinkedTo.Num()==1 && GlobalThen->LinkedTo[0]==MemberThen->LinkedTo[0];
            if (bDirect)
            {
                auto* BranchExec=Branch->FindPin(UEdGraphSchema_K2::PN_Execute);
                auto* PreviousGuard=BranchExec && BranchExec->LinkedTo.Num()==1 ? Cast<UK2Node_IfThenElse>(BranchExec->LinkedTo[0]->GetOwningNode()) : nullptr;
                auto* Log=Cast<UK2Node_CallFunction>(GlobalThen->LinkedTo[0]->GetOwningNode());
                bDirect=PreviousGuard && BranchExec->LinkedTo[0]==PreviousGuard->FindPin(UEdGraphSchema_K2::PN_Then) &&
                    Log && Log->FunctionReference.GetMemberName()==TEXT("LogInformationMessage");
            }
            for (FName Name:{FName(TEXT("PropertyIdentifier")),FName(TEXT("PlayerStableID")),FName(TEXT("Value"))})
            {
                auto* A=GlobalCall->FindPin(Name); auto* B=MemberCall->FindPin(Name);
                bDirect &= A && B && A->LinkedTo==B->LinkedTo && A->DefaultValue==B->DefaultValue && A->DefaultObject==B->DefaultObject && A->DefaultTextValue.EqualTo(B->DefaultTextValue);
            }
        }
        Report->SetBoolField(TEXT("DirectStoreRoutingVerified"),bDirect);
        return bReference && bDirect;
    };
    if (Verify()) return Finish(0,TEXT("Persistent map reference and original global/member scope routing independently verified"));
    if (!bApply) return Finish(1,TEXT("Setter still mutates a temporary map; verification performed no mutation"));
    if (Calls.Num()!=1 || UserMap->PinType.bIsReference || EntryMap->PinType.bIsReference) return Finish(1,TEXT("Unexpected partial setter migration; nothing changed"));
    auto* Original=Calls[0];
    auto* OldMap=Original->FindPin(TEXT("TargetContainer"));
    auto* OldExec=Original->FindPin(UEdGraphSchema_K2::PN_Execute);
    auto* OldThen=Original->FindPin(UEdGraphSchema_K2::PN_Then);
    if (!OldMap || OldMap->LinkedTo.Num()!=1 || !OldExec || OldExec->LinkedTo.Num()!=1 || !OldThen || OldThen->LinkedTo.Num()!=1)
        return Finish(1,TEXT("Unexpected original setter call links"));
    UEdGraphPin* MapSource=OldMap->LinkedTo[0];
    for (int32 Depth=0;Depth<8;++Depth)
    {
        auto* Knot=Cast<UK2Node_Knot>(MapSource->GetOwningNode());
        if (!Knot) break;
        if (Knot->GetInputPin()->LinkedTo.Num()!=1) return Finish(1,TEXT("Unexpected map reroute fan-in"));
        MapSource=Knot->GetInputPin()->LinkedTo[0];
    }
    if (MapSource!=Select->FindPin(UEdGraphSchema_K2::PN_ReturnValue)) return Finish(1,TEXT("Original setter does not consume the known scope selection"));
    UEdGraphPin* Previous=OldExec->LinkedTo[0]; UEdGraphPin* Continuation=OldThen->LinkedTo[0];
    const FString Filename=FPackageName::LongPackageNameToFilename(AssetPath,FPackageName::GetAssetPackageExtension());
    TArray<uint8> Bytes; if (!FFileHelper::LoadFileToArray(Bytes,*Filename)) return Finish(1,TEXT("Cannot read Manager original bytes"));
    FSHAHash Hash; FSHA1::HashBuffer(Bytes.GetData(),Bytes.Num(),Hash.Hash);
    FString Expected;
    if (!FParse::Value(*Params,TEXT("VDGConsentExpectedSHA1="),Expected) || !Expected.Equals(Hash.ToString(),ESearchCase::IgnoreCase)) return Finish(1,TEXT("Explicit Manager SHA1 missing or mismatched"));
    const FString Backup=FPaths::ProjectSavedDir()/TEXT("GorgeousThings/VDGStoreCommitBefore")/FGuid::NewGuid().ToString(EGuidFormats::Digits)/TEXT("BP_VisualDataGeatheringManager_AC.uasset");
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Backup),true);
    if (IFileManager::Get().Copy(*Backup,*Filename)!=COPY_OK) return Finish(1,TEXT("Original package backup failed; no graph change"));
    Report->SetStringField(TEXT("OriginalSHA1"),Hash.ToString()); Report->SetStringField(TEXT("Backup"),Backup);
    Blueprint->Modify(); SetterGraph->Modify(); WriteGraph->Modify(); Entry->Modify();
    UserMap->PinType.bIsReference=true; UserMap->PinType.bIsConst=false;
    EntryMap->PinType.bIsReference=true; EntryMap->PinType.bIsConst=false;
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
    Original->ReconstructNode();
    auto* Alternate=NewObject<UK2Node_CallFunction>(WriteGraph);
    WriteGraph->AddNode(Alternate,false,false); Alternate->CreateNewGuid(); Alternate->FunctionReference=Original->FunctionReference; Alternate->AllocateDefaultPins();
    Alternate->NodePosX=Original->NodePosX; Alternate->NodePosY=Original->NodePosY+260;
    auto* Branch=NewObject<UK2Node_IfThenElse>(WriteGraph);
    WriteGraph->AddNode(Branch,false,false); Branch->CreateNewGuid(); Branch->AllocateDefaultPins();
    Branch->NodePosX=Original->NodePosX-240; Branch->NodePosY=Original->NodePosY;
    const auto* Schema=WriteGraph->GetSchema();
    bool bConnected=true;
    for (UEdGraphPin* Pin:Original->Pins)
        if (Pin->Direction==EGPD_Input && Pin->PinType.PinCategory!=UEdGraphSchema_K2::PC_Exec && Pin->PinName!=TEXT("TargetContainer"))
        {
            auto* Copy=Alternate->FindPin(Pin->PinName);
            if (!Copy) { bConnected=false; continue; }
            Copy->DefaultValue=Pin->DefaultValue; Copy->DefaultObject=Pin->DefaultObject; Copy->DefaultTextValue=Pin->DefaultTextValue;
            for (auto* Link:Pin->LinkedTo) bConnected &= Schema->TryCreateConnection(Link,Copy);
        }
    auto* GlobalMap=Original->FindPin(TEXT("TargetContainer")); auto* MemberMap=Alternate->FindPin(TEXT("TargetContainer"));
    auto* GlobalExec=Original->FindPin(UEdGraphSchema_K2::PN_Execute); auto* MemberExec=Alternate->FindPin(UEdGraphSchema_K2::PN_Execute);
    auto* GlobalThen=Original->FindPin(UEdGraphSchema_K2::PN_Then); auto* MemberThen=Alternate->FindPin(UEdGraphSchema_K2::PN_Then);
    if (!GlobalMap || !MemberMap || !GlobalExec || !MemberExec || !GlobalThen || !MemberThen) return Finish(1,TEXT("Setter reconstruction lost required pins; nothing saved"));
    GlobalMap->BreakAllPinLinks(); GlobalExec->BreakAllPinLinks(); GlobalThen->BreakAllPinLinks();
    bConnected &= Schema->TryCreateConnection(GlobalValue,GlobalMap);
    bConnected &= Schema->TryCreateConnection(MemberValue,MemberMap);
    bConnected &= Schema->TryCreateConnection(Previous,Branch->FindPin(UEdGraphSchema_K2::PN_Execute));
    bConnected &= Schema->TryCreateConnection(Index->LinkedTo[0],Branch->GetConditionPin());
    bConnected &= Schema->TryCreateConnection(Branch->FindPin(UEdGraphSchema_K2::PN_Then),GlobalExec);
    bConnected &= Schema->TryCreateConnection(Branch->GetElsePin(),MemberExec);
    bConnected &= Schema->TryCreateConnection(GlobalThen,Continuation);
    bConnected &= Schema->TryCreateConnection(MemberThen,Continuation);
    if (!bConnected) return Finish(1,TEXT("Direct persistent-store wiring failed; original package remains on disk"));
    Calls.Add(Alternate);
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
    FCompilerResultsLog Results; FKismetEditorUtilities::CompileBlueprint(Blueprint,EBlueprintCompileOptions::SkipGarbageCollection,&Results);
    if (Results.NumErrors || Blueprint->Status==BS_Error || !Verify()) return Finish(1,TEXT("Persistent-store graph compile/verification failed; original package remains on disk"));
    FSavePackageArgs SaveArgs; SaveArgs.TopLevelFlags=RF_Public|RF_Standalone; SaveArgs.SaveFlags=SAVE_NoError;
    if (!UPackage::SavePackage(Blueprint->GetOutermost(),Blueprint,*Filename,SaveArgs)) return Finish(1,TEXT("Manager commit save failed; retain backup"));
    Report->SetBoolField(TEXT("Saved"),true);
    return Finish(0,TEXT("Existing setter now writes the selected persistent map by reference; verify in a fresh process"));
}

/** Two existing Manager data links; explicit apply and original-byte guard. */
static int32 RunGorgeousVDGStoreAssetPatch(const FString& Params)
{
    const bool bApply=FParse::Param(*Params,TEXT("PatchVDGConsent")) && FParse::Param(*Params,TEXT("Apply"));
    const bool bPatchLookup=FParse::Param(*Params,TEXT("VDGStoreLookup"));
    const FString AssetPath=TEXT("/GorgeousCore/Systems/VisualDataGeathering/Classes/ActorComponents/BP_VisualDataGeatheringManager_AC");
    auto Report=MakeShared<FJsonObject>();
    Report->SetStringField(TEXT("Asset"),AssetPath); Report->SetBoolField(TEXT("Apply"),bApply); Report->SetBoolField(TEXT("Saved"),false);
    auto Finish=[&](int32 Code,const FString& Message)
    {
        Report->SetNumberField(TEXT("ExitCode"),Code); Report->SetStringField(TEXT("Message"),Message);
        FString Text; auto Writer=TJsonWriterFactory<>::Create(&Text); FJsonSerializer::Serialize(Report,Writer);
        const FString Filename=FPaths::ProjectSavedDir()/TEXT("GorgeousThings/VDGStoreRoutingPatch.json");
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename),true);
        if (!FFileHelper::SaveStringToFile(Text,*Filename)) return 1;
        UE_LOG(LogTemp,Display,TEXT("VDG_STORE_PATCH: %s; code=%d"),*Message,Code); return Code;
    };
    auto* Blueprint=LoadObject<UBlueprint>(nullptr,*AssetPath);
    if (!Blueprint) return Finish(1,TEXT("Manager Blueprint did not load"));
    TArray<UEdGraph*> Graphs; Blueprint->GetAllGraphs(Graphs);
    UEdGraph* LookupGraph=nullptr; UEdGraph* RoutingGraph=nullptr;
    int32 LookupGraphs=0,RoutingGraphs=0;
    for (auto* Graph:Graphs)
    {
        if (Graph->GetFName()==TEXT("GetContainerValue")) { LookupGraph=Graph; ++LookupGraphs; }
        if (Graph->GetFName()==TEXT("ShouldProcessSync")) { RoutingGraph=Graph; ++RoutingGraphs; }
    }
    if (LookupGraphs!=1 || RoutingGraphs!=1) return Finish(1,TEXT("Expected exactly one public lookup and routing graph"));
    UK2Node_CallFunction* Contains=nullptr; UK2Node_CallFunction* Token=nullptr; UK2Node_CallFunction* Existing=nullptr;
    UK2Node_FunctionResult* Result=nullptr;
    int32 ContainsCount=0,TokenCount=0,ResultCount=0,HelperCount=0;
    UFunction* Helper=UGorgeousVDGStoreRouting::StaticClass()->FindFunctionByName(TEXT("ShouldUseLocalVDGStore"));
    if (!Helper) return Finish(1,TEXT("Native active-store helper is unavailable"));
    for (UEdGraphNode* Node:LookupGraph->Nodes) if (auto* Call=Cast<UK2Node_CallFunction>(Node))
    {
        if (Call->FunctionReference.GetMemberName()==TEXT("Map_Contains")) { Contains=Call; ++ContainsCount; }
        if (Call->FunctionReference.GetMemberName()==TEXT("GetSearchToken")) { Token=Call; ++TokenCount; }
    }
    for (UEdGraphNode* Node:RoutingGraph->Nodes)
    {
        if (auto* Return=Cast<UK2Node_FunctionResult>(Node)) { Result=Return; ++ResultCount; }
        if (auto* Call=Cast<UK2Node_CallFunction>(Node)) if (Call->GetTargetFunction()==Helper) { Existing=Call; ++HelperCount; }
    }
    auto* ContainsKey=Contains ? Contains->FindPin(TEXT("Key")) : nullptr;
    auto* TokenValue=Token ? Token->FindPin(UEdGraphSchema_K2::PN_ReturnValue) : nullptr;
    auto* ResultValue=Result ? Result->FindPin(UEdGraphSchema_K2::PN_ReturnValue) : nullptr;
    if (ContainsCount!=1 || TokenCount!=1 || ResultCount!=1 || HelperCount>1 || !ContainsKey || !TokenValue || !ResultValue)
        return Finish(1,TEXT("Unexpected Manager lookup/routing node structure"));
    const bool bLookup=ContainsKey->LinkedTo.Num()==1 && ContainsKey->LinkedTo[0]==TokenValue;
    const bool bRouting=Existing && ResultValue->LinkedTo.Num()==1 && ResultValue->LinkedTo[0]==Existing->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
    Report->SetBoolField(TEXT("LookupVerified"),bLookup); Report->SetBoolField(TEXT("RoutingVerified"),bRouting);
    if (bRouting && (!bPatchLookup || bLookup)) return Finish(0,bPatchLookup ? TEXT("Existing active routing and identical lookup token independently verified") : TEXT("Existing active routing independently verified; lookup unchanged"));
    if (!bApply) return Finish(1,TEXT("Existing Manager routing/lookup defects remain; verification performed no mutation"));
    // Accept only the known original scalar-key edge, or our already-fixed edge.
    if (bPatchLookup && !bLookup && (ContainsKey->LinkedTo.Num()!=1 || ContainsKey->LinkedTo[0]->PinName!=TEXT("PropertyIdentifier")))
        return Finish(1,TEXT("Unexpected original lookup edge; no package change"));
    if (!bRouting && (Existing || ResultValue->LinkedTo.Num()!=1)) return Finish(1,TEXT("Unexpected original routing edge; no package change"));
    const FString Filename=FPackageName::LongPackageNameToFilename(AssetPath,FPackageName::GetAssetPackageExtension());
    TArray<uint8> Bytes; if (!FFileHelper::LoadFileToArray(Bytes,*Filename)) return Finish(1,TEXT("Cannot read Manager package"));
    FSHAHash Hash; FSHA1::HashBuffer(Bytes.GetData(),Bytes.Num(),Hash.Hash);
    FString Expected;
    if (!FParse::Value(*Params,TEXT("VDGConsentExpectedSHA1="),Expected) || !Expected.Equals(Hash.ToString(),ESearchCase::IgnoreCase))
        return Finish(1,TEXT("Explicit original-package SHA1 missing or mismatched"));
    const FString Backup=FPaths::ProjectSavedDir()/TEXT("GorgeousThings/VDGStoreBefore")/FGuid::NewGuid().ToString(EGuidFormats::Digits)/TEXT("BP_VisualDataGeatheringManager_AC.uasset");
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Backup),true);
    if (IFileManager::Get().Copy(*Backup,*Filename)!=COPY_OK) return Finish(1,TEXT("Manager backup failed; no graph change"));
    Report->SetStringField(TEXT("OriginalSHA1"),Hash.ToString()); Report->SetStringField(TEXT("Backup"),Backup);
    Blueprint->Modify(); LookupGraph->Modify(); RoutingGraph->Modify();
    if (bPatchLookup && !bLookup)
    {
        ContainsKey->BreakAllPinLinks();
        if (!LookupGraph->GetSchema()->TryCreateConnection(TokenValue,ContainsKey)) return Finish(1,TEXT("Lookup token connection failed; nothing saved"));
    }
    if (!bRouting)
    {
        auto* Call=NewObject<UK2Node_CallFunction>(RoutingGraph);
        RoutingGraph->AddNode(Call,false,false); Call->CreateNewGuid(); Call->SetFromFunction(Helper); Call->AllocateDefaultPins();
        Call->NodePosX=Result->NodePosX-320; Call->NodePosY=Result->NodePosY;
        auto* Output=Call->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
        if (!Output) return Finish(1,TEXT("Active-store helper output missing; nothing saved"));
        ResultValue->BreakAllPinLinks();
        if (!RoutingGraph->GetSchema()->TryCreateConnection(Output,ResultValue)) return Finish(1,TEXT("Active-store connection failed; nothing saved"));
    }
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
    FCompilerResultsLog Results; FKismetEditorUtilities::CompileBlueprint(Blueprint,EBlueprintCompileOptions::SkipGarbageCollection,&Results);
    if (Results.NumErrors || Blueprint->Status==BS_Error) return Finish(1,TEXT("Manager compile failed; original package remains on disk"));
    FSavePackageArgs SaveArgs; SaveArgs.TopLevelFlags=RF_Public|RF_Standalone; SaveArgs.SaveFlags=SAVE_NoError;
    if (!UPackage::SavePackage(Blueprint->GetOutermost(),Blueprint,*Filename,SaveArgs)) return Finish(1,TEXT("Manager save failed; retain backup"));
    Report->SetBoolField(TEXT("Saved"),true);
    return Finish(0,TEXT("Manager active-store routing and shared lookup token compiled and saved; verify fresh process"));
}

/** Explicit, one-asset migration. Verification never changes or saves content. */
static int32 RunGorgeousVDGConsentAssetPatch(const FString& Params)
{
    if (FParse::Param(*Params,TEXT("VDGStoreCommit"))) return RunGorgeousVDGStoreCommitPatch(Params);
    if (FParse::Param(*Params,TEXT("VDGStoreRouting")) || FParse::Param(*Params,TEXT("VDGStoreLookup"))) return RunGorgeousVDGStoreAssetPatch(Params);
    const bool bApply=FParse::Param(*Params,TEXT("PatchVDGConsent")) && FParse::Param(*Params,TEXT("Apply"));
    const FString AssetPath=TEXT("/GorgeousCore/Systems/VisualDataGeathering/Classes/ActorComponents/BP_VisualDataGeatheringInteractor_AC");
    auto Report=MakeShared<FJsonObject>();
    Report->SetStringField(TEXT("Asset"),AssetPath);
    Report->SetBoolField(TEXT("Apply"),bApply);
    Report->SetBoolField(TEXT("Saved"),false);
    auto Finish=[&](int32 Code,const FString& Message)
    {
        Report->SetNumberField(TEXT("ExitCode"),Code); Report->SetStringField(TEXT("Message"),Message);
        FString Text; auto Writer=TJsonWriterFactory<>::Create(&Text); FJsonSerializer::Serialize(Report,Writer);
        const FString Filename=FPaths::ProjectSavedDir()/TEXT("GorgeousThings/VDGConsentPatch.json");
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename),true);
        if (!FFileHelper::SaveStringToFile(Text,*Filename)) return 1;
        UE_LOG(LogTemp,Display,TEXT("VDG_CONSENT_PATCH: %s; code=%d; report=%s"),*Message,Code,*Filename);
        return Code;
    };
    auto* Blueprint=LoadObject<UBlueprint>(nullptr,*AssetPath);
    if (!Blueprint) return Finish(1,TEXT("Interactor Blueprint did not load"));
    TArray<UEdGraph*> Graphs; Blueprint->GetAllGraphs(Graphs);
    UK2Node_CustomEvent* Entry=nullptr; UK2Node_AsyncAction* Existing=nullptr;
    int32 MatchingEvents=0,ConsentNodes=0;
    for (UEdGraph* Graph:Graphs) for (UEdGraphNode* Node:Graph->Nodes)
    {
        if (auto* Event=Cast<UK2Node_CustomEvent>(Node)) if (Event->CustomFunctionName==TEXT("GeatherBySchemaClient")) { Entry=Event; ++MatchingEvents; }
        if (auto* Action=Cast<UK2Node_AsyncAction>(Node))
            if (Action->GetFactoryFunction()==UAsyncAction_RequestVDGConsent::StaticClass()->FindFunctionByName(TEXT("RequestVDGConsent"))) { Existing=Action; ++ConsentNodes; }
    }
    if (MatchingEvents!=1 || !Entry) return Finish(1,TEXT("Expected exactly one existing gathering event"));
    UEdGraphPin* Then=Entry->FindPin(UEdGraphSchema_K2::PN_Then);
    if (!Then || Then->LinkedTo.Num()!=1) return Finish(1,TEXT("Gathering event must have exactly one original execution edge"));
    if (Existing)
    {
        UEdGraphPin* Execute=Existing->FindPin(UEdGraphSchema_K2::PN_Execute);
        UEdGraphPin* Accepted=Existing->FindPin(TEXT("OnAccepted"));
        const bool bValid=ConsentNodes==1 && Execute && Then->LinkedTo[0]==Execute && Accepted && Accepted->LinkedTo.Num()==1;
        Report->SetBoolField(TEXT("GateVerified"),bValid);
        return Finish(bValid ? 0 : 1,bValid ? TEXT("Existing consent gate independently verified") : TEXT("Existing gate wiring is not valid"));
    }
    Report->SetBoolField(TEXT("GateVerified"),false);
    if (!bApply) return Finish(1,TEXT("Consent gate is absent; verification performed no mutation"));
    const FString Filename=FPackageName::LongPackageNameToFilename(AssetPath,FPackageName::GetAssetPackageExtension());
    TArray<uint8> Bytes; if (!FFileHelper::LoadFileToArray(Bytes,*Filename)) return Finish(1,TEXT("Cannot read original package for backup"));
    FSHAHash Hash; FSHA1::HashBuffer(Bytes.GetData(),Bytes.Num(),Hash.Hash);
    FString Expected;
    if (!FParse::Value(*Params,TEXT("VDGConsentExpectedSHA1="),Expected) || !Expected.Equals(Hash.ToString(),ESearchCase::IgnoreCase))
        return Finish(1,TEXT("Explicit original-package SHA1 guard missing or mismatched"));
    const FString Backup=FPaths::ProjectSavedDir()/TEXT("GorgeousThings/VDGConsentBefore")/FGuid::NewGuid().ToString(EGuidFormats::Digits)/TEXT("BP_VisualDataGeatheringInteractor_AC.uasset");
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Backup),true);
    if (IFileManager::Get().Copy(*Backup,*Filename)!=COPY_OK) return Finish(1,TEXT("Original package backup failed; no graph change made"));
    Report->SetStringField(TEXT("OriginalSHA1"),Hash.ToString()); Report->SetStringField(TEXT("Backup"),Backup);
    UEdGraphPin* OriginalDestination=Then->LinkedTo[0];
    Blueprint->Modify(); Entry->GetGraph()->Modify();
    auto* Gate=NewObject<UK2Node_AsyncAction>(Entry->GetGraph());
    Entry->GetGraph()->AddNode(Gate,false,false); Gate->CreateNewGuid();
    Gate->InitializeProxyFromFunction(UAsyncAction_RequestVDGConsent::StaticClass()->FindFunctionByName(TEXT("RequestVDGConsent")));
    Gate->AllocateDefaultPins(); Gate->NodePosX=Entry->NodePosX+240; Gate->NodePosY=Entry->NodePosY;
    Gate->NodeComment=TEXT("Explicit local-player VDG session consent. Declining stops collection.");
    UEdGraphPin* Execute=Gate->FindPin(UEdGraphSchema_K2::PN_Execute);
    UEdGraphPin* Accepted=Gate->FindPin(TEXT("OnAccepted"));
    if (!Execute || !Accepted) { Gate->DestroyNode(); return Finish(1,TEXT("Native consent proxy pins missing; original graph edge retained")); }
    Then->BreakLinkTo(OriginalDestination);
    const UEdGraphSchema* Schema=Entry->GetGraph()->GetSchema();
    if (!Schema->TryCreateConnection(Then,Execute) || !Schema->TryCreateConnection(Accepted,OriginalDestination))
    {
        Gate->DestroyNode(); Schema->TryCreateConnection(Then,OriginalDestination);
        return Finish(1,TEXT("Gate connection failed; original in-memory edge restored and nothing saved"));
    }
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
    FCompilerResultsLog Results; FKismetEditorUtilities::CompileBlueprint(Blueprint,EBlueprintCompileOptions::SkipGarbageCollection,&Results);
    if (Results.NumErrors || Blueprint->Status==BS_Error) return Finish(1,TEXT("Patched Blueprint did not compile; original package remains on disk"));
    FSavePackageArgs SaveArgs; SaveArgs.TopLevelFlags=RF_Public|RF_Standalone; SaveArgs.SaveFlags=SAVE_NoError;
    if (!UPackage::SavePackage(Blueprint->GetOutermost(),Blueprint,*Filename,SaveArgs)) return Finish(1,TEXT("Patched package save failed; retain explicit backup"));
    Report->SetBoolField(TEXT("Saved"),true); Report->SetBoolField(TEXT("GateVerified"),true);
    Report->SetStringField(TEXT("AcceptedDestination"),OriginalDestination->GetOwningNode()->NodeGuid.ToString());
    return Finish(0,TEXT("One consent gate added, compiled and saved; run fresh-process verification next"));
}
