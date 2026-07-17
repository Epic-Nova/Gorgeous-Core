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

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include "Components/SceneComponent.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
//<-------------------------------------------------------------------------->

#if GORGEOUS_SYSTEM_INSTALLED_COMMONUIFOUNDATION
	#include "GeneralSystems/InteractionFoundation/InteractionFoundationStructures.h"
	#include "GeneralSystems/InteractionFoundation/GorgeousInteractionFoundation.h"
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
					NewComp->AttachToComponent(Cast<USceneComponent>(GetComponentByClass(UBillboardComponent::StaticClass())), FAttachmentTransformRules::KeepRelativeTransform); \
				} \
			} \
		} while(0)

	#define GORGEOUS_CLASS_INTERACTION_CHARACTER_HANDLER() \
		StaticLoadClass(UActorComponent::StaticClass(), nullptr, TEXT("/GorgeousCore/Systems/InteractionFoundation/Classes/BP_InteractionCharacterHandler_AC.BP_InteractionCharacterHandler_AC_C"))

	#define GORGEOUS_CLASS_INTERACTION_FOUNDATION_PROMPT_WIDGET() \
		StaticLoadClass(UActorComponent::StaticClass(), nullptr, TEXT("/GorgeousCore/Systems/InteractionFoundation/Classes/BP_InteractionFoundationPromptWidget_AC.BP_InteractionFoundationPromptWidget_AC_C"))

	#define GORGEOUS_CLASS_INTERACTION_BASE_HANDLER() \
		StaticLoadClass(UActorComponent::StaticClass(), nullptr, TEXT("/GorgeousCore/Systems/InteractionFoundation/Classes/BP_InteractionBaseHandler_AC.BP_InteractionBaseHandler_AC_C"))

	#define GORGEOUS_CLASS_INTERACTION_ACTION_WIDGET() \
		StaticLoadClass(UActorComponent::StaticClass(), nullptr, TEXT("/GorgeousCore/Systems/InteractionFoundation/Classes/BP_InteractionActionWidget_AC.BP_InteractionActionWidget_AC_C"))

	#define GORGEOUS_CLASS_INTERACTION_PROMPT_WIDGET() \
		StaticLoadClass(UActorComponent::StaticClass(), nullptr, TEXT("/GorgeousCore/Systems/InteractionFoundation/Classes/BP_InteractionPromptWidget_AC.BP_InteractionPromptWidget_AC_C"))
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