#include "Tests/GorgeousMigrationAuditTypes.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "DataSchemaMapping/GorgeousDataSchemaMapper.h"
#include "DataSchemaMapping/GorgeousDataSchemaMapping_DA.h"
#include "DataSchemaMapping/GorgeousDataSchemaMappingEditorService.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "Editor/TransBuffer.h"
#include "Misc/ScopeExit.h"
#include "Misc/EngineVersionComparison.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectHash.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousMapperOwnershipAudit,
    "GorgeousCore.Limitations.DataSchemaMapping.ExternalObjectSafety",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousMapperOwnershipAudit::RunTest(const FString& Parameters)
{
    TStrongObjectPtr<UGorgeousMigrationAuditAsset> Target(NewObject<UGorgeousMigrationAuditAsset>());
    TStrongObjectPtr<UGorgeousMigrationAuditAsset> Foreign(NewObject<UGorgeousMigrationAuditAsset>());
    TStrongObjectPtr<UGorgeousDataSchemaMapping_DA> Schema(NewObject<UGorgeousDataSchemaMapping_DA>());
    FGorgeousDataSchemaFieldMapping_S Mapping;
    Mapping.SourcePath = TEXT("Input"); Mapping.TargetPath = TEXT("Reference.Value");
    Schema->FieldMappings.Add(Mapping);
    FGorgeousDataSchemaSourceSnapshot_S Snapshot;
    FGorgeousDataSchemaSourceValueEntry_S Entry; Entry.Path = TEXT("Input"); Entry.ExportedValue = TEXT("42");
    Snapshot.ValueEntries.Add(Entry);
    Target->Reference = Foreign.Get();
    TArray<FString> Warnings, Errors;
    TestFalse(TEXT("Existing external asset is never modified through target path"), FGorgeousDataSchemaMapper::ApplySnapshotToObject(Snapshot, Target.Get(), Schema.Get(), Warnings, Errors));
    TestEqual(TEXT("Existing referenced value remains unchanged"), Foreign->Value, 17.0f);
    TestTrue(TEXT("Ownership violation reports actionable diagnostic"), Errors.Num() > 0 && Errors[0].Contains(TEXT("external object")));
    Target->Reference = NewObject<UGorgeousMigrationAuditAsset>(Target.Get());
    Errors.Reset(); Warnings.Reset();
    TestTrue(TEXT("Owned target subobject remains writable"), FGorgeousDataSchemaMapper::ApplySnapshotToObject(Snapshot, Target.Get(), Schema.Get(), Warnings, Errors));
    TestEqual(TEXT("Owned value updated"), Target->Reference->Value, 42.0f);
    Schema->TargetDefinition.TargetContentRootPath = TEXT("/GorgeousCore/AuditOutput/");
    TestEqual(TEXT("Mounted plugin folder is supported"), Schema->GetNormalizedTargetContentRootPath(), FString(TEXT("/GorgeousCore/AuditOutput")));
    Schema->TargetDefinition.TargetContentRootPath = TEXT("/NotAnInstalledPlugin/Output");
    TestTrue(TEXT("Unknown mount rejected"), Schema->GetNormalizedTargetContentRootPath().IsEmpty());
    Schema->TargetDefinition.TargetContentRootPath = TEXT("/Game/");
    TestEqual(TEXT("Game root accepts trailing slash"), Schema->GetNormalizedTargetContentRootPath(), FString(TEXT("/Game")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousMigrationUndoAudit,
    "GorgeousCore.Limitations.DataSchemaMapping.CreatedAssetUndoRedo",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousMigrationUndoAudit::RunTest(const FString& Parameters)
{
    if (!GEditor || IsRunningCommandlet()) { AddError(TEXT("Requires the real editor transaction system, not a commandlet.")); return false; }
    TStrongObjectPtr<UTransactor> OriginalTransactor(GEditor->Trans);
    TStrongObjectPtr<UTransBuffer> IsolatedTransactions(NewObject<UTransBuffer>());
    IsolatedTransactions->Initialize(8 * 1024 * 1024);
    GEditor->Trans = IsolatedTransactions.Get();
    ON_SCOPE_EXIT { GEditor->Trans = OriginalTransactor.Get(); };
    const FString Suffix = FGuid::NewGuid().ToString(EGuidFormats::Digits);
    UPackage* SourcePackage = CreatePackage(*(TEXT("/Game/__GorgeousMigrationSource_") + Suffix));
    TStrongObjectPtr<UGorgeousMigrationAuditAsset> Source(NewObject<UGorgeousMigrationAuditAsset>(SourcePackage, TEXT("AuditSource"), RF_Public | RF_Standalone));
    Source->Value = 42.0f;
    TStrongObjectPtr<UGorgeousDataSchemaMapping_DA> Schema(NewObject<UGorgeousDataSchemaMapping_DA>());
    Schema->SourceDefinition.SourceKind = EGorgeousDataSchemaSourceKind_E::Source_AnyUObject;
    Schema->SourceDefinition.RequiredSourceClass = UGorgeousMigrationAuditAsset::StaticClass();
    Schema->TargetDefinition.TargetClass = UGorgeousMigrationAuditAsset::StaticClass();
    Schema->TargetDefinition.TargetContentRootPath = TEXT("/Game/__GorgeousMigrationUndo_") + Suffix;
    FGorgeousDataSchemaFieldMapping_S Mapping; Mapping.SourcePath = TEXT("Value"); Mapping.TargetPath = TEXT("Value");
    Schema->FieldMappings.Add(Mapping);
    TArray<UPackage*> Created;
    const bool bMigrated = FGorgeousDataSchemaMappingEditorService::ExecuteMigration(Schema.Get(), {FAssetData(Source.Get())}, &Created);
    TestTrue(TEXT("Migration executes through actual editor service"), bMigrated);
    TestEqual(TEXT("Exactly one generated package"), Created.Num(), 1);
    if (Created.Num() == 1)
    {
        TArray<UObject*> Objects;
#if UE_VERSION_NEWER_THAN(5,7,99)
        GetObjectsWithOuter(Created[0], Objects, EGetObjectsFlags::None);
#else
        GetObjectsWithOuter(Created[0], Objects, false);
#endif
        UGorgeousMigrationAuditAsset* Asset = nullptr;
        for (UObject* Object : Objects) if (auto* Candidate = Cast<UGorgeousMigrationAuditAsset>(Object)) Asset = Candidate;
        TestNotNull(TEXT("Generated reflected asset exists"), Asset);
        if (Asset)
        {
            const FSoftObjectPath OriginalPath(Asset);
            TestEqual(TEXT("Migration imported real source value"), Asset->Value, 42.0f);
            TestTrue(TEXT("Editor Undo succeeds"), GEditor->UndoTransaction());
            TestFalse(TEXT("Undo removes original asset identity"), Asset->GetPathName() == OriginalPath.ToString());
            TestTrue(TEXT("Undo moves asset into transient package"), Asset->GetOutermost() == GetTransientPackage());
            TestNull(TEXT("Undo original identity cannot resolve"), OriginalPath.ResolveObject());
            TestFalse(TEXT("Undo removes public asset membership"), Asset->HasAnyFlags(RF_Public | RF_Standalone));
            TestTrue(TEXT("Editor Redo succeeds"), GEditor->RedoTransaction());
            TestTrue(TEXT("Redo restores a live asset"), IsValid(Asset));
            TestEqual(TEXT("Redo restores asset identity"), Asset->GetPathName(), OriginalPath.ToString());
            TestTrue(TEXT("Redo identity resolves to original asset"), OriginalPath.ResolveObject() == Asset);
            TestTrue(TEXT("Redo restores public asset membership"), Asset->HasAllFlags(RF_Public | RF_Standalone));
            TestEqual(TEXT("Redo restores mapped value"), Asset->Value, 42.0f);
            TestTrue(TEXT("Final undo cleans generated asset"), GEditor->UndoTransaction());
        }
        Created[0]->SetDirtyFlag(false);
    }
    Source->ClearFlags(RF_Public | RF_Standalone);
    Source->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_NonTransactional);
    SourcePackage->SetDirtyFlag(false);
    return true;
}
#endif
