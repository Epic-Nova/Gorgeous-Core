// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Actions/AsyncAction_CreateGorgeousWidgetAsync.h"
#include "Blueprint/UserWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"

#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
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
		if (auto* Subsystem = UGorgeousUIFoundationSubsystem::GetForContext(WorldContextObject))
		if (APlayerController* PC = Subsystem->GetLocalPlayer() ? Subsystem->GetLocalPlayer()->GetPlayerController(Subsystem->GetWorld()) : nullptr)
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
		if (auto* Subsystem = UGorgeousUIFoundationSubsystem::GetForContext(WorldContextObject))
		if (APlayerController* PC = Subsystem->GetLocalPlayer() ? Subsystem->GetLocalPlayer()->GetPlayerController(Subsystem->GetWorld()) : nullptr)
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

	// Preserve the requesting local player rather than assigning player zero.
	auto* Subsystem = UGorgeousUIFoundationSubsystem::GetForContext(WorldContextObject);
	APlayerController* Owner = Subsystem && Subsystem->GetLocalPlayer() ? Subsystem->GetLocalPlayer()->GetPlayerController(Subsystem->GetWorld()) : nullptr;
	UUserWidget* NewWidget = Owner ? CreateWidget<UUserWidget>(Owner, LoadedClass) : nullptr;
	if (NewWidget && Subsystem) if (auto* Theme = Subsystem->GetMostRecentTheme()) Subsystem->ApplyThemeToWidget(NewWidget, Theme);

	OnComplete.Broadcast(NewWidget);
	SetReadyToDestroy();
}
