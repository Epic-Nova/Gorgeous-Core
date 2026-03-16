// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Insight Matrix (Runtime)                   |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

#include "InsightMatrix/GorgeousInsightMatrixSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "InsightMatrix/GorgeousInsightHarness.h"
#include "InsightMatrix/GorgeousInsightMatrixProvider.h"
#include "InsightMatrix/GorgeousInsightMatrixTypes.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "InsightMatrix/GorgeousInsightHarness.h"
#include "InsightMatrix/GorgeousInsightBaselineSettings.h"
#include "Helpers/Macros/GorgeousProfilingHelperMacros.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include "Misc/FileHelper.h"
#include "Misc/CoreDelegates.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/PlatformTime.h"
#include "Stats/Stats.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "InsightMatrix/Slate/GorgeousInsightDebugPanel.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/IInputProcessor.h"
#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"
#include "HAL/IConsoleManager.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SNullWidget.h"
#include "GameFramework/PlayerController.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#endif

#if WITH_DEV_AUTOMATION_TESTS
namespace
{
	class FGorgeousInsightRuntimeAutomationTest final : public FAutomationTestBase
	{
	public:
		FGorgeousInsightRuntimeAutomationTest()
			: FAutomationTestBase(TEXT("GorgeousInsight.Runtime"), /*bIsComplexTest*/ false)
		{
		}

		GORGEOUS_55_SWITCH(
			virtual uint32 GetTestFlags() const override
			{
				return static_cast<uint32>(EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);
			},
			virtual EAutomationTestFlags GetTestFlags() const override
			{
				return EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
			})

		virtual uint32 GetRequiredDeviceNum() const override
		{
			return 1;
		}

		virtual FString GetBeautifiedTestName() const override
		{
			return TEXT("Gorgeous Insight Runtime");
		}

		virtual FString GetTestSourceFileName() const override
		{
			return TEXT("GorgeousInsightMatrixSubsystem.cpp");
		}

		virtual int32 GetTestSourceFileLine() const override
		{
			return 0;
		}

		virtual void GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const override
		{
			OutBeautifiedNames.Reset();
			OutTestCommands.Reset();
		}

		virtual bool RunTest(const FString& Parameters) override
		{
			return true;
		}
	};
}
#endif

namespace
{
	static FString MakeBaselineKey(FName ProviderName, FName TestId)
	{
		return UGorgeousInsightBaselineSettings::MakeKey(ProviderName, TestId);
	}

	static bool ResolveProviderForTestId(const TArray<IGorgeousInsightMatrixProvider*>& Providers, FName TestId, FName& OutProvider)
	{
		OutProvider = NAME_None;
		FName Candidate = NAME_None;
		for (IGorgeousInsightMatrixProvider* Provider : Providers)
		{
			if (!Provider)
			{
				continue;
			}
			TArray<FGorgeousInsightTest> Tests;
			Provider->GetTests(Tests);
			if (Tests.ContainsByPredicate([TestId](const FGorgeousInsightTest& Test)
			{
				return Test.Id == TestId;
			}))
			{
				if (Candidate.IsNone())
				{
					Candidate = Provider->GetProviderName();
				}
				else
				{
					OutProvider = NAME_None;
					return false;
				}
			}
		}
		OutProvider = Candidate;
		return !OutProvider.IsNone();
	}

	static void SetBaselineCommand(const TArray<FString>& Args)
	{
		if (Args.Num() < 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("Usage: Gorgeous.Insight.SetBaseline <TestId> [ProviderName]"));
			return;
		}

		UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get();
		if (!Subsystem)
		{
			UE_LOG(LogTemp, Warning, TEXT("Insight Matrix subsystem not available."));
			return;
		}

		const FName TestId(*Args[0]);
		FName ProviderName = NAME_None;
		if (Args.Num() > 1)
		{
			ProviderName = FName(*Args[1]);
		}
		else
		{
			TArray<IGorgeousInsightMatrixProvider*> Providers = Subsystem->GetProviders();
			if (!ResolveProviderForTestId(Providers, TestId, ProviderName))
			{
				UE_LOG(LogTemp, Warning, TEXT("Could not resolve provider for test '%s'. Provide the provider name as second argument."), *TestId.ToString());
				return;
			}
		}

		if (!Subsystem->SetBaselineFromLastResult(ProviderName, TestId))
		{
			UE_LOG(LogTemp, Warning, TEXT("No last result found for test '%s' (%s). Run the test first."), *TestId.ToString(), *ProviderName.ToString());
			return;
		}

		Subsystem->RefreshDebugPanel();
		UE_LOG(LogTemp, Display, TEXT("Baseline updated for test '%s' (%s)."), *TestId.ToString(), *ProviderName.ToString());
	}

	static FAutoConsoleCommand GInsightSetBaselineCmd(
		TEXT("Gorgeous.Insight.SetBaseline"),
		TEXT("Sets baseline from last test result. Usage: Gorgeous.Insight.SetBaseline <TestId> [ProviderName]"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&SetBaselineCommand));

	DECLARE_STATS_GROUP(TEXT("Gorgeous Insight Matrix"), STATGROUP_GorgeousInsightMatrix, STATCAT_Advanced);
	DECLARE_DWORD_COUNTER_STAT(TEXT("Providers"), STAT_GIM_Providers, STATGROUP_GorgeousInsightMatrix);
	DECLARE_DWORD_COUNTER_STAT(TEXT("Stats"), STAT_GIM_Stats, STATGROUP_GorgeousInsightMatrix);
	DECLARE_DWORD_COUNTER_STAT(TEXT("Actions"), STAT_GIM_Actions, STATGROUP_GorgeousInsightMatrix);
	DECLARE_DWORD_COUNTER_STAT(TEXT("Tests"), STAT_GIM_Tests, STATGROUP_GorgeousInsightMatrix);
	DECLARE_DWORD_COUNTER_STAT(TEXT("Actions Executed"), STAT_GIM_ActionsExecuted, STATGROUP_GorgeousInsightMatrix);
	DECLARE_DWORD_COUNTER_STAT(TEXT("Tests Run"), STAT_GIM_TestsRun, STATGROUP_GorgeousInsightMatrix);
	DECLARE_DWORD_COUNTER_STAT(TEXT("CSV Exports"), STAT_GIM_ExportsCsv, STATGROUP_GorgeousInsightMatrix);
	DECLARE_DWORD_COUNTER_STAT(TEXT("JSON Exports"), STAT_GIM_ExportsJson, STATGROUP_GorgeousInsightMatrix);

	DECLARE_DELEGATE_OneParam(FGorgeousInsightDragDelta, const FVector2D&);

	class SGorgeousInsightDraggablePanel : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(SGorgeousInsightDraggablePanel) {}
			SLATE_ARGUMENT(float, HeaderHeight)
			SLATE_ATTRIBUTE(FVector2D, PanelPosition)
			SLATE_EVENT(FGorgeousInsightDragDelta, OnDragDelta)
			SLATE_ARGUMENT(TSharedPtr<SWidget>, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			HeaderHeight = InArgs._HeaderHeight;
			PanelPosition = InArgs._PanelPosition;
			OnDragDelta = InArgs._OnDragDelta;

			ChildSlot
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
				.Padding(2.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[ 
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
						.Padding(FMargin(6.f, 2.f))
						[ 
							SNew(STextBlock)
							.Text(NSLOCTEXT("GorgeousInsightDebugPanel", "DragHandle", "Gorgeous Insight Matrix"))
							.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[ 
						InArgs._Content.IsValid() ? InArgs._Content.ToSharedRef() : SNullWidget::NullWidget
					]
				]
			];
		}

		virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				const FVector2D LocalPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
				if (LocalPos.Y <= HeaderHeight)
				{
					bDragging = true;
					DragStart = MouseEvent.GetScreenSpacePosition();
					PanelStart = PanelPosition.Get();
					return FReply::Handled().CaptureMouse(SharedThis(this));
				}
			}
			return FReply::Unhandled();
		}

		virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (bDragging && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				bDragging = false;
				return FReply::Handled().ReleaseMouseCapture();
			}
			return FReply::Unhandled();
		}

		virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (bDragging && OnDragDelta.IsBound())
			{
				const FVector2D Delta = MouseEvent.GetScreenSpacePosition() - DragStart;
				OnDragDelta.Execute(PanelStart + Delta);
				return FReply::Handled();
			}
			return FReply::Unhandled();
		}

	private:
		float HeaderHeight = 28.f;
		TAttribute<FVector2D> PanelPosition;
		FGorgeousInsightDragDelta OnDragDelta;
		bool bDragging = false;
		FVector2D DragStart = FVector2D::ZeroVector;
		FVector2D PanelStart = FVector2D::ZeroVector;
	};

	class FGorgeousInsightInputPreProcessor final : public IInputProcessor
	{
	public:
		explicit FGorgeousInsightInputPreProcessor(TWeakObjectPtr<UGorgeousInsightMatrixSubsystem> InSubsystem)
			: Subsystem(MoveTemp(InSubsystem))
		{
		}

		virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
		{
			(void)DeltaTime;
			(void)SlateApp;
			(void)Cursor;
		}

		virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
		{
			if (InKeyEvent.GetKey() == EKeys::Tab)
			{
				bTabDown = true;
				return false;
			}

			if (bTabDown && InKeyEvent.GetKey() == EKeys::G)
			{
				if (UGorgeousInsightMatrixSubsystem* StrongSubsystem = Subsystem.Get())
				{
					StrongSubsystem->ToggleInGamePanel();
					return true;
				}
			}

			return false;
		}

		virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
		{
			if (InKeyEvent.GetKey() == EKeys::Tab)
			{
				bTabDown = false;
			}
			return false;
		}

	private:
		TWeakObjectPtr<UGorgeousInsightMatrixSubsystem> Subsystem;
		bool bTabDown = false;
	};

	UObject* ResolveWorldContextObject(UObject* Requested)
	{
		if (Requested)
		{
			return Requested;
		}

		if (!GEngine)
		{
			return nullptr;
		}

		for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
		{
			if (!WorldContext.World())
			{
				continue;
			}

			switch (WorldContext.WorldType)
			{
				case EWorldType::PIE:
				case EWorldType::Game:
				case EWorldType::GamePreview:
					return WorldContext.World();
				default:
					break;
			}
		}

		for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
		{
			if (WorldContext.World())
			{
				return WorldContext.World();
			}
		}

		return nullptr;
	}
}

