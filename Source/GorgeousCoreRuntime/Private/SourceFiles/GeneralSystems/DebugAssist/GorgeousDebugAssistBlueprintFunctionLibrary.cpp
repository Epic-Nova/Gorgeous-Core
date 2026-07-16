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

#include "GeneralSystems/DebugAssist/GorgeousDebugAssistBlueprintFunctionLibrary.h"

//<=============================--- Includes ---=============================>
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "HAL/IConsoleManager.h"
//<-------------------------------------------------------------------------->
static int32 GActiveDebugBeaconsCount = 0;
static double GDebugAssistRenderOverheadMS = 0.0;

int32 UGorgeousDebugAssistBlueprintFunctionLibrary::GetTotalActiveDebugBeacons()
{
	return GActiveDebugBeaconsCount;
}

void UGorgeousDebugAssistBlueprintFunctionLibrary::IncrementActiveDebugBeacons()
{
	GActiveDebugBeaconsCount++;
}

void UGorgeousDebugAssistBlueprintFunctionLibrary::DecrementActiveDebugBeacons()
{
	GActiveDebugBeaconsCount--;
}

double UGorgeousDebugAssistBlueprintFunctionLibrary::GetDebugAssistRenderOverheadMS()
{
	return GDebugAssistRenderOverheadMS;
}

void UGorgeousDebugAssistBlueprintFunctionLibrary::SetDebugAssistRenderOverheadMS(double InOverhead)
{
	GDebugAssistRenderOverheadMS = InOverhead;
}

struct FGhostBox
{
    FVector Origin;
    FVector BaseExtent;
    double Expiration;
    uint64 LastActiveFrame; // Frame-level precision for focus
    float FadeTime;
    float LastPulseOffset; // To freeze the pulse on loss
    float InitialDuration;

    // Visual settings snapshot
    FLinearColor Color;
    FLinearColor FillColor;
    float Thickness;
    bool bDraw;
    bool bDrawFilled;
    bool bAnimateFade;
    bool bPulse;
    float PulseSpeed;
    float Inflation;
};

static TAutoConsoleVariable<int32> CVarDebugAssist(
    TEXT("g.DebugAssist"), //@TODO Document
    1,
    TEXT("Master toggle for Gorgeous Debug Assist visuals.\n")
    TEXT("0: Disabled\n")
    TEXT("1: Enabled"),
    ECVF_Cheat);

static TMap<TWeakObjectPtr<const AActor>, TArray<FGhostBox>> GlobalGhostRegistryMap;
 
static void TickAndRenderGhosts(const UObject* WorldContextObject)
{
    if (!WorldContextObject || CVarDebugAssist.GetValueOnGameThread() == 0) return;
    UWorld* World = WorldContextObject->GetWorld();
    if (!World) return;

    const double Time = World->GetTimeSeconds();

    for (auto It = GlobalGhostRegistryMap.CreateIterator(); It; ++It)
    {
        TArray<FGhostBox>& GhostRegistry = It.Value();
        
        // Remove ghosts for invalid actors immediately
        if (!It.Key().IsValid())
        {
            GhostRegistry.Empty();
            It.RemoveCurrent();
            continue;
        }

        // Cleanup expired ghosts
        for (int32 i = GhostRegistry.Num() - 1; i >= 0; --i)
        {
            if (GhostRegistry[i].Expiration <= Time)
            {
                GhostRegistry.RemoveAtSwap(i);
            }
        }

        if (GhostRegistry.Num() == 0)
        {
            if (!It.Key().IsValid())
            {
                It.RemoveCurrent();
            }
            continue;
        }

        // Determine if the WHOLE actor has focus this frame
        bool bActorHasFocus = false;
        for (const auto& Ghost : GhostRegistry)
        {
            if (Ghost.LastActiveFrame >= (GFrameCounter - 1))
            {
                bActorHasFocus = true;
                break;
            }
        }

        // Draw remaining ghosts with animation
        for (auto& Ghost : GhostRegistry)
        {
            if (!Ghost.bDraw && !Ghost.bDrawFilled) continue;

            // Focus is active if the actor itself is being hit
            const bool bHasFocus = bActorHasFocus;
            
            // OPTIMIZATION: If we have focus, and this isn't the LATEST ghost, skip drawing it 
            // to prevent the "second box" overlapping flickering the user mentioned.
            if (bHasFocus && &Ghost != &GhostRegistry.Last())
            {
                continue;
            }

            float Alpha = 1.0f;
            if (!bHasFocus)
            {
                // Evaporation phase
                Alpha = Ghost.bAnimateFade ? FMath::Clamp((float)((Ghost.Expiration - Time) / Ghost.FadeTime), 0.0f, 1.0f) : 1.0f;
            }
            else
            {
                // Active focus phase
                Ghost.Expiration = Time + Ghost.FadeTime; 
            }

            // 1. Calculate Base Size
            FVector CurrentExtent = Ghost.BaseExtent;
            
            // 2. Apply Evaporation Scale (Shrink toward mesh center)
            if (Ghost.bAnimateFade && !bHasFocus)
            {
                CurrentExtent *= Alpha;
            }

            // 3. Apply Pulse (on top of scaled extent)
            if (Ghost.bPulse)
            {
                float PulseValue = 0.0f;
                if (bHasFocus)
                {
                    PulseValue = (FMath::Sin(Time * Ghost.PulseSpeed) + 1.0f) * 0.5f;
                    Ghost.LastPulseOffset = PulseValue; 
                }
                else
                {
                    PulseValue = Ghost.LastPulseOffset * Alpha; // Pulse also evaporates
                }
                
                CurrentExtent += FVector(PulseValue * Ghost.Inflation);
            }

            // 4. Apply Inflation LAST (Fixed buffer, does not scale)
            // This ensures the box always stays outside the mesh even when tiny
            CurrentExtent += FVector(Ghost.Inflation);

            FLinearColor FadeColor = Ghost.Color;
            FadeColor.A *= Alpha;
            FLinearColor FadeFillColor = Ghost.FillColor;
            FadeFillColor.A *= Alpha;
            float CurrentThickness = Ghost.Thickness * Alpha;

            if (Ghost.bDrawFilled)
            {
                DrawDebugSolidBox(World, Ghost.Origin, CurrentExtent, FadeFillColor.ToFColor(true), false, 0.0f, 0);
            }
            if (Ghost.bDraw)
            {
                DrawDebugBox(World, Ghost.Origin, CurrentExtent, FQuat::Identity, FadeColor.ToFColor(true), false, 0.0f, 0, CurrentThickness);
            }
        }
    }
}

