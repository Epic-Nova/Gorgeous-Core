// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Actions/AsyncAction_CreateGorgeousWidgetAsync.h"
#include "Blueprint/UserWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"

#include "GameFramework/PlayerController.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIExtensions.h"

#include "UObject/Stack.h"

UAsyncAction_CreateGorgeousWidgetAsync* UAsyncAction_CreateGorgeousWidgetAsync::CreateGorgeousWidgetAsync(
	UObject* InWorldContextObject, 
	TSoftClassPtr<UUserWidget> WidgetClass,
	bool bSuspendInputUntilComplete)
{
	if (WidgetClass.IsNull())
	{
		FFrame::KismetExecutionMessage(TEXT("CreateGorgeousWidgetAsync: WidgetClass is null! Please provide a valid soft class pointer."), ELogVerbosity::Error);
		return nullptr;
	}

	UAsyncAction_CreateGorgeousWidgetAsync* Action = NewObject<UAsyncAction_CreateGorgeousWidgetAsync>();
	Action->WorldContextObject = InWorldContextObject;
	Action->WidgetSoftClass = WidgetClass;
	Action->bSuspendInput = bSuspendInputUntilComplete;
	Action->RegisterWithGameInstance(InWorldContextObject);
	return Action;
}

void UAsyncAction_CreateGorgeousWidgetAsync::Activate()
{
	if (WidgetSoftClass.IsNull())
	{
		OnComplete.Broadcast(nullptr);
		SetReadyToDestroy();
		return;
	}

	// 1. Optional Input Suspension
	if (bSuspendInput && WorldContextObject)
	{
		if (APlayerController* PC = WorldContextObject->GetWorld()->GetFirstPlayerController())
		{
			SuspendToken = UGorgeousUIExtensions::SuspendGorgeousInput(PC, FName("AsyncWidgetCreation"));
		}
	}

	// 2. Check if class is already loaded
	if (WidgetSoftClass.Get())
	{
		OnWidgetClassLoaded();
		return;
	}

	// 3. Load the class asynchronously
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(WidgetSoftClass.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this, &UAsyncAction_CreateGorgeousWidgetAsync::OnWidgetClassLoaded));
}

void UAsyncAction_CreateGorgeousWidgetAsync::OnWidgetClassLoaded()
{
	// 4. Resume Input before firing result
	if (bSuspendInput && !SuspendToken.IsNone() && WorldContextObject)
	{
		if (APlayerController* PC = WorldContextObject->GetWorld()->GetFirstPlayerController())
		{
			UGorgeousUIExtensions::ResumeGorgeousInput(PC, SuspendToken);
		}
	}

	UClass* LoadedClass = WidgetSoftClass.Get();
	if (!LoadedClass || !WorldContextObject)
	{
		OnComplete.Broadcast(nullptr);
		SetReadyToDestroy();
		return;
	}

	// 5. Create the widget
	UUserWidget* NewWidget = CreateWidget<UUserWidget>(WorldContextObject->GetWorld(), LoadedClass);
	
	if (NewWidget)
	{
		// 6. WOW FACTOR: Automatically apply the current global theme
		// UGorgeousUIFoundationSubsystem is a LocalPlayerSubsystem; resolve via the first local player/controller
		if (APlayerController* PC = WorldContextObject->GetWorld()->GetFirstPlayerController())
		{
			if (ULocalPlayer* LP = PC->GetLocalPlayer())
			{
				if (UGorgeousUIFoundationSubsystem* Subsystem = LP->GetSubsystem<UGorgeousUIFoundationSubsystem>())
				{
					if (UGorgeousUITheme_DA* Theme = Subsystem->GetMostRecentTheme())
					{
						Subsystem->BroadcastThemeApplied(Theme);
					}
				}
			}
		}
	}

	OnComplete.Broadcast(NewWidget);
	SetReadyToDestroy();
}
