// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "Validators/GorgeousInteractionFoundationSystemValidator.h"
#include "Engine/CollisionProfile.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Misc/DataValidation.h"
#include "Libraries/GorgeousEditorLoggingBlueprintFunctionLibrary.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"

#include "GeneralSystems/GorgeousPrimaryDataAsset.h"

#include "Helpers/Macros/GorgeousExtensionHelperMacros.h"

UGorgeousInteractionFoundationSystemValidator::UGorgeousInteractionFoundationSystemValidator()
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(INTERACTIONFOUNDATION)
	UGT_EditorLogging_FL::RegisterLogHyperlinkAction(
		StaticClass(),
		FName("GT.System.InteractionFoundation.Validator.FixCollisionChannel"),
		FName("HandleFixCollisionChannel"));

	UGT_EditorLogging_FL::RegisterLogHyperlinkCondition(
		StaticClass(),
		FName("GT.System.InteractionFoundation.Validator.CanFixCollisionChannel"),
		FName("HandleCanFixCollisionChannel"));
#endif
}

bool UGorgeousInteractionFoundationSystemValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(INTERACTIONFOUNDATION)
	return InObject && InObject->IsA<UGorgeousPrimaryDataAsset>();
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
		UGT_EditorLogging_FL::LogMessageWithActionHyperlink(
			ErrorMessage.ToString(),
			"GT.System.MissingCollisionChannel",
			Logging_Warning,
			FName("GT.System.InteractionFoundation.Validator.FixCollisionChannel"),
			"",
			"Add Collision Channel",
			true,
			FName("GT.System.InteractionFoundation.Validator.CanFixCollisionChannel")
		);
	}
#endif
}

void UGorgeousInteractionFoundationSystemValidator::HandleFixCollisionChannel(const FString& Payload)
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(INTERACTIONFOUNDATION)
	UCollisionProfile* CollisionProfile = UCollisionProfile::Get();
	
	// Find the first available GameTraceChannel
	ECollisionChannel FirstAvailable = ECC_MAX;
	for (int32 i = ECC_GameTraceChannel1; i <= ECC_GameTraceChannel18; ++i)
	{
		const ECollisionChannel Channel = static_cast<ECollisionChannel>(i);
		if (CollisionProfile->ReturnChannelNameFromContainerIndex(static_cast<int32>(Channel)).IsNone())
		{
			FirstAvailable = Channel;
			break;
		}
	}

	if (FirstAvailable == ECC_MAX)
	{
		FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("GT.Validation", "NoAvailableChannels", "No available Game Trace Channels found in Project Settings. Please remove an unused channel first."));
		return;
	}

	// Update DefaultEngine.ini
	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	
	// Format: +DefaultChannelResponses=(Channel=ECC_GameTraceChannel1,Name="Interactable",DefaultResponse=ECR_Ignore,bCanModify=True,bStaticObject=False)
	const FString Section = TEXT("/Script/Engine.CollisionProfile");
	const FString Entry = FString::Printf(TEXT("(Channel=%s,Name=\"Interactable\",DefaultResponse=ECR_Ignore,bCanModify=True,bStaticObject=False)"), 
		*UCollisionProfile::Get()->ReturnChannelNameFromContainerIndex(static_cast<int32>(FirstAvailable)).ToString());
	
	// We need to use FConfigCacheIni to add to the array
	TArray<FString> ExistingResponses;
	GConfig->GetArray(*Section, TEXT("DefaultChannelResponses"), ExistingResponses, ConfigPath);
	ExistingResponses.Add(Entry);
	GConfig->SetArray(*Section, TEXT("DefaultChannelResponses"), ExistingResponses, ConfigPath);
	GConfig->Flush(false, ConfigPath);

	FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("GT.Validation", "RestartRequired", "Collision channel 'Interactable' added. You must restart the editor for the changes to take effect."));
#endif
}

bool UGorgeousInteractionFoundationSystemValidator::HandleCanFixCollisionChannel(const FString& Payload)
{
#if GORGEOUS_GENERAL_SYSTEM_INSTALLED(INTERACTIONFOUNDATION)
	return true;
#else
	return false;
#endif
}