UGorgeousInsightMatrixSubsystem* UGorgeousInsightMatrixSubsystem::Get()
{
	if (!GEngine)
	{
		return nullptr;
	}

	return GEngine->GetEngineSubsystem<UGorgeousInsightMatrixSubsystem>();
}

void UGorgeousInsightMatrixSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BaselineResultsCache.Reset();
	if (const UGorgeousInsightBaselineSettings* Settings = GetDefault<UGorgeousInsightBaselineSettings>())
	{
		for (const TPair<FString, FGorgeousInsightBaselineEntry>& Pair : Settings->Baselines)
		{
			BaselineResultsCache.Add(Pair.Key, Pair.Value.ToResult());
		}
	}

	RegisterInputPreProcessor();
	FCoreDelegates::OnPostEngineInit.AddUObject(this, &UGorgeousInsightMatrixSubsystem::RegisterInputPreProcessor);
	LoadCachedStats();
	LoadPanelState();
}

void UGorgeousInsightMatrixSubsystem::Deinitialize()
{
	UnregisterInputPreProcessor();

	HideInGamePanel();
	HideDebugPanel();
	SaveCachedStats();
	SavePanelState();

	Super::Deinitialize();
}

void UGorgeousInsightMatrixSubsystem::RegisterInputPreProcessor()
{
	if (!FSlateApplication::IsInitialized())
	{
		return;
	}

	if (!InputPreProcessor.IsValid())
	{
		InputPreProcessor = MakeShared<FGorgeousInsightInputPreProcessor>(this);
		FSlateApplication::Get().RegisterInputPreProcessor(InputPreProcessor, 0);
	}
}

void UGorgeousInsightMatrixSubsystem::UnregisterInputPreProcessor()
{
	if (FSlateApplication::IsInitialized() && InputPreProcessor.IsValid())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputPreProcessor);
		InputPreProcessor.Reset();
	}
}

void UGorgeousInsightMatrixSubsystem::ShowDebugPanel(int32 ZOrder)
{
	if (!FSlateApplication::IsInitialized())
	{
		return;
	}

	if (!DebugPanelWindow.IsValid())
	{
		DebugPanelWidget.Reset();
		DebugPanelWindow = SNew(SWindow)
			.Title(NSLOCTEXT("GorgeousInsightDebugPanel", "WindowTitle", "Gorgeous Insight Matrix"))
			.ClientSize(FVector2D(1200.f, 700.f))
			.MinWidth(800.f)
			.MinHeight(500.f)
			.SupportsMinimize(true)
			.SupportsMaximize(true);

		DebugPanelWindow->SetContent(
			SAssignNew(DebugPanelWidget, SGorgeousInsightDebugPanel)
		);
		BindPanelState(DebugPanelWidget);
		DebugPanelWindow->SetOnWindowClosed(FOnWindowClosed::CreateLambda([this](const TSharedRef<SWindow>&)
		{
			DebugPanelWidget.Reset();
			DebugPanelWindow.Reset();
		}));

		FSlateApplication::Get().AddWindow(DebugPanelWindow.ToSharedRef(), true);
	}

	if (DebugPanelWindow.IsValid())
	{
		DebugPanelWindow->BringToFront(true);
	}

	if (DebugPanelWidget.IsValid())
	{
		DebugPanelWidget->RefreshFromSubsystem();
	}
}

void UGorgeousInsightMatrixSubsystem::HideDebugPanel()
{
	if (DebugPanelWindow.IsValid())
	{
		DebugPanelWindow->RequestDestroyWindow();
	}
	DebugPanelWidget.Reset();
	DebugPanelWindow.Reset();
}

void UGorgeousInsightMatrixSubsystem::ToggleDebugPanel(int32 ZOrder)
{
	if (IsDebugPanelOpen())
	{
		HideDebugPanel();
		return;
	}

	ShowDebugPanel(ZOrder);
}

bool UGorgeousInsightMatrixSubsystem::IsDebugPanelOpen() const
{
	return DebugPanelWindow.IsValid();
}

void UGorgeousInsightMatrixSubsystem::RefreshDebugPanel()
{
	if (DebugPanelWidget.IsValid())
	{
		DebugPanelWidget->RefreshFromSubsystem();
	}
}

void UGorgeousInsightMatrixSubsystem::ShowInGamePanel(int32 ZOrder)
{
	RegisterInputPreProcessor();

	if (!GEngine || !GEngine->GameViewport || !FSlateApplication::IsInitialized())
	{
		return;
	}

	if (!InGameCanvas.IsValid())
	{
		SAssignNew(InGameCanvas, SConstraintCanvas);
	}

	if (!InGamePanelWidget.IsValid())
	{
		SAssignNew(InGamePanelWidget, SGorgeousInsightDebugPanel);
		BindPanelState(InGamePanelWidget);
	}

	if (!InGamePanelContainer.IsValid())
	{
		FVector2D ViewportSize(0.f, 0.f);
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const float MaxPanelHeight = ViewportSize.Y > 0.f ? ViewportSize.Y * 0.95f : 720.f;
		TSharedRef<SWidget> PanelContent = SNew(SBox)
			.MaxDesiredHeight(MaxPanelHeight)
			[
				InGamePanelWidget.ToSharedRef()
			];

		InGamePanelContainer = SNew(SGorgeousInsightDraggablePanel)
			.HeaderHeight(28.f)
			.PanelPosition(TAttribute<FVector2D>::CreateLambda([this]()
			{
				return GetInGamePanelPosition();
			}))
			.OnDragDelta(FGorgeousInsightDragDelta::CreateUObject(this, &UGorgeousInsightMatrixSubsystem::OnInGamePanelDragged))
			.Content(PanelContent);
		InGamePanelContainer->SlatePrepass();
		InGamePanelSize = InGamePanelContainer->GetDesiredSize();
		if (InGamePanelSize.IsNearlyZero())
		{
			InGamePanelSize = FVector2D(900.f, 620.f);
		}

		const float Margin = 24.f;
		InGamePanelSize.Y = FMath::Min(InGamePanelSize.Y, MaxPanelHeight);
		const float MaxY = FMath::Max(0.f, ViewportSize.Y - InGamePanelSize.Y - Margin);
		const float MaxX = FMath::Max(0.f, ViewportSize.X - InGamePanelSize.X - Margin);
		InGamePanelPosition = FVector2D(MaxX, FMath::Clamp(Margin, 0.f, MaxY));

		InGameCanvas->AddSlot()
		.Anchors(FAnchors(0.f, 0.f, 0.f, 0.f))
		.Alignment(FVector2D(0.f, 0.f))
		.AutoSize(false)
		.Offset(TAttribute<FMargin>::CreateLambda([this]()
		{
			return FMargin(InGamePanelPosition.X, InGamePanelPosition.Y, InGamePanelSize.X, InGamePanelSize.Y);
		}))
		[
			InGamePanelContainer.ToSharedRef()
		];

		GEngine->GameViewport->AddViewportWidgetContent(InGameCanvas.ToSharedRef(), ZOrder);
	}

	bInGamePanelVisible = true;
	UpdateInGameInputMode(true);

	if (InGamePanelWidget.IsValid())
	{
		InGamePanelWidget->RefreshFromSubsystem();
	}
}

