// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "Validators/GorgeousInteractionFoundationSystemValidator.h"
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

namespace
{
	constexpr int32 FirstGameTraceChannel = ECC_GameTraceChannel1;
	constexpr int32 LastGameTraceChannel = ECC_GameTraceChannel18;

	bool IsUnconfiguredTraceChannel(const FName ChannelName)
	{
		return ChannelName.IsNone() || ChannelName.ToString().StartsWith(TEXT("GameTraceChannel"));
	}

	ECollisionChannel FindAvailableTraceChannel(const UCollisionProfile& CollisionProfile)
	{
		for (int32 ChannelIndex = FirstGameTraceChannel; ChannelIndex <= LastGameTraceChannel; ++ChannelIndex)
		{
			if (IsUnconfiguredTraceChannel(CollisionProfile.ReturnChannelNameFromContainerIndex(ChannelIndex)))
			{
				return static_cast<ECollisionChannel>(ChannelIndex);
			}
		}
		return ECC_MAX;
	}

	ECollisionChannel PromptForTraceChannelReplacement(const UCollisionProfile& CollisionProfile)
	{
		TArray<TSharedPtr<FString>> ChannelOptions;
		TArray<ECollisionChannel> Channels;
		for (int32 ChannelIndex = FirstGameTraceChannel; ChannelIndex <= LastGameTraceChannel; ++ChannelIndex)
		{
			const FName ChannelName = CollisionProfile.ReturnChannelNameFromContainerIndex(ChannelIndex);
			if (!IsUnconfiguredTraceChannel(ChannelName))
			{
				Channels.Add(static_cast<ECollisionChannel>(ChannelIndex));
				ChannelOptions.Add(MakeShared<FString>(FString::Printf(TEXT("GameTraceChannel%d - %s"),
					ChannelIndex - FirstGameTraceChannel + 1, *ChannelName.ToString())));
			}
		}

		if (Channels.IsEmpty())
		{
			return ECC_MAX;
		}

		TSharedPtr<FString> SelectedOption = ChannelOptions[0];
		ECollisionChannel SelectedChannel = Channels[0];
		TSharedRef<SWindow> Window = SNew(SWindow)
			.Title(NSLOCTEXT("GT.Validation", "ReplaceCollisionChannelTitle", "Replace Interaction Trace Channel"))
			.ClientSize(FVector2D(520.0f, 180.0f))
			.SupportsMinimize(false)
			.SupportsMaximize(false);

		Window->SetContent(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(12.0f)
			[
				SNew(STextBlock).Text(NSLOCTEXT("GT.Validation", "ReplaceCollisionChannelText",
					"All Game Trace Channels are configured. Choose a channel to replace with 'Interactable'."))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(12.0f)
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&ChannelOptions)
				.InitiallySelectedItem(SelectedOption)
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Option)
				{
					return SNew(STextBlock).Text(FText::FromString(Option.IsValid() ? *Option : FString()));
				})
				.OnSelectionChanged_Lambda([&SelectedOption, &SelectedChannel, &ChannelOptions, &Channels](TSharedPtr<FString> NewOption, ESelectInfo::Type)
				{
					SelectedOption = NewOption;
					const int32 Index = ChannelOptions.IndexOfByKey(NewOption);
					if (Channels.IsValidIndex(Index))
					{
						SelectedChannel = Channels[Index];
					}
				})
				[
					SNew(STextBlock).Text_Lambda([&SelectedOption]()
					{
						return FText::FromString(SelectedOption.IsValid() ? *SelectedOption : FString());
					})
				]
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(12.0f)
			[
				SNew(SButton)
				.Text(NSLOCTEXT("GT.Validation", "ReplaceCollisionChannelButton", "Replace Channel"))
				.OnClicked_Lambda([&Window, &SelectedChannel]()
				{
					Window->RequestDestroyWindow();
					return FReply::Handled();
				})
			]);

		FSlateApplication::Get().AddModalWindow(Window, nullptr);
		return SelectedChannel;
	}
}

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
	
	ECollisionChannel TargetChannel = FindAvailableTraceChannel(*CollisionProfile);
	bool bReplacingExistingChannel = false;
	if (TargetChannel == ECC_MAX)
	{
		const EAppReturnType::Type ReplaceAnswer = FMessageDialog::Open(
			EAppMsgType::YesNo,
			NSLOCTEXT("GT.Validation", "NoAvailableChannels",
				"All Game Trace Channels are configured. Would you like to replace one with the 'Interactable' channel?"));
		if (ReplaceAnswer != EAppReturnType::Yes)
		{
			return;
		}

		TargetChannel = PromptForTraceChannelReplacement(*CollisionProfile);
		bReplacingExistingChannel = TargetChannel != ECC_MAX;
	}
	if (TargetChannel == ECC_MAX)
	{
		return;
	}

	// Update DefaultEngine.ini
	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	
	const FString Section = TEXT("/Script/Engine.CollisionProfile");
	const FString ChannelToken = FString::Printf(TEXT("ECC_GameTraceChannel%d"),
		static_cast<int32>(TargetChannel) - FirstGameTraceChannel + 1);
	const FString Entry = FString::Printf(TEXT("(Channel=%s,Name=\"Interactable\",DefaultResponse=ECR_Block,bCanModify=True,bStaticObject=False)"),
		*ChannelToken);
	
	FString ConfigContents;
	if (!FFileHelper::LoadFileToString(ConfigContents, *ConfigPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to read project configuration file '%s' while adding the Interactable collision channel."), *ConfigPath);
		return;
	}

	TArray<FString> Lines;
	ConfigContents.ParseIntoArrayLines(Lines, false);
	const FString SectionHeader = FString::Printf(TEXT("[%s]"), *Section);
	int32 SectionIndex = INDEX_NONE;
	for (int32 LineIndex = 0; LineIndex < Lines.Num(); ++LineIndex)
	{
		FString TrimmedLine = Lines[LineIndex];
		TrimmedLine.TrimStartAndEndInline();
		if (TrimmedLine.Equals(SectionHeader, ESearchCase::IgnoreCase))
		{
			SectionIndex = LineIndex;
			break;
		}
	}

	if (SectionIndex == INDEX_NONE)
	{
		Lines.Add(SectionHeader);
		SectionIndex = Lines.Num() - 1;
	}

	if (bReplacingExistingChannel)
	{
		for (int32 LineIndex = Lines.Num() - 1; LineIndex > SectionIndex; --LineIndex)
		{
			FString TrimmedLine = Lines[LineIndex];
			TrimmedLine.TrimStartAndEndInline();
			if (TrimmedLine.StartsWith(TEXT("[")))
			{
				break;
			}
			if (TrimmedLine.Contains(TEXT("DefaultChannelResponses=")) && TrimmedLine.Contains(ChannelToken))
			{
				Lines.RemoveAt(LineIndex);
			}
		}
	}

	int32 InsertIndex = SectionIndex + 1;
	for (int32 LineIndex = SectionIndex + 1; LineIndex < Lines.Num(); ++LineIndex)
	{
		FString TrimmedLine = Lines[LineIndex];
		TrimmedLine.TrimStartAndEndInline();
		if (TrimmedLine.StartsWith(TEXT("[")))
		{
			break;
		}
		if (TrimmedLine.StartsWith(TEXT("+DefaultChannelResponses=")))
		{
			InsertIndex = LineIndex + 1;
		}
	}

	if (InsertIndex == SectionIndex + 1)
	{
		for (int32 LineIndex = SectionIndex + 1; LineIndex < Lines.Num(); ++LineIndex)
		{
			FString TrimmedLine = Lines[LineIndex];
			TrimmedLine.TrimStartAndEndInline();
			if (TrimmedLine.StartsWith(TEXT("[")))
			{
				break;
			}
			if (TrimmedLine.StartsWith(TEXT("+Profiles=")))
			{
				InsertIndex = LineIndex + 1;
			}
		}
	}

	Lines.Insert(FString::Printf(TEXT("+DefaultChannelResponses=%s"), *Entry), InsertIndex);
	const FString UpdatedConfigContents = FString::Join(Lines, TEXT("\r\n")) + TEXT("\r\n");
	if (!FFileHelper::SaveStringToFile(UpdatedConfigContents, *ConfigPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to write project configuration file '%s' while adding the Interactable collision channel."), *ConfigPath);
		return;
	}

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
