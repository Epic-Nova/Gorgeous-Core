// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "Commandlets/GorgeousAuditFixupCommandlet.h"
#include "Commandlets/GorgeousVDGConsentAssetPatch.h"
#include "Commandlets/GorgeousGuardRequirementsExporter.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Blueprint.h"
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuard.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "KismetCompiler.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/FileHelper.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/LinkerLoad.h"
#include "UObject/UObjectHash.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousPrimaryGameLayout.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSettings.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/GeneralSystemConfiguration_PDA.h"
#include "GeneralSystems/GorgeousProjectSettingsContribution.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Variable.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_MacroInstance.h"
#include "UObject/UnrealType.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Factories/WorldFactory.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/SpectatorPawn.h"
#include "QualityOfLife/GorgeousGameMode.h"
#include "QualityOfLife/GorgeousGameState.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "QualityOfLife/GorgeousPlayerState.h"
#include "QualityOfLife/GorgeousWorldSettings.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousHUD.h"

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousAuditFixup, Log, All);

namespace
{
const TCHAR* VDGSurveyRowPackage = TEXT("/GorgeousCore/Systems/VisualDataGeathering/UserInterfaces/Defaults/BP_VDG_SurveyRow_CUW");

// Mirror the engine's parent-first redirect lookup through its public Blueprint
// API. UStruct::FindPropertyNameFromGuid is protected on UE5.7.
FName ResolveBlueprintPropertyNameFromGuid(const UClass* Class, const FGuid& Guid)
{
    if (const auto* BlueprintClass = Cast<UBlueprintGeneratedClass>(Class))
    {
        const FName Inherited = ResolveBlueprintPropertyNameFromGuid(BlueprintClass->GetSuperClass(), Guid);
        if (!Inherited.IsNone()) return Inherited;
        return BlueprintClass->FindBlueprintPropertyNameFromGuid(Guid);
    }
    return NAME_None;
}

/** Read-only, export-bounded reader for the original version1012 tagged data.
 * Name and export tables come from the actual engine linker; no guessed strings,
 * object loading through an untyped archive, shared-linker seeks or binary writes.
 */
struct FVDGLegacyReader
{
    const TArray<uint8>& Bytes;
    const TArray<FNameEntryId>& Names;
    int64 Position;
    const int64 End;
    template<typename T> bool Read(T& Value)
    {
        if (Position < 0 || Position > End || int64(sizeof(T)) > End - Position || End > Bytes.Num()) return false;
        FMemory::Memcpy(&Value, Bytes.GetData() + Position, sizeof(T));
        Position += sizeof(T);
        return true;
    }
    bool Name(FName& Value)
    {
        int32 Index = INDEX_NONE, Number = -1;
        if (!Read(Index) || !Read(Number) || !Names.IsValidIndex(Index) || Number < 0) return false;
        Value = FName::CreateFromDisplayId(Names[Index], Number);
        return true;
    }
};

/** Recovers the actual serialized names before the current FString schema loses them.
 * This deliberately supports only the observed legacy package version/layout.
 * Unknown layouts fail the audit rather than saving guessed or empty filters.
 */
bool RecoverLegacyVDGCompatibility(UObject* Root, const TArray<uint8>& Bytes, bool bApply,
    bool& bChanged, TArray<TSharedPtr<FJsonValue>>& Evidence)
{
    FLinkerLoad* Linker = Root->GetLinker();
    if (!Linker || Linker->LinkerRoot != Root->GetOutermost())
    {
        UE_LOG(LogGorgeousAuditFixup, Error, TEXT("Cannot inspect original VDG compatibility: package linker unavailable."));
        return false;
    }
    // Current resaves have FString tags. Independent reload still emits the
    // actual live arrays below; no legacy binary interpretation is attempted.
    if (Linker->Summary.GetFileVersionUE().FileVersionUE5 != 1012) return true;
    int32 LegacyArrays = 0;
    TArray<TPair<UObject*, TArray<FString>>> PendingRepairs;
    for (int32 ExportIndex = 0; ExportIndex < Linker->ExportMap.Num(); ++ExportIndex)
    {
        const FObjectExport& Export = Linker->ExportMap[ExportIndex];
        if (Export.ObjectName != TEXT("VDG_NamedSlot")) continue;
        UObject* Target = Export.Object;
        if (!IsValid(Target) || Target->GetOutermost() != Root->GetOutermost()) return false;
        FArrayProperty* Array = FindFProperty<FArrayProperty>(Target->GetClass(), TEXT("CompatibleParameters"));
        FStrProperty* String = Array ? CastField<FStrProperty>(Array->Inner) : nullptr;
        if (!String || Export.SerialOffset < 0 || Export.SerialSize < 0 ||
            Export.SerialOffset > Bytes.Num() || Export.SerialSize > Bytes.Num() - Export.SerialOffset ||
            Export.ScriptSerializationStartOffset < 0 || Export.ScriptSerializationEndOffset < Export.ScriptSerializationStartOffset ||
            Export.ScriptSerializationEndOffset > Export.SerialSize) return false;
        FVDGLegacyReader Reader{Bytes, Linker->NameMap,
            Export.SerialOffset + Export.ScriptSerializationStartOffset,
            Export.SerialOffset + Export.ScriptSerializationEndOffset};
        uint8 ClassExtensions = 0xff;
        if (!Reader.Read(ClassExtensions) || ClassExtensions != 0) return false;
        bool bTerminated = false;
        for (int32 TagIndex = 0; TagIndex < 10000 && Reader.Position < Reader.End; ++TagIndex)
        {
            FName StoredName;
            if (!Reader.Name(StoredName)) return false;
            if (StoredName.IsNone()) { bTerminated = true; break; }
            TArray<TPair<FName, int32>, TInlineAllocator<8>> Types;
            int32 Remaining = 1;
            while (Remaining > 0)
            {
                FName Type;
                int32 Children = 0;
                if (Types.Num() >= 64 || !Reader.Name(Type) || !Reader.Read(Children) || Children < 0 || Children > 16) return false;
                Types.Emplace(Type, Children);
                Remaining += Children - 1;
            }
            int32 Size = -1, ArrayIndex = 0;
            uint8 Flags = 0xff;
            if (!Reader.Read(Size) || !Reader.Read(Flags) || Size < 0 || (Flags & ~uint8(0x3f))) return false;
            if ((Flags & 1) && !Reader.Read(ArrayIndex)) return false;
            FGuid Guid;
            if ((Flags & 2) && !Reader.Read(Guid)) return false;
            if (Flags & 4)
            {
                uint8 Extensions = 0xff;
                if (!Reader.Read(Extensions) || Extensions != 0) return false;
            }
            if (Size > Reader.End - Reader.Position) return false;
            const int64 PayloadEnd = Reader.Position + Size;
            const FName ResolvedName = Guid.IsValid() ? ResolveBlueprintPropertyNameFromGuid(Target->GetClass(), Guid) : StoredName;
            if (ResolvedName == TEXT("CompatibleParameters"))
            {
                if (ArrayIndex != 0 || Types.Num() != 2 || Types[0].Key != NAME_ArrayProperty || Types[0].Value != 1 ||
                    Types[1].Key != NAME_NameProperty || Types[1].Value != 0) return false;
                int32 Count = -1;
                if (!Reader.Read(Count) || Count < 0 || Count > 10000 || int64(Size) != 4 + int64(Count) * 8) return false;
                TArray<FString> Recovered;
                TArray<TSharedPtr<FJsonValue>> OriginalNames;
                for (int32 Index = 0; Index < Count; ++Index)
                {
                    FName Value;
                    if (!Reader.Name(Value)) return false;
                    Recovered.Add(Value.ToString());
                    OriginalNames.Add(MakeShared<FJsonValueString>(Value.ToString()));
                }
                if (Reader.Position != PayloadEnd) return false;
                FScriptArrayHelper Current(Array, Array->ContainerPtrToValuePtr<void>(Target));
                TArray<TSharedPtr<FJsonValue>> LoadedBefore;
                for (int32 Index = 0; Index < Current.Num(); ++Index)
                    LoadedBefore.Add(MakeShared<FJsonValueString>(String->GetPropertyValue(Current.GetRawPtr(Index))));
                TSharedRef<FJsonObject> Item = MakeShared<FJsonObject>();
                Item->SetNumberField(TEXT("ExportIndex"), ExportIndex);
                Item->SetStringField(TEXT("Object"), Target->GetPathName());
                Item->SetStringField(TEXT("StoredProperty"), StoredName.ToString());
                Item->SetStringField(TEXT("PropertyGuid"), Guid.ToString());
                Item->SetArrayField(TEXT("OriginalSerializedNames"), OriginalNames);
                Item->SetArrayField(TEXT("LoadedBeforeRepair"), LoadedBefore);
                Item->SetBoolField(TEXT("RequestedApply"), bApply);
                Item->SetBoolField(TEXT("Applied"), false);
                Evidence.Add(MakeShared<FJsonValueObject>(Item));
                UE_LOG(LogGorgeousAuditFixup, Display, TEXT("Recovered original VDG Name-array export=%d object=%s field=%s count=%d values=[%s] apply=%s"),
                    ExportIndex, *Target->GetPathName(), *StoredName.ToString(), Count, *FString::Join(Recovered, TEXT(", ")), bApply ? TEXT("true") : TEXT("false"));
                PendingRepairs.Emplace(Target, MoveTemp(Recovered));
                ++LegacyArrays;
            }
            Reader.Position = PayloadEnd;
        }
        if (!bTerminated) return false;
    }
    // The original asset contains exactly two corresponding widget exports.
    // Refuse a changed structure rather than silently dropping a filter.
    if (LegacyArrays != 2) return false;
    if (!bApply)
    {
        UE_LOG(LogGorgeousAuditFixup, Warning, TEXT("Two legacy VDG compatibility filters need explicit -Apply; original values recovered without saving."));
        return false;
    }
    // Decode and validate both exports completely before mutating either one.
    for (const auto& Repair : PendingRepairs)
    {
        FArrayProperty* Array = FindFProperty<FArrayProperty>(Repair.Key->GetClass(), TEXT("CompatibleParameters"));
        FStrProperty* String = CastFieldChecked<FStrProperty>(Array->Inner);
        Repair.Key->Modify();
        FScriptArrayHelper Current(Array, Array->ContainerPtrToValuePtr<void>(Repair.Key));
        Current.EmptyAndAddValues(Repair.Value.Num());
        for (int32 Index = 0; Index < Repair.Value.Num(); ++Index)
            String->SetPropertyValue(Current.GetRawPtr(Index), Repair.Value[Index]);
        Repair.Key->MarkPackageDirty();
    }
    bChanged = true;
    for (const auto& Item : Evidence) Item->AsObject()->SetBoolField(TEXT("Applied"), true);
    return true;
}

void CollectVDGCompatibility(UObject* Root, TArray<TSharedPtr<FJsonValue>>& Evidence)
{
    TArray<UObject*> Objects;
    GetObjectsWithPackage(Root->GetOutermost(), Objects);
    for (UObject* Object : Objects)
    {
        FArrayProperty* Array = FindFProperty<FArrayProperty>(Object->GetClass(), TEXT("CompatibleParameters"));
        FStrProperty* String = Array ? CastField<FStrProperty>(Array->Inner) : nullptr;
        if (!String) continue;
        FScriptArrayHelper Current(Array, Array->ContainerPtrToValuePtr<void>(Object));
        TSharedRef<FJsonObject> Item = MakeShared<FJsonObject>();
        Item->SetStringField(TEXT("Object"), Object->GetPathName());
        TArray<TSharedPtr<FJsonValue>> Values;
        for (int32 Index = 0; Index < Current.Num(); ++Index)
            Values.Add(MakeShared<FJsonValueString>(String->GetPropertyValue(Current.GetRawPtr(Index))));
        Item->SetArrayField(TEXT("CurrentStrings"), Values);
        Evidence.Add(MakeShared<FJsonValueObject>(Item));
    }
}

bool ContainsSerializedToken(const TArray<uint8>& Bytes, const ANSICHAR* Token)
{
	const int32 Length = FCStringAnsi::Strlen(Token);
	for (int32 Offset = 0; Offset <= Bytes.Num() - Length; ++Offset)
	{
		if (FMemory::Memcmp(Bytes.GetData() + Offset, Token, Length) == 0) return true;
	}
	return false;
}
}