void UGorgeousInsightMatrixSubsystem::HideInGamePanel()
{
	if (GEngine && GEngine->GameViewport && InGameCanvas.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(InGameCanvas.ToSharedRef());
	}

	InGamePanelContainer.Reset();
	InGamePanelWidget.Reset();
	InGameCanvas.Reset();
	bInGamePanelVisible = false;
	UpdateInGameInputMode(false);
}

void UGorgeousInsightMatrixSubsystem::ToggleInGamePanel(int32 ZOrder)
{
	if (IsInGamePanelOpen())
	{
		HideInGamePanel();
		return;
	}

	ShowInGamePanel(ZOrder);
}

bool UGorgeousInsightMatrixSubsystem::IsInGamePanelOpen() const
{
	return bInGamePanelVisible && InGamePanelWidget.IsValid();
}

FMargin UGorgeousInsightMatrixSubsystem::GetInGamePanelOffset() const
{
	return FMargin(InGamePanelPosition.X, InGamePanelPosition.Y, 0.f, 0.f);
}

FVector2D UGorgeousInsightMatrixSubsystem::GetInGamePanelPosition() const
{
	return InGamePanelPosition;
}

void UGorgeousInsightMatrixSubsystem::OnInGamePanelDragged(const FVector2D& NewPosition)
{
	FVector2D ViewportSize(0.f, 0.f);
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	const float Margin = 12.f;
	const float MaxX = FMath::Max(0.f, ViewportSize.X - InGamePanelSize.X - Margin);
	const float MaxY = FMath::Max(0.f, ViewportSize.Y - InGamePanelSize.Y - Margin);
	InGamePanelPosition.X = FMath::Clamp(NewPosition.X, 0.f, MaxX);
	InGamePanelPosition.Y = FMath::Clamp(NewPosition.Y, 0.f, MaxY);
	if (InGameCanvas.IsValid())
	{
		InGameCanvas->Invalidate(EInvalidateWidget::Layout);
	}
}

void UGorgeousInsightMatrixSubsystem::UpdateInGameInputMode(bool bEnable) const
{
	if (!GEngine)
	{
		return;
	}

	UWorld* World = nullptr;
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (WorldContext.World() && (WorldContext.WorldType == EWorldType::Game || WorldContext.WorldType == EWorldType::PIE))
		{
			World = WorldContext.World();
			break;
		}
	}

	if (!World)
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	if (bEnable)
	{
		PC->bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
	}
	else
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void UGorgeousInsightMatrixSubsystem::BindPanelState(const TSharedPtr<SGorgeousInsightDebugPanel>& Panel)
{
	if (!Panel.IsValid())
	{
		return;
	}

	Panel->OnStateChanged().AddUObject(this, &UGorgeousInsightMatrixSubsystem::HandlePanelStateChanged);

	if (bHasCachedPanelState)
	{
		Panel->ImportState(CachedPanelState);
		return;
	}

	TSharedPtr<SGorgeousInsightDebugPanel> OtherPanel;
	if (Panel == DebugPanelWidget)
	{
		OtherPanel = InGamePanelWidget;
	}
	else if (Panel == InGamePanelWidget)
	{
		OtherPanel = DebugPanelWidget;
	}

	if (OtherPanel.IsValid())
	{
		CachedPanelState = OtherPanel->ExportState();
		bHasCachedPanelState = true;
		Panel->ImportState(CachedPanelState);
	}
}

void UGorgeousInsightMatrixSubsystem::ApplyPanelStateToAll(const SGorgeousInsightDebugPanel::FInsightPanelState& State)
{
	TGuardValue<bool> Guard(bApplyingPanelState, true);
	if (DebugPanelWidget.IsValid())
	{
		DebugPanelWidget->ImportState(State);
	}
	if (InGamePanelWidget.IsValid())
	{
		InGamePanelWidget->ImportState(State);
	}
}

void UGorgeousInsightMatrixSubsystem::HandlePanelStateChanged(const SGorgeousInsightDebugPanel::FInsightPanelState& State)
{
	if (bApplyingPanelState)
	{
		return;
	}

	CachedPanelState = State;
	bHasCachedPanelState = true;
	ApplyPanelStateToAll(State);
	SavePanelState();
}

bool UGorgeousInsightMatrixSubsystem::RegisterProvider(IGorgeousInsightMatrixProvider* Provider)
{
	if (!Provider)
	{
		return false;
	}

	FScopeLock Lock(&ProviderMutex);
	if (Providers.Contains(Provider))
	{
		return false;
	}

	Providers.Add(Provider);
	OnProviderRegistered.Broadcast(Provider);
	return true;
}

bool UGorgeousInsightMatrixSubsystem::UnregisterProvider(IGorgeousInsightMatrixProvider* Provider)
{
	if (!Provider)
	{
		return false;
	}

	FScopeLock Lock(&ProviderMutex);
	if (!Providers.Remove(Provider))
	{
		return false;
	}

	OnProviderUnregistered.Broadcast(Provider);
	return true;
}

TArray<IGorgeousInsightMatrixProvider*> UGorgeousInsightMatrixSubsystem::GetProviders() const
{
	GORGEOUS_PROFILE_SCOPE(GIM_GetProviders);
	FScopeLock Lock(&ProviderMutex);
	TArray<IGorgeousInsightMatrixProvider*> Result;
	Result.Reserve(Providers.Num());
	for (IGorgeousInsightMatrixProvider* Provider : Providers)
	{
		Result.Add(Provider);
	}
	SET_DWORD_STAT(STAT_GIM_Providers, Result.Num());
	GORGEOUS_CSV_CUSTOM_STAT_SET(InsightProviders, Result.Num());
	return Result;
}

IGorgeousInsightMatrixProvider* UGorgeousInsightMatrixSubsystem::FindProvider(const FName ProviderName) const
{
	FScopeLock Lock(&ProviderMutex);
	for (IGorgeousInsightMatrixProvider* Provider : Providers)
	{
		if (Provider && Provider->GetProviderName() == ProviderName)
		{
			return Provider;
		}
	}
	return nullptr;
}

void UGorgeousInsightMatrixSubsystem::GatherAllStats(TArray<FGorgeousInsightStat>& OutStats) const
{
	GORGEOUS_PROFILE_SCOPE(GIM_GatherAllStats);
	FScopeLock Lock(&ProviderMutex);
	for (IGorgeousInsightMatrixProvider* Provider : Providers)
	{
		if (!Provider)
		{
			continue;
		}
		TArray<FGorgeousInsightStat> ProviderStats;
		Provider->GatherStats(ProviderStats);
		if (!ProviderStats.IsEmpty())
		{
			OutStats.Append(ProviderStats);
			CacheProviderStats(Provider->GetProviderName(), ProviderStats);
		}
		else
		{
			FScopeLock CacheLock(&StatsCacheMutex);
			if (const TArray<FGorgeousInsightStat>* Cached = CachedProviderStats.Find(Provider->GetProviderName()))
			{
				OutStats.Append(*Cached);
			}
		}
	}
	SET_DWORD_STAT(STAT_GIM_Stats, OutStats.Num());
	GORGEOUS_CSV_CUSTOM_STAT_SET(InsightStats, OutStats.Num());
}

