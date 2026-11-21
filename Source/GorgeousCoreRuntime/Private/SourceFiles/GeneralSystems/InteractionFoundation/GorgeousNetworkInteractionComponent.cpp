#include "GeneralSystems/InteractionFoundation/GorgeousNetworkInteractionComponent.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GeneralSystems/InteractionFoundation/InteractionFoundation_I.h"
#include "TimerManager.h"

namespace
{
	const FName InteractionStateKey(TEXT("InteractionFoundation.State"));
}

EGorgeousInteractionOwnership IInteractionFoundation_I::GetInteractionOwnership_Implementation() const
{
	return EGorgeousInteractionOwnership::UseComponentDefault;
}

void UGorgeousNetworkInteractionState_OV::OnReplicationActivated_Implementation(const FGorgeousAutoReplicationContext& Context)
{
	Super::OnReplicationActivated_Implementation(Context);
	RegisterReplicatedProperty(GET_MEMBER_NAME_CHECKED(UGorgeousNetworkInteractionState_OV, State));
}

void UGorgeousNetworkInteractionState_OV::Publish(APlayerState* ReservedBy)
{
	State.ReservedBy = ReservedBy;
	State.Revision = State.Revision == MAX_int32 ? 1 : State.Revision + 1;
	MarkPropertyDirty(GET_MEMBER_NAME_CHECKED(UGorgeousNetworkInteractionState_OV, State));
}

UGorgeousNetworkInteractionState_OV* UGorgeousNetworkInteractionComponent::GetInteractionState()
{
	return Cast<UGorgeousNetworkInteractionState_OV>(FindObjectVariable(InteractionStateKey));
}

bool UGorgeousNetworkInteractionComponent::RequestInteraction(AActor* Target, FGameplayTag Tag)
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!IsValid(Target) || !Pawn || !Pawn->IsLocallyControlled() || !Pawn->GetIsReplicated()
		|| Pawn->HasAuthority() || Target->GetWorld() != GetWorld() || !Target->GetIsReplicated())
	{
		return false;
	}

	const double Now = GetWorld()->GetTimeSeconds();
	if (Now < NextRequestTime)
	{
		return false;
	}
	NextRequestTime = Now + 0.1;
	ServerRequestInteraction(Target, Tag);
	return true;
}

void UGorgeousNetworkInteractionComponent::ServerRequestInteraction_Implementation(AActor* Target, FGameplayTag Tag)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	const APlayerController* Controller = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!Controller || Controller->GetPawn() != Pawn || !Pawn->HasAuthority() || !IsValid(Target)
		|| !Target->GetIsReplicated() || Target->GetWorld() != GetWorld())
	{
		return;
	}

	const double Now = GetWorld()->GetTimeSeconds();
	if (Now < NextRequestTime)
	{
		return;
	}
	NextRequestTime = Now + 0.1;

	UGorgeousNetworkInteractionComponent* TargetComponent = Target->FindComponentByClass<UGorgeousNetworkInteractionComponent>();
	if (!TargetComponent)
	{
		return;
	}

	FVector Start;
	FRotator ViewRotation;
	Pawn->GetActorEyesViewPoint(Start, ViewRotation);
	const float Range = FMath::Clamp(TargetComponent->MaxDistance, 1.0f, 2000.0f);
	const FVector End = Start + ViewRotation.Vector() * Range;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(GorgeousServerInteraction), false, Pawn);
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, TargetComponent->TraceChannel, Params) && Hit.GetActor() == Target)
	{
		TargetComponent->ExecuteInteraction(Pawn, Hit, Tag);
	}
}

bool UGorgeousNetworkInteractionComponent::ExecuteInteraction(AActor* Interactor, const FHitResult& Hit, FGameplayTag Tag)
{
	AActor* Target = GetOwner();
	if (!IsValid(Target) || !Target->HasAuthority() || !IsValid(Interactor)
		|| Interactor->GetWorld() != GetWorld() || Hit.GetActor() != Target || bExecutingInteraction
		|| !Target->Implements<UInteractionFoundation_I>())
	{
		return false;
	}

	TGuardValue<bool> ExecutionGuard(bExecutingInteraction, true);
	const EGorgeousInteractionOwnership Override = IInteractionFoundation_I::Execute_GetInteractionOwnership(Target);
	const EGorgeousInteractionOwnership Ownership =
		Override == EGorgeousInteractionOwnership::UseComponentDefault ? DefaultOwnership : Override;
	const APawn* Pawn = Cast<APawn>(Interactor);
	const AController* Controller = Pawn ? Pawn->GetController() : nullptr;
	if ((Ownership == EGorgeousInteractionOwnership::FirstInteractor && ReservedInteractor.IsValid() && ReservedInteractor.Get() != Interactor)
		|| (Ownership == EGorgeousInteractionOwnership::ActorOwnerOnly && !Target->IsOwnedBy(Interactor)
			&& (!Controller || !Target->IsOwnedBy(Controller))))
	{
		return false;
	}
	if ((Tag.IsValid() && !IInteractionFoundation_I::Execute_RequestInteractionTags(Target).HasTag(Tag))
		|| !IsValid(Target) || !IsValid(Interactor)
		|| !IInteractionFoundation_I::Execute_CanInteract(Target, Interactor) || !IsValid(Target) || !IsValid(Interactor))
	{
		return false;
	}

	APlayerState* ReservationOwner = nullptr;
	if (Ownership == EGorgeousInteractionOwnership::FirstInteractor)
	{
		ReservedInteractor = Interactor;
		ReservationOwner = Pawn ? Pawn->GetPlayerState() : nullptr;
		GetWorld()->GetTimerManager().SetTimer(ReservationTimer, this,
			&UGorgeousNetworkInteractionComponent::ClearReservation, FMath::Clamp(ReservationSeconds, 0.1f, 30.0f), false);
	}
	if (Target->GetIsReplicated())
	{
		UGorgeousNetworkInteractionState_OV* State = GetInteractionState();
		if (!State)
		{
			State = Cast<UGorgeousNetworkInteractionState_OV>(RegisterObjectVariable(
				InteractionStateKey, UGorgeousNetworkInteractionState_OV::StaticClass(), StateStream));
		}
		if (State)
		{
			State->Publish(ReservationOwner);
		}
	}
	IInteractionFoundation_I::Execute_Interact(Target, Interactor, Hit);
	return true;
}

void UGorgeousNetworkInteractionComponent::ClearReservation()
{
	ReservedInteractor.Reset();
	if (UGorgeousNetworkInteractionState_OV* State = GetInteractionState())
	{
		State->Publish(nullptr);
	}
}
