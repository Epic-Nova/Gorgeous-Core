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
#include "NudgeHelper/SGorgeousNudgeCarousel.h"
#include "Helpers/GorgeousPluginHelper.h"
#include "LibraryWizard/GorgeousUpdateManager.h"
#include "NudgeHelper/GorgeousNudgeSubsystem.h"
#include "Editor.h"
#include "SWebBrowser.h"
#include "HAL/PlatformProcess.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/STextBlock.h"

//=============================================================================
// SGorgeousNudgeCarousel Implementation
//=============================================================================

void SGorgeousNudgeCarousel::Construct(const FArguments& InArgs)
{
	NudgeSubsystem = InArgs._NudgeSubsystem;

	TSharedRef<SUniformGridPanel> ProgressPanel = SNew(SUniformGridPanel).SlotPadding(FMargin(3.0f, 0.0f));
	PageSwitcher = SNew(SWidgetSwitcher);
	if (UGorgeousNudgeSubsystem* Subsystem = NudgeSubsystem.Get())
	{
		const TArray<FGorgeousNudgeEntry>& Entries = Subsystem->GetVisibleNudges();
		for (int32 Index = 0; Index < Entries.Num(); ++Index)
		{
			const FGorgeousNudgeEntry& Entry = Entries[Index];
			const FString HtmlPage = Entry.HtmlPage.IsEmpty()
				? FString::Printf(TEXT("<html><head><style>body{margin:0;min-height:100vh;background:radial-gradient(circle at 70%% 12%%,#0b3545 0,#041820 36%%,#020b10 100%%);color:#eaf8fc;font-family:Segoe UI,Arial,sans-serif;display:grid;place-items:center}.card{max-width:660px;padding:52px;border:1px solid rgba(49,171,211,.2);border-radius:22px;background:rgba(4,27,36,.83);box-shadow:0 28px 90px rgba(0,0,0,.45)}.eyebrow{color:#23a8d8;text-transform:uppercase;font-size:12px;letter-spacing:2px;font-weight:700}h1{font-size:42px;margin:14px 0;color:#f2fbff}p{font-size:18px;line-height:1.6;color:#9cc5d2}a{display:inline-block;margin-top:16px;padding:12px 20px;border-radius:10px;background:#147da5;color:#f4fcff;text-decoration:none;font-weight:700;box-shadow:0 8px 24px rgba(13,124,165,.3)}</style></head><body><main class='card'><div class='eyebrow'>Gorgeous Things</div><h1>%s</h1><p>%s</p></main></body></html>"), *Entry.Title.ToString(), *Entry.Description.ToString())
				: Entry.HtmlPage;
			TSharedPtr<SWebBrowser> PreviewBrowserPage;
			TSharedPtr<SWebBrowser> DestinationBrowserPage;
			TSharedPtr<SWidgetSwitcher> BrowserContentSwitcher = SNew(SWidgetSwitcher);
			BrowserContentSwitcher->AddSlot()
			[
				SAssignNew(PreviewBrowserPage, SWebBrowser)
				.ShowControls(false)
				.ShowAddressBar(false)
				.ContentsToLoad(HtmlPage)
			];
			BrowserContentSwitcher->AddSlot()
			[
				SAssignNew(DestinationBrowserPage, SWebBrowser)
				.ShowControls(false)
				.ShowAddressBar(false)
			];
			BrowserContentSwitcher->SetActiveWidgetIndex(0);
			PageSwitcher->AddSlot()
			[
				BrowserContentSwitcher.ToSharedRef()
			];
			BrowserPages.Add(PreviewBrowserPage);
			DestinationBrowserPages.Add(DestinationBrowserPage);
			BrowserContentSwitchers.Add(BrowserContentSwitcher);
			bViewingDestinationPage.Add(false);

			TSharedPtr<SButton> Dot;
			ProgressPanel->AddSlot(Index, 0)
			[
				SNew(SBox).WidthOverride(22.0f).HeightOverride(22.0f)
				[
					SAssignNew(Dot, SButton)
					.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
					.ButtonColorAndOpacity(FLinearColor(0.04f, 0.18f, 0.23f, 1.0f))
					.ContentPadding(0.0f)
					.OnClicked_Lambda([this, Index]() { SelectPage(Index); return FReply::Handled(); })
					[
						SNew(STextBlock).Justification(ETextJustify::Center).Text(FText::FromString(TEXT("●"))).ColorAndOpacity(FLinearColor(0.30f, 0.64f, 0.75f, 1.0f))
					]
				]
			];
			ProgressDots.Add(Dot);
		}
	}

	ChildSlot
	[
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.012f, 0.055f, 0.073f, 1.0f))
		.Padding(16.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(12.0f, 6.0f, 12.0f, 10.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("GorgeousNudges", "CarouselEyebrow", "GORGEOUS THINGS  /  ECOSYSTEM BRIEFING"))
						.ColorAndOpacity(FLinearColor(0.20f, 0.66f, 0.84f, 1.0f))
						.Font(FAppStyle::GetFontStyle("SmallFont"))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f, 0.0f, 0.0f)
					[
						SAssignNew(HeaderTitleText, STextBlock)
						.ColorAndOpacity(FLinearColor(0.72f, 0.87f, 0.92f, 1.0f))
						.Font(FAppStyle::GetFontStyle("NormalFont"))
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SAssignNew(PageCounterText, STextBlock)
					.ColorAndOpacity(FLinearColor(0.46f, 0.72f, 0.80f, 1.0f))
				]
			]
			+ SVerticalBox::Slot().FillHeight(1.0f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.035f, 0.14f, 0.18f, 1.0f))
				.Padding(1.0f)
				[
					PageSwitcher.ToSharedRef()
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 14.0f, 0.0f, 0.0f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.018f, 0.085f, 0.11f, 1.0f))
				.Padding(FMargin(8.0f, 6.0f))
				[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox).HeightOverride(22.0f)
					[
						SAssignNew(BrowserBackButton, SButton)
						.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
						.ButtonColorAndOpacity(FLinearColor(0.05f, 0.24f, 0.30f, 1.0f))
						.ContentPadding(FMargin(8.0f, 1.0f))
						.OnClicked(this, &SGorgeousNudgeCarousel::OnBrowserBack)
						[
							SNew(STextBlock)
							.Font(FAppStyle::GetFontStyle("SmallFont"))
							.Text(NSLOCTEXT("GorgeousNudges", "BrowserBack", "‹  Web back"))
							.ColorAndOpacity(FLinearColor(0.69f, 0.89f, 0.95f, 1.0f))
						]
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FLinearColor(0.025f, 0.15f, 0.19f, 1.0f))
					.Padding(FMargin(3.0f, 1.0f))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SBox).WidthOverride(22.0f).HeightOverride(22.0f)
							[
								SAssignNew(PreviousButton, SButton)
								.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
								.ContentPadding(0.0f)
								.OnClicked(this, &SGorgeousNudgeCarousel::OnPrevious)
								[
									SNew(STextBlock).Justification(ETextJustify::Center).Text(FText::FromString(TEXT("‹"))).Font(FAppStyle::GetFontStyle("SmallFont"))
								]
							]
						]
						+ SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SBox).WidthOverride(22.0f).HeightOverride(22.0f)
							[
								SAssignNew(NextButton, SButton)
								.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
								.ContentPadding(0.0f)
								.OnClicked(this, &SGorgeousNudgeCarousel::OnNext)
								[
									SNew(STextBlock).Justification(ETextJustify::Center).Text(FText::FromString(TEXT("›"))).Font(FAppStyle::GetFontStyle("SmallFont"))
								]
							]
						]
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Center)
				[
					ProgressPanel
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox).HeightOverride(22.0f)
					[
						SAssignNew(ActionButton, SButton)
						.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
						.ButtonColorAndOpacity(FLinearColor(0.06f, 0.48f, 0.63f, 1.0f))
						.ContentPadding(FMargin(9.0f, 1.0f))
						.OnClicked(this, &SGorgeousNudgeCarousel::OnAction)
						[
							SAssignNew(ActionButtonText, STextBlock)
							.Font(FAppStyle::GetFontStyle("SmallFont"))
							.Text(NSLOCTEXT("GorgeousNudges", "Action", "Open"))
						]
					]
				]
			]
			]
		]
	];
	Refresh();
}

