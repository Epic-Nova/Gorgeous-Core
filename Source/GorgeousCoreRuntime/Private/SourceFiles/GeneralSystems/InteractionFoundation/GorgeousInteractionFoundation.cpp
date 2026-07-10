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
#include "GeneralSystems/InteractionFoundation/GorgeousInteractionFoundation.h"
//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GeneralSystems/DebugAssist/GorgeousDebugAssistBlueprintFunctionLibrary.h"
#include "GorgeousCoreRuntimeGlobals.h"
#include "QualityOfLife/GorgeousPlayerController.h"
//<-------------------------------------------------------------------------->

TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<AActor>> UGorgeousInteractionFoundation::InteractionActors = TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<AActor>>();

namespace GorgeousInteractionFoundation
{
    static AActor* ResolveInteractingActor(const UObject* ContextObject)
    {
        if (!ContextObject)
        {
            return nullptr;
        }

        if (AActor* Actor = Cast<AActor>(const_cast<UObject*>(ContextObject)))
        {
            return Actor;
        }
        
        if (const UActorComponent* ActorComponent = Cast<UActorComponent>(const_cast<UObject*>(ContextObject)))
        {
            return ActorComponent->GetOwner();
        }

        if (APlayerController* PlayerController = Cast<APlayerController>(const_cast<UObject*>(ContextObject)))
        {
            return PlayerController;
        }

        const TArray<UObject*> QoLReferences = UGorgeousCoreRuntimeGlobals::GetQualityOfLifeReferences(ContextObject, AGorgeousPlayerController::StaticClass());
        for (UObject* Object : QoLReferences)
        {
            if (APlayerController* PlayerController = Cast<APlayerController>(Object))
            {
                return PlayerController;
            }
        }

        return nullptr;
    }

    static bool IsValidInteractionTarget(const AActor* TargetActor)
    {
        return TargetActor != nullptr && TargetActor->GetClass()->ImplementsInterface(UInteractionFoundation_I::StaticClass());
    }

    static bool DoesTargetSupportTag(const AActor* TargetActor, const FGameplayTag& InteractionTag)
    {
        if (!IsValidInteractionTarget(TargetActor) || !InteractionTag.IsValid())
        {
            return false;
        }

        const FGameplayTagContainer InteractionTags = IInteractionFoundation_I::Execute_RequestInteractionTags(TargetActor);
        return InteractionTags.HasTag(InteractionTag);
    }

