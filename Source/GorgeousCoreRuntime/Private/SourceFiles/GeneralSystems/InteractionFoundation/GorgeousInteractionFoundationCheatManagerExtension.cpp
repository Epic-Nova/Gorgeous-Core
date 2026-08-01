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
#include "GeneralSystems/InteractionFoundation/GorgeousInteractionFoundationCheatManagerExtension.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GeneralSystems/InteractionFoundation/GorgeousInteractionFoundation.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
//<-------------------------------------------------------------------------->

namespace
{
	bool BuildViewTraceParameters(APlayerController* PlayerController, float TraceDistance, FGorgeousInteractionSphereTraceParameters& OutParameters)
	{
		if (!IsValid(PlayerController) || TraceDistance <= 0.0f)
		{
			return false;
		}

		FRotator ViewRotation;
		PlayerController->GetPlayerViewPoint(OutParameters.Start, ViewRotation);
		OutParameters.End = OutParameters.Start + (ViewRotation.Vector() * TraceDistance);
		OutParameters.ActorsToIgnore.Add(PlayerController);
		if (APawn* Pawn = PlayerController->GetPawn())
		{
			OutParameters.ActorsToIgnore.Add(Pawn);
		}

		return true;
	}

	bool TryParseInteractionTag(const FString& TagName, FGameplayTag& OutTag)
	{
		OutTag = FGameplayTag::RequestGameplayTag(FName(*TagName), false);
		return OutTag.IsValid();
	}
}

void UGorgeousInteractionFoundationCheatManagerExtension::Cheat_InspectInteractionFocus()
{
	APlayerController* PlayerController = GetPlayerController();
	if (!IsValid(PlayerController))
	{
		GT_W_LOG(TEXT("Cheat"), TEXT("Interaction focus inspection failed: no player controller is available."));
		return;
	}

	GT_I_LOG(TEXT("Cheat"), TEXT("Interaction focus: Controller=%s, Pawn=%s, TrackedActors=%d, Successful=%d, Rejected=%d, PermissionDenied=%d"),
		*PlayerController->GetName(),
		PlayerController->GetPawn() ? *PlayerController->GetPawn()->GetName() : TEXT("None"),
		UGorgeousInteractionFoundation::GetActiveTrackedInteractionActors(),
		UGorgeousInteractionFoundation::GetTotalSuccessfulInteractions(),
		UGorgeousInteractionFoundation::GetTotalRejectedInteractions(),
		UGorgeousInteractionFoundation::GetTotalPermissionDeniedInteractions());
}

void UGorgeousInteractionFoundationCheatManagerExtension::Cheat_ClearInteractionFocus()
{
	APlayerController* PlayerController = GetPlayerController();
	if (!IsValid(PlayerController))
	{
		GT_W_LOG(TEXT("Cheat"), TEXT("Interaction focus clear failed: no player controller is available."));
		return;
	}

	FInstancedStruct FocusData;
	bool bWasRefreshRequest = false;
	bool bWasUnfocus = false;
	UGorgeousInteractionFoundation::TryFocus(nullptr, PlayerController, true, FocusData, bWasRefreshRequest, bWasUnfocus);
	GT_I_LOG(TEXT("Cheat"), TEXT("Interaction focus clear requested. Unfocus sent: %s"), bWasUnfocus ? TEXT("true") : TEXT("false"));
}

void UGorgeousInteractionFoundationCheatManagerExtension::Cheat_TraceInteractionFocus(FString InteractionTag, float TraceDistance)
{
	FGameplayTag ParsedTag;
	if (!TryParseInteractionTag(InteractionTag, ParsedTag))
	{
		GT_W_LOG(TEXT("Cheat"), TEXT("Interaction focus trace failed: invalid interaction tag %s."), *InteractionTag);
		return;
	}

	APlayerController* PlayerController = GetPlayerController();
	FGorgeousInteractionSphereTraceParameters TraceParameters;
	if (!BuildViewTraceParameters(PlayerController, TraceDistance, TraceParameters))
	{
		GT_W_LOG(TEXT("Cheat"), TEXT("Interaction focus trace failed: no player controller or invalid trace distance."));
		return;
	}

	FInstancedStruct FocusData;
	FHitResult HitResult;
	bool bWasRefreshRequest = false;
	bool bWasUnfocus = false;
	const bool bSucceeded = UGorgeousInteractionFoundation::TrySphereTraceFocus(PlayerController, TraceParameters, ParsedTag, true, FocusData, bWasRefreshRequest, bWasUnfocus, HitResult);
	GT_I_LOG(TEXT("Cheat"), TEXT("Interaction focus trace %s. Hit=%s, Refresh=%s, Unfocus=%s"),
		bSucceeded ? TEXT("succeeded") : TEXT("failed"),
		HitResult.GetActor() ? *HitResult.GetActor()->GetName() : TEXT("None"),
		bWasRefreshRequest ? TEXT("true") : TEXT("false"),
		bWasUnfocus ? TEXT("true") : TEXT("false"));
}

void UGorgeousInteractionFoundationCheatManagerExtension::Cheat_TraceInteraction(FString InteractionTag, float TraceDistance)
{
	FGameplayTag ParsedTag;
	if (!TryParseInteractionTag(InteractionTag, ParsedTag))
	{
		GT_W_LOG(TEXT("Cheat"), TEXT("Interaction trace failed: invalid interaction tag %s."), *InteractionTag);
		return;
	}

	APlayerController* PlayerController = GetPlayerController();
	FGorgeousInteractionSphereTraceParameters TraceParameters;
	if (!BuildViewTraceParameters(PlayerController, TraceDistance, TraceParameters))
	{
		GT_W_LOG(TEXT("Cheat"), TEXT("Interaction trace failed: no player controller or invalid trace distance."));
		return;
	}

	UWorld* World = PlayerController->GetWorld();
	if (!IsValid(World))
	{
		GT_W_LOG(TEXT("Cheat"), TEXT("Interaction trace failed: the player controller has no world."));
		return;
	}

	FCollisionQueryParams QueryParameters(SCENE_QUERY_STAT(GorgeousInteractionCheatTrace), TraceParameters.bTraceComplex);
	for (AActor* IgnoredActor : TraceParameters.ActorsToIgnore)
	{
		QueryParameters.AddIgnoredActor(IgnoredActor);
	}

	FHitResult HitResult;
	const bool bHit = World->SweepSingleByChannel(HitResult, TraceParameters.Start, TraceParameters.End, FQuat::Identity, TraceParameters.TraceChannel, FCollisionShape::MakeSphere(TraceParameters.Radius), QueryParameters);
	const bool bSucceeded = bHit && UGorgeousInteractionFoundation::TrySphereTraceInteract(PlayerController, HitResult, ParsedTag);
	GT_I_LOG(TEXT("Cheat"), TEXT("Interaction trace %s. Hit=%s"), bSucceeded ? TEXT("succeeded") : TEXT("failed"), HitResult.GetActor() ? *HitResult.GetActor()->GetName() : TEXT("None"));
}
