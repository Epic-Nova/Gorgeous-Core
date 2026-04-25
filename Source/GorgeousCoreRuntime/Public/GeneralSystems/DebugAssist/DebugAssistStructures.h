// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#pragma once

#include "CoreMinimal.h"
#include "DebugAssistStructures.generated.h"

UENUM(BlueprintType)
enum class EGorgeousDebugAssistPointState : uint8
{
    InBounds     UMETA(DisplayName = "In Bounds"),
    OnBounds     UMETA(DisplayName = "On Bounds"),
    OutOfBounds  UMETA(DisplayName = "Out of Bounds")
};

/** Parameters for visualizing points relative to actor bounds states. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDebugAssistStatePointParameters
{
    GENERATED_BODY()

    /** If true, draws points showing their state relative to target bounds (In/On/Out). */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist")
    bool bDraw = true;

    /** Size of the state point markers. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw"))
    float Size = 8.0f;

    /** Color used when the point is inside the target bounds. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw"))
    FLinearColor InBoundsColor = FLinearColor::Green;

    /** Color used when the point is on the surface of the target bounds. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw"))
    FLinearColor OnBoundsColor = FLinearColor::Yellow;

    /** Color used when the point is outside the target bounds. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw"))
    FLinearColor OutOfBoundsColor = FLinearColor::Red;
};

/** Parameters for visualizing the exact impact point of a trace. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDebugAssistHitPointParameters
{
    GENERATED_BODY()

    /** If true, draws a point at the trace impact location. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist")
    bool bDraw = true;

    /** Size of the hit point marker. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw"))
    float Size = 10.0f;

    /** Color of the hit point marker. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw"))
    FLinearColor Color = FLinearColor::Green;

    /** If true, the point is only drawn for the current frame (0s duration), following the trace every frame. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw"))
    bool bFollowTrace = true;
};

/** Parameters for visualizing a sphere at the impact location. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDebugAssistHitSphereParameters
{
    GENERATED_BODY()

    /** If true, this point should be drawn. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist")
    bool bDraw = false;

    /** Radius of the hit sphere. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw || bDrawFilled"))
    float Radius = 10.0f;

    /** Color of the hit sphere (wireframe and/or fill base). */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw || bDrawFilled"))
    FLinearColor Color = FColor::Orange;

    /** If true, draws a solid/filled sphere at the impact location. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist")
    bool bDrawFilled = false;

    /** If true, the sphere is only drawn for the current frame (0s duration), following the trace every frame. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw || bDrawFilled"))
    bool bFollowTrace = true;

    /** Fill color for the solid sphere. Usually includes transparency. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDrawFilled"))
    FLinearColor FillColor = FColor(255, 128, 0, 180);
};

/** Parameters for visualizing a box at the impact location. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDebugAssistHitBoxParameters
{
    GENERATED_BODY()

    /** If true, draws a wireframe box at the impact location. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist")
    bool bDraw = false;

    /** Half-extents of the hit box. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw || bDrawFilled"))
    FVector Extents = FVector(20.0f);

    /** Color of the hit box (wireframe and/or fill base). */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw || bDrawFilled"))
    FLinearColor Color = FColor::Cyan;

    /** If true, draws a solid/filled box at the impact location. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist")
    bool bDrawFilled = false;

    /** If true, the box is only drawn for the current frame (0s duration), following the trace every frame. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw || bDrawFilled"))
    bool bFollowTrace = true;

    /** Fill color for the solid box. Usually includes transparency. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDrawFilled"))
    FLinearColor FillColor = FColor(0, 200, 255, 160);
};

/** Parameters for visualizing a text label at the impact location. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDebugAssistHitLabelParameters
{
    GENERATED_BODY()

    /** If true, this sphere should be drawn. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist")
    bool bDraw = true;

    /** If true, the label is only drawn for the current frame (0s duration), following the trace every frame. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw"))
    bool bFollowTrace = true;

    /** Color of the label text. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw"))
    FLinearColor Color = FLinearColor::White;

    /** The text content to display. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw"))
    FString Label;
};

/** Parameters for visualizing the bounds of the hit actor. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDebugAssistHitBoundsParameters
{
    GENERATED_BODY()

    /** If true, draws the bounding box of the hit actor. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist")
    bool bDraw = false;

    /** If true, draws a solid filled bounding box. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist")
    bool bDrawFilled = false;

    /** If true, the bounds are only drawn for the current frame (0s duration), following the trace every frame. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw || bDrawFilled"))
    bool bFollowTrace = true;

    /** Color of the wireframe bounds. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw"))
    FLinearColor Color = FLinearColor(0.0f, 0.4f, 0.8f);

    /** Fill color for the solid bounds. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDrawFilled"))
    FLinearColor FillColor = FLinearColor(0.68f, 0.85f, 0.9f, 0.2f);

    /** Thickness of the wireframe lines. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw"))
    float Thickness = 2.0f;

    /** Additional size added to the bounds to prevent Z-fighting with the actor's mesh. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw || bDrawFilled"))
    float Inflation = 1.0f;

    /** If true, the bounds will pulsate in size for a more dynamic visual. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw || bDrawFilled"))
    bool bPulse = false;

    /** If true, the bounds will smoothly fade out over their lifetime. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bDraw || bDrawFilled"))
    bool bAnimateFade = true;
    
    /** How long the fade out animation takes in seconds. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bAnimateFade"))
    float FadeTime = 0.75f;

    /** Speed of the pulsation effect. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bPulse"))
    float PulseSpeed = 4.0f;

    FGorgeousDebugAssistHitBoundsParameters()
    {
    }
};

/** Parameters for premium visual effects in the debug system. */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDebugAssistVFXParameters
{
    GENERATED_BODY()

    /** If true, enables animated flow along trace paths. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist")
    bool bEnablePathFlow = true;

    /** If true, enables an expanding shockwave effect at hit locations. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist")
    bool bEnableShockwave = true;

    /** If true, draws a grounding ring beneath hit actors. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist")
    bool bEnableGroundingRing = true;

    /** Speed of the shockwave expansion. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnableShockwave"))
    float ShockwaveSpeed = 0.75f;

    /** If true, uses a custom color for the shockwave instead of the shared trace color. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnableShockwave"))
    bool bOverrideShockwaveColor = false;

    /** Custom color for the shockwave. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnableShockwave && bOverrideShockwaveColor"))
    FLinearColor ShockwaveColor = FLinearColor(1.0f, 0.8f, 0.2f, 0.5f);

    /** Speed of the path flow animation. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnablePathFlow"))
    float FlowSpeed = 1.0f;

    /** Density of the animated flow segments. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnablePathFlow"))
    float FlowDensity = 1.0f;
};

/**
 * Combined parameters for all debug assist visualizations.
 * Used to configure how traces, hits, and bounds states are rendered.
 */
USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousDebugAssistVisualParameters
{
    GENERATED_BODY()

    FGorgeousDebugAssistVisualParameters()
        : bEnabled(true)
        , Duration(1.0f)
        , bPersistent(false)
        , bDrawTracePath(false)
        , TracePathThickness(2.0f)
        , bDrawSweptSphere(true)
        , bWireframe(false)
        , SweptSphereSteps(2)
        , SweptSphereSegments(16)
    {
    }

    /** Master switch for all debug visualizations in this set. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist")
    bool bEnabled;

    /** Lifetime of the debug primitives in seconds (ignored if bPersistent is true). */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled"))
    float Duration;

    /** If true, debug primitives stay visible indefinitely. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled"))
    bool bPersistent = false;

    /** If true, draws the line or path of the trace. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled"))
    bool bDrawTracePath = false;

    /** Color of the trace path line and shared base color for VFX like the expanding shockwave. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled"))
    FLinearColor TracePathColor = FLinearColor::White;

    /** Thickness of the trace path line. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled"))
    float TracePathThickness = 2.0f;

    /** If true, draws a swept sphere along the trace path (for sphere traces). */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled"))
    bool bDrawSweptSphere = true;

    /** If true, the swept sphere is drawn as a wireframe. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled && bDrawSweptSphere"))
    bool bWireframe = false;

    /** Number of intermediate spheres to draw along the sweep. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled && bDrawSweptSphere"))
    int32 SweptSphereSteps = 10;

    /** Visual fidelity (segments) of the swept spheres. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled && bDrawSweptSphere"))
    int32 SweptSphereSegments = 16;

    /** Color used for the swept sphere when it hits something. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled && bDrawSweptSphere"))
    FLinearColor SweptSphereHitColor = FColor(0, 255, 0, 32);

    /** Color used for the swept sphere when it misses everything. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled && bDrawSweptSphere"))
    FLinearColor SweptSphereMissColor = FColor(255, 0, 0, 32);

    /** Visualization settings for points relative to bounds states. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled"))
    FGorgeousDebugAssistStatePointParameters StatePoints;

    /** Visualization settings for the trace impact point. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled"))
    FGorgeousDebugAssistHitPointParameters HitPoint;

    /** Visualization settings for an impact sphere. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled"))
    FGorgeousDebugAssistHitSphereParameters HitSphere;

    /** Visualization settings for an impact box. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled"))
    FGorgeousDebugAssistHitBoxParameters HitBox;

    /** Visualization settings for an impact label. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled"))
    FGorgeousDebugAssistHitLabelParameters HitLabel;

    /** Visualization settings for hit actor bounds. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled"))
    FGorgeousDebugAssistHitBoundsParameters HitBounds;

    /** Premium visual effect settings. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gorgeous Core|Debug Assist", meta = (EditCondition = "bEnabled"))
    FGorgeousDebugAssistVFXParameters VFX;
};
