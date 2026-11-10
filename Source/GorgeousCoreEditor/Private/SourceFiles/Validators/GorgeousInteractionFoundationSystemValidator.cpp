// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "Validators/GorgeousInteractionFoundationSystemValidator.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Engine/CollisionProfile.h"
#include "Engine/SCS_Node.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Misc/DataValidation.h"
#include "Libraries/GorgeousEditorLoggingBlueprintFunctionLibrary.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"

#include "GeneralSystems/GorgeousPrimaryDataAsset.h"

#include "Helpers/Macros/GorgeousExtensionHelperMacros.h"


UGorgeousInteractionFoundationSystemValidator::UGorgeousInteractionFoundationSystemValidator()
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(INTERACTIONFOUNDATION)
	

	
#endif
}


bool UGorgeousInteractionFoundationSystemValidator::CanValidateAsset_Implementation(
    const FAssetData& InAssetData,
    UObject* InObject,
    FDataValidationContext& InContext) const
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(INTERACTIONFOUNDATION)

    if (!InObject)
    {
        return false;
    }

    // This validator performs interaction-foundation project checks. Do not run
    // it for every Gorgeous primary asset, otherwise unrelated systems receive
    // the interaction prompt during validation.

	if (const UBlueprint* Blueprint = Cast<UBlueprint>(InObject))
	{
		if (Blueprint->ParentClass.Get() &&
			Blueprint->ParentClass->IsChildOf(AActor::StaticClass()))
		{
			static const UClass* BaseClass = LoadClass<UActorComponent>(
				nullptr,
				TEXT("/GorgeousCore/Systems/InteractionFoundation/Classes/BP_InteractionBaseHandler_AC.BP_InteractionBaseHandler_AC_C")
			);

			if (BaseClass)
			{
				// Check components newly added to this Blueprint.
				if (const USimpleConstructionScript* SCS =
					Blueprint->SimpleConstructionScript)
				{
					for (const USCS_Node* Node : SCS->GetAllNodes())
					{
						if (Node && Node->ComponentClass &&
							Node->ComponentClass->IsChildOf(BaseClass))
						{
							return true;
						}
					}
				}

				// Check inherited and native default components.
				if (const UBlueprintGeneratedClass* GeneratedClass =
					Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
				{
					if (const AActor* DefaultActor =
						Cast<AActor>(GeneratedClass->GetDefaultObject()))
					{
						TInlineComponentArray<UActorComponent*> Components(DefaultActor);

						for (const UActorComponent* Component : Components)
						{
							if (Component &&
								Component->GetClass()->IsChildOf(BaseClass))
							{
								return true;
							}
						}
					}
				}
			}
		}
	}

    return InObject->IsA<UGorgeousPrimaryDataAsset>() &&
        InAssetData.AssetName == FName(TEXT("PDA_InteractionFoundation"));

#else
    return false;
#endif
}


EDataValidationResult UGorgeousInteractionFoundationSystemValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(INTERACTIONFOUNDATION)
	ValidateProjectState(Context);
#endif
	
	return Context.GetNumErrors() > 0 ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}

void UGorgeousInteractionFoundationSystemValidator::ValidateProjectState(FDataValidationContext& InContext)
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(INTERACTIONFOUNDATION)
	const UCollisionProfile* CollisionProfile = UCollisionProfile::Get();
	bool bFoundInteractable = false;
	
	//@TODO: Register a trace channel instead of a preset
	
	// Check if 'Interactable' collision channel exists
	for (int32 i = ECC_GameTraceChannel1; i <= ECC_GameTraceChannel18; ++i)
	{
		const ECollisionChannel Channel = static_cast<ECollisionChannel>(i);
		if (CollisionProfile->ReturnChannelNameFromContainerIndex(static_cast<int32>(Channel)) == FName("Interactable"))
		{
			bFoundInteractable = true;
			break;
		}
	}

	if (!bFoundInteractable)
	{
		const FText ErrorMessage = NSLOCTEXT("GT.Validation", "MissingInteractableChannel", "The required collision channel 'Interactable' is missing in Project Settings. Interaction systems may not function correctly.");
		InContext.AddError(ErrorMessage);
		
		// Log with hyperlink for easy fixing
		GT_W_LOG(TEXT("GT.Validation"), TEXT("%s"), *(ErrorMessage.ToString()));
	}
#endif
}

void UGorgeousInteractionFoundationSystemValidator::HandleFixCollisionChannel(const FString& Payload)
{
	GT_W_LOG(TEXT("GT.Validation"), TEXT("Validators only diagnose. Configure the system through a GSC Contribute Project Settings action or edit Project Settings explicitly."));
}

bool UGorgeousInteractionFoundationSystemValidator::HandleCanFixCollisionChannel(const FString& Payload)
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(INTERACTIONFOUNDATION)
	return true;
#else
	return false;
#endif
}
