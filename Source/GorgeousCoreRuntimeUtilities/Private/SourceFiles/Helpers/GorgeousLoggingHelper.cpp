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
#include "Helpers/GorgeousLoggingHelper.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#if WITH_EDITOR
#include "Containers/Ticker.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Styling/CoreStyle.h"
#endif
//<-------------------------------------------------------------------------->

#if WITH_EDITOR
//=============================================================================
// Helpers
//=============================================================================

namespace
{
	// Holds notifications until the editor permits their display.
	TArray<FGorgeousToastParams> QueuedToastNotifications;

	// Records whether the initial editor-readiness queue has been flushed.
	bool GbToastNotificationsFlushed = false;
}
#endif

//=============================================================================
// Gorgeous Logging Toast Implementation
//=============================================================================

#if WITH_EDITOR
TSharedPtr<SNotificationItem> ShowToastNotificationImmediate(const FGorgeousToastParams& Params)
{
	auto& NotificationManager = FSlateNotificationManager::Get();

	FNotificationInfo Info(FText::FromString(Params.Title));
	Info.SubText = FText::FromString(Params.Message);
	Info.bFireAndForget = Params.bFireAndForget;
	Info.ExpireDuration = Params.ExpireDuration;
	Info.bUseThrobber = Params.bUseThrobber;
	Info.bUseSuccessFailIcons = Params.bUseIcons;
	Info.bUseLargeFont = Params.bUseLargeFont;
	Info.ButtonDetails = Params.Buttons;

    switch (Params.ToastIconKind)
    {
        case INDEX_NONE:
            break;
        case 0:
            Info.Image = FCoreStyle::Get().GetBrush(TEXT("Icons.InfoWithColor"));
            break;
        case 1:
            Info.Image = FCoreStyle::Get().GetBrush(TEXT("Icons.WarningWithColor"));
            break;
        case 2:
            Info.Image = FCoreStyle::Get().GetBrush(TEXT("Icons.SuccessWithColor"));
            break;
        case 3:
        default:
            Info.Image = FCoreStyle::Get().GetBrush(TEXT("Icons.ErrorWithColor"));
            break;
    }

	if (!Params.HyperlinkText.IsEmpty() && Params.HyperlinkDelegate.IsBound())
	{
		Info.Hyperlink = Params.HyperlinkDelegate;
		Info.HyperlinkText = Params.HyperlinkText;
	}

	return NotificationManager.AddNotification(Info);
}

TSharedPtr<SNotificationItem> ShowTrackedToastNotification(const FGorgeousToastParams& Params)
{
	if (IsRunningCommandlet() || !FSlateApplication::IsInitialized()
		|| !FSlateNotificationManager::Get().AreNotificationsAllowed())
	{
		return nullptr;
	}
	return ShowToastNotificationImmediate(Params);
}
#endif

void TryFlushQueuedToastNotifications()
{
#if WITH_EDITOR
	if (!FSlateNotificationManager::Get().AreNotificationsAllowed())
	{
		// Schedule retry
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([](float DeltaTime)
		{
			TryFlushQueuedToastNotifications();
			return false;
		}), 0.5f);
		return;
	}

	GbToastNotificationsFlushed = true;

	for (const FGorgeousToastParams& Params : QueuedToastNotifications)
	{
		ShowToastNotificationImmediate(Params);
	}

	QueuedToastNotifications.Empty();
#endif
}

void ShowToastNotification(const FGorgeousToastParams& Params)
{
#if WITH_EDITOR
	// Queue if editor not ready yet
	if (!GbToastNotificationsFlushed)
	{
		QueuedToastNotifications.Add(Params);
		TryFlushQueuedToastNotifications();
		return;
	}

	ShowToastNotificationImmediate(Params);
#endif
}

void ShowToastNotification(const FString& Title, const FString& Message, const int32 ToastIconKind)
{
	FGorgeousToastParams Params;
	Params.Title = Title;
	Params.Message = Message;
    Params.ToastIconKind = ToastIconKind;
	ShowToastNotification(Params);
}