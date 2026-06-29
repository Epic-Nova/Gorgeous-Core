// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
#include "Components/SceneComponent.h"

#ifndef WITH_INTERACTION_FOUNDATION
#define WITH_INTERACTION_FOUNDATION 0
#endif

#if WITH_INTERACTION_FOUNDATION
	#include "GeneralSystems/InteractionFoundation/InteractionFoundation_I.h"
	#define GORGEOUS_INTERACTION_FOUNDATION_INTERFACE IInteractionFoundation_I
	
	#define IMPLEMENT_INTERACTION_FOUNDATION_INTERFACE_OVERRIDES \
		virtual FGameplayTagContainer RequestInteractionTags_Implementation() const override; \
		virtual bool CanInteract_Implementation(AActor* InteractingActor) const override; \
		virtual FInstancedStruct Focus_Implementation(AActor* InteractingActor, bool bRefreshRequest) const override; \
		virtual void Unfocus_Implementation(AActor* InteractingActor) const override; \
		virtual void Interact_Implementation(AActor* InteractingActor, const FHitResult& HitResult) override;

	#define IMPLEMENT_INTERACTION_FOUNDATION_INPUT_OVERRIDES \
		virtual void InteractSecondaryButton_Implementation(AActor* InteractingActor, const FGameplayTag& KeyTag, const FHitResult& HitResult) override; \
		virtual void InteractHold_Implementation(AActor* InteractingActor, const float& HoldDuration, const float& RemainingDuration, const FGameplayTag& KeyTag, const FHitResult& HitResult) override; \
		virtual void InteractRelease_Implementation(AActor* InteractingActor, const FGameplayTag& KeyTag, const FHitResult& HitResult) override; \
		virtual void InteractCancel_Implementation(AActor* InteractingActor, const float& HoldDuration, const float& RemainingDuration, const FGameplayTag& KeyTag, const FHitResult& HitResult) override;

#define GORGEOUS_INITIALIZE_INTERACTION_FOUNDATION_COMPONENTS() \
		do { \
			UClass* WidgetCompClass = GORGEOUS_CLASS_INTERACTION_FOUNDATION_PROMPT_WIDGET(); \
			if (WidgetCompClass) \
			{ \
				USceneComponent* NewComp = NewObject<USceneComponent>(this, WidgetCompClass, TEXT("InteractionFoundationPromptWidget")); \
				if (NewComp) \
				{ \
					NewComp->RegisterComponent(); \
					NewComp->AttachToComponent(this->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform); \
				} \
			} \
		} while(0)

	#define GORGEOUS_CLASS_INTERACTION_CHARACTER_HANDLER() \
		StaticLoadClass(UActorComponent::StaticClass(), nullptr, TEXT("/GorgeousCore/Systems/InteractionFoundation/Classes/BP_InteractionCharacterHandler_AC.BP_InteractionCharacterHandler_AC_C"))

	#define GORGEOUS_CLASS_INTERACTION_FOUNDATION_PROMPT_WIDGET() \
		StaticLoadClass(UActorComponent::StaticClass(), nullptr, TEXT("/GorgeousCore/Systems/InteractionFoundation/Classes/BP_InteractionFoundationPromtWidget_AC.BP_InteractionFoundationPromtWidget_AC_C"))

	#define GORGEOUS_CLASS_INTERACTION_BASE_HANDLER() \
		StaticLoadClass(UActorComponent::StaticClass(), nullptr, TEXT("/GorgeousCore/Systems/InteractionFoundation/Classes/BP_InteractionBaseHandler_AC.BP_InteractionBaseHandler_AC_C"))

	#define GORGEOUS_CLASS_INTERACTION_ACTION_WIDGET() \
		StaticLoadClass(UActorComponent::StaticClass(), nullptr, TEXT("/GorgeousCore/Systems/InteractionFoundation/Classes/BP_InteractionActionWidget_AC.BP_InteractionActionWidget_AC_C"))

	#define GORGEOUS_CLASS_INTERACTION_PROMPT_WIDGET() \
		StaticLoadClass(UActorComponent::StaticClass(), nullptr, TEXT("/GorgeousCore/Systems/InteractionFoundation/Classes/BP_InteractionPromtWidget_AC.BP_InteractionPromtWidget_AC_C"))
#else
	class IGorgeousEmptyInteractionInterface
	{
	public:
		virtual ~IGorgeousEmptyInteractionInterface() = default;
	};
	#define GORGEOUS_INTERACTION_FOUNDATION_INTERFACE IGorgeousEmptyInteractionInterface

	#define IMPLEMENT_INTERACTION_FOUNDATION_INTERFACE_OVERRIDES \
		virtual FGameplayTagContainer RequestInteractionTags_Implementation() const; \
		virtual bool CanInteract_Implementation(AActor* InteractingActor) const; \
		virtual FInstancedStruct Focus_Implementation(AActor* InteractingActor, bool bRefreshRequest) const; \
		virtual void Unfocus_Implementation(AActor* InteractingActor) const; \
		virtual void Interact_Implementation(AActor* InteractingActor, const FHitResult& HitResult);

	#define IMPLEMENT_INTERACTION_FOUNDATION_INPUT_OVERRIDES \
		virtual void InteractSecondaryButton_Implementation(AActor* InteractingActor, const FGameplayTag& KeyTag, const FHitResult& HitResult); \
		virtual void InteractHold_Implementation(AActor* InteractingActor, const float& HoldDuration, const float& RemainingDuration, const FGameplayTag& KeyTag, const FHitResult& HitResult); \
		virtual void InteractRelease_Implementation(AActor* InteractingActor, const FGameplayTag& KeyTag, const FHitResult& HitResult); \
		virtual void InteractCancel_Implementation(AActor* InteractingActor, const float& HoldDuration, const float& RemainingDuration, const FGameplayTag& KeyTag, const FHitResult& HitResult);

	#define GORGEOUS_INITIALIZE_INTERACTION_FOUNDATION_COMPONENTS()

	#define GORGEOUS_CLASS_INTERACTION_CHARACTER_HANDLER() nullptr
	#define GORGEOUS_CLASS_INTERACTION_FOUNDATION_PROMPT_WIDGET() nullptr
	#define GORGEOUS_CLASS_INTERACTION_BASE_HANDLER() nullptr
	#define GORGEOUS_CLASS_INTERACTION_ACTION_WIDGET() nullptr
	#define GORGEOUS_CLASS_INTERACTION_PROMPT_WIDGET() nullptr
#endif
