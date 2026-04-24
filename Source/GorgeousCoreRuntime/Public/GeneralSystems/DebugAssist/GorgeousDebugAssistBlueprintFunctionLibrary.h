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
#include "Engine/EngineTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GeneralSystems/DebugAssist/DebugAssistStructures.h"
#include "GorgeousDebugAssistBlueprintFunctionLibrary.generated.h"

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousDebugAssistBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Draws a debug line between Start and End with the specified Color, Duration, Thickness, and persistence.
     * 
     * @param WorldContextObject - Context object to get the world from.
     * @param Start - Starting point of the line.
     * @param End - Ending point of the line.
     * @param Color - Color of the line (default is Blue).
     * @param Duration - How long the line should be visible in seconds (default is 1.0f).
     * @param Thickness - Thickness of the line (default is 2.0f).
     * @param bPersistentLines - If true, the line will stay visible indefinitely (default is false).
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Debug Assist", meta = (WorldContext = "WorldContextObject"))
    static void DrawDebugAssistLine(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const FLinearColor& Color = FLinearColor::Blue, const float Duration = 1.0f, const float Thickness = 2.0f, const bool bPersistentLines = false);

    
    /** Draws a debug point at the specified Location with the given Color, Size, Duration, and persistence.
     * 
     * @param WorldContextObject - Context object to get the world from.
     * @param Location - Location of the point to draw.
     * @param Color - Color of the point (default is Red).
     * @param Size - Size of the point (default is 10.0f).
     * @param Duration - How long the point should be visible in seconds (default is 1.0f).
     * @param bPersistentLines - If true, the point will stay visible indefinitely (default is false).
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Debug Assist", meta = (WorldContext = "WorldContextObject"))
    static void DrawDebugAssistPoint(const UObject* WorldContextObject, const FVector& Location, const FLinearColor& Color = FLinearColor::Red, const float Size = 10.0f, const float Duration = 1.0f, const bool bPersistentLines = false);

    /** Draws a debug sphere at the specified Center with the given Radius, Color, Duration, and persistence.
     * 
     * @param WorldContextObject - Context object to get the world from.
     * @param Center - Center of the sphere to draw.
     * @param Radius - Radius of the sphere (default is 30.0f).
     * @param Color - Color of the sphere (default is White).
     * @param bWireframe - If true, draws a wireframe sphere (default is true).
     * @param bFilled - If true, draws a solid/filled sphere (default is false).
     * @param Duration - How long the sphere should be visible in seconds (default is 1.0f).
     * @param bPersistentLines - If true, the sphere will stay visible indefinitely (default is false).
     * @param Thickness - Thickness of the wireframe lines (default is 2.0f).
     * @param FillColor - Color used for the filled sphere (default is semi-transparent gray).
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Debug Assist", meta = (WorldContext = "WorldContextObject"))
    static void DrawDebugAssistSphere(const UObject* WorldContextObject, const FVector& Center, const float Radius, const FLinearColor& Color = FLinearColor::White, const bool bWireframe = true, const bool bFilled = false, const float Duration = 1.0f, const bool bPersistentLines = false, const float Thickness = 2.0f, const FLinearColor& FillColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.2f));

    /** 
     * Draws a debug line between Start and End with the specified Color, Duration, Thickness, and persistence.
     * 
     * @param WorldContextObject - Context object to get the world from.
     * @param Center - Center of the box to draw.
     * @param Extent - Extent of the box to draw (half-size in each
     * @param Color - Color of the line (default is Blue).
     * @param bWireframe - If true, draws a wireframe box (default is true).
     * @param bFilled - If true, draws a solid/filled box (default is
     * @param Duration - How long the line should be visible in seconds (default is 1.0f).
     * @param Thickness - Thickness of the line (default is 2.0f)
     * @param FillColor - Color used for the filled box (default is semi-transparent green).
     * @param bPersistentLines - If true, the line will stay visible indefinitely (default is false).
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Debug Assist", meta = (WorldContext = "WorldContextObject"))
    static void DrawDebugAssistBox(const UObject* WorldContextObject, const FVector& Center, const FVector& Extent, const FLinearColor& Color = FLinearColor::Green, const bool bWireframe = true, const bool bFilled = false, const float Duration = 1.0f, const bool bPersistentLines = false, const float Thickness = 2.0f, const FLinearColor& FillColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.2f));

    /** 
     * Draws debug visualizations for a trace based on the provided hit result and visual parameters.
     * This can include the trace path, hit point, hit normal, and swept sphere along the trace.
     * 
     * @param WorldContextObject - Context object to get the world from.
     * @param TraceStart - Starting point of the trace.
     * @param TraceEnd - Ending point of the trace.
     * @param HitResult - The hit result from the trace to visualize.
     * @param VisualParameters - Parameters that control which debug visuals to draw and their appearance.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Debug Assist", meta = (WorldContext = "WorldContextObject"))
    static void DrawDebugAssistHitResult(const UObject* WorldContextObject, const FVector& TraceStart, const FVector& TraceEnd, const FHitResult& HitResult, const FGorgeousDebugAssistVisualParameters& VisualParameters);


    /** 
     * Draws a debug line between Start and End with visual parameters for the trace path and swept sphere.
     * The appearance of the trace can be customized based on the provided visual parameters.
     * 
     * @param WorldContextObject - Context object to get the world from.
     * @param Start - Starting point of the trace.
     * @param End - Ending point of the trace.
     * @param Radius - Radius of the swept sphere (for sphere traces).
     * @param TraceColor - Color of the trace path line.
     * @param VisualParameters - Parameters that control which debug visuals to draw and their appearance for the trace.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Debug Assist", meta = (WorldContext = "WorldContextObject"))
    static void DrawDebugAssistTrace(const UObject* WorldContextObject, const FVector& Start, const FVector& End, const float Radius, const FLinearColor& TraceColor, const FGorgeousDebugAssistVisualParameters& VisualParameters);

    /** 
     * Draws a debug circle at the specified Location with the given Radius, Color, Duration, and persistence.
     * 
     * @param WorldContextObject - Context object to get the world from.
     * @param Location - Center of the circle to draw.
     * @param Radius - Radius of the circle (default is 50.0f).
     * @param Color - Color of the circle (default is Green).
     * @param bWireframe - If true, draws a wireframe circle (default is true).
     * @param Duration - How long the circle should be visible in seconds (default is 1.0f).
     * @param bPersistentLines - If true, the circle will stay visible indefinitely (default is false).
     * @param Thickness - Thickness of the circle line (default is 2.0f).
     * @param Normal - Normal vector of the circle (default is Up).
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Debug Assist", meta = (WorldContext = "WorldContextObject"))
    static void DrawDebugAssistCircle(const UObject* WorldContextObject, const FVector& Location, const float Radius = 50.0f, const FLinearColor& Color = FLinearColor::Green, const bool bWireframe = true, const float Duration = 1.0f, const bool bPersistentLines = false, const float Thickness = 2.0f, const FVector& Normal = FVector(0,0,1));

    /** 
     * Draws a debug circle that projects onto geometry faces to "wrap" around corners.
     * 
     * @param WorldContextObject - Context object to get the world from.
     * @param Location - Center of the circle to draw.
     * @param Radius - Radius of the circle.
     * @param Color - Color of the circle.
     * @param Duration - How long the circle should be visible.
     * @param bPersistentLines - If true, the circle stays indefinitely.
     * @param Thickness - Thickness of the circle lines.
     * @param Normal - Normal vector of the projection plane.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Debug Assist", meta = (WorldContext = "WorldContextObject"))
    static void DrawDebugAssistProjectedCircle(const UObject* WorldContextObject, const FVector& Location, const float Radius, const FLinearColor& Color, const float Duration = 0.0f, const bool bPersistentLines = false, const float Thickness = 2.0f, const FVector& Normal = FVector(0,0,1), const AActor* IgnoreActor = nullptr);

    /** Draws a recognizable diamond (octahedron) at the specified location. */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Debug Assist", meta = (WorldContext = "WorldContextObject"))
    static void DrawDebugAssistDiamond(const UObject* WorldContextObject, const FVector& Location, float Size = 10.0f, const FLinearColor& Color = FLinearColor::White, float Duration = 1.0f, bool bPersistent = false, float Thickness = 2.0f);

    /** Draws a state-aware point marker (using the diamond shape) based on its relationship to bounds. */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Debug Assist", meta = (WorldContext = "WorldContextObject"))
    static void DrawDebugAssistPointWithState(const UObject* WorldContextObject, const FVector& Location, EGorgeousDebugAssistPointState State, const FGorgeousDebugAssistVisualParameters& VisualParameters);

    /** Draws a gorgeous downward pointing arrow to mark grounding or locations. */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Debug Assist", meta = (WorldContext = "WorldContextObject"))
    static void DrawDebugAssistArrow(const UObject* WorldContextObject, const FVector& Start, const FVector& End, float Size, const FLinearColor& Color, float Duration = 1.0f, bool bPersistent = false, float Thickness = 2.0f);
};