UGorgeousAuditFixupCommandlet::UGorgeousAuditFixupCommandlet()
{
	IsEditor = true;
	IsClient = false;
	IsServer = false;
	LogToConsole = true;
	HelpDescription = TEXT("Validate bundled resource guards and compile all Gorgeous Core Blueprints. -Apply explicitly saves repairs and redirected assets.");
}

int32 UGorgeousAuditFixupCommandlet::Main(const FString& Params)
{
	if (FParse::Param(*Params, TEXT("ExportGuardRequirements"))) return GorgeousGuardRequirements::Export(Params);
    if (FParse::Param(*Params,TEXT("PatchVDGConsent")) || FParse::Param(*Params,TEXT("VerifyVDGConsent")))
        return RunGorgeousVDGConsentAssetPatch(Params);
	const bool bApply = FParse::Param(*Params, TEXT("Apply"));
	const bool bDumpGraphs = FParse::Param(*Params, TEXT("DumpGraphs"));
	const bool bCreateTestMap = FParse::Param(*Params, TEXT("CreateTestMap"));
	TArray<TSharedPtr<FJsonValue>> GraphDumpAssets;
    TArray<TSharedPtr<FJsonValue>> VDGRecoveryEvidence, VDGCurrentEvidence;
    TArray<TSharedPtr<FJsonValue>> ReferenceAudit;
	const TSet<FName> GraphDumpTargets = {
		TEXT("BP_TeamManager_AC"), TEXT("BP_TeamInteractor_AC"), TEXT("BP_TeamPlayerModifier_A"),
		TEXT("BP_PlaylistObject"), TEXT("BP_PlaylistHandling_AC"),
		TEXT("BP_VisualDataGeatheringInteractor_AC"), TEXT("BP_Reconnect_BFL"), TEXT("BP_InteractionCharacterHandler_AC"), TEXT("BP_InteractionBaseHandler_AC"), TEXT("BP_TeamCard_CUW"),
        TEXT("BP_VDG_SurveyCard_CUW"), TEXT("BP_VDG_Survey_Default_Schema_CUW"),
        TEXT("BP_VDG_NamedSlot"), TEXT("BP_HorizontallyAlligned_VDGP")
	};
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	Registry.SearchAllAssets(true);
	TSet<UPackage*> PackagesToSave;
	int32 Failures = 0;
	int32 CompiledBlueprints = 0;
	int32 RepairedGuards = 0;
	const TMap<FString, TArray<FName>> BundledRequirements = {
		{ TEXT("DA_General"), { FName(TEXT("DataValidation")) } },
		{ TEXT("DA_Playlist"), { FName(TEXT("DataRegistry")), FName(TEXT("GameFeatures")) } },
		{ TEXT("DA_Team"), { FName(TEXT("CommonUI")) } },
		{ TEXT("DA_VisualDataGeathering"), { FName(TEXT("CommonUI")) } },
		{ TEXT("DA_CommonUIFoundation"), { FName(TEXT("CommonUI")) } }
	};

	TArray<FAssetData> GuardAssets;
	Registry.GetAssetsByPath(FName(TEXT("/GorgeousCore/Data/CoreData/ExtensionResourceGuards")), GuardAssets, true);
	for (const FAssetData& Asset : GuardAssets)
	{
		UGorgeousExtensionResourceGuard* Guard = Cast<UGorgeousExtensionResourceGuard>(Asset.GetAsset());
		const TArray<FName>* Expected = BundledRequirements.Find(Asset.AssetName.ToString());
		if (!Guard || !Expected) continue;
		bool bChanged = false;
		for (FName Plugin : *Expected)
		{
			if (Guard->RequiredPlugins.Contains(Plugin)) continue;
			UE_LOG(LogGorgeousAuditFixup, Warning, TEXT("Missing serialized guard requirement: %s -> %s"), *Guard->GetPathName(), *Plugin.ToString());
			if (bApply)
			{
				Guard->Modify();
				Guard->RequiredPlugins.Add(Plugin, false);
				bChanged = true;
			}
			else ++Failures;
		}
		if (bChanged)
		{
			++RepairedGuards;
			Guard->MarkPackageDirty();
			PackagesToSave.Add(Guard->GetOutermost());
		}
	}

	const UGorgeousUIFoundationSettings* Settings = GetDefault<UGorgeousUIFoundationSettings>();
	if (!Settings->DefaultLayoutClass.LoadSynchronous())
	{
		UE_LOG(LogGorgeousAuditFixup, Error, TEXT("Default layout class does not load: %s"), *Settings->DefaultLayoutClass.ToString());
		++Failures;
	}
	if (!Settings->DefaultTheme.LoadSynchronous())
	{
		UE_LOG(LogGorgeousAuditFixup, Error, TEXT("Default theme does not load: %s"), *Settings->DefaultTheme.ToString());
		++Failures;
	}

	TArray<FAssetData> CoreAssets;
	Registry.GetAssetsByPath(FName(TEXT("/GorgeousCore")), CoreAssets, true);
	for (const FAssetData& Asset : CoreAssets)
	{
        if (bDumpGraphs && (GraphDumpTargets.Contains(Asset.AssetName) ||
            Asset.AssetName == TEXT("BP_Playlist_GGFD") || Asset.AssetName == TEXT("VisualDataGeatheringParameter_S") ||
            Asset.AssetName == TEXT("BP_Difficulty_GPDA") || Asset.AssetName == TEXT("DT_DefaultPlaylists")))
        {
            TSharedRef<FJsonObject> Reference = MakeShared<FJsonObject>();
            Reference->SetStringField(TEXT("Asset"), Asset.GetSoftObjectPath().ToString());
            Reference->SetStringField(TEXT("AssetClass"), Asset.AssetClassPath.ToString());
            TArray<FName> Referencers;
            Registry.GetReferencers(Asset.PackageName, Referencers, UE::AssetRegistry::EDependencyCategory::All);
            TArray<TSharedPtr<FJsonValue>> ReferencerJson;
            for (const FName Referencer : Referencers) ReferencerJson.Add(MakeShared<FJsonValueString>(Referencer.ToString()));
            Reference->SetArrayField(TEXT("OnDiskReferencersAllMountedPlugins"), ReferencerJson);
            ReferenceAudit.Add(MakeShared<FJsonValueObject>(Reference));
        }

		TArray<uint8> SerializedBytes;
		const FString AssetFilename = FPackageName::LongPackageNameToFilename(Asset.PackageName.ToString(), FPackageName::GetAssetPackageExtension());
		FFileHelper::LoadFileToArray(SerializedBytes, *AssetFilename);
		const bool bHasLegacySerializedReferences =
			ContainsSerializedToken(SerializedBytes, "AuthorativeComponent") ||
			ContainsSerializedToken(SerializedBytes, "/GorgeousCore/Systems/VisualDataGeathering/Data/VisualDataGeatheringSlotPanel_E") ||
			ContainsSerializedToken(SerializedBytes, "/GorgeousCore/Systems/VisualDataGeathering/Data/VisualDataGeatheringSlotResolutionRule_E") ||
			(Asset.AssetName == FName(TEXT("BP_DefaultPrimaryGameLayout_CUW")) && ContainsSerializedToken(SerializedBytes, "/GorgeousInventory/"));
		bool bChangedExplicitly = false;
		UObject* Object = Asset.GetAsset();
		if (!Object)
		{
			UE_LOG(LogGorgeousAuditFixup, Error, TEXT("Asset failed to load: %s"), *Asset.GetSoftObjectPath().ToString());
			++Failures;
			continue;
		}
		UBlueprint* Blueprint = Cast<UBlueprint>(Object);
        if (Asset.PackageName == VDGSurveyRowPackage &&
            !RecoverLegacyVDGCompatibility(Object, SerializedBytes, bApply, bChangedExplicitly, VDGRecoveryEvidence))
        {
            UE_LOG(LogGorgeousAuditFixup, Error, TEXT("Original VDG compatibility filters could not be safely validated/repaired; package will not be saved."));
            ++Failures;
            continue;
        }
		if (bApply)
		{
			if (UGeneralSystemConfiguration_PDA* Configuration = Cast<UGeneralSystemConfiguration_PDA>(Object);
				Configuration)
			{
				bool bChanged = Configuration->RefreshRuntimeRequirements();
				TArray<EGorgeousProjectSettingContribution> Contributions = {
					EGorgeousProjectSettingContribution::AssetManagerSystems,
					EGorgeousProjectSettingContribution::DataRegistryDirectories };
				if (Asset.AssetName == TEXT("PDA_General")) Contributions.Add(EGorgeousProjectSettingContribution::InteractionTraceChannel);
				if (Asset.AssetName == TEXT("PDA_CommonUIFoundation")) Contributions.Add(EGorgeousProjectSettingContribution::CommonUIViewport);
				for (const auto Kind : Contributions)
				{
					if (Configuration->Actions.ContainsByPredicate([&](const UGorgeousGeneralSystemAction* Action)
					{
						const auto* Contribution = Cast<UGorgeousProjectSettingsContribution>(Action);
						return Contribution && Contribution->Contribution == Kind;
					})) continue;
					UGorgeousProjectSettingsContribution* Contribution = NewObject<UGorgeousProjectSettingsContribution>(Configuration);
					Contribution->Contribution = Kind;
					Configuration->Actions.Add(Contribution);
					bChanged = true;
				}
				if (Asset.AssetName == TEXT("PDA_General") || Asset.AssetName == TEXT("PDA_CommonUIFoundation"))
				{
					bChanged |= Configuration->Actions.RemoveAll([](const UGorgeousGeneralSystemAction* Action)
					{
						const auto* Prompt = Cast<UGorgeousTriggerPromptAction>(Action);
						const auto* Condition = Prompt ? Cast<UGorgeousClassInheritanceCondition>(Prompt->ConditionObject) : nullptr;
						if (!Condition) return false;
						const FString Expected = Condition->ExpectedClass.ToSoftObjectPath().ToString();
						return Expected.StartsWith(TEXT("/Script/GorgeousCoreRuntime.GorgeousGame")) ||
							Expected == TEXT("/Script/GorgeousCoreRuntime.GorgeousWorldSettings") || Expected == TEXT("/Script/CommonUI.CommonGameViewportClient");
					}) > 0;
				}
				if (bChanged)
				{
					Configuration->MarkPackageDirty();
					bChangedExplicitly = true;
					UE_LOG(LogGorgeousAuditFixup, Display, TEXT("Updated GSC requirement mirrors and explicit settings contributions: %s"), *Configuration->GetPathName());
				}
			}
		}
		if (Blueprint)
		{
			if (bDumpGraphs && GraphDumpTargets.Contains(Asset.AssetName))
			{
				TSharedRef<FJsonObject> AssetJson = MakeShared<FJsonObject>();
				AssetJson->SetStringField(TEXT("Asset"), Blueprint->GetPathName());
				AssetJson->SetStringField(TEXT("ParentClass"), GetPathNameSafe(Blueprint->ParentClass));
                TArray<TSharedPtr<FJsonValue>> Interfaces;
                TArray<TSharedPtr<FJsonValue>> ClassHierarchy;
                if (UClass* GeneratedClass = Blueprint->GeneratedClass)
                {
                    for (UClass* Class = GeneratedClass; Class; Class = Class->GetSuperClass())
                    {
                        ClassHierarchy.Add(MakeShared<FJsonValueString>(Class->GetPathName()));
                        for (const FImplementedInterface& Interface : Class->Interfaces)
                        {
                            const FString Path = GetPathNameSafe(Interface.Class);
                            if (!Path.IsEmpty()) Interfaces.Add(MakeShared<FJsonValueString>(Path));
                        }
                    }
                    TArray<TSharedPtr<FJsonValue>> SimpleDefaults;
                    if (const UObject* CDO = GeneratedClass->GetDefaultObject(false))
                    {
                        for (TFieldIterator<FProperty> It(GeneratedClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
                        {
                            const FProperty* Property = *It;
                            if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_Deprecated)) continue;
                            const bool bSimple = CastField<FNumericProperty>(Property) || CastField<FBoolProperty>(Property) ||
                                CastField<FEnumProperty>(Property) || CastField<FNameProperty>(Property) ||
                                CastField<FStrProperty>(Property) || CastField<FTextProperty>(Property) ||
                                CastField<FObjectPropertyBase>(Property) || CastField<FSoftObjectProperty>(Property);
                            if (!bSimple || Property->ArrayDim != 1) continue;
                            FString Value;
                            Property->ExportText_InContainer(0, Value, CDO, nullptr, const_cast<UObject*>(CDO), PPF_None);
                            TSharedRef<FJsonObject> Default = MakeShared<FJsonObject>();
                            Default->SetStringField(TEXT("Property"), Property->GetName());
                            Default->SetStringField(TEXT("Type"), Property->GetCPPType());
                            Default->SetStringField(TEXT("Value"), Value);
                            SimpleDefaults.Add(MakeShared<FJsonValueObject>(Default));
                        }
                    }
                    AssetJson->SetArrayField(TEXT("SimpleClassDefaults"), SimpleDefaults);
                }
                AssetJson->SetArrayField(TEXT("ImplementedInterfacesIncludingParents"), Interfaces);
                AssetJson->SetArrayField(TEXT("ClassHierarchy"), ClassHierarchy);

				TArray<TSharedPtr<FJsonValue>> GraphJsonValues;
				TArray<UEdGraph*> Graphs;
				Blueprint->GetAllGraphs(Graphs);
				for (UEdGraph* Graph : Graphs)
				{
					TSharedRef<FJsonObject> GraphJson = MakeShared<FJsonObject>();
					GraphJson->SetStringField(TEXT("Name"), Graph->GetName());
					TArray<TSharedPtr<FJsonValue>> Nodes;
					for (UEdGraphNode* Node : Graph->Nodes)
					{
						if (!Node) continue;
						TSharedRef<FJsonObject> NodeJson = MakeShared<FJsonObject>();
						NodeJson->SetStringField(TEXT("Id"), Node->NodeGuid.ToString(EGuidFormats::Digits));
						NodeJson->SetStringField(TEXT("Class"), Node->GetClass()->GetPathName());
						NodeJson->SetStringField(TEXT("Title"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
						NodeJson->SetStringField(TEXT("Comment"), Node->NodeComment);
                        NodeJson->SetStringField(TEXT("EnabledState"), LexToString(Node->GetDesiredEnabledState()));
                        if (const UK2Node_MacroInstance* Macro = Cast<UK2Node_MacroInstance>(Node))
                            NodeJson->SetStringField(TEXT("MacroGraph"), GetPathNameSafe(Macro->GetMacroGraph()));

						if (const UK2Node_CallFunction* Call = Cast<UK2Node_CallFunction>(Node))
						{
							NodeJson->SetStringField(TEXT("Function"), Call->FunctionReference.GetMemberName().ToString());
							NodeJson->SetStringField(TEXT("FunctionOwner"), GetPathNameSafe(Call->FunctionReference.GetMemberParentClass()));
                            if (const UFunction* Function = Call->GetTargetFunction())
                            {
                                NodeJson->SetBoolField(TEXT("FunctionNet"), Function->HasAnyFunctionFlags(FUNC_Net));
                                NodeJson->SetBoolField(TEXT("FunctionNetServer"), Function->HasAnyFunctionFlags(FUNC_NetServer));
                                NodeJson->SetBoolField(TEXT("FunctionNetClient"), Function->HasAnyFunctionFlags(FUNC_NetClient));
                                NodeJson->SetBoolField(TEXT("FunctionNetMulticast"), Function->HasAnyFunctionFlags(FUNC_NetMulticast));
                            }

						}
						if (const UK2Node_Variable* Variable = Cast<UK2Node_Variable>(Node))
							NodeJson->SetStringField(TEXT("Variable"), Variable->VariableReference.GetMemberName().ToString());
						if (const UK2Node_CustomEvent* Event = Cast<UK2Node_CustomEvent>(Node))
							NodeJson->SetStringField(TEXT("Event"), Event->CustomFunctionName.ToString());
						TArray<TSharedPtr<FJsonValue>> Pins;
						for (const UEdGraphPin* Pin : Node->Pins)
						{
							if (!Pin) continue;
							TSharedRef<FJsonObject> PinJson = MakeShared<FJsonObject>();
							PinJson->SetStringField(TEXT("Name"), Pin->PinName.ToString());
							PinJson->SetStringField(TEXT("Direction"), Pin->Direction == EGPD_Input ? TEXT("Input") : TEXT("Output"));
							PinJson->SetStringField(TEXT("Category"), Pin->PinType.PinCategory.ToString());
							PinJson->SetStringField(TEXT("TypeObject"), GetPathNameSafe(Pin->PinType.PinSubCategoryObject.Get()));
							PinJson->SetStringField(TEXT("DefaultValue"), Pin->DefaultValue);
							PinJson->SetStringField(TEXT("DefaultObject"), GetPathNameSafe(Pin->DefaultObject));
							PinJson->SetStringField(TEXT("DefaultText"), Pin->DefaultTextValue.ToString());
							TArray<TSharedPtr<FJsonValue>> Links;
							for (const UEdGraphPin* Link : Pin->LinkedTo)
							{
								if (Link && Link->GetOwningNode()) Links.Add(MakeShared<FJsonValueString>(Link->GetOwningNode()->NodeGuid.ToString(EGuidFormats::Digits) + TEXT(".") + Link->PinName.ToString()));
							}
							PinJson->SetArrayField(TEXT("Links"), Links);
							Pins.Add(MakeShared<FJsonValueObject>(PinJson));
						}
						NodeJson->SetArrayField(TEXT("Pins"), Pins);
						Nodes.Add(MakeShared<FJsonValueObject>(NodeJson));
					}
					GraphJson->SetArrayField(TEXT("Nodes"), Nodes);
					GraphJsonValues.Add(MakeShared<FJsonValueObject>(GraphJson));
				}
				AssetJson->SetArrayField(TEXT("Graphs"), GraphJsonValues);
				GraphDumpAssets.Add(MakeShared<FJsonValueObject>(AssetJson));
			}
			if (bApply && Asset.AssetName == FName(TEXT("BP_DefaultPrimaryGameLayout_CUW")))
			{
				TArray<UEdGraph*> Graphs;
				Blueprint->GetAllGraphs(Graphs);
				for (UEdGraph* Graph : Graphs)
				{
					for (UEdGraphNode* Node : Graph->Nodes)
					{
						for (UEdGraphPin* Pin : Node->Pins)
						{
							if (Pin && Pin->DefaultObject && Pin->DefaultObject->GetPathName().StartsWith(TEXT("/GorgeousInventory/")))
							{
								Node->Modify();
								Pin->DefaultObject = nullptr;
								Pin->DefaultValue.Reset();
								FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
								bChangedExplicitly = true;
								UE_LOG(LogGorgeousAuditFixup, Display, TEXT("Removed optional Inventory class pin from Core default layout: %s"), *Pin->PinName.ToString());
							}
						}
					}
				}
			}
			FCompilerResultsLog Results;
			FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::SkipGarbageCollection, &Results);
			++CompiledBlueprints;
			if (Results.NumErrors > 0)
			{
				UE_LOG(LogGorgeousAuditFixup, Error, TEXT("Blueprint compilation failed: %s (%d errors)"), *Blueprint->GetPathName(), Results.NumErrors);
				++Failures;
				continue;
			}
		}
        if (Asset.PackageName == VDGSurveyRowPackage) CollectVDGCompatibility(Object, VDGCurrentEvidence);
		if (bApply && (bHasLegacySerializedReferences || bChangedExplicitly) && Object->GetOutermost() != GetTransientPackage())
		{
			// Only known legacy references or explicit repairs are persisted; compiling alone never saves assets.
			PackagesToSave.Add(Object->GetOutermost());
		}
	}

    {
        TSharedRef<FJsonObject> Audit = MakeShared<FJsonObject>();
        Audit->SetBoolField(TEXT("Apply"), bApply);
        Audit->SetArrayField(TEXT("LegacyRecovery"), VDGRecoveryEvidence);
        Audit->SetArrayField(TEXT("CurrentCompatibilityStrings"), VDGCurrentEvidence);
        FString Json;
        FJsonSerializer::Serialize(Audit, TJsonWriterFactory<>::Create(&Json));
        const FString Directory = FPaths::ProjectSavedDir() / TEXT("GorgeousThings");
        IFileManager::Get().MakeDirectory(*Directory, true);
        FFileHelper::SaveStringToFile(Json, *(Directory / TEXT("VDGCompatibilityAudit.json")));
    }
	if (bCreateTestMap)
	{
		const FString MapPackageName = TEXT("/GorgeousCore/Examples/Maps/GorgeousCoreSmoke");
		const FString ModePackageName = TEXT("/GorgeousCore/Examples/BP_GorgeousCoreSmokeGameMode");
		if (FPackageName::DoesPackageExist(MapPackageName))
		{
			UE_LOG(LogGorgeousAuditFixup, Display, TEXT("Smoke map already exists and was preserved: %s"), *MapPackageName);
		}
		else
		{
			UBlueprint* ModeBlueprint = FPackageName::DoesPackageExist(ModePackageName)
				? LoadObject<UBlueprint>(nullptr, *(ModePackageName + TEXT(".BP_GorgeousCoreSmokeGameMode"))) : nullptr;
			if (!ModeBlueprint && !FPackageName::DoesPackageExist(ModePackageName))
			{
				UPackage* ModePackage = CreatePackage(*ModePackageName);
				ModeBlueprint = FKismetEditorUtilities::CreateBlueprint(AGorgeousGameMode::StaticClass(), ModePackage,
					TEXT("BP_GorgeousCoreSmokeGameMode"), BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
				if (ModeBlueprint)
				{
					FKismetEditorUtilities::CompileBlueprint(ModeBlueprint);
					if (!ModeBlueprint->GeneratedClass)
					{
						UE_LOG(LogGorgeousAuditFixup, Error, TEXT("Smoke game mode compilation failed."));
						return 1;
					}
					AGameModeBase* Mode = ModeBlueprint->GeneratedClass->GetDefaultObject<AGameModeBase>();
					Mode->PlayerControllerClass = AGorgeousPlayerController::StaticClass();
					Mode->PlayerStateClass = AGorgeousPlayerState::StaticClass();
					Mode->GameStateClass = AGorgeousGameState::StaticClass();
					Mode->HUDClass = AGorgeousHUD::StaticClass();
					Mode->DefaultPawnClass = ASpectatorPawn::StaticClass();
					FBlueprintEditorUtils::MarkBlueprintAsModified(ModeBlueprint);
					FAssetRegistryModule::AssetCreated(ModeBlueprint);
					PackagesToSave.Add(ModePackage);
				}
			}
			UPackage* MapPackage = CreatePackage(*MapPackageName);
			UWorldFactory* Factory = NewObject<UWorldFactory>();
			Factory->WorldType = EWorldType::Inactive;
			Factory->bCreateWorldPartition = false;
			UWorld* World = Cast<UWorld>(Factory->FactoryCreateNew(UWorld::StaticClass(), MapPackage, TEXT("GorgeousCoreSmoke"), RF_Public | RF_Standalone, nullptr, GWarn));
			if (!World || !ModeBlueprint || !ModeBlueprint->GeneratedClass)
			{
				UE_LOG(LogGorgeousAuditFixup, Error, TEXT("Failed to create smoke map/game mode."));
				++Failures;
			}
			else
			{
				FActorSpawnParameters Spawn;
				Spawn.OverrideLevel = World->PersistentLevel;
				Spawn.ObjectFlags = RF_Transactional;
				AGorgeousWorldSettings* SmokeWorldSettings = World->SpawnActor<AGorgeousWorldSettings>(Spawn);
				if (!SmokeWorldSettings)
				{
					UE_LOG(LogGorgeousAuditFixup, Error, TEXT("Failed to spawn smoke-map WorldSettings."));
					return 1;
				}
				World->PersistentLevel->SetWorldSettings(SmokeWorldSettings);
				SmokeWorldSettings->DefaultGameMode = ModeBlueprint->GeneratedClass;
				APlayerStart* Start = World->SpawnActor<APlayerStart>(FVector(0, 0, 100), FRotator::ZeroRotator, Spawn);
				if (Start) Start->SetActorLabel(TEXT("Gorgeous Core smoke-test PlayerStart"));
				MapPackage->SetPackageFlags(PKG_ContainsMap);
				World->MarkPackageDirty();
				FAssetRegistryModule::AssetCreated(World);
				PackagesToSave.Add(MapPackage);
				UE_LOG(LogGorgeousAuditFixup, Display, TEXT("Created QoL framework smoke map (no project GameInstance setting changes): %s"), *MapPackageName);
			}
		}
	}
	if (bApply || bCreateTestMap)
	{
		for (UPackage* Package : PackagesToSave)
		{
			UE_LOG(LogGorgeousAuditFixup, Display, TEXT("Saving repaired asset package: %s"), *Package->GetName());
			const FString Filename = FPackageName::LongPackageNameToFilename(Package->GetName(), Package->ContainsMap() ? FPackageName::GetMapPackageExtension() : FPackageName::GetAssetPackageExtension());
			IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
			FSavePackageArgs SaveArgs;
			SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
			SaveArgs.SaveFlags = SAVE_NoError;
			if (!UPackage::SavePackage(Package, nullptr, *Filename, SaveArgs))
			{
				UE_LOG(LogGorgeousAuditFixup, Error, TEXT("Failed to save repaired package: %s"), *Package->GetName());
				++Failures;
			}
		}
	}
	if (bDumpGraphs)
	{
		TSharedRef<FJsonObject> Dump = MakeShared<FJsonObject>();
		Dump->SetArrayField(TEXT("Assets"), GraphDumpAssets);
        Dump->SetArrayField(TEXT("ReferenceAudit"), ReferenceAudit);
        TArray<FString> MountedRoots;
        FPackageName::QueryRootContentPaths(MountedRoots, true);
        TArray<TSharedPtr<FJsonValue>> RootJson;
        for (const FString& Root : MountedRoots) RootJson.Add(MakeShared<FJsonValueString>(Root));
        Dump->SetArrayField(TEXT("MountedContentRoots"), RootJson);
        Dump->SetStringField(TEXT("ReferenceScope"), TEXT("On-disk AssetRegistry package/searchable/manage referencers in every mounted project/plugin root. Dynamic native loads and unmounted plugins are not proven absent."));

		FString Json;
		FJsonSerializer::Serialize(Dump, TJsonWriterFactory<>::Create(&Json));
		const FString Filename = FPaths::ProjectSavedDir() / TEXT("GorgeousThings/AuditGraphs.json");
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
		if (!FFileHelper::SaveStringToFile(Json, *Filename)) ++Failures;
		UE_LOG(LogGorgeousAuditFixup, Display, TEXT("Read-only Blueprint graph dump: %s (%d assets)"), *Filename, GraphDumpAssets.Num());
	}
	UE_LOG(LogGorgeousAuditFixup, Display, TEXT("AUDIT_RESULT Mode=%s GuardRepairs=%d BlueprintsCompiled=%d PackagesSaved=%d Failures=%d"),
		bApply ? TEXT("Apply") : bCreateTestMap ? TEXT("CreateTestMap") : TEXT("ReadOnly"), RepairedGuards, CompiledBlueprints, (bApply || bCreateTestMap) ? PackagesToSave.Num() : 0, Failures);
	return Failures == 0 ? 0 : 1;
}