static float ResolveDebugDuration(const FGorgeousDebugAssistVisualParameters& VisualParameters)
{
    return VisualParameters.bPersistent ? -1.0f : VisualParameters.Duration;
}

static bool ShouldDrawDebug(const UWorld* World, const FGorgeousDebugAssistVisualParameters& VisualParameters)
{
    if (!World || !VisualParameters.bEnabled)
    {
        return false;
    }

    // Don't draw on dedicated servers
    if (World->IsNetMode(NM_DedicatedServer))
    {
        return false;
    }

#if !UE_BUILD_SHIPPING
    // 1. Master Plugin Toggle (CVar: g.DebugAssist)
    if (CVarDebugAssist.GetValueOnGameThread() == 0)
    {
        return false;
    }

    // 2. Respect Editor Viewport "Show -> Debug" toggle if possible
    if (UGameViewportClient* Viewport = World->GetGameViewport())
    {
        if (!Viewport->EngineShowFlags.GameplayDebug)
        {
            return false;
        }
    }
#endif

    return true;
}

//=============================================================================
// UGorgeousDebugAssistBlueprintFunctionLibrary Implementation
//=============================================================================

void UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistLine(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const FLinearColor& Color, const float Duration, const float Thickness, const bool bPersistentLines)
{
    if (!WorldContextObject)
    {
        return;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!ShouldDrawDebug(World, FGorgeousDebugAssistVisualParameters()))
    {
        return;
    }

    DrawDebugLine(World, Start, End, Color.ToFColor(true), bPersistentLines, Duration, 0, Thickness);
}

void UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistPoint(const UObject* WorldContextObject, const FVector& Location, const FLinearColor& Color, const float Size, const float Duration, const bool bPersistentLines)
{
    if (!WorldContextObject)
    {
        return;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!ShouldDrawDebug(World, FGorgeousDebugAssistVisualParameters()))
    {
        return;
    }

    DrawDebugPoint(World, Location, Size, Color.ToFColor(true), bPersistentLines, Duration, 0);
}