FReply SGorgeousNudgeCarousel::OnPrevious()
{
	SelectPage(CurrentIndex - 1);
	return FReply::Handled();
}

FReply SGorgeousNudgeCarousel::OnBrowserBack()
{
	if (BrowserContentSwitchers.IsValidIndex(CurrentIndex) && BrowserContentSwitchers[CurrentIndex].IsValid() && bViewingDestinationPage.IsValidIndex(CurrentIndex) && bViewingDestinationPage[CurrentIndex])
	{
		BrowserContentSwitchers[CurrentIndex]->SetActiveWidgetIndex(0);
		bViewingDestinationPage[CurrentIndex] = false;
		Refresh();
	}
	return FReply::Handled();
}

FReply SGorgeousNudgeCarousel::OnNext()
{
	SelectPage(CurrentIndex + 1);
	return FReply::Handled();
}

FReply SGorgeousNudgeCarousel::OnAction()
{
	if (UGorgeousNudgeSubsystem* Subsystem = NudgeSubsystem.Get())
	{
		const TArray<FGorgeousNudgeEntry>& Entries = Subsystem->GetVisibleNudges();
		if (Entries.IsValidIndex(CurrentIndex))
		{
			const FGorgeousNudgeEntry& Entry = Entries[CurrentIndex];
			if (Entry.Scope == EGorgeousNudgeScope::PluginUpdate && Entry.bUsePluginDownloader && GEditor)
			{
				if (UGorgeousUpdateManager* UpdateManager = GEditor->GetEditorSubsystem<UGorgeousUpdateManager>())
				{
					if (UGorgeousPluginHelper* PluginHelper = UGorgeousPluginHelper::GetSingleton())
					{
						const FString DownloadToken = Entry.PluginDownloadToken.IsEmpty() ? PluginHelper->GetPluginUpdateDownloadToken(FName(*Entry.TargetId)) : Entry.PluginDownloadToken;
						UpdateManager->DownloadPluginUpdate(Entry.TargetId, DownloadToken);
					}
				}
			}
			else if (!Entry.ActionUrl.IsEmpty())
			{
				if (DestinationBrowserPages.IsValidIndex(CurrentIndex) && DestinationBrowserPages[CurrentIndex].IsValid() && BrowserContentSwitchers.IsValidIndex(CurrentIndex) && BrowserContentSwitchers[CurrentIndex].IsValid())
				{
					DestinationBrowserPages[CurrentIndex]->LoadURL(Entry.ActionUrl);
					BrowserContentSwitchers[CurrentIndex]->SetActiveWidgetIndex(1);
					bViewingDestinationPage[CurrentIndex] = true;
					Refresh();
				}
			}
		}
	}
	return FReply::Handled();
}

