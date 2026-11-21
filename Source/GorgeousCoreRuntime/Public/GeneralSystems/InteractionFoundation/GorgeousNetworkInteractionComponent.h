#pragma once

#include "AutoReplication/GorgeousAutoReplicationOwnerComponent.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagContainer.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "TimerManager.h"
#include "GorgeousNetworkInteractionComponent.generated.h"

class APlayerState;

UENUM(BlueprintType)
enum class EGorgeousInteractionOwnership : uint8
{
	UseComponentDefault,
	Shared,
	FirstInteractor,
	ActorOwnerOnly
};

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousNetworkInteractionState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<APlayerState> ReservedBy = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	int32 Revision = 0;
};

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousNetworkInteractionState_OV : public UGorgeousObjectVariable
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FGorgeousNetworkInteractionState State;

	void Publish(APlayerState* ReservedBy);
	virtual void OnReplicationActivated_Implementation(const FGorgeousAutoReplicationContext& Context) override;
};

/** Add to the replicated interactor pawn and to each replicated interactable opting into server routing. */
UCLASS(ClassGroup = (Gorgeous), meta = (BlueprintSpawnableComponent))
class GORGEOUSCORERUNTIME_API UGorgeousNetworkInteractionComponent : public UGorgeousAutoReplicationOwnerComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	EGorgeousInteractionOwnership DefaultOwnership = EGorgeousInteractionOwnership::Shared;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.1", ClampMax = "30.0"))
	float ReservationSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "1.0", ClampMax = "2000.0"))
	float MaxDistance = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FGorgeousAutoReplicationStreamConfig StateStream;

	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Interaction Foundation")
	UGorgeousNetworkInteractionState_OV* GetInteractionState();

	/** True on clients means the request was sent. Only the server decides whether it succeeds. */
	bool RequestInteraction(AActor* Target, FGameplayTag Tag);
	bool ExecuteInteraction(AActor* Interactor, const FHitResult& Hit, FGameplayTag Tag);

private:
	UFUNCTION(Server, Reliable)
	void ServerRequestInteraction(AActor* Target, FGameplayTag Tag);

	void ClearReservation();

	TWeakObjectPtr<AActor> ReservedInteractor;
	FTimerHandle ReservationTimer;
	double NextRequestTime = 0.0;
	bool bExecutingInteraction = false;
};