void UGorgeousInsightMatrixSubsystem::GatherProviderStats(const FName ProviderName, TArray<FGorgeousInsightStat>& OutStats) const
{
	GORGEOUS_PROFILE_SCOPE(GIM_GatherProviderStats);
	TArray<FGorgeousInsightStat> ProviderStats;
	if (IGorgeousInsightMatrixProvider* Provider = FindProvider(ProviderName))
	{
		Provider->GatherStats(ProviderStats);
	}

	if (!ProviderStats.IsEmpty())
	{
		OutStats = ProviderStats;
		CacheProviderStats(ProviderName, ProviderStats);
	}
	else
	{
		FScopeLock CacheLock(&StatsCacheMutex);
		if (const TArray<FGorgeousInsightStat>* Cached = CachedProviderStats.Find(ProviderName))
		{
			OutStats = *Cached;
		}
	}
	SET_DWORD_STAT(STAT_GIM_Stats, OutStats.Num());
	GORGEOUS_CSV_CUSTOM_STAT_SET(InsightStats, OutStats.Num());
}

void UGorgeousInsightMatrixSubsystem::GatherProviderCharts(FName ProviderName, TArray<FGorgeousInsightChartDefinition>& OutCharts) const
{
	if (IGorgeousInsightMatrixProvider* Provider = FindProvider(ProviderName))
	{
		Provider->GatherCharts(OutCharts);
	}
}

void UGorgeousInsightMatrixSubsystem::LoadCachedStats()
{
	FScopeLock CacheLock(&StatsCacheMutex);
	if (bStatsCacheLoaded)
	{
		return;
	}

	const FString FilePath = FPaths::ProjectSavedDir() / TEXT("GorgeousInsightMatrix") / TEXT("ProviderStats.json");
	FString Contents;
	if (!FFileHelper::LoadFileToString(Contents, *FilePath))
	{
		bStatsCacheLoaded = true;
		return;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Contents);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		bStatsCacheLoaded = true;
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* ProvidersArray = nullptr;
	if (!Root->TryGetArrayField(TEXT("providers"), ProvidersArray) || !ProvidersArray)
	{
		bStatsCacheLoaded = true;
		return;
	}

	CachedProviderStats.Reset();
	for (const TSharedPtr<FJsonValue>& ProviderValue : *ProvidersArray)
	{
		const TSharedPtr<FJsonObject> ProviderObj = ProviderValue.IsValid() ? ProviderValue->AsObject() : nullptr;
		if (!ProviderObj.IsValid())
		{
			continue;
		}
		const FString ProviderName = ProviderObj->GetStringField(TEXT("provider"));
		const TArray<TSharedPtr<FJsonValue>>* StatsArray = nullptr;
		if (!ProviderObj->TryGetArrayField(TEXT("stats"), StatsArray) || !StatsArray)
		{
			continue;
		}

		TArray<FGorgeousInsightStat> Stats;
		for (const TSharedPtr<FJsonValue>& StatValue : *StatsArray)
		{
			const TSharedPtr<FJsonObject> StatObj = StatValue.IsValid() ? StatValue->AsObject() : nullptr;
			if (!StatObj.IsValid())
			{
				continue;
			}
			FGorgeousInsightStat Stat;
			Stat.Id = FName(*StatObj->GetStringField(TEXT("id")));
			Stat.DisplayName = FText::FromString(StatObj->GetStringField(TEXT("displayName")));
			Stat.Description = FText::FromString(StatObj->GetStringField(TEXT("description")));
			Stat.Category = FName(*StatObj->GetStringField(TEXT("category")));
			Stat.ValueType = static_cast<EGorgeousInsightStatValueType>(StatObj->GetIntegerField(TEXT("valueType")));
			Stat.NumericValue = StatObj->GetNumberField(TEXT("numericValue"));
			Stat.TextValue = FText::FromString(StatObj->GetStringField(TEXT("textValue")));
			Stat.Unit = FText::FromString(StatObj->GetStringField(TEXT("unit")));
			Stats.Add(Stat);
		}
		CachedProviderStats.Add(FName(*ProviderName), Stats);
	}

	bStatsCacheLoaded = true;
}

void UGorgeousInsightMatrixSubsystem::SaveCachedStats() const
{
	FScopeLock CacheLock(&StatsCacheMutex);
	if (!bStatsCacheLoaded || !bStatsCacheDirty)
	{
		return;
	}

	TArray<TSharedPtr<FJsonValue>> ProviderArray;
	for (const TPair<FName, TArray<FGorgeousInsightStat>>& Pair : CachedProviderStats)
	{
		TArray<TSharedPtr<FJsonValue>> StatArray;
		for (const FGorgeousInsightStat& Stat : Pair.Value)
		{
			TSharedRef<FJsonObject> StatObj = MakeShared<FJsonObject>();
			StatObj->SetStringField(TEXT("id"), Stat.Id.ToString());
			StatObj->SetStringField(TEXT("displayName"), Stat.DisplayName.ToString());
			StatObj->SetStringField(TEXT("description"), Stat.Description.ToString());
			StatObj->SetStringField(TEXT("category"), Stat.Category.ToString());
			StatObj->SetNumberField(TEXT("valueType"), static_cast<int32>(Stat.ValueType));
			StatObj->SetNumberField(TEXT("numericValue"), Stat.NumericValue);
			StatObj->SetStringField(TEXT("textValue"), Stat.TextValue.ToString());
			StatObj->SetStringField(TEXT("unit"), Stat.Unit.ToString());
			StatArray.Add(MakeShared<FJsonValueObject>(StatObj));
		}
		TSharedRef<FJsonObject> ProviderObj = MakeShared<FJsonObject>();
		ProviderObj->SetStringField(TEXT("provider"), Pair.Key.ToString());
		ProviderObj->SetArrayField(TEXT("stats"), StatArray);
		ProviderArray.Add(MakeShared<FJsonValueObject>(ProviderObj));
	}

	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetArrayField(TEXT("providers"), ProviderArray);

	FString Output;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
	if (!FJsonSerializer::Serialize(Root, Writer))
	{
		return;
	}

	const FString Directory = FPaths::ProjectSavedDir() / TEXT("GorgeousInsightMatrix");
	IFileManager::Get().MakeDirectory(*Directory, true);
	const FString FilePath = Directory / TEXT("ProviderStats.json");
	if (FFileHelper::SaveStringToFile(Output, *FilePath))
	{
		bStatsCacheDirty = false;
	}
}

void UGorgeousInsightMatrixSubsystem::CacheProviderStats(FName ProviderName, const TArray<FGorgeousInsightStat>& Stats) const
{
	FScopeLock CacheLock(&StatsCacheMutex);
	CachedProviderStats.FindOrAdd(ProviderName) = Stats;
	bStatsCacheLoaded = true;
	bStatsCacheDirty = true;
}

void UGorgeousInsightMatrixSubsystem::GatherAllActions(TArray<FGorgeousInsightAction>& OutActions) const
{
	GORGEOUS_PROFILE_SCOPE(GIM_GatherAllActions);
	FScopeLock Lock(&ProviderMutex);
	for (IGorgeousInsightMatrixProvider* Provider : Providers)
	{
		if (!Provider)
		{
			continue;
		}
		Provider->GetActions(OutActions);
	}
	SET_DWORD_STAT(STAT_GIM_Actions, OutActions.Num());
	GORGEOUS_CSV_CUSTOM_STAT_SET(InsightActions, OutActions.Num());
}

void UGorgeousInsightMatrixSubsystem::GatherProviderActions(const FName ProviderName, TArray<FGorgeousInsightAction>& OutActions) const
{
	GORGEOUS_PROFILE_SCOPE(GIM_GatherProviderActions);
	if (IGorgeousInsightMatrixProvider* Provider = FindProvider(ProviderName))
	{
		Provider->GetActions(OutActions);
	}
	SET_DWORD_STAT(STAT_GIM_Actions, OutActions.Num());
	GORGEOUS_CSV_CUSTOM_STAT_SET(InsightActions, OutActions.Num());
}

void UGorgeousInsightMatrixSubsystem::GatherAllTests(TArray<FGorgeousInsightTest>& OutTests) const
{
	GORGEOUS_PROFILE_SCOPE(GIM_GatherAllTests);
	FScopeLock Lock(&ProviderMutex);
	for (IGorgeousInsightMatrixProvider* Provider : Providers)
	{
		if (!Provider)
		{
			continue;
		}
		Provider->GetTests(OutTests);
	}
	SET_DWORD_STAT(STAT_GIM_Tests, OutTests.Num());
	GORGEOUS_CSV_CUSTOM_STAT_SET(InsightTests, OutTests.Num());
}

