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
#pragma once

#include "CoreMinimal.h"
#include "GorgeousCoreMinimalShared.h"
#include "GeneralSystems/DebugAssist/DebugAssistStructures.h"
#include "InteractionFoundationStructures.generated.h"

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousInteractionSphereTraceParameters
{
    GENERATED_BODY()

    FGorgeousInteractionSphereTraceParameters()
        : Start(FVector::ZeroVector)
        , End(FVector::ZeroVector)
        , Radius(100.0f)
        , TraceChannel(ECC_WorldDynamic)
        , bTraceComplex(false)
        , bIgnoreSelf(true)
        , ActorsToIgnore()
        , DebugVisualParameters()
    {
        DebugVisualParameters.bEnabled = true;
        DebugVisualParameters.bDrawSweptSphere = true;
        DebugVisualParameters.bWireframe = false;
        DebugVisualParameters.TracePathColor = FColor::White;
        DebugVisualParameters.SweptSphereHitColor = FColor(0, 255, 0, 32);
        DebugVisualParameters.SweptSphereMissColor = FColor(255, 0, 0, 32);
        DebugVisualParameters.SweptSphereSteps = 10;
        DebugVisualParameters.SweptSphereSegments = 16;
        
        DebugVisualParameters.HitSphere.bDraw = false;
        
        DebugVisualParameters.HitBounds.bDraw = true;
        DebugVisualParameters.HitBounds.bDrawFilled = false;
        DebugVisualParameters.HitBounds.Inflation = 1.5f;
        DebugVisualParameters.HitBounds.bPulse = true;
    }

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
    FVector Start = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
    FVector End = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
    float Radius = 15.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_WorldDynamic;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
    bool bTraceComplex = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
    bool bIgnoreSelf = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
    TArray<AActor*> ActorsToIgnore;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
    FGorgeousDebugAssistVisualParameters DebugVisualParameters;
};
