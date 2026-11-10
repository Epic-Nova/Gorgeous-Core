// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "ModuleCore/GorgeousInGameLogging.h"
#include "ModuleCore/GorgeousLoggingDeveloperSettings.h"
#include "ModuleCore/GorgeousCoreRuntimeUtilitiesGlobals.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Async/Async.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UGorgeousInGameLogWidget::RebuildWidget()
{
    SAssignNew(MessageBox, SVerticalBox);
    RefreshMessages();
    return SNew(SBorder).Padding(8.f).HAlign(HAlign_Left).VAlign(VAlign_Top)
        [ SNew(SScrollBox) + SScrollBox::Slot()[MessageBox.ToSharedRef()] ];
}
void UGorgeousInGameLogWidget::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    MessageBox.Reset();
}
void UGorgeousInGameLogWidget::RefreshMessages()
{
    if (!MessageBox) { return; }
    MessageBox->ClearChildren();
    for (const FString& Message : Messages)
    {
        MessageBox->AddSlot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(Message)).AutoWrapText(true)];
    }
}
void UGorgeousInGameLogWidget::ReceiveGorgeousLogMessage_Implementation(const FString& Message, FName, EGorgeousLoggingImportance)
{
    Messages.Add(Message);
    if (Messages.Num() > 100) { Messages.RemoveAt(0, Messages.Num() - 100); }
    RefreshMessages();
}
void UGorgeousInGameLogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LogHandle = GetGorgeousLogEntryDelegate().AddUObject(this, &ThisClass::ReceiveLogEntry);
}
void UGorgeousInGameLogSubsystem::Deinitialize()
{
    GetGorgeousLogEntryDelegate().Remove(LogHandle);
    if (LogWidget) { LogWidget->RemoveFromParent(); LogWidget = nullptr; }
    Super::Deinitialize();
}
void UGorgeousInGameLogSubsystem::ReceiveLogEntry(const FGorgeousLogEntry& Entry)
{
    if (!IsInGameThread())
    {
        AsyncTask(ENamedThreads::GameThread, [WeakThis = TWeakObjectPtr<UGorgeousInGameLogSubsystem>(this), Entry]
        { if (WeakThis.IsValid()) { WeakThis->ReceiveLogEntry(Entry); } });
        return;
    }
    const UGorgeousLoggingDeveloperSettings* Settings = GetDefault<UGorgeousLoggingDeveloperSettings>();
    if (!Settings->bShowInGame || !Settings->bShowOnScreen || !GetGameInstance())
    {
        if (LogWidget) { LogWidget->RemoveFromParent(); LogWidget = nullptr; }
        return;
    }
    UWorld* World = GetGameInstance()->GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer) { return; }
    if (const UObject* Context = Entry.WorldContextObject.Get())
    {
        if (const UWorld* ContextWorld = Context->GetWorld(); ContextWorld && ContextWorld->GetGameInstance() != GetGameInstance()) { return; }
    }
    if (!LogWidget)
    {
        APlayerController* Player = GetGameInstance()->GetFirstLocalPlayerController();
        if (!Player) { return; }
        UClass* Class = Settings->InGameLogWidgetClass.IsNull() ? UGorgeousInGameLogWidget::StaticClass() : Settings->InGameLogWidgetClass.LoadSynchronous();
        if (!Class || !Class->ImplementsInterface(UGorgeousLogWidgetReceiver::StaticClass())) { return; }
        LogWidget = CreateWidget<UUserWidget>(Player, Class);
        if (LogWidget) { LogWidget->SetVisibility(ESlateVisibility::HitTestInvisible); LogWidget->AddToPlayerScreen(10000); }
    }
    if (LogWidget)
    {
        IGorgeousLogWidgetReceiver::Execute_ReceiveGorgeousLogMessage(LogWidget, Entry.Message, Entry.LoggingKey, Entry.Importance);
    }
}