void SGorgeousNudgeCarousel::SelectPage(int32 PageIndex)
{
	const int32 PageCount = ProgressDots.Num();
	if (PageCount == 0)
	{
		return;
	}
	CurrentIndex = FMath::Clamp(PageIndex, 0, PageCount - 1);
	Refresh();
}

void SGorgeousNudgeCarousel::Refresh()
{
	const bool bHasPages = ProgressDots.Num() > 0;
	if (PageSwitcher.IsValid() && bHasPages)
	{
		PageSwitcher->SetActiveWidgetIndex(CurrentIndex);
	}
	if (PreviousButton.IsValid()) PreviousButton->SetEnabled(bHasPages && CurrentIndex > 0);
	if (NextButton.IsValid()) NextButton->SetEnabled(bHasPages && CurrentIndex + 1 < ProgressDots.Num());
	if (BrowserBackButton.IsValid())
	{
		BrowserBackButton->SetVisibility(bViewingDestinationPage.IsValidIndex(CurrentIndex) && bViewingDestinationPage[CurrentIndex] ? EVisibility::Visible : EVisibility::Collapsed);
	}
	const TArray<FGorgeousNudgeEntry>* Entries = NudgeSubsystem.IsValid() ? &NudgeSubsystem->GetVisibleNudges() : nullptr;
	if (Entries && Entries->IsValidIndex(CurrentIndex))
	{
		if (UGorgeousNudgeSubsystem* Subsystem = NudgeSubsystem.Get())
		{
			Subsystem->ResizeCarouselForEntry((*Entries)[CurrentIndex]);
		}
	}
	if (HeaderTitleText.IsValid())
	{
		HeaderTitleText->SetText(Entries && Entries->IsValidIndex(CurrentIndex)
			? FText::Format(NSLOCTEXT("GorgeousNudges", "NowViewing", "Now viewing: {0}"), (*Entries)[CurrentIndex].Title)
			: NSLOCTEXT("GorgeousNudges", "NoItems", "You're all caught up."));
	}
	if (PageCounterText.IsValid())
	{
		PageCounterText->SetText(bHasPages ? FText::Format(NSLOCTEXT("GorgeousNudges", "PageCounter", "{0} / {1}"), FText::AsNumber(CurrentIndex + 1), FText::AsNumber(ProgressDots.Num())) : FText::GetEmpty());
	}
	for (int32 Index = 0; Index < ProgressDots.Num(); ++Index)
	{
		if (ProgressDots[Index].IsValid())
		{
			ProgressDots[Index]->SetBorderBackgroundColor(Index == CurrentIndex ? FLinearColor(0.10f, 0.70f, 0.93f, 1.0f) : FLinearColor(0.04f, 0.18f, 0.23f, 1.0f));
		}
	}
	if (ActionButton.IsValid())
	{
		ActionButton->SetVisibility(bHasPages ? EVisibility::Visible : EVisibility::Collapsed);
		if (Entries && Entries->IsValidIndex(CurrentIndex) && !(*Entries)[CurrentIndex].ActionLabel.IsEmpty())
		{
			if (ActionButtonText.IsValid())
			{
				ActionButtonText->SetText(FText::FromString((*Entries)[CurrentIndex].ActionLabel));
			}
		}
	}
}