    static bool PerformSphereTrace(const UObject* WorldContextObject, const FGorgeousInteractionSphereTraceParameters& TraceParameters, FHitResult& OutHitResult)
    {
        if (!WorldContextObject)
        {
            return false;
        }

        UWorld* World = WorldContextObject->GetWorld();
        if (!World)
        {
            return false;
        }

        FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GorgeousInteractionSphereTrace), TraceParameters.bTraceComplex);
        QueryParams.bReturnPhysicalMaterial = false;
        QueryParams.AddIgnoredActors(TraceParameters.ActorsToIgnore);

        if (TraceParameters.bIgnoreSelf)
        {
            if (const AActor* SelfActor = Cast<AActor>(WorldContextObject))
            {
                QueryParams.AddIgnoredActor(SelfActor);
            }
        }

        const FCollisionShape TraceShape = FCollisionShape::MakeSphere(TraceParameters.Radius);
        return World->SweepSingleByChannel(OutHitResult, TraceParameters.Start, TraceParameters.End, FQuat::Identity, TraceParameters.TraceChannel, TraceShape, QueryParams);
    }

    static FLinearColor ResolveTraceColor(const UObject* WorldContextObject, const FHitResult& HitResult, const FGameplayTag InteractionTag, const FGorgeousDebugAssistVisualParameters& VisualParameters)
    {
        if (!HitResult.bBlockingHit || !HitResult.GetActor())
        {
            return FLinearColor::White;
        }

        AActor* TargetActor = HitResult.GetActor();
        if (!GorgeousInteractionFoundation::DoesTargetSupportTag(TargetActor, InteractionTag))
        {
            return VisualParameters.SweptSphereMissColor;
        }

        AActor* InteractingActor = ResolveInteractingActor(WorldContextObject);
        if (!InteractingActor)
        {
            return FLinearColor::White;
        }

        const bool bCanInteract = IInteractionFoundation_I::Execute_CanInteract(TargetActor, InteractingActor);
        return bCanInteract ? VisualParameters.SweptSphereHitColor : VisualParameters.SweptSphereMissColor;
    }

    static void DrawDebugActorBounds(const UObject* WorldContextObject, const AActor* TargetActor, float Duration)
    {
        if (!WorldContextObject || !TargetActor || Duration <= 0.0f)
        {
            return;
        }

        UWorld* World = WorldContextObject->GetWorld();
        if (!World)
        {
            return;
        }

        FVector BoundsOrigin;
        FVector BoundsExtent;
        TargetActor->GetActorBounds(true, BoundsOrigin, BoundsExtent, true);

        if (BoundsExtent.IsNearlyZero())
        {
            return;
        }

        const FVector InflatedExtent = BoundsExtent * 1.04f + FVector(2.0f);
        UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistBox(WorldContextObject, BoundsOrigin, InflatedExtent, FLinearColor(0, 0.58f, 1.0f), true, true, Duration, false, 2.0f, FColor(0, 148, 255, 96));
    }

    static EGorgeousDebugAssistPointState ResolvePointState(const AActor* TargetActor, const FVector& Location)
    {
        if (!TargetActor)
        {
            return EGorgeousDebugAssistPointState::OutOfBounds;
        }

        FVector Origin;
        FVector BoxExtent;
        TargetActor->GetActorBounds(true, Origin, BoxExtent, true);

        if (BoxExtent.IsNearlyZero())
        {
            return EGorgeousDebugAssistPointState::OutOfBounds;
        }

        const FBox ActorBox = FBox(Origin - BoxExtent, Origin + BoxExtent).ExpandBy(2.0f);
 
        if (ActorBox.IsInsideOrOn(Location))
        {
            // Point is inside or on the surface. 
            // For hit visualizations, we favor InBounds (Green) for clarity on interactables.
            const FVector DistToMin = (Location - (Origin - BoxExtent)).GetAbs();
            const FVector DistToMax = (Location - (Origin + BoxExtent)).GetAbs();
            const float SurfaceThreshold = 0.1f; // Much tighter threshold for 'On' state

            if (DistToMin.X < SurfaceThreshold || DistToMin.Y < SurfaceThreshold || DistToMin.Z < SurfaceThreshold ||
                DistToMax.X < SurfaceThreshold || DistToMax.Y < SurfaceThreshold || DistToMax.Z < SurfaceThreshold)
            {
                // Only use Yellow if it's truly "glued" to the outer shell
                return EGorgeousDebugAssistPointState::OnBounds;
            }

            return EGorgeousDebugAssistPointState::InBounds;
        }

        return EGorgeousDebugAssistPointState::OutOfBounds;
    }
}

//=============================================================================
// UGorgeousInteractionFoundationBlueprintFunctionLibrary Implementation
//=============================================================================

bool UGorgeousInteractionFoundation::TryRequestInteractionTags(AActor* TargetActor, FGameplayTagContainer& OutInteractionTags)
{
    if (!GorgeousInteractionFoundation::IsValidInteractionTarget(TargetActor))
    {
        return false;
    }

    OutInteractionTags = IInteractionFoundation_I::Execute_RequestInteractionTags(TargetActor);
    return true;
}

bool UGorgeousInteractionFoundation::TryCanInteract(AActor* TargetActor, AActor* InteractingActor, bool& bCanInteract)
{
    if (!GorgeousInteractionFoundation::IsValidInteractionTarget(TargetActor))
    {
        return false;
    }

    bCanInteract = IInteractionFoundation_I::Execute_CanInteract(TargetActor, InteractingActor);
    return true;
}