static void DrawDebugAssistSolidSphereMesh(UWorld* World, const FVector& Center, float Radius, int32 LatitudeSegments, int32 LongitudeSegments, const FLinearColor& Color, bool bPersistentLines, float LifeTime, uint8 DepthPriority)
{
    if (!World || Radius <= 0.0f || LatitudeSegments < 3 || LongitudeSegments < 3)
    {
        return;
    }

    const int32 LatCount = FMath::Clamp(LatitudeSegments, 3, 64);
    const int32 LonCount = FMath::Clamp(LongitudeSegments, 3, 128);

    TArray<FVector> Vertices;
    TArray<int32> Indices;
    Vertices.Reserve((LatCount + 1) * (LonCount + 1));
    Indices.Reserve(LatCount * LonCount * 12);

    for (int32 LatIndex = 0; LatIndex <= LatCount; ++LatIndex)
    {
        const float Theta = PI * static_cast<float>(LatIndex) / static_cast<float>(LatCount);
        const float SinTheta = FMath::Sin(Theta);
        const float CosTheta = FMath::Cos(Theta);

        for (int32 LonIndex = 0; LonIndex <= LonCount; ++LonIndex)
        {
            const float Phi = 2.0f * PI * static_cast<float>(LonIndex) / static_cast<float>(LonCount);
            const float CosPhi = FMath::Cos(Phi);
            const float SinPhi = FMath::Sin(Phi);
            Vertices.Add(Center + FVector(Radius * SinTheta * CosPhi, Radius * SinTheta * SinPhi, Radius * CosTheta));
        }
    }

    for (int32 LatIndex = 0; LatIndex < LatCount; ++LatIndex)
    {
        for (int32 LonIndex = 0; LonIndex < LonCount; ++LonIndex)
        {
            const int32 Current = LatIndex * (LonCount + 1) + LonIndex;
            const int32 Next = Current + LonCount + 1;

            Indices.Add(Current);
            Indices.Add(Current + 1);
            Indices.Add(Next);

            Indices.Add(Current + 1);
            Indices.Add(Next + 1);
            Indices.Add(Next);
        }
    }

    DrawDebugMesh(World, Vertices, Indices, Color.ToFColor(true), bPersistentLines, LifeTime, DepthPriority);

}

static void DrawDebugAssistSolidBoxMesh(UWorld* World, const FVector& Center, const FVector& Extent, const FLinearColor& Color, bool bPersistentLines, float LifeTime, uint8 DepthPriority)
{
    if (!World || Extent.IsNearlyZero())
    {
        return;
    }

    const FVector Min = Center - Extent;
    const FVector Max = Center + Extent;

    TArray<FVector> Vertices;
    TArray<int32> Indices;
    Vertices.Reserve(8);
    Indices.Reserve(24 * 3);

    Vertices.Add(FVector(Min.X, Min.Y, Max.Z)); // 0
    Vertices.Add(FVector(Max.X, Min.Y, Max.Z)); // 1
    Vertices.Add(FVector(Min.X, Min.Y, Min.Z)); // 2
    Vertices.Add(FVector(Max.X, Min.Y, Min.Z)); // 3
    Vertices.Add(FVector(Min.X, Max.Y, Max.Z)); // 4
    Vertices.Add(FVector(Max.X, Max.Y, Max.Z)); // 5
    Vertices.Add(FVector(Min.X, Max.Y, Min.Z)); // 6
    Vertices.Add(FVector(Max.X, Max.Y, Min.Z)); // 7

    const int32 Quads[6][4] = {
        {3, 2, 0, 1},
        {7, 3, 1, 5},
        {6, 7, 5, 4},
        {2, 6, 4, 0},
        {1, 0, 4, 5},
        {7, 6, 2, 3}
    };

    for (const int32* Quad : Quads)
    {
        const int32 A = Quad[0];
        const int32 B = Quad[1];
        const int32 C = Quad[2];
        const int32 D = Quad[3];

        Indices.Add(A);
        Indices.Add(B);
        Indices.Add(C);

        Indices.Add(A);
        Indices.Add(C);
        Indices.Add(B);

        Indices.Add(C);
        Indices.Add(D);
        Indices.Add(A);

        Indices.Add(C);
        Indices.Add(A);
        Indices.Add(D);
    }

    DrawDebugMesh(World, Vertices, Indices, Color.ToFColor(true), bPersistentLines, LifeTime, DepthPriority);

}

void UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistSphere(const UObject* WorldContextObject, const FVector& Center, const float Radius, const FLinearColor& Color, const bool bWireframe, const bool bFilled, const float Duration, const bool bPersistentLines, const float Thickness, const FLinearColor& FillColor)
{
    if (!WorldContextObject || Radius <= 0.0f)
    {
        return;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!ShouldDrawDebug(World, FGorgeousDebugAssistVisualParameters()))
    {
        return;
    }

    if (bFilled)
    {
        const int32 LatitudeSegments = FMath::Clamp(FMath::CeilToInt(Radius / 6.0f), 8, 48);
        const int32 LongitudeSegments = LatitudeSegments * 2;
        DrawDebugAssistSolidSphereMesh(World, Center, Radius, LatitudeSegments, LongitudeSegments, FillColor, bPersistentLines, Duration, 0);
    }

    if (bWireframe)
    {
        DrawDebugSphere(World, Center, Radius, 16, Color.ToFColor(true), bPersistentLines, Duration, 0, Thickness);
    }
}

void UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistBox(const UObject* WorldContextObject, const FVector& Center, const FVector& Extent, const FLinearColor& Color, const bool bWireframe, const bool bFilled, const float Duration, const bool bPersistentLines, const float Thickness, const FLinearColor& FillColor)
{
    if (!WorldContextObject)
    {
        return;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!ShouldDrawDebug(World, FGorgeousDebugAssistVisualParameters()))
    {
        return;
    }

    if (bFilled || bWireframe)
    {
        static TArray<double> ActiveDebugBoxExpirations;
        const double CurrentTime = World->GetTimeSeconds();

        for (int32 ExpirationIndex = ActiveDebugBoxExpirations.Num() - 1; ExpirationIndex >= 0; --ExpirationIndex)
        {
            if (ActiveDebugBoxExpirations[ExpirationIndex] <= CurrentTime)
            {
                ActiveDebugBoxExpirations.RemoveAtSwap(ExpirationIndex);
            }
        }

        const int32 MaxActiveDebugBoxes = 32;
        if (ActiveDebugBoxExpirations.Num() >= MaxActiveDebugBoxes)
        {
            ActiveDebugBoxExpirations.Sort();
            ActiveDebugBoxExpirations.RemoveAt(0, ActiveDebugBoxExpirations.Num() - MaxActiveDebugBoxes + 1);
        }

        ActiveDebugBoxExpirations.Add(CurrentTime + FMath::Max(0.01f, Duration));
    }

    if (bFilled)
    {
        DrawDebugSolidBox(World, Center, Extent, FillColor.ToFColor(true), bPersistentLines, Duration, 0);
    }

    if (bWireframe)
    {
        DrawDebugBox(World, Center, Extent, FQuat::Identity, Color.ToFColor(true), bPersistentLines, Duration, 0, Thickness);
    }
}

void UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistHitResult(const UObject* WorldContextObject, const FVector& TraceStart, const FVector& TraceEnd, const FHitResult& HitResult, const FGorgeousDebugAssistVisualParameters& VisualParameters)
{
    if (!WorldContextObject)
    {
        return;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!ShouldDrawDebug(World, VisualParameters))
    {
        return;
    }

    const float Duration = ResolveDebugDuration(VisualParameters);
    const bool bPersistent = VisualParameters.bPersistent;
    const double Time = World->GetTimeSeconds();

    if (VisualParameters.HitPoint.bDraw)
    {
        const float HitPointDuration = VisualParameters.HitPoint.bFollowTrace ? 0.0f : Duration;
        const bool bHitPointPersistent = VisualParameters.HitPoint.bFollowTrace ? false : bPersistent;
        DrawDebugPoint(World, HitResult.ImpactPoint, VisualParameters.HitPoint.Size, VisualParameters.HitPoint.Color.ToFColor(true), bHitPointPersistent, HitPointDuration, 0);

        if (VisualParameters.VFX.bEnableShockwave)
        {
            const float Pulse = FMath::Frac(Time * VisualParameters.VFX.ShockwaveSpeed);
            
            for (int32 i = 0; i < 2; ++i)
            {
                float Phase = FMath::Frac(Pulse + (i * 0.5f));
                float Radius = 60.0f * Phase;
                
                FLinearColor WaveColor = VisualParameters.VFX.bOverrideShockwaveColor ? VisualParameters.VFX.ShockwaveColor : VisualParameters.TracePathColor;
                WaveColor.A *= (1.0f - Phase) * 0.5f;
                
                // Use projected circle for grounded snapping
                UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistProjectedCircle(WorldContextObject, HitResult.ImpactPoint, Radius, WaveColor, 0.0f, false, 2.0f, HitResult.ImpactNormal);
            }
        }
    }

    if (VisualParameters.HitSphere.bDraw || VisualParameters.HitSphere.bDrawFilled)
    {
        const float HitSphereDuration = VisualParameters.HitSphere.bFollowTrace ? 0.0f : Duration;
        const bool bHitSpherePersistent = VisualParameters.HitSphere.bFollowTrace ? false : bPersistent;
        DrawDebugAssistSphere(WorldContextObject, HitResult.ImpactPoint, VisualParameters.HitSphere.Radius, VisualParameters.HitSphere.Color, VisualParameters.HitSphere.bDraw, VisualParameters.HitSphere.bDrawFilled, HitSphereDuration, bHitSpherePersistent, 2.0f, VisualParameters.HitSphere.FillColor);
    }

    if (VisualParameters.HitBox.bDraw || VisualParameters.HitBox.bDrawFilled)
    {
        const float HitBoxDuration = VisualParameters.HitBox.bFollowTrace ? 0.0f : Duration;
        const bool bHitBoxPersistent = VisualParameters.HitBox.bFollowTrace ? false : bPersistent;

        if (VisualParameters.HitBox.bDrawFilled)
        {
            const FBox HitBox(-VisualParameters.HitBox.Extents, VisualParameters.HitBox.Extents);
            DrawDebugSolidBox(World, HitBox, VisualParameters.HitBox.FillColor.ToFColor(true), FTransform(FQuat::Identity, HitResult.ImpactPoint), bHitBoxPersistent, HitBoxDuration, 0);
        }

        if (VisualParameters.HitBox.bDraw)
        {
            DrawDebugBox(World, HitResult.ImpactPoint, VisualParameters.HitBox.Extents, FQuat::Identity, VisualParameters.HitBox.Color.ToFColor(true), bHitBoxPersistent, HitBoxDuration, 0, 2.0f);
        }
    }

    if (VisualParameters.HitLabel.bDraw && !VisualParameters.HitLabel.Label.IsEmpty())
    {
        const float LabelDuration = VisualParameters.HitLabel.bFollowTrace ? 0.0f : (bPersistent ? 10000.0f : Duration);
        DrawDebugString(World, HitResult.ImpactPoint + FVector(0.0f, 0.0f, 20.0f), VisualParameters.HitLabel.Label, nullptr, VisualParameters.HitLabel.Color.ToFColor(true), LabelDuration, false);
    }

    if (VisualParameters.HitBounds.bDraw || VisualParameters.HitBounds.bDrawFilled)
    {
        if (const AActor* Actor = HitResult.GetActor())
        {
            TArray<FGhostBox>& GhostRegistry = GlobalGhostRegistryMap.FindOrAdd(Actor);

            FVector Origin;
            FVector BaseExtent;
            Actor->GetActorBounds(false, Origin, BaseExtent);
            // We no longer add inflation here, we add it in the render loop so it stays fixed
            
            // 2. Add or UPDATE ghost (prevents stacking flicker)
            const float UpdateThreshold = 2.0f; 
            const float TrailThreshold = 15.0f; 
            
            bool bUpdated = false;
            if (GhostRegistry.Num() > 0)
            {
                FGhostBox& LastGhost = GhostRegistry.Last();
                const float DistSq = FVector::DistSquared(LastGhost.Origin, Origin);
                
                if (DistSq < (UpdateThreshold * UpdateThreshold))
                {
                    LastGhost.LastActiveFrame = GFrameCounter;
                    LastGhost.BaseExtent = BaseExtent;
                    LastGhost.Origin = Origin;
                    bUpdated = true;
                }
                else if (DistSq < (TrailThreshold * TrailThreshold))
                {
                    // Within trail range but moved enough to be "newish" - 
                    // Refresh focus but keep original position to prevent flickering 
                    // until we actually hit the trail threshold.
                    LastGhost.LastActiveFrame = GFrameCounter;
                    bUpdated = true;
                }
            }

            if (!bUpdated)
            {
                // Truly a new position, capture with visual settings snapshot
                FGhostBox NewGhost;
                NewGhost.Origin = Origin;
                NewGhost.BaseExtent = BaseExtent;
                NewGhost.Expiration = Time + VisualParameters.HitBounds.FadeTime;
                NewGhost.LastActiveFrame = GFrameCounter;
                NewGhost.FadeTime = FMath::Max(0.01f, VisualParameters.HitBounds.FadeTime);
                NewGhost.LastPulseOffset = 0.0f;
                
                NewGhost.Color = VisualParameters.HitBounds.Color;
                NewGhost.FillColor = VisualParameters.HitBounds.FillColor;
                NewGhost.Thickness = VisualParameters.HitBounds.Thickness;
                NewGhost.bDraw = VisualParameters.HitBounds.bDraw;
                NewGhost.bDrawFilled = VisualParameters.HitBounds.bDrawFilled;
                NewGhost.bAnimateFade = VisualParameters.HitBounds.bAnimateFade;
                NewGhost.bPulse = VisualParameters.HitBounds.bPulse;
                NewGhost.PulseSpeed = VisualParameters.HitBounds.PulseSpeed;
                NewGhost.Inflation = VisualParameters.HitBounds.Inflation;

                GhostRegistry.Add(NewGhost);
                if (GhostRegistry.Num() > 32) GhostRegistry.RemoveAt(0);
            }
        }
    }

    // Always tick and render all ghosts (even if no hit this frame)
    TickAndRenderGhosts(WorldContextObject);

}