void UGorgeousInsightMatrixSubsystem::GatherProviderTests(const FName ProviderName, TArray<FGorgeousInsightTest>& OutTests) const
{
	GORGEOUS_PROFILE_SCOPE(GIM_GatherProviderTests);
	if (IGorgeousInsightMatrixProvider* Provider = FindProvider(ProviderName))
	{
		Provider->GetTests(OutTests);
	}
	SET_DWORD_STAT(STAT_GIM_Tests, OutTests.Num());
	GORGEOUS_CSV_CUSTOM_STAT_SET(InsightTests, OutTests.Num());
}

bool UGorgeousInsightMatrixSubsystem::ExecuteAction(const FName ProviderName, const FName ActionId, const FGorgeousInsightActionContext& Context)
{
	GORGEOUS_PROFILE_SCOPE(GIM_ExecuteAction);
	if (IGorgeousInsightMatrixProvider* Provider = FindProvider(ProviderName))
	{
		FGorgeousInsightActionContext ResolvedContext = Context;
		ResolvedContext.WorldContextObject = ResolveWorldContextObject(Context.WorldContextObject);
		const double StartSeconds = FPlatformTime::Seconds();
		Provider->ExecuteAction(ActionId, ResolvedContext);
		const double DurationSeconds = FPlatformTime::Seconds() - StartSeconds;
		FGorgeousInsightLastActionStats& ActionStats = LastActionStats.FindOrAdd(ProviderName).FindOrAdd(ActionId);
		ActionStats.bHasRun = true;
		ActionStats.Timestamp = FDateTime::UtcNow();
		ActionStats.ProviderName = ProviderName;
		ActionStats.ActionId = ActionId;
		ActionStats.DurationSeconds = DurationSeconds;
		ActionStats.bSucceeded = true;
		INC_DWORD_STAT(STAT_GIM_ActionsExecuted);
		GORGEOUS_CSV_CUSTOM_STAT_ACCUMULATE(InsightActionsExecuted, 1);
		GORGEOUS_TRACE_BOOKMARK(TEXT("InsightAction %s.%s"), *ProviderName.ToString(), *ActionId.ToString());
		SavePanelState(); // Save after action execution
		return true;
	}
	return false;
}

bool UGorgeousInsightMatrixSubsystem::GetLastActionStats(const FName ProviderName, const FName ActionId, FGorgeousInsightLastActionStats& OutStats) const
{
	if (const TMap<FName, FGorgeousInsightLastActionStats>* ProviderStats = LastActionStats.Find(ProviderName))
	{
		if (const FGorgeousInsightLastActionStats* Found = ProviderStats->Find(ActionId))
		{
			OutStats = *Found;
			return true;
		}
	}
	return false;
}

FGorgeousInsightTestResult UGorgeousInsightMatrixSubsystem::RunTest(const FName ProviderName, const FName TestId, const FGorgeousInsightTestContext& Context)
{
	GORGEOUS_PROFILE_SCOPE(GIM_RunTest);
	if (IGorgeousInsightMatrixProvider* Provider = FindProvider(ProviderName))
	{
		FGorgeousInsightTestContext ResolvedContext = Context;
		ResolvedContext.WorldContextObject = ResolveWorldContextObject(Context.WorldContextObject);
		const double StartSeconds = FPlatformTime::Seconds();
		FGorgeousInsightTestResult Result = Provider->RunTest(TestId, ResolvedContext);
		const double DurationSeconds = FPlatformTime::Seconds() - StartSeconds;
		UpdateLastRunStats(ProviderName, TestId, Result, DurationSeconds);
		LastTestResults.FindOrAdd(ProviderName).Add(TestId, Result);
		INC_DWORD_STAT(STAT_GIM_TestsRun);
		GORGEOUS_CSV_CUSTOM_STAT_ACCUMULATE(InsightTestsRun, 1);
		GORGEOUS_TRACE_BOOKMARK(TEXT("InsightTest %s.%s"), *ProviderName.ToString(), *TestId.ToString());
		SavePanelState(); // Save after test execution
		return Result;
	}
	return FGorgeousInsightTestResult();
}

void UGorgeousInsightMatrixSubsystem::UpdateLastRunStats(const FName ProviderName, const FName TestId, const FGorgeousInsightTestResult& Result, const double DurationSeconds)
{
	LastRunStats.bHasRun = true;
	LastRunStats.Timestamp = FDateTime::UtcNow();
	LastRunStats.ProviderName = ProviderName;
	LastRunStats.TestId = TestId;
	LastRunStats.DurationSeconds = DurationSeconds;
	LastRunStats.ErrorCount = Result.Errors.Num();
	LastRunStats.WarningCount = Result.Warnings.Num();
	LastRunStats.NoteCount = Result.Notes.Num();

	const double OvStart = FPlatformTime::Seconds();
	LastRunStats.ObjectVariablesRegistered = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry().Num();
	LastRunStats.ObjectVariableOverheadMs = (FPlatformTime::Seconds() - OvStart) * 1000.0;
}

const FGorgeousInsightTestResult* UGorgeousInsightMatrixSubsystem::GetBaselineResult(FName ProviderName, FName TestId) const
{
	const FString Key = MakeBaselineKey(ProviderName, TestId);
	return BaselineResultsCache.Find(Key);
}

bool UGorgeousInsightMatrixSubsystem::SetBaselineFromLastResult(FName ProviderName, FName TestId)
{
	const TMap<FName, FGorgeousInsightTestResult>* ProviderMap = LastTestResults.Find(ProviderName);
	if (!ProviderMap)
	{
		return false;
	}

	const FGorgeousInsightTestResult* LastResult = ProviderMap->Find(TestId);
	if (!LastResult)
	{
		return false;
	}

	UGorgeousInsightBaselineSettings* Settings = GetMutableDefault<UGorgeousInsightBaselineSettings>();
	if (!Settings)
	{
		return false;
	}

	const FString Key = MakeBaselineKey(ProviderName, TestId);
	Settings->Baselines.Add(Key, FGorgeousInsightBaselineEntry::FromResult(*LastResult));
	Settings->SaveConfig();
	BaselineResultsCache.Add(Key, *LastResult);
	return true;
}

bool UGorgeousInsightMatrixSubsystem::ExportStatsToCSV(const FString& AbsoluteFilePath, FName ProviderName) const
{
	GORGEOUS_PROFILE_SCOPE(GIM_ExportStatsCsv);
	if (AbsoluteFilePath.IsEmpty())
	{
		return false;
	}
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(AbsoluteFilePath), true);

	TArray<FGorgeousInsightStat> Stats;
	if (ProviderName.IsNone())
	{
		GatherAllStats(Stats);
	}
	else
	{
		GatherProviderStats(ProviderName, Stats);
	}

	FString Csv = TEXT("Id,DisplayName,Category,ValueType,NumericValue,TextValue,Unit\n");
	for (const FGorgeousInsightStat& Stat : Stats)
	{
		Csv += FString::Printf(TEXT("%s,%s,%s,%d,%.6f,%s,%s\n"),
			*Stat.Id.ToString(),
			*Stat.DisplayName.ToString(),
			*Stat.Category.ToString(),
			static_cast<int32>(Stat.ValueType),
			Stat.NumericValue,
			*Stat.TextValue.ToString(),
			*Stat.Unit.ToString());
	}

	const bool bSaved = FFileHelper::SaveStringToFile(Csv, *AbsoluteFilePath);
	if (bSaved)
	{
		INC_DWORD_STAT(STAT_GIM_ExportsCsv);
		GORGEOUS_CSV_CUSTOM_STAT_ACCUMULATE(InsightExportsCsv, 1);
		GORGEOUS_TRACE_BOOKMARK(TEXT("InsightExportCSV %s"), *AbsoluteFilePath);
	}
	return bSaved;
}