bool UGorgeousInteractionFoundation::TryFocus(AActor* TargetActor, AActor* InteractingActor, const bool bAutoSendUnfocus, FInstancedStruct& OutFocusData, bool& bOutWasRefreshRequest, bool& bOutWasUnfocus)
{
    // Always initialize out parameters to an explicit default state at the absolute start
    bOutWasRefreshRequest = false;
    bOutWasUnfocus = false;

    if (!IsValid(InteractingActor))
    {
        return false;
    }
    
    if (bAutoSendUnfocus)
    {
        // Query the map using the raw pointer (TMap can find items using the underlying raw pointer)
        if (const TWeakObjectPtr<AActor>* PreviousTargetPtr = InteractionActors.Find(InteractingActor))
        {
            // Resolve the weak pointer to a raw pointer safely
            const AActor* PreviousTarget = PreviousTargetPtr->Get();

            // If we are already focusing this exact target, just refresh focus data and return
            if (PreviousTarget == TargetActor && IsValid(TargetActor))
            {
                OutFocusData = IInteractionFoundation_I::Execute_Focus(TargetActor, InteractingActor, true);
                bOutWasRefreshRequest = true;
                return true;
            }
            
            // If it's a different actor, send the Unfocus event immediately
            if (IsValid(PreviousTarget))
            {
                IInteractionFoundation_I::Execute_Unfocus(PreviousTarget, InteractingActor);
                bOutWasUnfocus = true;
            }

            // Clean the map tracking early so we don't leave stale references 
            InteractionActors.Remove(InteractingActor);
        }
    }

    // Now validate if the new target is allowed to be focused
    if (!GorgeousInteractionFoundation::IsValidInteractionTarget(TargetActor))
    {
        // If it's invalid, ensure it's wiped from our tracking map (in case bAutoSendUnfocus was false)
        InteractionActors.Remove(InteractingActor);
        return false;
    }

    // Successfully assign the new focus target (implicit conversion to TWeakObjectPtr)
    InteractionActors.Add(InteractingActor, TargetActor);
    OutFocusData = IInteractionFoundation_I::Execute_Focus(TargetActor, InteractingActor, false);

    return true;
}

bool UGorgeousInteractionFoundation::TryInteract(AActor* TargetActor, AActor* InteractingActor, const FHitResult& HitResult)
{
    if (!GorgeousInteractionFoundation::IsValidInteractionTarget(TargetActor))
    {
        return false;
    }

    IInteractionFoundation_I::Execute_Interact(TargetActor, InteractingActor, HitResult);
    return true;
}

bool UGorgeousInteractionFoundation::TryInteractSecondaryButton(AActor* TargetActor, AActor* InteractingActor, const FGameplayTag& KeyTag, const FHitResult& HitResult)
{
	if (!TargetActor || !TargetActor->Implements<UInteractionFoundation_I>())
	{
		return false;
	}

	IInteractionFoundation_I::Execute_InteractSecondaryButton(TargetActor, InteractingActor, KeyTag, HitResult);
	return true;
}

bool UGorgeousInteractionFoundation::TryInteractHold(AActor* TargetActor, AActor* InteractingActor, const float& HoldDuration, const float& RemainingDuration, const FGameplayTag& KeyTag, const FHitResult& HitResult)
{
	if (!TargetActor || !TargetActor->Implements<UInteractionFoundation_I>())
	{
		return false;
	}

	IInteractionFoundation_I::Execute_InteractHold(TargetActor, InteractingActor, HoldDuration, RemainingDuration, KeyTag, HitResult);
	return true;
}

bool UGorgeousInteractionFoundation::TryInteractRelease(AActor* TargetActor, AActor* InteractingActor, const FGameplayTag& KeyTag, const FHitResult& HitResult)
{
	if (!TargetActor || !TargetActor->Implements<UInteractionFoundation_I>())
	{
		return false;
	}

	IInteractionFoundation_I::Execute_InteractRelease(TargetActor, InteractingActor, KeyTag, HitResult);
	return true;
}