static void DrawDebugAssistSweptSphere(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float Radius, const FLinearColor& TraceColor, const FGorgeousDebugAssistVisualParameters& VisualParameters)
{
    if (!WorldContextObject || Radius <= 0.0f)
    {
        return;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return;
    }

    const FVector Path = End - Start;
    const float Distance = Path.Size();
    const bool bPersistent = VisualParameters.bPersistent;
    const float Duration = ResolveDebugDuration(VisualParameters);
    const float EffectiveDuration = FMath::Min(Duration, 0.05f);
    const FLinearColor TraceFillColor = FLinearColor(TraceColor.R, TraceColor.G, TraceColor.B, 0.2f);
    const FQuat Rotation = Distance > KINDA_SMALL_NUMBER ? FQuat::FindBetweenNormals(FVector::UpVector, Path / Distance) : FQuat::Identity;
    const FVector Center = (Start + End) * 0.5f;
    const float HalfHeight = Distance * 0.5f;

    if (VisualParameters.bDrawTracePath)
    {
        UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistLine(WorldContextObject, Start, End, TraceColor, VisualParameters.Duration, VisualParameters.TracePathThickness, VisualParameters.bPersistent);
    }

    if (VisualParameters.VFX.bEnablePathFlow)
    {
        const float Time = World->GetTimeSeconds();
        const float FlowOffset = FMath::Frac(Time * VisualParameters.VFX.FlowSpeed);
        const FVector Direction = Path.GetSafeNormal();
        const float StepSize = 100.0f / FMath::Max(0.1f, VisualParameters.VFX.FlowDensity);

        for (float d = FlowOffset * StepSize; d < Distance; d += StepSize)
        {
            FVector Point = Start + Direction * d;
            DrawDebugPoint(World, Point, VisualParameters.TracePathThickness * 4.0f, TraceColor.ToFColor(true), false, 0.0f);
        }
    }

    if (VisualParameters.bDrawSweptSphere)
    {
        if (VisualParameters.bWireframe)
        {
            DrawDebugCapsule(World, Center, HalfHeight, Radius, Rotation, TraceColor.ToFColor(true), bPersistent, EffectiveDuration, 0, VisualParameters.TracePathThickness);
        }

        const int32 FillSteps = FMath::Clamp(VisualParameters.SweptSphereSteps, 4, 32);
        const int32 SegmentCount = FMath::Clamp(VisualParameters.SweptSphereSegments, 8, 64);
        const FVector AxisX = Rotation.GetAxisX();
        const FVector AxisY = Rotation.GetAxisY();

        for (int32 FillIndex = 0; FillIndex <= FillSteps; ++FillIndex)
        {
            const float Alpha = static_cast<float>(FillIndex) / static_cast<float>(FillSteps);
            const FVector Position = FMath::Lerp(Start, End, Alpha);
            DrawDebugCircle(World, Position, Radius, SegmentCount, TraceFillColor.ToFColor(true), bPersistent, EffectiveDuration, 0, 0.0f, AxisX, AxisY, false);
        }

        UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistSphere(WorldContextObject, Start, Radius, TraceColor, VisualParameters.bWireframe, true, EffectiveDuration, bPersistent, VisualParameters.TracePathThickness, TraceFillColor);
        UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistSphere(WorldContextObject, End, Radius, TraceColor, VisualParameters.bWireframe, true, EffectiveDuration, bPersistent, VisualParameters.TracePathThickness, TraceFillColor);
    }
}

void UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistTrace(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float Radius, const FLinearColor& TraceColor, const FGorgeousDebugAssistVisualParameters& VisualParameters)
{
    if (!WorldContextObject)
    {
        return;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!ShouldDrawDebug(World, VisualParameters))
    {
        return;
    }

    if (VisualParameters.bDrawTracePath)
    {
        const float Duration = ResolveDebugDuration(VisualParameters);
            const bool bPersistent = VisualParameters.bPersistent;

            // If any follow-trace flag is enabled, we assume the trace path should also follow (be short-lived)
            const bool bFollowTrace = VisualParameters.HitSphere.bFollowTrace || VisualParameters.HitBox.bFollowTrace || VisualParameters.HitPoint.bFollowTrace;
            const float PathDuration = bFollowTrace ? 0.0f : Duration;
            const bool bPathPersistent = bFollowTrace ? false : bPersistent;

            DrawDebugLine(World, Start, End, TraceColor.ToFColor(true), bPathPersistent, PathDuration, 0, VisualParameters.TracePathThickness);
    }

    DrawDebugAssistSweptSphere(WorldContextObject, Start, End, Radius, TraceColor, VisualParameters);
 
    // Always tick and render all ghosts to ensure smooth evaporation even after trace loss
    TickAndRenderGhosts(WorldContextObject);
}

void UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistCircle(const UObject* WorldContextObject, const FVector& Location, const float Radius, const FLinearColor& Color, const bool bWireframe, const float Duration, const bool bPersistentLines, const float Thickness, const FVector& Normal)
{
    if (!WorldContextObject)
    {
        return;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!ShouldDrawDebug(World, FGorgeousDebugAssistVisualParameters())) // Uses default params for basic check
    {
        return;
    }

    const FVector AxisX = FMath::Abs(Normal.Z) < 0.99f ? FVector::VectorPlaneProject(FVector::UpVector, Normal).GetSafeNormal() : FVector::ForwardVector;
    const FVector AxisY = FVector::CrossProduct(Normal, AxisX);

    DrawDebugCircle(World, Location, Radius, 32, Color.ToFColor(true), bPersistentLines, Duration, 0, Thickness, Normal, AxisX, false);
}

void UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistProjectedCircle(const UObject* WorldContextObject, const FVector& Location, const float Radius, const FLinearColor& Color, const float Duration, const bool bPersistentLines, const float Thickness, const FVector& Normal, const AActor* IgnoreActor)
{
    if (!WorldContextObject) return;
    UWorld* World = WorldContextObject->GetWorld();
    if (!ShouldDrawDebug(World, FGorgeousDebugAssistVisualParameters())) // Uses default params for basic check
    {
        return;
    }

    FVector AxisX, AxisY;
    if (Normal.Equals(FVector::UpVector, 0.1f))
    {
        // Absolute Horizon Lock for grounding rings
        AxisX = FVector::ForwardVector;
        AxisY = FVector::RightVector;
    }
    else
    {
        AxisX = FVector::VectorPlaneProject(FVector::ForwardVector, Normal).GetSafeNormal();
        if (AxisX.IsNearlyZero()) AxisX = FVector::VectorPlaneProject(FVector::RightVector, Normal).GetSafeNormal();
        AxisY = FVector::CrossProduct(Normal, AxisX);
    }

    const int32 Segments = 32;
    TArray<FVector> Points;
    Points.AddZeroed(Segments);
    TArray<bool> Hits;
    Hits.AddZeroed(Segments);

    FCollisionQueryParams Params;
    Params.bTraceComplex = true;
    Params.AddIgnoredActor(WorldContextObject->GetTypedOuter<AActor>());
    if (IgnoreActor)
    {
        Params.AddIgnoredActor(IgnoreActor);
    }

    for (int32 i = 0; i < Segments; ++i)
    {
        const float Angle = (static_cast<float>(i) / Segments) * 2.0f * PI;
        const FVector Offset = (AxisX * FMath::Cos(Angle) + AxisY * FMath::Sin(Angle)) * Radius;
        
        // Tracing from significantly higher and lower to ensure we catch the floor
        const FVector TraceStart = Location + Offset + (Normal * 250.0f);
        const FVector TraceEnd = Location + Offset - (Normal * 250.0f);

        FHitResult Hit;
        if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
        {
            Points[i] = Hit.ImpactPoint + Normal * 2.0f; // Small Z-offset to prevent Z-fighting
            Hits[i] = true;
        }
    }

    const FColor DrawColor = Color.ToFColor(true);
    const float MaxEdgeDistSq = FMath::Square(Radius * 2.0f * PI / Segments * 2.5f);

    for (int32 i = 0; i < Segments; ++i)
    {
        const int32 NextIndex = (i + 1) % Segments;
        if (Hits[i] && Hits[NextIndex])
        {
            if (FVector::DistSquared(Points[i], Points[NextIndex]) < MaxEdgeDistSq)
            {
                DrawDebugLine(World, Points[i], Points[NextIndex], DrawColor, bPersistentLines, Duration, 0, Thickness);
            }
        }
    }
}

void UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistDiamond(const UObject* WorldContextObject, const FVector& Location, float Size, const FLinearColor& Color, float Duration, bool bPersistent, float Thickness)
{
    if (!WorldContextObject) return;
    UWorld* World = WorldContextObject->GetWorld();
    if (!World) return;

    const FColor DrawColor = Color.ToFColor(true);
    const FVector Up = FVector::UpVector * Size;
    const FVector Forward = FVector::ForwardVector * Size;
    const FVector Right = FVector::RightVector * Size;

    // Draw Diamond (Octahedron)
    DrawDebugLine(World, Location + Up, Location + Forward, DrawColor, bPersistent, Duration, 0, Thickness);
    DrawDebugLine(World, Location + Up, Location - Forward, DrawColor, bPersistent, Duration, 0, Thickness);
    DrawDebugLine(World, Location + Up, Location + Right, DrawColor, bPersistent, Duration, 0, Thickness);
    DrawDebugLine(World, Location + Up, Location - Right, DrawColor, bPersistent, Duration, 0, Thickness);

    DrawDebugLine(World, Location - Up, Location + Forward, DrawColor, bPersistent, Duration, 0, Thickness);
    DrawDebugLine(World, Location - Up, Location - Forward, DrawColor, bPersistent, Duration, 0, Thickness);
    DrawDebugLine(World, Location - Up, Location + Right, DrawColor, bPersistent, Duration, 0, Thickness);
    DrawDebugLine(World, Location - Up, Location - Right, DrawColor, bPersistent, Duration, 0, Thickness);

    DrawDebugLine(World, Location + Forward, Location + Right, DrawColor, bPersistent, Duration, 0, Thickness);
    DrawDebugLine(World, Location + Right, Location - Forward, DrawColor, bPersistent, Duration, 0, Thickness);
    DrawDebugLine(World, Location - Forward, Location - Right, DrawColor, bPersistent, Duration, 0, Thickness);
    DrawDebugLine(World, Location - Right, Location + Forward, DrawColor, bPersistent, Duration, 0, Thickness);
}

void UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistPointWithState(const UObject* WorldContextObject, const FVector& Location, EGorgeousDebugAssistPointState State, const FGorgeousDebugAssistVisualParameters& VisualParameters)
{
    if (!WorldContextObject || !VisualParameters.StatePoints.bDraw) return;
    UWorld* World = WorldContextObject->GetWorld();
    if (!World) return;

    FLinearColor StateColor;
    switch (State)
    {
    case EGorgeousDebugAssistPointState::InBounds:
        StateColor = VisualParameters.StatePoints.InBoundsColor;
        break;
    case EGorgeousDebugAssistPointState::OnBounds:
        StateColor = VisualParameters.StatePoints.OnBoundsColor;
        break;
    case EGorgeousDebugAssistPointState::OutOfBounds:
    default:
        StateColor = VisualParameters.StatePoints.OutOfBoundsColor;
        break;
    }

    const float Duration = ResolveDebugDuration(VisualParameters);
    DrawDebugAssistDiamond(WorldContextObject, Location, VisualParameters.StatePoints.Size, StateColor, Duration, VisualParameters.bPersistent, 2.0f);
}

void UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistArrow(const UObject* WorldContextObject, const FVector& Start, const FVector& End, float Size, const FLinearColor& Color, float Duration, bool bPersistent, float Thickness)
{
    if (!WorldContextObject) return;
    UWorld* World = WorldContextObject->GetWorld();
    if (!World) return;

    const FColor DrawColor = Color.ToFColor(true);
    
    // Main stem
    DrawDebugLine(World, Start, End, DrawColor, bPersistent, Duration, 0, Thickness);

    // Arrow head (cone)
    const FVector Direction = (End - Start).GetSafeNormal();
    if (!Direction.IsNearlyZero())
    {
        // Simple 4-line pyramid for the arrow head
        const FQuat Rotation = FRotationMatrix::MakeFromZ(Direction).ToQuat();
        const FVector Right = Rotation.GetRightVector() * Size * 0.5f;
        const FVector Forward = Rotation.GetForwardVector() * Size * 0.5f;
        const FVector Back = End - Direction * Size;

        DrawDebugLine(World, End, Back + Right, DrawColor, bPersistent, Duration, 0, Thickness);
        DrawDebugLine(World, End, Back - Right, DrawColor, bPersistent, Duration, 0, Thickness);
        DrawDebugLine(World, End, Back + Forward, DrawColor, bPersistent, Duration, 0, Thickness);
        DrawDebugLine(World, End, Back - Forward, DrawColor, bPersistent, Duration, 0, Thickness);
        
        // Connect the base of the pyramid
        DrawDebugLine(World, Back + Right, Back + Forward, DrawColor, bPersistent, Duration, 0, Thickness);
        DrawDebugLine(World, Back + Forward, Back - Right, DrawColor, bPersistent, Duration, 0, Thickness);
        DrawDebugLine(World, Back - Right, Back - Forward, DrawColor, bPersistent, Duration, 0, Thickness);
        DrawDebugLine(World, Back - Forward, Back + Right, DrawColor, bPersistent, Duration, 0, Thickness);
    }
}
void UGorgeousDebugAssistBlueprintFunctionLibrary::ClearDebugAssistGhosts()
{
    GlobalGhostRegistryMap.Empty();
}