bool UGorgeousInsightMatrixSubsystem::ExportStatsToJson(const FString& AbsoluteFilePath, FName ProviderName) const
{
	GORGEOUS_PROFILE_SCOPE(GIM_ExportStatsJson);
	if (AbsoluteFilePath.IsEmpty())
	{
		return false;
	}
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(AbsoluteFilePath), true);

	TArray<FGorgeousInsightStat> Stats;
	if (ProviderName.IsNone())
	{
		GatherAllStats(Stats);
	}
	else
	{
		GatherProviderStats(ProviderName, Stats);
	}

	TArray<TSharedPtr<FJsonValue>> StatArray;
	StatArray.Reserve(Stats.Num());
	for (const FGorgeousInsightStat& Stat : Stats)
	{
		TSharedRef<FJsonObject> StatObj = MakeShared<FJsonObject>();
		StatObj->SetStringField(TEXT("id"), Stat.Id.ToString());
		StatObj->SetStringField(TEXT("displayName"), Stat.DisplayName.ToString());
		StatObj->SetStringField(TEXT("category"), Stat.Category.ToString());
		StatObj->SetNumberField(TEXT("valueType"), static_cast<int32>(Stat.ValueType));
		StatObj->SetNumberField(TEXT("numericValue"), Stat.NumericValue);
		StatObj->SetStringField(TEXT("textValue"), Stat.TextValue.ToString());
		StatObj->SetStringField(TEXT("unit"), Stat.Unit.ToString());
		StatArray.Add(MakeShared<FJsonValueObject>(StatObj));
	}

	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetArrayField(TEXT("stats"), StatArray);
	if (!ProviderName.IsNone())
	{
		Root->SetStringField(TEXT("provider"), ProviderName.ToString());
	}

	FString Output;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
	if (!FJsonSerializer::Serialize(Root, Writer))
	{
		return false;
	}

	const bool bSaved = FFileHelper::SaveStringToFile(Output, *AbsoluteFilePath);
	if (bSaved)
	{
		INC_DWORD_STAT(STAT_GIM_ExportsJson);
		GORGEOUS_CSV_CUSTOM_STAT_ACCUMULATE(InsightExportsJson, 1);
		GORGEOUS_TRACE_BOOKMARK(TEXT("InsightExportJSON %s"), *AbsoluteFilePath);
	}
	return bSaved;
}

bool UGorgeousInsightMatrixSubsystem::EnqueueScenario(FName ScenarioName, const FString& Parameters, int32 VariantIndex, UObject* WorldContextObject)
{
	if (ScenarioName.IsNone())
	{
		return false;
	}

	FGorgeousInsightQueuedScenario Entry;
	Entry.ScenarioName = ScenarioName;
	Entry.Parameters = Parameters;
	Entry.VariantIndex = VariantIndex;
	Entry.WorldContextObject = WorldContextObject;

	{
		FScopeLock QueueLock(&QueueMutex);
		ScenarioQueue.Add(MoveTemp(Entry));
	}

	return true;
}

FGorgeousInsightScenarioResult UGorgeousInsightMatrixSubsystem::RunScenarioByName(FName ScenarioName, const FString& Parameters, int32 VariantIndex, UObject* WorldContextObject)
{
	FGorgeousInsightScenarioResult Result;
	(void)WorldContextObject;

	if (ScenarioName.IsNone())
	{
		Result.bSuccess = false;
		Result.AddError(TEXT("Scenario name is empty"));
		return Result;
	}

	FGorgeousInsightScenarioDescriptor Descriptor;
	bool bFoundDescriptor = false;
	{
		const TArray<FGorgeousInsightScenarioDescriptor> Scenarios = FGorgeousInsightTestMatrix::GetRegisteredScenarios();
		for (const FGorgeousInsightScenarioDescriptor& Candidate : Scenarios)
		{
			if (Candidate.ScenarioName == ScenarioName)
			{
				Descriptor = Candidate;
				bFoundDescriptor = true;
				break;
			}
		}
	}

	if (!bFoundDescriptor)
	{
		Result.bSuccess = false;
		Result.AddError(FString::Printf(TEXT("Scenario '%s' not found"), *ScenarioName.ToString()));
		return Result;
	}

	FGorgeousInsightMatrixRequest Request = FGorgeousInsightMatrixRequest::FromParameters(Parameters);
	FGorgeousInsightScenarioResult ScenarioResult;

	if (Request.bEnableSharedHarness)
	{
		if (!FGorgeousInsightHarness::StartHarness(Request))
		{
			ScenarioResult.bSuccess = false;
			ScenarioResult.AddError(TEXT("Insight harness failed to start"));
			return ScenarioResult;
		}
	}

#if WITH_DEV_AUTOMATION_TESTS
	FGorgeousInsightRuntimeAutomationTest RuntimeTest;
	FGorgeousInsightScenarioContext ScenarioContext(Request, Parameters, VariantIndex, RuntimeTest, Descriptor, WorldContextObject);
	ScenarioResult = FGorgeousInsightTestMatrix::ExecuteScenario(Descriptor, ScenarioContext);
	ScenarioResult.AddNote(TEXT("Executed via Insight Matrix runtime"));
#else
	ScenarioResult.bSuccess = false;
	ScenarioResult.AddError(TEXT("Runtime execution requires WITH_DEV_AUTOMATION_TESTS"));
#endif

	if (Request.bEnableSharedHarness)
	{
		FGorgeousInsightHarness::StopHarness();
	}

	FGorgeousInsightHarness::SaveScenarioResult(Descriptor, ScenarioResult, TEXT("Runtime"));

	return ScenarioResult;
}

TArray<FGorgeousInsightScenarioResult> UGorgeousInsightMatrixSubsystem::RunQueuedScenarios()
{
	TArray<FGorgeousInsightQueuedScenario> ToRun;
	{
		FScopeLock QueueLock(&QueueMutex);
		ToRun = MoveTemp(ScenarioQueue);
		ScenarioQueue.Reset();
	}

	TArray<FGorgeousInsightScenarioResult> Results;
	Results.Reserve(ToRun.Num());
	for (const FGorgeousInsightQueuedScenario& Entry : ToRun)
	{
		Results.Add(RunScenarioByName(Entry.ScenarioName, Entry.Parameters, Entry.VariantIndex, Entry.WorldContextObject.Get()));
	}

	return Results;
}

bool UGorgeousInsightMatrixSubsystem::EnqueueTest(FName ProviderName, FName TestId, const FGorgeousInsightTestContext& Context)
{
	if (ProviderName.IsNone() || TestId.IsNone())
	{
		return false;
	}

	FGorgeousInsightQueuedTest Entry;
	Entry.ProviderName = ProviderName;
	Entry.TestId = TestId;
	Entry.Context = Context;

	{
		FScopeLock QueueLock(&QueueMutex);
		TestQueue.Add(MoveTemp(Entry));
	}

	return true;
}

TArray<UGorgeousInsightMatrixSubsystem::FGorgeousInsightQueuedTestResult> UGorgeousInsightMatrixSubsystem::RunQueuedTests()
{
	TArray<FGorgeousInsightQueuedTest> ToRun;
	{
		FScopeLock QueueLock(&QueueMutex);
		ToRun = MoveTemp(TestQueue);
		TestQueue.Reset();
	}

	TArray<FGorgeousInsightQueuedTestResult> Results;
	Results.Reserve(ToRun.Num());
	for (const FGorgeousInsightQueuedTest& Entry : ToRun)
	{
		FGorgeousInsightQueuedTestResult ResultEntry;
		ResultEntry.ProviderName = Entry.ProviderName;
		ResultEntry.TestId = Entry.TestId;
		ResultEntry.Result = RunTest(Entry.ProviderName, Entry.TestId, Entry.Context);
		Results.Add(MoveTemp(ResultEntry));
	}

	return Results;
}

int32 UGorgeousInsightMatrixSubsystem::GetQueuedTestCount() const
{
	FScopeLock QueueLock(&QueueMutex);
	return TestQueue.Num();
}

int32 UGorgeousInsightMatrixSubsystem::GetQueuedScenarioCount() const
{
	FScopeLock QueueLock(&QueueMutex);
	return ScenarioQueue.Num();
}

TArray<FGorgeousInsightScenarioRunResult> UGorgeousInsightMatrixSubsystem::RunMatrix(const FString& Parameters, UObject* WorldContextObject)
{
	const FGorgeousInsightMatrixRequest Request = FGorgeousInsightMatrixRequest::FromParameters(Parameters);
	TArray<FGorgeousInsightScenarioRunResult> Results;

	bool bHarnessStarted = false;
	if (Request.bEnableSharedHarness)
	{
		bHarnessStarted = FGorgeousInsightHarness::StartHarness(Request);
		if (!bHarnessStarted)
		{
			return Results;
		}
	}

#if WITH_DEV_AUTOMATION_TESTS
	FGorgeousInsightRuntimeAutomationTest RuntimeTest;
	Results = FGorgeousInsightTestMatrix::ExecuteMatrix(Request, Parameters, RuntimeTest, WorldContextObject);
#else
	(void)WorldContextObject;
#endif

	if (bHarnessStarted)
	{
		FGorgeousInsightHarness::StopHarness();
	}

	for (const FGorgeousInsightScenarioRunResult& Entry : Results)
	{
		FGorgeousInsightHarness::SaveScenarioResult(Entry.Descriptor, Entry.Result, TEXT("Runtime"));
	}

	return Results;
}