bool UGorgeousInteractionFoundation::TryInteractCancel(AActor* TargetActor, AActor* InteractingActor, const float& HoldDuration, const float& RemainingDuration, const FGameplayTag& KeyTag, const FHitResult& HitResult)
{
	if (!TargetActor || !TargetActor->Implements<UInteractionFoundation_I>())
	{
		return false;
	}

	IInteractionFoundation_I::Execute_InteractCancel(TargetActor, InteractingActor, HoldDuration, RemainingDuration, KeyTag, HitResult);
	return true;
}

bool UGorgeousInteractionFoundation::TrySphereTraceInteract(const UObject* WorldContextObject,
    const FHitResult& HitResult,
    const FGameplayTag InteractionTag)
{
    AActor* TargetActor = HitResult.GetActor();
    if (!TargetActor)
    {
        GT_W_LOG("GT.InteractionFoundation.Trace", TEXT("TrySphereTraceInteract failed: HitResult has no Actor."));
        return false;
    }
    
    if (!GorgeousInteractionFoundation::DoesTargetSupportTag(TargetActor, InteractionTag))
    {
        GT_W_LOG("GT.InteractionFoundation.Trace", TEXT("TrySphereTraceInteract failed: TargetActor %s does not support tag %s."), *TargetActor->GetName(), *InteractionTag.ToString());
        return false;
    }

    AActor* InteractingActor = GorgeousInteractionFoundation::ResolveInteractingActor(WorldContextObject);
    if (!InteractingActor)
    {
        GT_W_LOG("GT.InteractionFoundation.Trace", TEXT("TrySphereTraceInteract failed: Could not resolve InteractingActor."));
        return false;
    }

    bool bCanInteract = false;
    if (!TryCanInteract(TargetActor, InteractingActor, bCanInteract) || !bCanInteract)
    {
        GT_W_LOG("GT.InteractionFoundation.Trace", TEXT("TrySphereTraceInteract failed: TryCanInteract returned false for TargetActor %s."), *TargetActor->GetName());
        return false;
    }

    GT_I_LOG("GT.InteractionFoundation.Trace", TEXT("TrySphereTraceInteract success: Calling TryInteract on %s."), *TargetActor->GetName());
    return TryInteract(TargetActor, InteractingActor, HitResult);
}

