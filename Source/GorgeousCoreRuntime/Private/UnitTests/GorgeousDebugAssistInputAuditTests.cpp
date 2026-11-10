#include "DrawDebugHelpers.h"
#if WITH_DEV_AUTOMATION_TESTS && ENABLE_DRAW_DEBUG
#include "Misc/AutomationTest.h"
#include "GeneralSystems/DebugAssist/GorgeousDebugAssistBlueprintFunctionLibrary.h"
#include "Components/LineBatchComponent.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "HAL/IConsoleManager.h"
#include "Misc/ScopeExit.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousDebugAssistInputAudit,
    "GorgeousCore.Limitations.DebugAssist.DrawInputs",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousDebugAssistInputAudit::RunTest(const FString& Parameters)
{
    IConsoleVariable* Toggle = IConsoleManager::Get().FindConsoleVariable(TEXT("g.DebugAssist"));
    const int32 Previous = Toggle ? Toggle->GetInt() : 1;
    if (Toggle) Toggle->Set(1, ECVF_SetByCode);
    ON_SCOPE_EXIT { if (Toggle) Toggle->Set(Previous, ECVF_SetByCode); };
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, false);
    if (!TestNotNull(TEXT("Isolated actual engine world"), World)) return false;
    ON_SCOPE_EXIT { World->DestroyWorld(false); };
    // HUD::AddDebugText is a native client RPC. AActor::ProcessEvent correctly
    // absorbs it until actors have been initialized for play.
    World->InitializeActorsForPlay(FURL());
    if (!TestTrue(TEXT("Actual actor RPC execution is initialized"), World->AreActorsInitialized())) return false;
    ULineBatchComponent* Lines = World->GetLineBatcher(UWorld::ELineBatcherType::World);
    ULineBatchComponent* Persistent = World->GetLineBatcher(UWorld::ELineBatcherType::WorldPersistent);
    if (!TestNotNull(TEXT("World line batcher"), Lines) || !TestNotNull(TEXT("Persistent line batcher"), Persistent)) return false;
    const FVector Center(10, 20, 30);
    UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistCircle(World, Center, 50, FLinearColor::White, true, 0, false, 1, FVector::UpVector);
    TestTrue(TEXT("Circle emits real debug vertices"), !Lines->BatchedLines.IsEmpty());
    for (const auto& Line : Lines->BatchedLines)
    {
        TestTrue(TEXT("Every circle segment lies in normal plane"), FMath::IsNearlyEqual(Line.Start.Z, Center.Z) && FMath::IsNearlyEqual(Line.End.Z, Center.Z));
    }
    TestTrue(TEXT("Wireframe circle emits no filled mesh"), Lines->BatchedMeshes.IsEmpty());
    Lines->Flush(); Persistent->Flush();
    UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistCircle(World, Center, 50, FLinearColor::White, false, 0, false, 1, FVector(0.01, 0, 3));
    TestTrue(TEXT("Wireframe=false creates filled disc mesh"), !Lines->BatchedMeshes.IsEmpty());
    for (const auto& Mesh : Lines->BatchedMeshes)
        for (const FVector& Vertex : Mesh.MeshVerts)
            TestTrue(TEXT("Filled disc honors arbitrary unnormalized normal"), FMath::IsNearlyZero(FVector::DotProduct(Vertex - Center, FVector(0.01, 0, 3).GetSafeNormal()), 0.001));

    Lines->Flush(); Persistent->Flush();
    FGorgeousDebugAssistVisualParameters Visual;
    Visual.Duration = 0.0f;
    Visual.HitPoint.bDraw = true;
    Visual.VFX.bEnableShockwave = false;
    Visual.HitSphere.bDraw = Visual.HitSphere.bDrawFilled = false;
    Visual.HitBox.bDraw = Visual.HitBox.bDrawFilled = false;
    Visual.HitBounds.bDraw = Visual.HitBounds.bDrawFilled = false;
    Visual.HitLabel.bDraw = false;
    FHitResult NoHit;
    UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistHitResult(World, FVector(1, 2, 3), FVector(4, 5, 6), NoHit, Visual);
    TestTrue(TEXT("Empty hit emits no origin hit point"), Lines->BatchedPoints.IsEmpty());
    Visual.bDrawTracePath = true;
    Visual.HitPoint.bDraw = false;
    UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistHitResult(World, FVector(1, 2, 3), FVector(4, 5, 6), NoHit, Visual);
    TestEqual(TEXT("HitResult path uses supplied trace endpoints"), Lines->BatchedLines.Num(), 1);
    if (Lines->BatchedLines.Num() == 1)
    {
        TestTrue(TEXT("TraceStart honored"), Lines->BatchedLines[0].Start.Equals(FVector(1, 2, 3)));
        TestTrue(TEXT("TraceEnd honored"), Lines->BatchedLines[0].End.Equals(FVector(4, 5, 6)));
    }
    Lines->Flush(); Persistent->Flush();
    APlayerController* Controller = World->SpawnActor<APlayerController>();
    AHUD* HUD = World->SpawnActor<AHUD>();
    if (!TestNotNull(TEXT("Text-test controller"), Controller) || !TestNotNull(TEXT("Text-test HUD"), HUD)) return false;
    Controller->Player = NewObject<ULocalPlayer>(GEngine);
    Controller->MyHUD = HUD;
    HUD->SetOwner(Controller);
    bool bInWorldList = false;
    for (auto It = World->GetPlayerControllerIterator(); It; ++It) bInWorldList |= It->Get() == Controller;
    if (!bInWorldList) World->AddController(Controller);
    FGorgeousDebugAssistTextBlockParameters Text;
    Text.bDraw = true;
    Text.bFollowTrace = false;
    FGorgeousDebugAssistTextLineParameters Line;
    Line.Text = TEXT("Persistent audit text"); Text.Lines.Add(Line);
    UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistTextBlock(World, Center, Text, 2.0f, true);
    FArrayProperty* DebugTexts = FindFProperty<FArrayProperty>(AHUD::StaticClass(), TEXT("DebugTextList"));
    if (!TestNotNull(TEXT("HUD real debug text array"), DebugTexts)) return false;
    FScriptArrayHelper Values(DebugTexts, DebugTexts->ContainerPtrToValuePtr<void>(HUD));
    TestEqual(TEXT("Persistent text reaches actual HUD"), Values.Num(), 1);
    if (Values.Num() == 1)
    {
        FStructProperty* TextInfo = CastField<FStructProperty>(DebugTexts->Inner);
        FFloatProperty* Remaining = TextInfo ? FindFProperty<FFloatProperty>(TextInfo->Struct, TEXT("TimeRemaining")) : nullptr;
        FBoolProperty* Shadow = TextInfo ? FindFProperty<FBoolProperty>(TextInfo->Struct, TEXT("bDrawShadow")) : nullptr;
        TestNotNull(TEXT("Engine exposes text lifetime"), Remaining);
        if (Remaining) TestEqual(TEXT("Persistent flag sets indefinite lifetime"), Remaining->GetPropertyValue_InContainer(Values.GetRawPtr(0)), -1.0f);
        if (Shadow) TestFalse(TEXT("Persistent flag does not masquerade as text shadow"), Shadow->GetPropertyValue_InContainer(Values.GetRawPtr(0)));
    }
    return true;
}
#endif