FString UGorgeousInsightMatrixSubsystem::GetInsightMatrixIniPath()
{
	// Get the path to the GorgeousCore plugin's Config folder
	FString PluginDir;
	if (const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("GorgeousCore")))
	{
		PluginDir = Plugin->GetBaseDir() / TEXT("Config");
	}
	else
	{
		PluginDir = FPaths::ProjectPluginsDir() / TEXT("GorgeousThings") / TEXT("GorgeousCore") / TEXT("Config");
	}
	IFileManager::Get().MakeDirectory(*PluginDir, true);
	return PluginDir / TEXT("BaseInsightMatrix.ini");
}

void UGorgeousInsightMatrixSubsystem::LoadPanelState()
{
	const FString IniPath = GetInsightMatrixIniPath();
	
	if (!FPaths::FileExists(IniPath))
	{
		return;
	}
	
	FConfigFile ConfigFile;
	ConfigFile.Read(IniPath);
	
	// Load Panel State
	const FString PanelSectionName = TEXT("InsightPanelState");
	if (const FConfigSection* Section = ConfigFile.FindSection(PanelSectionName))
	{
		// Load SelectedProvider
		if (const FConfigValue* Value = Section->Find(TEXT("SelectedProvider")))
		{
			CachedPanelState.SelectedProvider = FName(*Value->GetValue());
		}
		
		// Load ProviderFilter
		if (const FConfigValue* Value = Section->Find(TEXT("ProviderFilter")))
		{
			CachedPanelState.ProviderFilter = Value->GetValue();
		}
		
		// Load bRunWithHarness
		if (const FConfigValue* Value = Section->Find(TEXT("bRunWithHarness")))
		{
			CachedPanelState.bRunWithHarness = Value->GetValue().ToBool();
		}
		
		// Load StatWarningThresholds
		CachedPanelState.StatWarningThresholds.Reset();
		TArray<FConfigValue> WarningThresholds;
		Section->MultiFind(TEXT("StatWarningThreshold"), WarningThresholds);
		for (const FConfigValue& Entry : WarningThresholds)
		{
			FString KeyPart, ValuePart;
			if (Entry.GetValue().Split(TEXT("="), &KeyPart, &ValuePart))
			{
				CachedPanelState.StatWarningThresholds.Add(FName(*KeyPart), FCString::Atod(*ValuePart));
			}
		}
		
		// Load StatCriticalThresholds
		CachedPanelState.StatCriticalThresholds.Reset();
		TArray<FConfigValue> CriticalThresholds;
		Section->MultiFind(TEXT("StatCriticalThreshold"), CriticalThresholds);
		for (const FConfigValue& Entry : CriticalThresholds)
		{
			FString KeyPart, ValuePart;
			if (Entry.GetValue().Split(TEXT("="), &KeyPart, &ValuePart))
			{
				CachedPanelState.StatCriticalThresholds.Add(FName(*KeyPart), FCString::Atod(*ValuePart));
			}
		}
		
		// Load TestInputValues
		CachedPanelState.TestInputValues.Reset();
		TArray<FConfigValue> TestInputs;
		Section->MultiFind(TEXT("TestInputValue"), TestInputs);
		for (const FConfigValue& Entry : TestInputs)
		{
			// Format: ProviderName.TestId.InputId=Value
			FString Remainder, InputValue;
			if (Entry.GetValue().Split(TEXT("="), &Remainder, &InputValue))
			{
				FString ProviderPart, TestAndInputPart;
				if (Remainder.Split(TEXT("."), &ProviderPart, &TestAndInputPart))
				{
					FString TestPart, InputPart;
					if (TestAndInputPart.Split(TEXT("."), &TestPart, &InputPart))
					{
						const FName ProviderName(*ProviderPart);
						const FName TestInputKey(*FString::Printf(TEXT("%s.%s"), *TestPart, *InputPart));
						CachedPanelState.TestInputValues.FindOrAdd(ProviderName).Add(TestInputKey, InputValue);
					}
				}
			}
		}
		
		bHasCachedPanelState = true;
	}
	
	// Load Last Run Stats
	const FString LastRunSectionName = TEXT("LastRunStats");
	if (const FConfigSection* Section = ConfigFile.FindSection(LastRunSectionName))
	{
		if (const FConfigValue* Value = Section->Find(TEXT("bHasRun")))
		{
			LastRunStats.bHasRun = Value->GetValue().ToBool();
		}
		if (const FConfigValue* Value = Section->Find(TEXT("Timestamp")))
		{
			FDateTime::Parse(Value->GetValue(), LastRunStats.Timestamp);
		}
		if (const FConfigValue* Value = Section->Find(TEXT("ProviderName")))
		{
			LastRunStats.ProviderName = FName(*Value->GetValue());
		}
		if (const FConfigValue* Value = Section->Find(TEXT("TestId")))
		{
			LastRunStats.TestId = FName(*Value->GetValue());
		}
		if (const FConfigValue* Value = Section->Find(TEXT("DurationSeconds")))
		{
			LastRunStats.DurationSeconds = FCString::Atod(*Value->GetValue());
		}
		if (const FConfigValue* Value = Section->Find(TEXT("ErrorCount")))
		{
			LastRunStats.ErrorCount = FCString::Atoi(*Value->GetValue());
		}
		if (const FConfigValue* Value = Section->Find(TEXT("WarningCount")))
		{
			LastRunStats.WarningCount = FCString::Atoi(*Value->GetValue());
		}
		if (const FConfigValue* Value = Section->Find(TEXT("NoteCount")))
		{
			LastRunStats.NoteCount = FCString::Atoi(*Value->GetValue());
		}
		if (const FConfigValue* Value = Section->Find(TEXT("ObjectVariablesRegistered")))
		{
			LastRunStats.ObjectVariablesRegistered = FCString::Atoi(*Value->GetValue());
		}
		if (const FConfigValue* Value = Section->Find(TEXT("ObjectVariableOverheadMs")))
		{
			LastRunStats.ObjectVariableOverheadMs = FCString::Atod(*Value->GetValue());
		}
	}
	
	// Load Last Test Results
	LastTestResults.Reset();
	TArray<FString> SectionNames;
	ConfigFile.GetKeys(SectionNames);
	for (const FString& SectionName : SectionNames)
	{
		if (!SectionName.StartsWith(TEXT("TestResult.")))
		{
			continue;
		}
		
		// Format: TestResult.ProviderName.TestId
		FString Remainder = SectionName.Mid(11); // Skip "TestResult."
		FString ProviderPart, TestPart;
		if (!Remainder.Split(TEXT("."), &ProviderPart, &TestPart))
		{
			continue;
		}
		
		const FConfigSection* Section = ConfigFile.FindSection(SectionName);
		if (!Section)
		{
			continue;
		}
		
		FGorgeousInsightTestResult Result;
		
		if (const FConfigValue* Value = Section->Find(TEXT("bSuccess")))
		{
			Result.bSuccess = Value->GetValue().ToBool();
		}
		
		TArray<FConfigValue> Errors;
		Section->MultiFind(TEXT("Error"), Errors);
		for (const FConfigValue& Entry : Errors)
		{
			Result.Errors.Add(Entry.GetValue());
		}
		
		TArray<FConfigValue> Warnings;
		Section->MultiFind(TEXT("Warning"), Warnings);
		for (const FConfigValue& Entry : Warnings)
		{
			Result.Warnings.Add(Entry.GetValue());
		}
		
		TArray<FConfigValue> Notes;
		Section->MultiFind(TEXT("Note"), Notes);
		for (const FConfigValue& Entry : Notes)
		{
			Result.Notes.Add(Entry.GetValue());
		}
		
		TArray<FConfigValue> Metrics;
		Section->MultiFind(TEXT("Metric"), Metrics);
		for (const FConfigValue& Entry : Metrics)
		{
			FString KeyPart, ValuePart;
			if (Entry.GetValue().Split(TEXT("="), &KeyPart, &ValuePart))
			{
				Result.Metrics.Add(KeyPart, ValuePart);
			}
		}
		
		LastTestResults.FindOrAdd(FName(*ProviderPart)).Add(FName(*TestPart), Result);
	}
	
	// Load Last Action Stats
	LastActionStats.Reset();
	for (const FString& SectionName : SectionNames)
	{
		if (!SectionName.StartsWith(TEXT("ActionStats.")))
		{
			continue;
		}
		
		// Format: ActionStats.ProviderName.ActionId
		FString Remainder = SectionName.Mid(12); // Skip "ActionStats."
		FString ProviderPart, ActionPart;
		if (!Remainder.Split(TEXT("."), &ProviderPart, &ActionPart))
		{
			continue;
		}
		
		const FConfigSection* Section = ConfigFile.FindSection(SectionName);
		if (!Section)
		{
			continue;
		}
		
		FGorgeousInsightLastActionStats Stats;
		
		if (const FConfigValue* Value = Section->Find(TEXT("bHasRun")))
		{
			Stats.bHasRun = Value->GetValue().ToBool();
		}
		if (const FConfigValue* Value = Section->Find(TEXT("Timestamp")))
		{
			FDateTime::Parse(Value->GetValue(), Stats.Timestamp);
		}
		Stats.ProviderName = FName(*ProviderPart);
		Stats.ActionId = FName(*ActionPart);
		if (const FConfigValue* Value = Section->Find(TEXT("DurationSeconds")))
		{
			Stats.DurationSeconds = FCString::Atod(*Value->GetValue());
		}
		if (const FConfigValue* Value = Section->Find(TEXT("bSucceeded")))
		{
			Stats.bSucceeded = Value->GetValue().ToBool();
		}
		
		LastActionStats.FindOrAdd(FName(*ProviderPart)).Add(FName(*ActionPart), Stats);
	}
}