bool UGorgeousInteractionFoundation::TrySphereTraceFocus(const UObject* WorldContextObject,
    const FGorgeousInteractionSphereTraceParameters& TraceParameters,
    const FGameplayTag InteractionTag,
    bool bAutoSendUnfocus,
    FInstancedStruct& OutFocusData,
    bool& bOutWasRefreshRequest,
    bool& bOutWasUnfocus,
    FHitResult& OutHitResult)
{
// Explicitly initialize out booleans at the root function call boundary
    bOutWasRefreshRequest = false;
    bOutWasUnfocus = false;

    const FGorgeousDebugAssistVisualParameters& DebugVisualParameters = TraceParameters.DebugVisualParameters;

    const bool bHit = GorgeousInteractionFoundation::PerformSphereTrace(WorldContextObject, TraceParameters, OutHitResult);
    const FLinearColor TraceColor = GorgeousInteractionFoundation::ResolveTraceColor(WorldContextObject, OutHitResult, InteractionTag, DebugVisualParameters);
    
    if (DebugVisualParameters.bEnabled)
    {
        const FVector EndPoint = bHit ? OutHitResult.Location : TraceParameters.End;
        UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistTrace(WorldContextObject, TraceParameters.Start, EndPoint, TraceParameters.Radius, TraceColor, DebugVisualParameters);
    }

    // Helper lambda to clean up focus safely before returning false
    auto HandleFailedFocus = [WorldContextObject, bAutoSendUnfocus, &OutFocusData, &bOutWasRefreshRequest, &bOutWasUnfocus]() -> bool
    {
        if (bAutoSendUnfocus)
        {
            if (AActor* InteractingActor = GorgeousInteractionFoundation::ResolveInteractingActor(WorldContextObject))
            {
                // Passing nullptr as TargetActor tells TryFocus to clear the tracking and send Unfocus
                // TryFocus will handle mutating bOutWasRefreshRequest and bOutWasUnfocus contextually
                return TryFocus(nullptr, InteractingActor, bAutoSendUnfocus, OutFocusData, bOutWasRefreshRequest, bOutWasUnfocus);
            }
        }
        return false;
    };

    if (!bHit)
    {
        return HandleFailedFocus();
    }

    if (DebugVisualParameters.bEnabled)
    {
        const bool bFollowTrace = DebugVisualParameters.HitSphere.bFollowTrace || DebugVisualParameters.HitBox.bFollowTrace || DebugVisualParameters.HitPoint.bFollowTrace;
        const float BoundsDuration = bFollowTrace ? 0.0f : 0.75f;

        FGorgeousDebugAssistVisualParameters SyncedParams = DebugVisualParameters;
        SyncedParams.TracePathColor = TraceColor;
        UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistHitResult(WorldContextObject, TraceParameters.Start, TraceParameters.End, OutHitResult, SyncedParams);

        if (DebugVisualParameters.VFX.bEnableGroundingRing)
        {
            if (AActor* HitActor = OutHitResult.GetActor())
            {
                FVector Origin = HitActor->GetActorLocation();
                FCollisionQueryParams Params;
                Params.AddIgnoredActor(HitActor);
                
                if (FHitResult GroundHit; WorldContextObject->GetWorld()->LineTraceSingleByChannel(GroundHit, Origin, Origin + FVector(0, 0, -1000.0f), ECC_Visibility, Params))
                {
                    const FVector ArrowEnd = GroundHit.ImpactPoint;
                    const FVector ArrowStart = ArrowEnd + FVector(0, 0, 40.0f);
                    UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistArrow(WorldContextObject, ArrowStart, ArrowEnd, 15.0f, TraceColor, BoundsDuration, DebugVisualParameters.bPersistent, 3.0f);
                }
            }
        }

        if (DebugVisualParameters.StatePoints.bDraw)
        {
            const EGorgeousDebugAssistPointState StartState = GorgeousInteractionFoundation::ResolvePointState(OutHitResult.GetActor(), TraceParameters.Start);
            const EGorgeousDebugAssistPointState ImpactState = GorgeousInteractionFoundation::ResolvePointState(OutHitResult.GetActor(), OutHitResult.ImpactPoint);

            FGorgeousDebugAssistVisualParameters StateVisualParameters = DebugVisualParameters;
            StateVisualParameters.Duration = BoundsDuration;
            StateVisualParameters.bPersistent = BoundsDuration < 0.0f;

            UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistPointWithState(WorldContextObject, TraceParameters.Start, StartState, StateVisualParameters);
            UGorgeousDebugAssistBlueprintFunctionLibrary::DrawDebugAssistPointWithState(WorldContextObject, OutHitResult.ImpactPoint, ImpactState, StateVisualParameters);
        }
    }

    AActor* TargetActor = OutHitResult.GetActor();
    if (!TargetActor || !GorgeousInteractionFoundation::DoesTargetSupportTag(TargetActor, InteractionTag))
    {
        return HandleFailedFocus();
    }

    AActor* InteractingActor = GorgeousInteractionFoundation::ResolveInteractingActor(WorldContextObject);
    if (!InteractingActor)
    {
        return false; 
    }

    if (bool bCanInteract = false; !TryCanInteract(TargetActor, InteractingActor, bCanInteract) || !bCanInteract)
    {
        return HandleFailedFocus();
    }

    return TryFocus(TargetActor, InteractingActor, bAutoSendUnfocus, OutFocusData, bOutWasRefreshRequest, bOutWasUnfocus);
}