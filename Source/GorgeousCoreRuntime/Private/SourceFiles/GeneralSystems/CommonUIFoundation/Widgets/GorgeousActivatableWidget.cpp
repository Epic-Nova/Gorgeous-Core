// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActivatableWidget.h"

#include "Animation/WidgetAnimation.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperImplementation.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIState_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"

UE_UI_IMPLEMENT_WIDGET_INTERFACE(UGorgeousActivatableWidget)

UGorgeousActivatableWidget::UGorgeousActivatableWidget()
{
	StylePropertyAllowList = {
		"RenderOpacity",
		"Visibility",
		"IsEnabled"
	};
}

void UGorgeousActivatableWidget::InitializeMessage_Implementation(const FGorgeousUIMessageRequest& Request)
{
	MessageRequest = Request;
}

void UGorgeousActivatableWidget::SubmitMessageResult(FName ResultTag)
{
	if (!MessageRequest.Config || !MessageRequest.RequestID.IsValid()) return;
	FGorgeousUIMessageResult Result;
	Result.RequestID = MessageRequest.RequestID;
	Result.ResultTag = ResultTag;
	MessageRequest.Config = nullptr; // exactly one completion per dialog
	if (auto* Subsystem = UGorgeousUIFoundationSubsystem::GetForContext(this)) Subsystem->CompleteMessagePresentation(Result.RequestID);
	USignalBridgeBlueprintFunctionLibrary::DispatchLocal(this, TAG_Gorgeous_UI_System_Message_Result, FInstancedStruct::Make(Result));
	if (IsActivated()) DeactivateWidget();
}

void UGorgeousActivatableWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UE_UI_REGISTER_WIDGET_USER()
}

void UGorgeousActivatableWidget::NativeDestruct()
{
	if (MessageRequest.Config) SubmitMessageResult(TEXT("Cancel"));
	UE_UI_UNREGISTER_WIDGET()
	Super::NativeDestruct();
}

void UGorgeousActivatableWidget::ApplyThemeInterpolation(const UGorgeousUITheme_DA* Theme)
{
	UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
	if (Subsystem)
	{
		Subsystem->ApplyThemeToWidget(this, Theme);
		return;
	}

	UGorgeousUIProcessor::ApplyThemeToWidgetInternal(this, Theme);
}

void UGorgeousActivatableWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (AutoState)
	{
		UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
		if (Subsystem)
		{
			PreviousState = Subsystem->GetMostRecentUIState();
			Subsystem->PushUIState(AutoState);
		}
	}
}

void UGorgeousActivatableWidget::NativeOnDeactivated()
{
	if (MessageRequest.Config) SubmitMessageResult(TEXT("Cancel"));
	if (bRevertStateOnDeactivation && AutoState)
	{
		UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(Subsystem);
		if (Subsystem)
		{
			Subsystem->RemoveUIStateByObject(AutoState);
		}
	}

	Super::NativeOnDeactivated();
}

void UGorgeousActivatableWidget::PlayAnimationByName(const FName& AnimName)
{
	if (AnimName.IsNone()) return;

	UWidgetAnimation* Found = nullptr;
	if (FProperty* Property = GetClass()->FindPropertyByName(AnimName))
	{
		if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
		{
			Found = Cast<UWidgetAnimation>(ObjectProperty->GetObjectPropertyValue_InContainer(this));
		}
	}

	if (!Found) Found = FindObject<UWidgetAnimation>(GetClass(), *AnimName.ToString());

	if (Found) PlayAnimation(Found);
}

void UGorgeousActivatableWidget::OnUIStateChanged_Implementation(UGorgeousUIState_DA* NewState)
{
	if (!NewState) return;

	if (const FName* AnimName = StateAnimations.Find(NewState))
	{
		PlayAnimationByName(*AnimName);
	}
}