void UGorgeousInsightMatrixSubsystem::SavePanelState() const
{
	const FString IniPath = GetInsightMatrixIniPath();
	
	FConfigFile ConfigFile;
	if (FPaths::FileExists(IniPath))
	{
		ConfigFile.Read(IniPath);
	}
	
	// Save Panel State
	const FString PanelSectionName = TEXT("InsightPanelState");
	ConfigFile.Remove(PanelSectionName);
	
	if (bHasCachedPanelState)
	{
		// Save SelectedProvider
		ConfigFile.SetString(*PanelSectionName, TEXT("SelectedProvider"), *CachedPanelState.SelectedProvider.ToString());
		
		// Save ProviderFilter
		ConfigFile.SetString(*PanelSectionName, TEXT("ProviderFilter"), *CachedPanelState.ProviderFilter);
		
		// Save bRunWithHarness
		ConfigFile.SetString(*PanelSectionName, TEXT("bRunWithHarness"), CachedPanelState.bRunWithHarness ? TEXT("true") : TEXT("false"));
		
		// Save StatWarningThresholds
		for (const TPair<FName, double>& Pair : CachedPanelState.StatWarningThresholds)
		{
			ConfigFile.AddToSection(*PanelSectionName, TEXT("StatWarningThreshold"), FString::Printf(TEXT("%s=%f"), *Pair.Key.ToString(), Pair.Value));
		}
		
		// Save StatCriticalThresholds
		for (const TPair<FName, double>& Pair : CachedPanelState.StatCriticalThresholds)
		{
			ConfigFile.AddToSection(*PanelSectionName, TEXT("StatCriticalThreshold"), FString::Printf(TEXT("%s=%f"), *Pair.Key.ToString(), Pair.Value));
		}
		
		// Save TestInputValues
		for (const TPair<FName, TMap<FName, FString>>& ProviderPair : CachedPanelState.TestInputValues)
		{
			for (const TPair<FName, FString>& InputPair : ProviderPair.Value)
			{
				ConfigFile.AddToSection(*PanelSectionName, TEXT("TestInputValue"), FString::Printf(TEXT("%s.%s=%s"), 
					*ProviderPair.Key.ToString(), *InputPair.Key.ToString(), *InputPair.Value));
			}
		}
	}
	
	// Save Last Run Stats
	const FString LastRunSectionName = TEXT("LastRunStats");
	ConfigFile.Remove(LastRunSectionName);
	
	if (LastRunStats.bHasRun)
	{
		ConfigFile.SetString(*LastRunSectionName, TEXT("bHasRun"), TEXT("true"));
		ConfigFile.SetString(*LastRunSectionName, TEXT("Timestamp"), *LastRunStats.Timestamp.ToString());
		ConfigFile.SetString(*LastRunSectionName, TEXT("ProviderName"), *LastRunStats.ProviderName.ToString());
		ConfigFile.SetString(*LastRunSectionName, TEXT("TestId"), *LastRunStats.TestId.ToString());
		ConfigFile.SetString(*LastRunSectionName, TEXT("DurationSeconds"), *FString::Printf(TEXT("%f"), LastRunStats.DurationSeconds));
		ConfigFile.SetString(*LastRunSectionName, TEXT("ErrorCount"), *FString::Printf(TEXT("%d"), LastRunStats.ErrorCount));
		ConfigFile.SetString(*LastRunSectionName, TEXT("WarningCount"), *FString::Printf(TEXT("%d"), LastRunStats.WarningCount));
		ConfigFile.SetString(*LastRunSectionName, TEXT("NoteCount"), *FString::Printf(TEXT("%d"), LastRunStats.NoteCount));
		ConfigFile.SetString(*LastRunSectionName, TEXT("ObjectVariablesRegistered"), *FString::Printf(TEXT("%d"), LastRunStats.ObjectVariablesRegistered));
		ConfigFile.SetString(*LastRunSectionName, TEXT("ObjectVariableOverheadMs"), *FString::Printf(TEXT("%f"), LastRunStats.ObjectVariableOverheadMs));
	}
	
	// Clear old test result sections
	TArray<FString> SectionNames;
	ConfigFile.GetKeys(SectionNames);
	for (const FString& SectionName : SectionNames)
	{
		if (SectionName.StartsWith(TEXT("TestResult.")) || SectionName.StartsWith(TEXT("ActionStats.")))
		{
			ConfigFile.Remove(SectionName);
		}
	}
	
	// Save Last Test Results
	for (const TPair<FName, TMap<FName, FGorgeousInsightTestResult>>& ProviderPair : LastTestResults)
	{
		for (const TPair<FName, FGorgeousInsightTestResult>& TestPair : ProviderPair.Value)
		{
			const FString SectionName = FString::Printf(TEXT("TestResult.%s.%s"), *ProviderPair.Key.ToString(), *TestPair.Key.ToString());
			
			ConfigFile.SetString(*SectionName, TEXT("bSuccess"), TestPair.Value.bSuccess ? TEXT("true") : TEXT("false"));
			
			for (const FString& Error : TestPair.Value.Errors)
			{
				ConfigFile.AddToSection(*SectionName, TEXT("Error"), Error);
			}
			
			for (const FString& Warning : TestPair.Value.Warnings)
			{
				ConfigFile.AddToSection(*SectionName, TEXT("Warning"), Warning);
			}
			
			for (const FString& Note : TestPair.Value.Notes)
			{
				ConfigFile.AddToSection(*SectionName, TEXT("Note"), Note);
			}
			
			for (const TPair<FString, FString>& Metric : TestPair.Value.Metrics)
			{
				ConfigFile.AddToSection(*SectionName, TEXT("Metric"), FString::Printf(TEXT("%s=%s"), *Metric.Key, *Metric.Value));
			}
		}
	}
	
	// Save Last Action Stats
	for (const TPair<FName, TMap<FName, FGorgeousInsightLastActionStats>>& ProviderPair : LastActionStats)
	{
		for (const TPair<FName, FGorgeousInsightLastActionStats>& ActionPair : ProviderPair.Value)
		{
			const FString SectionName = FString::Printf(TEXT("ActionStats.%s.%s"), *ProviderPair.Key.ToString(), *ActionPair.Key.ToString());
			
			ConfigFile.SetString(*SectionName, TEXT("bHasRun"), ActionPair.Value.bHasRun ? TEXT("true") : TEXT("false"));
			ConfigFile.SetString(*SectionName, TEXT("Timestamp"), *ActionPair.Value.Timestamp.ToString());
			ConfigFile.SetString(*SectionName, TEXT("DurationSeconds"), *FString::Printf(TEXT("%f"), ActionPair.Value.DurationSeconds));
			ConfigFile.SetString(*SectionName, TEXT("bSucceeded"), ActionPair.Value.bSucceeded ? TEXT("true") : TEXT("false"));
		}
	}
	
	ConfigFile.Dirty = true;
	ConfigFile.Write(IniPath);
}
