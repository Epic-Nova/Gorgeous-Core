// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|                    Gorgeous Core - RPC Inspector Window                   |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#include "AutoReplication/Slate/SGorgeousRPCInspectorWindow.h"

#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SNullWidget.h"
#include "Styling/CoreStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/AppStyle.h"
#include "Fonts/SlateFontInfo.h"
#include "Fonts/CompositeFont.h"
#include "GorgeousCoreRuntimeMinimal.h"
#include "Styling/SlateStyleRegistry.h"

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  Helpers
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

namespace GorgeousRPCInspector_Private
{
	/**
	 * Returns an FSlateFontInfo that can render Unicode emoji (⌛ U+231B, ⏳ U+23F3, ❌ U+274C …).
	 *
	 * Returns an FSlateFontInfo pointing at the NotoColorEmoji font that is bundled
	 * inside the plugin's Resources folder and registered under the key
	 * "GorgeousCore.EmojiFont" in the "GorgeousCoreEditorStyle" Slate style set.
	 * Falls back to the engine default font if the style set is not yet loaded.
	 */
	static FSlateFontInfo GetEmojiFont(const int32 Size)
	{
		// Build a composite font once and cache it:
		//   - Default typeface: engine Roboto, handles all ASCII / Latin characters so that
		//     text blocks containing regular text alongside emoji don't trigger
		//     "Could not find Glyph" warnings (NotoColorEmoji has no Latin glyphs).
		//   - Sub-typeface: NotoColorEmoji bundled in the plugin, covers the common emoji
		//     Unicode blocks so that 💤 ✅ ⌛ etc. are rendered correctly on all platforms.
		static TSharedPtr<const FCompositeFont> CachedComposite;

		if (!CachedComposite.IsValid())
		{
			TSharedRef<FCompositeFont> Built = MakeShared<FCompositeFont>();

			// Default face: Roboto Regular (covers full Latin / ASCII range).
			{
				FTypefaceEntry& DefaultEntry = Built->DefaultTypeface.Fonts.AddDefaulted_GetRef();
				DefaultEntry.Name = TEXT("Regular");
				DefaultEntry.Font = FFontData(
					FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"),
					EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
			}

			// Emoji sub-face: NotoColorEmoji from the plugin's Resources folder.
			if (const ISlateStyle* GorgeousStyle = FSlateStyleRegistry::FindSlateStyle(TEXT("GorgeousCoreEditorStyle")))
			{
				const FSlateFontInfo EmojiInfo = GorgeousStyle->GetFontStyle(TEXT("GorgeousCore.EmojiFont"));
				if (const FCompositeFont* Src = EmojiInfo.GetCompositeFont())
				{
					if (Src->DefaultTypeface.Fonts.Num() > 0)
					{
						FCompositeSubFont EmojiSub;
						EmojiSub.Typeface.Fonts = Src->DefaultTypeface.Fonts;
						// Miscellaneous Technical (U+2300–U+23FF)
						EmojiSub.CharacterRanges.Add(TRange<int32>(
							TRangeBound<int32>::Inclusive(0x2300),
							TRangeBound<int32>::Inclusive(0x23FF)));
						// Miscellaneous Symbols + Dingbats (U+2600–U+27BF)
						EmojiSub.CharacterRanges.Add(TRange<int32>(
							TRangeBound<int32>::Inclusive(0x2600),
							TRangeBound<int32>::Inclusive(0x27BF)));
						// Full emoji blocks 💤🔴🎯 (U+1F000–U+1FFFF)
						EmojiSub.CharacterRanges.Add(TRange<int32>(
							TRangeBound<int32>::Inclusive(0x1F000),
							TRangeBound<int32>::Inclusive(0x1FFFF)));
						Built->SubTypefaces.Add(EmojiSub);
					}
				}
			}

			CachedComposite = Built;
		}

		if (CachedComposite.IsValid())
		{
			return FSlateFontInfo(TSharedPtr<const FCompositeFont>(CachedComposite), static_cast<float>(Size));
		}
		return FCoreStyle::GetDefaultFontStyle("Regular", Size);
	}
	/** Small tinted pill chip. */
	static TSharedRef<SWidget> MakeChip(
		const FString& Text, const FLinearColor& BgColor,
		const FLinearColor& FgColor = FLinearColor::White)
	{
		return SNew(SBorder)
			.Padding(FMargin(7.f, 2.f))
			.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
			.BorderBackgroundColor(BgColor)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Text))
				.Font(GorgeousRPCInspector_Private::GetEmojiFont(9))
				.ColorAndOpacity(FgColor)
			];
	}

	/** Section header bar with a coloured left accent stripe. */
	static TSharedRef<SWidget> MakeSectionHeader(
		const FString& Title, const FLinearColor& AccentColor)
	{
		return SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.07f, 0.9f))
			.Padding(0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBorder)
					.Padding(0.f)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(AccentColor)
					[
						SNew(SBox)
						.WidthOverride(3.f)
						.HeightOverride(22.f)
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(8.f, 4.f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Title))
					.Font(FAppStyle::GetFontStyle("BoldFont"))
					.ColorAndOpacity(FSlateColor(AccentColor))
				]
			];
	}
} // namespace GorgeousRPCInspector_Private

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  Static state
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

TWeakPtr<SWindow> SGorgeousRPCInspectorWindow::WindowWeak;

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  Open / Close / IsOpen
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

void SGorgeousRPCInspectorWindow::Open()
{
	if (TSharedPtr<SWindow> Existing = WindowWeak.Pin())
	{
		Existing->BringToFront();
		return;
	}

	FGorgeousRPCDebugTracker::SetEnabled(true);

	TSharedRef<SGorgeousRPCInspectorWindow> Content = SNew(SGorgeousRPCInspectorWindow);

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(FText::FromString(TEXT("RPC Inspector")))
		.ClientSize(FVector2D(1350, 720))
		.SupportsMinimize(true)
		.SupportsMaximize(true)
		[
			Content
		];

	WindowWeak = Window;
	FSlateApplication::Get().AddWindow(Window);
}

void SGorgeousRPCInspectorWindow::Close()
{
	if (TSharedPtr<SWindow> Window = WindowWeak.Pin())
	{
		Window->RequestDestroyWindow();
	}
	WindowWeak.Reset();
	FGorgeousRPCDebugTracker::SetEnabled(false);
}

bool SGorgeousRPCInspectorWindow::IsOpen()
{
	return WindowWeak.IsValid();
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  SGorgeousRPCResponseTimeline  ---  change detection
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

bool SGorgeousRPCResponseTimeline::HasVisiblyChanged(
	const FGorgeousRPCDebugEntry& Old, const FGorgeousRPCDebugEntry& New)
{
	if (!Old.RequestGuid.IsValid())
	{
		return true;
	}
	if (Old.RequestGuid != New.RequestGuid)
	{
		return true;
	}
	if (Old.Responses.Num() != New.Responses.Num())
	{
		return true;
	}
	if (Old.Status != New.Status)
	{
		return true;
	}
	if (Old.ExpectedResponseCount != New.ExpectedResponseCount)
	{
		return true;
	}

	for (int32 i = 0; i < Old.Responses.Num(); ++i)
	{
		if (Old.Responses[i].ReadyState != New.Responses[i].ReadyState)
		{
			return true;
		}
		if (Old.Responses[i].bHasReturnValue != New.Responses[i].bHasReturnValue)
		{
			return true;
		}
		if (Old.Responses[i].ReturnValuePreview != New.Responses[i].ReturnValuePreview)
		{
			return true;
		}
		// Label text and node colour both depend on ResponderDisplayName and bIsServer
		if (Old.Responses[i].ResponderDisplayName != New.Responses[i].ResponderDisplayName)
		{
			return true;
		}
		if (Old.Responses[i].bIsServer != New.Responses[i].bIsServer)
		{
			return true;
		}
		if (Old.Responses[i].Phase != New.Responses[i].Phase)
		{
			return true;
		}
	}
	return false;
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  SGorgeousRPCResponseTimeline  ---  Construct / Clear / UpdateEntry
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

void SGorgeousRPCResponseTimeline::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor(0.028f, 0.028f, 0.035f, 1.f))
		.Padding(0.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.055f, 0.035f, 0.115f, 1.f))
				.Padding(FMargin(12.f, 8.f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("RESPONSE TIMELINE")))
					.Font(FAppStyle::GetFontStyle("BoldFont"))
					.ColorAndOpacity(FLinearColor(0.62f, 0.47f, 0.97f, 1.f))
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Select an RPC from the list.")))
				.Font(FAppStyle::GetFontStyle("SmallFont"))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
		]
	];
}

void SGorgeousRPCResponseTimeline::Clear()
{
	bHasEntry = false;
	CachedEntry = FGorgeousRPCDebugEntry{};
	Construct(FArguments());
}

void SGorgeousRPCResponseTimeline::UpdateEntry(const FGorgeousRPCDebugEntry& NewEntry)
{
	if (!HasVisiblyChanged(CachedEntry, NewEntry))
	{
		return;
	}
	CachedEntry = NewEntry;
	bHasEntry = true;
	RebuildContent();
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  SGorgeousRPCResponseTimeline  ---  chip / badge factories
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

TSharedRef<SWidget> SGorgeousRPCResponseTimeline::MakeLatencyChip(double LatencyMs) const
{
	if (LatencyMs <= 0.0)
	{
		return SNullWidget::NullWidget;
	}

	FLinearColor ChipColor;
	if (LatencyMs < 50.0)
	{
		ChipColor = FLinearColor(0.15f, 0.50f, 0.20f, 1.f); // fast  green
	}
	else if (LatencyMs < 150.0)
	{
		ChipColor = FLinearColor(0.55f, 0.42f, 0.05f, 1.f); // mid   amber
	}
	else
	{
		ChipColor = FLinearColor(0.55f, 0.15f, 0.15f, 1.f); // slow  red
	}

	return GorgeousRPCInspector_Private::MakeChip(
		FString::Printf(TEXT("%.0f ms"), LatencyMs), ChipColor);
}

TSharedRef<SWidget> SGorgeousRPCResponseTimeline::MakeReadyStateBadge(
	EGorgeousRPCReadyState ReadyState, bool bIsPending) const
{
	if (bIsPending || ReadyState == EGorgeousRPCReadyState::Ready)
	{
		return SNullWidget::NullWidget;
	}
	if (ReadyState == EGorgeousRPCReadyState::NotReadyToCollect)
	{
		return GorgeousRPCInspector_Private::MakeChip(
			TEXT("deferred"), FLinearColor(0.45f, 0.25f, 0.0f, 1.f));
	}
	// ReadyForSingleResponderCallback
	return GorgeousRPCInspector_Private::MakeChip(
		TEXT("> cb pending"),
		FLinearColor(0.10f, 0.28f, 0.52f, 1.f),
		FLinearColor(0.40f, 0.75f, 1.0f, 1.f));
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  SGorgeousRPCResponseTimeline  ---  single node row
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

TSharedRef<SWidget> SGorgeousRPCResponseTimeline::MakeNodeRow(
	const FString& TimeLabel,
	bool bIsFirst,
	bool bIsLast,
	bool bIsPending,
	bool bIsServer,
	const FString& ResponderLabel,
	double LatencyMs,
	bool bHasReturnValue,
	const FString& ReturnValuePreview,
	EGorgeousRPCReadyState ReadyState) const
{
	const FLinearColor NodeColor = bIsFirst
		                               ? FLinearColor(0.50f, 0.70f, 1.00f, 1.f)
		                               : bIsPending
		                               ? FLinearColor(0.30f, 0.30f, 0.35f, 1.f)
		                               : bIsServer
		                               ? FLinearColor(0.30f, 0.60f, 0.95f, 1.f)
		                               : FLinearColor(0.90f, 0.55f, 0.20f, 1.f);

	const FLinearColor SpineColor = FLinearColor(0.18f, 0.18f, 0.24f, 1.f);

	const EVisibility SpineTopVis = bIsFirst ? EVisibility::Hidden : EVisibility::Visible;
	const EVisibility SpineBottomVis = bIsLast ? EVisibility::Hidden : EVisibility::Visible;

	// pre-build optional widgets so the ternary stays typed correctly
	const TSharedRef<SWidget> LatencyWidget =
		(bIsFirst || bIsPending)
			? TSharedRef<SWidget>(SNullWidget::NullWidget)
			: MakeLatencyChip(LatencyMs);

	const TSharedRef<SWidget> BadgeWidget = MakeReadyStateBadge(ReadyState, bIsPending);

	// A preview has real data only when it contains a '{' JSON block AND at least one
	// property value is non-empty. A block where every entry is "Key": "" means the
	// OV was captured before the values were written (deferred / empty state).
	auto HasNonEmptyPropertyValues = [](const FString& Preview) -> bool
	{
		const int32 BraceIdx = Preview.Find(TEXT("{"));
		if (BraceIdx == INDEX_NONE)
		{
			return false;
		}
		const FString Body   = Preview.Mid(BraceIdx);
		const FString Prefix = TEXT("\": \"");
		int32 SearchPos = 0;
		while (true)
		{
			const int32 Found = Body.Find(Prefix, ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchPos);
			if (Found == INDEX_NONE)
			{
				break;
			}
			const int32 ValueStart = Found + Prefix.Len();
			// A non-empty value has a character other than '"' right after the opening quote.
			if (ValueStart < Body.Len() && Body[ValueStart] != TCHAR('"'))
			{
				return true;
			}
			SearchPos = Found + 1;
		}
		return false;
	};
	const bool bPreviewHasData = bHasReturnValue && HasNonEmptyPropertyValues(ReturnValuePreview);
	const TSharedRef<SWidget> ReturnWidget = bPreviewHasData
		                                         ? TSharedRef<SWidget>(SNew(STextBlock)
		                                         .Text(FText::FromString(
			                                         FString::Printf(TEXT("= %s"), *ReturnValuePreview)))
		                                         .Font(GorgeousRPCInspector_Private::GetEmojiFont(9))
		                                         .ColorAndOpacity(FLinearColor(0.30f, 0.85f, 0.45f, 1.f))
		                                         .AutoWrapText(true)
		                                         .WrapTextAt(260.f))
		                                         : bHasReturnValue
		                                         ? TSharedRef<SWidget>(SNew(STextBlock)
		                                         .Text(FText::FromString(TEXT("Awaiting Result...")))
		                                         .Font(GorgeousRPCInspector_Private::GetEmojiFont(9))
		                                         .ColorAndOpacity(FLinearColor(0.45f, 0.45f, 0.50f, 1.f)))
		                                         : TSharedRef<SWidget>(SNullWidget::NullWidget);

	const FString PrefixGlyph =
		bIsFirst
			? TEXT("> ") // initiation node
			: bIsServer
			? TEXT("* ") // server response
			: TEXT("");
	const FLinearColor PrefixColor = bIsFirst
		                                 ? FLinearColor(0.5f, 0.7f, 1.0f, 1.f)
		                                 : FLinearColor(0.3f, 0.6f, 0.95f, 1.f);

	return SNew(SHorizontalBox)

			// ── Time column ───────────────────────────────────────────────
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(FMargin(10.f, 0.f, 6.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(72.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TimeLabel))
					.Font(GorgeousRPCInspector_Private::GetEmojiFont(9))
					.ColorAndOpacity(bIsPending
						? FLinearColor(0.30f, 0.30f, 0.35f, 1.f)
						: FLinearColor(0.60f, 0.60f, 0.70f, 1.f))
					.Justification(ETextJustify::Right)
				]
			]

			// ── Spine + node column ───────────────────────────────────────
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(28.f)
				[
					SNew(SOverlay)
					// top spine half
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Top)
					[
						SNew(SBox)
						.WidthOverride(2.f)
						.HeightOverride(22.f)
						.Visibility(SpineTopVis)
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
							.BorderBackgroundColor(SpineColor)
							.Padding(0.f)
						]
					]
					// bottom spine half
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Bottom)
					[
						SNew(SBox)
						.WidthOverride(2.f)
						.HeightOverride(22.f)
						.Visibility(SpineBottomVis)
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
							.BorderBackgroundColor(SpineColor)
							.Padding(0.f)
						]
					]
					// Node glow halo, SBorder-based circle avoids missing-glyph issues
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(16.f)
						.HeightOverride(16.f)
						.Visibility(bIsPending ? EVisibility::Hidden : EVisibility::Visible)
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
							.BorderBackgroundColor(NodeColor * FLinearColor(1.f, 1.f, 1.f, 0.18f))
							.Padding(0.f)
						]
					]
					// Node dot (filled for received, ring for pending)
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(bIsPending ? 8.f : 10.f)
						.HeightOverride(bIsPending ? 8.f : 10.f)
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
							.BorderBackgroundColor(bIsPending
								? FLinearColor(0.f, 0.f, 0.f, 0.f) // hollow
								: NodeColor)
							.Padding(0.f)
						]
					]
				]
			]

			// ── Content column ────────────────────────────────────────────
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			.Padding(FMargin(6.f, 7.f, 8.f, 7.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					// prefix ▶ / ★
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, (bIsFirst || bIsServer) ? 3.f : 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(PrefixGlyph))
						.Font(FAppStyle::GetFontStyle("SmallFont"))
						.ColorAndOpacity(PrefixColor)
						.Visibility((bIsFirst || bIsServer) ? EVisibility::Visible : EVisibility::Collapsed)
					]
					// responder label
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(ResponderLabel))
						.Font(FAppStyle::GetFontStyle("SmallFont"))
						.ColorAndOpacity(bIsPending
							                 ? FLinearColor(0.38f, 0.38f, 0.43f, 1.f)
							                 : FLinearColor::White)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					// latency chip
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(FMargin(4.f, 0.f, 0.f, 0.f))
					[
						LatencyWidget
					]
					// ready-state badge
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(FMargin(4.f, 0.f, 0.f, 0.f))
					[
						BadgeWidget
					]
				]
				// return value preview row
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 2.f, 0.f, 0.f)
				[
					SNew(SBox)
					.Visibility(bHasReturnValue ? EVisibility::Visible : EVisibility::Collapsed)
					[
						ReturnWidget
					]
				]
			];
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  SGorgeousRPCResponseTimeline  ---  RebuildContent
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

void SGorgeousRPCResponseTimeline::RebuildContent()
{
	const FGorgeousRPCDebugEntry& E = CachedEntry;

	// Status badge text + colour
	FString StatusText;
	FLinearColor StatusColor;
	switch (E.Status)
	{
	case EGorgeousRPCDebugStatus::Complete:
		StatusText = TEXT("Complete");
		StatusColor = FLinearColor(0.30f, 0.85f, 0.45f, 1.f);
		break;
	case EGorgeousRPCDebugStatus::Failed:
		StatusText = TEXT("Failed");
		StatusColor = FLinearColor(0.90f, 0.30f, 0.30f, 1.f);
		break;
	case EGorgeousRPCDebugStatus::TimedOut:
		StatusText = TEXT("Timed Out");
		StatusColor = FLinearColor(0.70f, 0.40f, 0.40f, 1.f);
		break;
	case EGorgeousRPCDebugStatus::PartialSuccess:
		StatusText = FString::Printf(TEXT("Partial %d/%d"),
			E.GetReceivedResponseCount(),
			E.ExpectedResponseCount > 0 ? E.ExpectedResponseCount : -1);
		StatusColor = FLinearColor(0.90f, 0.65f, 0.20f, 1.f);
		break;
	default: // Pending
		StatusText = (E.ExpectedResponseCount > 0)
			             ? FString::Printf(TEXT("%d / %d"),
			                                E.GetReceivedResponseCount(), E.ExpectedResponseCount)
			             : FString::Printf(TEXT("%d / ?"), E.GetReceivedResponseCount());
		StatusColor = FLinearColor(0.90f, 0.70f, 0.20f, 1.f);
		break;
	}

	// How many pending placeholder slots to show beyond received responses
	const int32 PendingExtras =
	(E.Status == EGorgeousRPCDebugStatus::Pending &&
		E.ExpectedResponseCount > E.Responses.Num())
		? (E.ExpectedResponseCount - E.Responses.Num())
		: 0;

	const int32 TotalNodes = 1 + E.Responses.Num() + PendingExtras; // 1 = initiation node

	// Build node rows
	TSharedRef<SVerticalBox> NodeContainer = SNew(SVerticalBox);

	// Initiation node
	{
		const bool bLast = (TotalNodes == 1);
		const FString InitLabel = E.InitiatorDescription.IsEmpty()
			                          ? FString(TEXT("RPC Created"))
			                          : E.InitiatorDescription;
		NodeContainer->AddSlot().AutoHeight()
		[
			MakeNodeRow(TEXT("T+0ms"), true, bLast, false, false,
			            InitLabel, 0.0, false, TEXT(""), EGorgeousRPCReadyState::Ready)
		];
	}

	// Received responses
	for (int32 i = 0; i < E.Responses.Num(); ++i)
	{
		const FGorgeousRPCResponseSnapshot& Resp = E.Responses[i];
		const bool bLast = (i == E.Responses.Num() - 1) && (PendingExtras == 0);
		const double LatMs = Resp.LatencyMs;
		const FString TimeLabel = (LatMs > 0.0)
			                          ? FString::Printf(TEXT("T+%.0fms"), LatMs)
			                          : TEXT("T+?ms");

		// Build "MyRPC · ResponderName [← Initiator] · Phase" label.
		// For server-executed responses on server-bound RPC types, append the initiating
		// actor name so the full call-chain is visible without needing to look at the
		// T+0ms initiation node separately.
		const bool bIsServerBoundRPC =
			E.Type == EGorgeousAutoReplicationRPCType::EReliableServer ||
			E.Type == EGorgeousAutoReplicationRPCType::EUnreliableServer;
		const FString ResponderPart =
			(bIsServerBoundRPC && Resp.bIsServer && !E.InitiatorDescription.IsEmpty())
			? FString::Printf(TEXT("%s \u2190 %s"), *Resp.ResponderDisplayName, *E.InitiatorDescription)
			: Resp.ResponderDisplayName;
		const FString BaseLabel = FString::Printf(TEXT("%s \u00b7 %s"),
			*E.Key.ToString(), *ResponderPart);
		const FString NodeLabel = Resp.Phase.IsEmpty()
			? BaseLabel
			: FString::Printf(TEXT("%s \u00b7 %s"), *BaseLabel, *Resp.Phase);

		NodeContainer->AddSlot().AutoHeight()
		[
			MakeNodeRow(TimeLabel, false, bLast, false, Resp.bIsServer,
			            NodeLabel, LatMs,
			            Resp.bHasReturnValue, Resp.ReturnValuePreview, Resp.ReadyState)
		];
	}

	// Pending placeholders (if count is known)
	for (int32 i = 0; i < PendingExtras; ++i)
	{
		const bool bLast = (i == PendingExtras - 1);
		NodeContainer->AddSlot().AutoHeight()
		[
			MakeNodeRow(TEXT("???"), false, bLast, true, false,
			            TEXT("Awaiting response..."), 0.0, false, TEXT(""),
			            EGorgeousRPCReadyState::NotReadyToCollect)
		];
	}

	// Footer text
	const FString FooterText = (E.ExpectedResponseCount > 0)
		                           ? FString::Printf(TEXT("Expected: %d  \u2022  Received: %d"),
		                                             E.ExpectedResponseCount, E.GetReceivedResponseCount())
		                           : FString::Printf(TEXT("Received: %d  (broadcast)"),
		                                             E.GetReceivedResponseCount());

	// Compose the full widget
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor(0.028f, 0.028f, 0.035f, 1.f))
		.Padding(0.f)
		[
			SNew(SVerticalBox)

			// --- Header bar ---
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.055f, 0.035f, 0.115f, 1.f))
				.Padding(FMargin(12.f, 8.f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("RESPONSE TIMELINE")))
						.Font(FAppStyle::GetFontStyle("BoldFont"))
						.ColorAndOpacity(FLinearColor(0.62f, 0.47f, 0.97f, 1.f))
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
						.BorderBackgroundColor(StatusColor * FLinearColor(0.22f, 0.22f, 0.22f, 1.f))
						.Padding(FMargin(9.f, 3.f))
						[
							SNew(STextBlock)
							.Text(FText::FromString(StatusText))
							.Font(GorgeousRPCInspector_Private::GetEmojiFont(9))
							.ColorAndOpacity(StatusColor)
						]
					]
				]
			]

			// --- Status accent line (1-pixel color band keyed to RPC status) ---
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(StatusColor * FLinearColor(0.45f, 0.45f, 0.45f, 1.f))
				.Padding(0.f)
				[
					SNew(SBox).HeightOverride(2.f)
				]
			]
			// --- GUID sub-bar ---
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.038f, 0.038f, 0.055f, 1.f))
				.Padding(FMargin(12.f, 4.f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(
						FString::Printf(TEXT("GUID: %s"), *E.RequestGuid.ToString(EGuidFormats::Short))))
					.Font(GorgeousRPCInspector_Private::GetEmojiFont(8))
					.ColorAndOpacity(FLinearColor(0.30f, 0.30f, 0.38f, 1.f))
				]
			]

			// --- Timeline nodes (scrollable) ---
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				.Padding(FMargin(0.f, 10.f, 0.f, 10.f))
				[
					NodeContainer
				]
			]

			// --- Footer bar ---
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.035f, 0.035f, 0.050f, 1.f))
				.Padding(FMargin(12.f, 6.f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(FooterText))
					.Font(GorgeousRPCInspector_Private::GetEmojiFont(9))
					.ColorAndOpacity(FLinearColor(0.45f, 0.45f, 0.52f, 1.f))
				]
			]
		]
	];
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  SGorgeousRPCInspectorWindow  ---  Construct
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

void SGorgeousRPCInspectorWindow::Construct(const FArguments& InArgs)
{
	// Type-filter dropdown options
	TypeFilterOptions.Add(MakeShared<FString>(TEXT("All")));
	TypeFilterOptions.Add(MakeShared<FString>(TEXT("Server")));
	TypeFilterOptions.Add(MakeShared<FString>(TEXT("Client")));
	TypeFilterOptions.Add(MakeShared<FString>(TEXT("Multicast")));
	CurrentTypeFilter = TypeFilterOptions[0];

	// Persistent left-details panel
	LeftDetailsBox = SNew(SVerticalBox);

	// Persistent timeline widget (survives list refreshes)
	TimelineWidget = SNew(SGorgeousRPCResponseTimeline);

	ChildSlot
	[
		SNew(SVerticalBox)

		// ======== Toolbar ================================================
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FLinearColor(0.04f, 0.04f, 0.04f, 0.95f))
			.Padding(FMargin(8.f, 6.f))
			[
				SNew(SHorizontalBox)

				// Live/Pause toggle
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.f, 0.f, 8.f, 0.f)
				[
					SNew(SButton)
					.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton"))
					.ContentPadding(FMargin(10.f, 4.f))
					.OnClicked(this, &SGorgeousRPCInspectorWindow::OnLivePauseToggled)
					[
						SAssignNew(LivePauseLabel, STextBlock)
						.Text(FText::FromString(TEXT("\u25CF Live")))
						.Font(FAppStyle::GetFontStyle("BoldFont"))
						.ColorAndOpacity(FLinearColor(0.3f, 0.9f, 0.4f, 1.f))
					]
				]

				// Filter text box
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				.Padding(4.f, 0.f)
				[
					SNew(SEditableTextBox)
					.HintText(FText::FromString(TEXT("Filter by key, handler, or initiator...")))
					.OnTextChanged_Lambda([this](const FText& NewText)
					{
						FilterText = NewText.ToString();
						RefreshList();
					})
				]

				// "Type:" label
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(8.f, 0.f, 4.f, 0.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Type:")))
					.Font(FAppStyle::GetFontStyle("SmallFont"))
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				]

				// Type-filter dropdown
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.f, 0.f, 8.f, 0.f)
				[
					SAssignNew(TypeFilterCombo, SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&TypeFilterOptions)
					.InitiallySelectedItem(CurrentTypeFilter)
					.ContentPadding(FMargin(8.f, 3.f))
					.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Selected, ESelectInfo::Type)
					{
						if (!Selected.IsValid())
						{
							return;
						}
						CurrentTypeFilter = Selected;
						if (*Selected == TEXT("Server"))
						{
							TypeFilterIndex = 1;
						}
						else if (*Selected == TEXT("Client"))
						{
							TypeFilterIndex = 2;
						}
						else if (*Selected == TEXT("Multicast"))
						{
							TypeFilterIndex = 3;
						}
						else
						{
							TypeFilterIndex = 0;
						}
						RefreshList();
					})
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item) -> TSharedRef<SWidget>
					{
						return SNew(STextBlock)
							.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("All")))
							.Font(FAppStyle::GetFontStyle("SmallFont"))
							.Margin(FMargin(6.f, 2.f));
					})
					[
						SNew(STextBlock)
						.Font(FAppStyle::GetFontStyle("SmallFont"))
						.Text_Lambda([this]() -> FText
						{
							return FText::FromString(
								CurrentTypeFilter.IsValid() ? *CurrentTypeFilter : TEXT("All"));
						})
					]
				]

				// Clear button
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton"))
					.ContentPadding(FMargin(10.f, 4.f))
					.OnClicked(this, &SGorgeousRPCInspectorWindow::OnClearClicked)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Clear")))
						.Font(FAppStyle::GetFontStyle("SmallFont"))
					]
				]
			]
		]

		// ======== Three-pane content =====================================
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)

			// -- Pane 1: RPC feed list ------------------------------------
			+ SSplitter::Slot()
			.Value(0.36f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor(FLinearColor(0.03f, 0.03f, 0.03f, 0.95f))
				.Padding(2)
				[
					SAssignNew(ListView, SListView<TSharedPtr<FRPCListItem>>)
					.ListItemsSource(&FilteredRPCItems)
					.OnGenerateRow(this, &SGorgeousRPCInspectorWindow::OnGenerateRow)
					.OnSelectionChanged(this, &SGorgeousRPCInspectorWindow::OnSelectionChanged)
					.SelectionMode(ESelectionMode::Single)
				]
			]

			// -- Pane 2: Request info + arguments -------------------------
			+ SSplitter::Slot()
			.Value(0.31f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor(FLinearColor(0.03f, 0.03f, 0.03f, 0.95f))
				.Padding(12)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						LeftDetailsBox.ToSharedRef()
					]
				]
			]

			// -- Pane 3: Response timeline widget -------------------------
			+ SSplitter::Slot()
			.Value(0.33f)
			[
				TimelineWidget.ToSharedRef()
			]
		]
	];

	FilteredRPCItems.Empty();
	ClearDetailPanels();
	RefreshList();
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  Tick  ---  live feed, timeline-only live updates
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

void SGorgeousRPCInspectorWindow::Tick(
	const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	FGorgeousRPCDebugTracker::Get().SweepTimedOut(30.0);

	if (!bIsLive)
	{
		return;
	}

	// Only update the currently selected entry's details/timeline in live mode
	// DO NOT refresh the entire list - that destroys UI interactivity
	if ((InCurrentTime - LastRefreshTime) >= RefreshIntervalSec)
	{
		LastRefreshTime = InCurrentTime;

		// Check ALL tracker entries for new ones (increased from 10 to 1000 for immediate display)
		const TArray<FGorgeousRPCDebugEntry> AllEntries =
			FGorgeousRPCDebugTracker::Get().GetRecentRPCs(1000);
		bool bAddedNewItems = false;
		
		for (const FGorgeousRPCDebugEntry& Entry : AllEntries)
		{
			if (!PassesFilter(Entry))
			{
				continue;
			}
			// Check if this entry already exists
			bool bExists = false;
			for (const TSharedPtr<FRPCListItem>& Item : RPCItems)
			{
				if (Item->Entry.RequestGuid == Entry.RequestGuid)
				{
					// Update existing entry data in-place
					Item->Entry = Entry;
					bExists = true;
					break;
				}
			}
			if (!bExists)
			{
				// Add new entry to the top
				TSharedPtr<FRPCListItem> NewItem = MakeShared<FRPCListItem>();
				NewItem->Entry = Entry;
				RPCItems.Insert(NewItem, 0);
				
				// Check if this new multicast should be grouped with existing entries
				const bool bIsMulticast =
					Entry.Type == EGorgeousAutoReplicationRPCType::EReliableMulticast ||
					Entry.Type == EGorgeousAutoReplicationRPCType::EUnreliableMulticast;
				if (bIsMulticast)
				{
					// Check all existing items for potential grouping
					for (int32 i = 1; i < RPCItems.Num(); ++i)
					{
						TSharedPtr<FRPCListItem>& ExistingItem = RPCItems[i];
						if (ExistingItem->bIsMulticastChild)
						{
							continue;
						}
						const bool bExistingIsMulticast =
							ExistingItem->Entry.Type == EGorgeousAutoReplicationRPCType::EReliableMulticast ||
							ExistingItem->Entry.Type == EGorgeousAutoReplicationRPCType::EUnreliableMulticast;
						if (!bExistingIsMulticast)
						{
							continue;
						}
						if (AreMulticastsRelated(Entry, ExistingItem->Entry))
						{
							// Determine parent based on which arrived first (earlier timestamp)
							if (Entry.InitiatedTimeSec <= ExistingItem->Entry.InitiatedTimeSec)
							{
								// New entry is older or same time - make it parent, existing becomes child
								NewItem->ChildGuids.Add(ExistingItem->Entry.RequestGuid);
								ExistingItem->bIsMulticastChild = true;
								ExistingItem->ParentGuid = Entry.RequestGuid;
							}
							else
							{
								// Existing entry is older - make it parent, new becomes child
								NewItem->bIsMulticastChild = true;
								NewItem->ParentGuid = ExistingItem->Entry.RequestGuid;
								ExistingItem->ChildGuids.Add(Entry.RequestGuid);
							}
							break;
						}
					}
				}
				
				bAddedNewItems = true;
			}
		}
		
		// If we added items, rebuild the filtered list and refresh the view
		if (bAddedNewItems)
		{
			FilteredRPCItems.Empty();
			for (const TSharedPtr<FRPCListItem>& Item : RPCItems)
			{
				if (!Item->bIsMulticastChild)
				{
					FilteredRPCItems.Add(Item);
				}
			}
			
			if (ListView.IsValid())
			{
				ListView->RequestListRefresh();
			}
		}
	}

	// Always update selected item's details and timeline (lightweight)
	if (SelectedItem.IsValid())
	{
		if (const FGorgeousRPCDebugEntry* LiveEntry =
			FGorgeousRPCDebugTracker::Get().FindByGuid(SelectedItem->Entry.RequestGuid))
		{
			// Update the cached entry data
			SelectedItem->Entry = *LiveEntry;

			// Build the display entry (combining grouped multicast responses if needed)
			FGorgeousRPCDebugEntry DisplayEntry = *LiveEntry;
			if (!SelectedItem->bIsMulticastChild && SelectedItem->ChildGuids.Num() > 0)
			{
				DisplayEntry.Responses = GetCombinedResponses(SelectedItem);
			}

			// Always push the latest entry to both panels so labels, colors,
			// ready-state badges and return-value previews update instantly.
			TimelineWidget->UpdateEntry(DisplayEntry);
			RebuildLeftDetails(DisplayEntry);
		}
	}
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  Filter
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

bool SGorgeousRPCInspectorWindow::PassesFilter(const FGorgeousRPCDebugEntry& Entry) const
{
	if (TypeFilterIndex == 1)
	{
		if (Entry.Type != EGorgeousAutoReplicationRPCType::EReliableServer &&
			Entry.Type != EGorgeousAutoReplicationRPCType::EUnreliableServer)
		{
			return false;
		}
	}
	else if (TypeFilterIndex == 2)
	{
		if (Entry.Type != EGorgeousAutoReplicationRPCType::EReliableClient &&
			Entry.Type != EGorgeousAutoReplicationRPCType::EUnreliableClient)
		{
			return false;
		}
	}
	else if (TypeFilterIndex == 3)
	{
		if (Entry.Type != EGorgeousAutoReplicationRPCType::EReliableMulticast &&
			Entry.Type != EGorgeousAutoReplicationRPCType::EUnreliableMulticast)
		{
			return false;
		}
	}

	if (!FilterText.IsEmpty())
	{
		const bool bMatchKey = Entry.Key.ToString().Contains(FilterText);
		const bool bMatchHandler = Entry.HandlerName.ToString().Contains(FilterText);
		const bool bMatchInitiator = Entry.InitiatorDescription.Contains(FilterText);
		if (!bMatchKey && !bMatchHandler && !bMatchInitiator)
		{
			return false;
		}
	}

	return true;
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  List population
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

void SGorgeousRPCInspectorWindow::RefreshList()
{
	const TArray<FGorgeousRPCDebugEntry> AllEntries =
		FGorgeousRPCDebugTracker::Get().GetRecentRPCs(500);

	const FGuid SelectedGuid =
		SelectedItem.IsValid() ? SelectedItem->Entry.RequestGuid : FGuid();

	// Build a map of current entries for fast lookup
	TMap<FGuid, FGorgeousRPCDebugEntry> EntryMap;
	for (const FGorgeousRPCDebugEntry& Entry : AllEntries)
	{
		if (PassesFilter(Entry))
		{
			EntryMap.Add(Entry.RequestGuid, Entry);
		}
	}

	// Incremental update: update existing items, remove stale ones, add new ones
	bool bListStructureChanged = false;
	TSharedPtr<FRPCListItem> NewSelected;

	// Update existing items and mark stale ones for removal
	for (int32 i = RPCItems.Num() - 1; i >= 0; --i)
	{
		TSharedPtr<FRPCListItem>& Item = RPCItems[i];
		if (const FGorgeousRPCDebugEntry* UpdatedEntry = EntryMap.Find(Item->Entry.RequestGuid))
		{
			// Update the cached entry data in-place
			Item->Entry = *UpdatedEntry;
			EntryMap.Remove(Item->Entry.RequestGuid);
			if (Item->Entry.RequestGuid == SelectedGuid)
			{
				NewSelected = Item;
			}
		}
		else
		{
			// Entry no longer exists in tracker - remove it
			RPCItems.RemoveAt(i);
			bListStructureChanged = true;
		}
	}

	// Add new entries that weren't in RPCItems yet
	for (const auto& Pair : EntryMap)
	{
		TSharedPtr<FRPCListItem> Item = MakeShared<FRPCListItem>();
		Item->Entry = Pair.Value;
		RPCItems.Insert(Item, 0); // Insert at top (most recent)
		bListStructureChanged = true;

		if (Item->Entry.RequestGuid == SelectedGuid)
		{
			NewSelected = Item;
		}
	}

	// Group related multicasts
	for (int32 i = 0; i < RPCItems.Num(); ++i)
	{
		TSharedPtr<FRPCListItem>& ItemA = RPCItems[i];
		if (ItemA->bIsMulticastChild)
		{
			continue; // Already grouped
		}
		const bool bIsMulticast =
			ItemA->Entry.Type == EGorgeousAutoReplicationRPCType::EReliableMulticast ||
			ItemA->Entry.Type == EGorgeousAutoReplicationRPCType::EUnreliableMulticast;
		if (!bIsMulticast)
		{
			continue;
		}
		// Check later entries for related multicasts
		ItemA->ChildGuids.Reset();
		for (int32 j = i + 1; j < RPCItems.Num(); ++j)
		{
			TSharedPtr<FRPCListItem>& ItemB = RPCItems[j];
			if (ItemB->bIsMulticastChild)
			{
				continue;
			}
			if (AreMulticastsRelated(ItemA->Entry, ItemB->Entry))
			{
				// Mark B as child of A
				ItemB->bIsMulticastChild = true;
				ItemB->ParentGuid = ItemA->Entry.RequestGuid;
				ItemA->ChildGuids.Add(ItemB->Entry.RequestGuid);
			}
		}
	}

	// Build filtered list (exclude multicast children - they're hidden, only parent shows)
	FilteredRPCItems.Empty();
	for (const TSharedPtr<FRPCListItem>& Item : RPCItems)
	{
		if (!Item->bIsMulticastChild)
		{
			FilteredRPCItems.Add(Item);
		}
	}

	if (ListView.IsValid())
	{
		// Only request full refresh if items were added/removed
		// For data-only updates, the existing widgets will pick up changes on next frame
		if (bListStructureChanged)
		{
			ListView->RequestListRefresh();
		}
		if (NewSelected.IsValid())
		{
			ListView->SetSelection(NewSelected);
		}
	}
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  Row generation
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

TSharedRef<ITableRow> SGorgeousRPCInspectorWindow::OnGenerateRow(
	TSharedPtr<FRPCListItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const FGorgeousRPCDebugEntry& Entry = Item->Entry;

	const bool bReliable =
		Entry.Type == EGorgeousAutoReplicationRPCType::EReliableServer
		|| Entry.Type == EGorgeousAutoReplicationRPCType::EReliableClient
		|| Entry.Type == EGorgeousAutoReplicationRPCType::EReliableMulticast;

	// Lambda to calculate aggregated status dynamically
	auto GetDisplayStatus = [Item]() -> TPair<FString, FLinearColor>
	{
		FGorgeousRPCDebugEntry DisplayEntry = Item->Entry;
		
		// Aggregate status from children if this is a parent
		if (!Item->bIsMulticastChild && Item->ChildGuids.Num() > 0)
		{
			int32 TotalResponses = Item->Entry.GetReceivedResponseCount();
			int32 TotalExpected = Item->Entry.ExpectedResponseCount;
			bool bAllComplete = (Item->Entry.Status == EGorgeousRPCDebugStatus::Complete);
			bool bAnyFailed = (Item->Entry.Status == EGorgeousRPCDebugStatus::Failed);
			bool bAnyTimedOut = (Item->Entry.Status == EGorgeousRPCDebugStatus::TimedOut);
			
			for (const FGuid& ChildGuid : Item->ChildGuids)
			{
				if (const FGorgeousRPCDebugEntry* ChildEntry = FGorgeousRPCDebugTracker::Get().FindByGuid(ChildGuid))
				{
					TotalResponses += ChildEntry->GetReceivedResponseCount();
					if (ChildEntry->ExpectedResponseCount > 0)
					{
						TotalExpected += ChildEntry->ExpectedResponseCount;
					}
					bAllComplete = bAllComplete && (ChildEntry->Status == EGorgeousRPCDebugStatus::Complete);
					bAnyFailed = bAnyFailed || (ChildEntry->Status == EGorgeousRPCDebugStatus::Failed);
					bAnyTimedOut = bAnyTimedOut || (ChildEntry->Status == EGorgeousRPCDebugStatus::TimedOut);
				}
			}
			
			if (bAnyFailed)
			{
				DisplayEntry.Status = EGorgeousRPCDebugStatus::Failed;
			}
			else if (bAnyTimedOut)
			{
				DisplayEntry.Status = EGorgeousRPCDebugStatus::TimedOut;
			}
			else if (bAllComplete)
			{
				DisplayEntry.Status = EGorgeousRPCDebugStatus::Complete;
			}
			// Store aggregated counts - add empty responses to match count
			DisplayEntry.Responses.Empty();
			for (int32 i = 0; i < TotalResponses; ++i)
			{
				DisplayEntry.Responses.AddDefaulted();
			}
			if (TotalExpected > 0)
			{
				DisplayEntry.ExpectedResponseCount = TotalExpected;
			}
		}
		
		FString StatusLabel;
		FLinearColor StatusColor;
		switch (DisplayEntry.Status)
		{
		case EGorgeousRPCDebugStatus::Pending:
			StatusColor = FLinearColor(0.9f, 0.7f, 0.2f, 1.f);
			StatusLabel = (DisplayEntry.ExpectedResponseCount > 0)
				? FString::Printf(TEXT("%d/%d"), DisplayEntry.Responses.Num(), DisplayEntry.ExpectedResponseCount)
				: FString::Printf(TEXT("%d/?"), DisplayEntry.Responses.Num());
			break;
		case EGorgeousRPCDebugStatus::Complete:
			StatusColor = FLinearColor(0.30f, 0.85f, 0.45f, 1.f);
			StatusLabel = TEXT("Done");
			break;
		case EGorgeousRPCDebugStatus::Failed:
			StatusColor = FLinearColor(0.90f, 0.30f, 0.30f, 1.f);
			StatusLabel = TEXT("Failed");
			break;
		case EGorgeousRPCDebugStatus::PartialSuccess:
			StatusColor = FLinearColor(0.90f, 0.65f, 0.20f, 1.f);
			StatusLabel = FString::Printf(TEXT("Partial %d/%d"),
				DisplayEntry.Responses.Num(),
				DisplayEntry.ExpectedResponseCount > 0 ? DisplayEntry.ExpectedResponseCount : -1);
			break;
		default: // TimedOut
			StatusColor = FLinearColor(0.70f, 0.40f, 0.40f, 1.f);
			StatusLabel = TEXT("Timeout");
			break;
		}
		return TPair<FString, FLinearColor>(StatusLabel, StatusColor);
	};

	return SNew(STableRow<TSharedPtr<FRPCListItem>>, OwnerTable)
		[
			SNew(SBorder)
			.Padding(FMargin(4.f))
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor_Lambda([this, Item]()
			{
				const bool bSel = ListView.IsValid() &&
					Item.IsValid() &&
					ListView->IsItemSelected(Item);
				return bSel
					       ? FLinearColor(0.10f, 0.45f, 0.85f, 0.35f)
					       : FLinearColor(0.06f, 0.06f, 0.06f, 0.35f);
			})
			[
				SNew(SHorizontalBox)
				// Accent bar
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBorder)
					.Padding(0.f)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor_Lambda([GetDisplayStatus]()
					{
						return GetDisplayStatus().Value;
					})
					[
						SNew(SBox)
						.WidthOverride(4.f)
						.HeightOverride(48.f)
					]
				]
				// Content
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(FMargin(4.f, 2.f, 8.f, 2.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(Entry.DirectionLabel))
							.Font(FAppStyle::GetFontStyle("BoldFont"))
							.ColorAndOpacity_Lambda([GetDisplayStatus]()
							{
								return GetDisplayStatus().Value;
							})
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(Entry.Key.ToString()))
							.Font(FAppStyle::GetFontStyle("BoldFont"))
						]
						+ SHorizontalBox::Slot().AutoWidth()
						[
							SNew(STextBlock)
							.Text(FText::FromString(
								FString::Printf(TEXT("::%s()"), *Entry.HandlerName.ToString())))
							.Font(FAppStyle::GetFontStyle("SmallFont"))
							.ColorAndOpacity(FSlateColor::UseSubduedForeground())
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)
						[
							GorgeousRPCInspector_Private::MakeChip(
								bReliable ? TEXT("Reliable") : TEXT("Unreliable"),
								bReliable
									? FLinearColor(0.3f, 0.6f, 0.9f, 1.f)
									: FLinearColor(0.6f, 0.6f, 0.3f, 1.f))
						]
						+ SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SBorder)
							.Padding(FMargin(7.f, 2.f))
							.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
							.BorderBackgroundColor_Lambda([GetDisplayStatus]()
							{
								return GetDisplayStatus().Value;
							})
							[
								SNew(STextBlock)
								.Text_Lambda([GetDisplayStatus]()
								{
									return FText::FromString(GetDisplayStatus().Key);
								})
								.Font(GorgeousRPCInspector_Private::GetEmojiFont(9))
								.ColorAndOpacity(FLinearColor::White)
							]
						]
					]
				]
			]
		];
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  Multicast grouping helpers
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

bool SGorgeousRPCInspectorWindow::AreMulticastsRelated(
	const FGorgeousRPCDebugEntry& A, const FGorgeousRPCDebugEntry& B) const
{
	// Must be same type
	if (A.Type != B.Type)
	{
		return false;
	}

	// Must both be multicast
	const bool bBothMulticast =
		(A.Type == EGorgeousAutoReplicationRPCType::EReliableMulticast ||
		 A.Type == EGorgeousAutoReplicationRPCType::EUnreliableMulticast) &&
		(B.Type == EGorgeousAutoReplicationRPCType::EReliableMulticast ||
		 B.Type == EGorgeousAutoReplicationRPCType::EUnreliableMulticast);
	if (!bBothMulticast)
	{
		return false;
	}

	// Must have same Key and HandlerName
	if (A.Key != B.Key || A.HandlerName != B.HandlerName)
	{
		return false;
	}

	// Must be initiated within 1 second of each other
	const double TimeDelta = FMath::Abs(A.InitiatedTimeSec - B.InitiatedTimeSec);
	return TimeDelta <= 1.0;
}

TArray<FGorgeousRPCResponseSnapshot> SGorgeousRPCInspectorWindow::GetCombinedResponses(
	const FGorgeousRPCDebugEntry& Entry) const
{
	TArray<FGorgeousRPCResponseSnapshot> Combined = Entry.Responses;

	// If this is a parent with children, merge all child responses
	const TSharedPtr<FRPCListItem>* Item = RPCItems.FindByPredicate(
		[&Entry](const TSharedPtr<FRPCListItem>& I) {
			return I.IsValid() && I->Entry.RequestGuid == Entry.RequestGuid;
		});

	if (Item && Item->IsValid() && Item->Get()->ChildGuids.Num() > 0)
	{
		FGorgeousRPCDebugTracker& Tracker = FGorgeousRPCDebugTracker::Get();
		for (const FGuid& ChildGuid : Item->Get()->ChildGuids)
		{
			const FGorgeousRPCDebugEntry* ChildEntry = Tracker.FindByGuid(ChildGuid);
			if (ChildEntry)
			{
				Combined.Append(ChildEntry->Responses);
			}
		}
	}

	return Combined;
}

TArray<FGorgeousRPCResponseSnapshot> SGorgeousRPCInspectorWindow::GetCombinedResponses(
	const TSharedPtr<FRPCListItem>& Item) const
{
	if (!Item.IsValid())
	{
		return {};
	}

	TArray<FGorgeousRPCResponseSnapshot> Combined = Item->Entry.Responses;

	// If this is a parent with children, merge all child responses
	if (Item->ChildGuids.Num() > 0)
	{
		FGorgeousRPCDebugTracker& Tracker = FGorgeousRPCDebugTracker::Get();
		for (const FGuid& ChildGuid : Item->ChildGuids)
		{
			const FGorgeousRPCDebugEntry* ChildEntry = Tracker.FindByGuid(ChildGuid);
			if (ChildEntry)
			{
				Combined.Append(ChildEntry->Responses);
			}
		}
	}

	return Combined;
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  Selection change  ---  GUID-guarded to prevent spurious left-panel rebuilds
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

void SGorgeousRPCInspectorWindow::OnSelectionChanged(
	TSharedPtr<FRPCListItem> Item, ESelectInfo::Type SelectInfo)
{
	// ESelectInfo::Direct is only fired by programmatic ListView->SetSelection() calls
	// (e.g. from RefreshList during live ticks). User mouse/keyboard events arrive as
	// OnMouseClick / OnKeyPress. Skip the full panel rebuild for Direct events, the
	// Tick path handles live timeline updates separately.
	if (SelectInfo == ESelectInfo::Direct)
	{
		SelectedItem = Item;
		return;
	}

	const FGuid NewGuid = Item.IsValid() ? Item->Entry.RequestGuid : FGuid();
	const FGuid OldGuid = SelectedItem.IsValid() ? SelectedItem->Entry.RequestGuid : FGuid();

	SelectedItem = Item;

	if (NewGuid != OldGuid)
	{
		if (Item.IsValid())
		{
			// Prefer the live-tracker copy (freshest responses)
			const FGorgeousRPCDebugEntry* Live =
				FGorgeousRPCDebugTracker::Get().FindByGuid(NewGuid);
			FGorgeousRPCDebugEntry Entry = Live ? *Live : Item->Entry;

			// For grouped multicasts, combine responses from all related dispatches
			if (!Item->bIsMulticastChild && Item->ChildGuids.Num() > 0)
			{
				Entry.Responses = GetCombinedResponses(Item);
			}

			RebuildLeftDetails(Entry);
			TimelineWidget->UpdateEntry(Entry);
			LastBuiltDetailGuid = NewGuid;
		}
		else
		{
			ClearDetailPanels();
			LastBuiltDetailGuid = FGuid();
		}
	}
	// Same GUID  =>  list was refreshed in Tick with the same selection restored.
	// Skip the left-panel rebuild, the Tick will push an UpdateEntry() to the
	// timeline which dirty-checks itself.
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  Left details helpers
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

void SGorgeousRPCInspectorWindow::AddDetailRow(
	const FString& Label, const FString& Value, const FLinearColor& ValueColor)
{
	LeftDetailsBox->AddSlot()
	              .AutoHeight()
	              .Padding(FMargin(12.f, 6.f, 4.f, 6.f))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(0.f, 0.f, 12.f, 0.f)
		[
			SNew(SBox)
			.WidthOverride(130.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Label))
				.Font(FAppStyle::GetFontStyle("SmallFont"))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Value))
			.Font(FAppStyle::GetFontStyle("SmallFont"))
			.ColorAndOpacity(FSlateColor(ValueColor))
			.AutoWrapText(true)
		]
	];
}

void SGorgeousRPCInspectorWindow::AddDetailSectionHeader(
	const FString& Title, const FLinearColor& AccentColor)
{
	LeftDetailsBox->AddSlot()
	              .AutoHeight()
	              .Padding(0.f, 16.f, 0.f, 6.f)
	[
		GorgeousRPCInspector_Private::MakeSectionHeader(Title, AccentColor)
	];
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  RebuildLeftDetails  ---  called only on genuine selection changes
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

void SGorgeousRPCInspectorWindow::RebuildLeftDetails(const FGorgeousRPCDebugEntry& Entry)
{
	LeftDetailsBox->ClearChildren();

	// Check if this entry is part of a grouped multicast
	const TSharedPtr<FRPCListItem>* ParentItemPtr = RPCItems.FindByPredicate(
		[&Entry](const TSharedPtr<FRPCListItem>& Item) {
			return Item.IsValid() && Item->Entry.RequestGuid == Entry.RequestGuid && Item->ChildGuids.Num() > 0;
		});

	if (ParentItemPtr && ParentItemPtr->IsValid())
	{
		// This is a grouped multicast - show multiple dispatch sections
		const TSharedPtr<FRPCListItem>& ParentItem = *ParentItemPtr;
		TArray<FGorgeousRPCDebugEntry> AllDispatches;
		AllDispatches.Add(ParentItem->Entry);

		// Collect all child entries
		for (const FGuid& ChildGuid : ParentItem->ChildGuids)
		{
			if (const FGorgeousRPCDebugEntry* ChildEntry = FGorgeousRPCDebugTracker::Get().FindByGuid(ChildGuid))
			{
				AllDispatches.Add(*ChildEntry);
			}
		}

		// Show each dispatch as a numbered section
		for (int32 DispatchIdx = 0; DispatchIdx < AllDispatches.Num(); ++DispatchIdx)
		{
			const FGorgeousRPCDebugEntry& DispatchEntry = AllDispatches[DispatchIdx];
			
			// Add dispatch header
			const FString DispatchTitle = FString::Printf(TEXT("Dispatch %d (GUID: %s)"), 
				DispatchIdx + 1, 
				*DispatchEntry.RequestGuid.ToString().Left(8));
			AddDetailSectionHeader(DispatchTitle, FLinearColor(0.20f, 0.60f, 0.90f, 1.f));

			AddDetailRow(TEXT("Request GUID"), DispatchEntry.RequestGuid.ToString());
			AddDetailRow(TEXT("Key"), DispatchEntry.Key.ToString());
			AddDetailRow(TEXT("Handler"), DispatchEntry.HandlerName.ToString());
			AddDetailRow(TEXT("Direction"), DispatchEntry.DirectionLabel);
			AddDetailRow(TEXT("Initiator"),
				DispatchEntry.InitiatorDescription.IsEmpty() ? TEXT("Unknown") : DispatchEntry.InitiatorDescription);

			const UEnum* TypeEnum = StaticEnum<EGorgeousAutoReplicationRPCType>();
			if (TypeEnum)
			{
				AddDetailRow(TEXT("Type"),
					TypeEnum->GetNameStringByValue(static_cast<int64>(DispatchEntry.Type)));
			}

			FString StatusStr;
			FLinearColor StatusColor = FLinearColor::White;
			switch (DispatchEntry.Status)
			{
			case EGorgeousRPCDebugStatus::Pending:
				StatusStr = TEXT("Pending");
				StatusColor = FLinearColor(0.90f, 0.70f, 0.20f, 1.f);
				break;
			case EGorgeousRPCDebugStatus::Complete:
				StatusStr = TEXT("Complete");
				StatusColor = FLinearColor(0.30f, 0.85f, 0.45f, 1.f);
				break;
			case EGorgeousRPCDebugStatus::Failed:
				StatusStr = TEXT("Failed");
				StatusColor = FLinearColor(0.90f, 0.30f, 0.30f, 1.f);
				break;
			case EGorgeousRPCDebugStatus::PartialSuccess:
				StatusStr = TEXT("Partial Success");
				StatusColor = FLinearColor(0.90f, 0.65f, 0.20f, 1.f);
				break;
			default:
				StatusStr = TEXT("Timed Out");
				StatusColor = FLinearColor(0.70f, 0.40f, 0.40f, 1.f);
				break;
			}
			AddDetailRow(TEXT("Status"), StatusStr, StatusColor);

			if (DispatchEntry.CompletedTimeSec > 0.0 && DispatchEntry.InitiatedTimeSec > 0.0)
			{
				const double TotalMs = (DispatchEntry.CompletedTimeSec - DispatchEntry.InitiatedTimeSec) * 1000.0;
				AddDetailRow(TEXT("Duration"), FString::Printf(TEXT("%.2f ms"), TotalMs));
			}

			AddDetailRow(TEXT("Expected"),
				DispatchEntry.ExpectedResponseCount >= 0
					? FString::Printf(TEXT("%d response(s)"), DispatchEntry.ExpectedResponseCount)
					: TEXT("Unknown (broadcast)"));
			
			AddDetailRow(TEXT("Received"),
				FString::Printf(TEXT("%d response(s)"), DispatchEntry.GetReceivedResponseCount()));
		}

		// Show arguments once (same for all dispatches)
		AddDetailSectionHeader(TEXT("Arguments"), FLinearColor(0.85f, 0.55f, 0.20f, 1.f));
		if (Entry.ArgumentNames.Num() == 0)
		{
			AddDetailRow(TEXT("(none)"), TEXT("No arguments captured"));
		}
		else
		{
			for (int32 i = 0; i < Entry.ArgumentNames.Num(); ++i)
			{
				const FString& ArgName = Entry.ArgumentNames[i];
				AddDetailRow(ArgName.IsEmpty() ? FString::Printf(TEXT("Arg[%d]"), i) : ArgName, TEXT("(captured)"));
			}
		}
		return;
	}

	// Not grouped - show single dispatch info as before
	// ---- Request Info ---------------------------------------------------
	AddDetailSectionHeader(TEXT("Request Info"), FLinearColor(0.20f, 0.60f, 0.90f, 1.f));
	AddDetailRow(TEXT("Request GUID"), Entry.RequestGuid.ToString());
	AddDetailRow(TEXT("Key"), Entry.Key.ToString());
	AddDetailRow(TEXT("Handler"), Entry.HandlerName.ToString());
	AddDetailRow(TEXT("Direction"), Entry.DirectionLabel);
	AddDetailRow(TEXT("Initiator"),
	             Entry.InitiatorDescription.IsEmpty() ? TEXT("Unknown") : Entry.InitiatorDescription);

	const UEnum* TypeEnum = StaticEnum<EGorgeousAutoReplicationRPCType>();
	if (TypeEnum)
	{
		AddDetailRow(TEXT("Type"),
		             TypeEnum->GetNameStringByValue(static_cast<int64>(Entry.Type)));
	}

	FString StatusStr;
	FLinearColor StatusColor = FLinearColor::White;
	switch (Entry.Status)
	{
	case EGorgeousRPCDebugStatus::Pending:
		StatusStr = TEXT("Pending");
		StatusColor = FLinearColor(0.90f, 0.70f, 0.20f, 1.f);
		break;
	case EGorgeousRPCDebugStatus::Complete:
		StatusStr = TEXT("Complete");
		StatusColor = FLinearColor(0.30f, 0.85f, 0.45f, 1.f);
		break;
	case EGorgeousRPCDebugStatus::Failed:
		StatusStr = TEXT("Failed");
		StatusColor = FLinearColor(0.90f, 0.30f, 0.30f, 1.f);
		break;
	case EGorgeousRPCDebugStatus::PartialSuccess:
		StatusStr = TEXT("Partial Success");
		StatusColor = FLinearColor(0.90f, 0.65f, 0.20f, 1.f);
		break;
	default:
		StatusStr = TEXT("Timed Out");
		StatusColor = FLinearColor(0.70f, 0.40f, 0.40f, 1.f);
		break;
	}
	AddDetailRow(TEXT("Status"), StatusStr, StatusColor);

	if (Entry.CompletedTimeSec > 0.0 && Entry.InitiatedTimeSec > 0.0)
	{
		const double TotalMs = (Entry.CompletedTimeSec - Entry.InitiatedTimeSec) * 1000.0;
		AddDetailRow(TEXT("Total Duration"), FString::Printf(TEXT("%.2f ms"), TotalMs));
	}

	AddDetailRow(TEXT("Expected"),
	             Entry.ExpectedResponseCount >= 0
		             ? FString::Printf(TEXT("%d response(s)"), Entry.ExpectedResponseCount)
		             : TEXT("Unknown (broadcast)"));

	// ---- Arguments ------------------------------------------------------
	AddDetailSectionHeader(TEXT("Arguments"), FLinearColor(0.85f, 0.55f, 0.20f, 1.f));

	if (Entry.ArgumentNames.Num() == 0)
	{
		AddDetailRow(TEXT("(none)"), TEXT("No arguments captured"));
	}
	else
	{
		for (int32 i = 0; i < Entry.ArgumentNames.Num(); ++i)
		{
			const FString& ArgName = Entry.ArgumentNames[i];
			// Use the actual argument name as the label, not "Arg[0]"
			AddDetailRow(ArgName.IsEmpty() ? FString::Printf(TEXT("Arg[%d]"), i) : ArgName, TEXT("(captured)"));
		}
	}

	// ---- Return value summary  (full detail is in the timeline pane) ----
	if (Entry.Responses.Num() > 0)
	{
		AddDetailSectionHeader(TEXT("Return Values"), FLinearColor(0.30f, 0.75f, 0.50f, 1.f));
		AddDetailRow(TEXT("Received"),
		             FString::Printf(TEXT("%d response(s)"), Entry.GetReceivedResponseCount()));

		for (int32 i = 0; i < Entry.Responses.Num(); ++i)
		{
			const FGorgeousRPCResponseSnapshot& Resp = Entry.Responses[i];
			// Same logic as the timeline node: '{' present + at least one non-empty value.
			auto HasNonEmptyVals = [](const FString& P) -> bool
			{
				const int32 B = P.Find(TEXT("{"));
				if (B == INDEX_NONE) return false;
				const FString Body = P.Mid(B);
				const FString Pfx  = TEXT("\": \"");
				int32 Pos = 0;
				while (true)
				{
					const int32 F = Body.Find(Pfx, ESearchCase::CaseSensitive, ESearchDir::FromStart, Pos);
					if (F == INDEX_NONE) break;
					const int32 VS = F + Pfx.Len();
					if (VS < Body.Len() && Body[VS] != TCHAR('"')) return true;
					Pos = F + 1;
				}
				return false;
			};
			const bool bPreviewHasData = Resp.bHasReturnValue && HasNonEmptyVals(Resp.ReturnValuePreview);
			const FString ValText = bPreviewHasData
				                        ? Resp.ReturnValuePreview
				                        : Resp.bHasReturnValue
				                        ? TEXT("Awaiting Result...")
				                        : TEXT("⌛ pending...");
			const FLinearColor ValColor = bPreviewHasData
				                              ? FLinearColor(0.30f, 0.85f, 0.45f, 1.f)
				                              : FLinearColor(0.50f, 0.50f, 0.50f, 1.f);
			AddDetailRow(
				FString::Printf(TEXT("[%s]"), *Resp.ResponderDisplayName),
				ValText, ValColor);
		}
	}
}

void SGorgeousRPCInspectorWindow::ClearDetailPanels()
{
	LeftDetailsBox->ClearChildren();
	LeftDetailsBox->AddSlot()
	              .AutoHeight()
	              .Padding(8.f, 16.f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("Select an RPC to inspect its details.")))
		.Font(FAppStyle::GetFontStyle("SmallFont"))
		.ColorAndOpacity(FSlateColor::UseSubduedForeground())
	];
	TimelineWidget->Clear();
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  Toolbar actions
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

FReply SGorgeousRPCInspectorWindow::OnLivePauseToggled()
{
	bIsLive = !bIsLive;
	if (LivePauseLabel.IsValid())
	{
		LivePauseLabel->SetText(
			FText::FromString(bIsLive ? TEXT("Live") : TEXT("Paused")));
		LivePauseLabel->SetColorAndOpacity(bIsLive
			                                   ? FLinearColor(0.30f, 0.90f, 0.40f, 1.f)
			                                   : FLinearColor(0.90f, 0.70f, 0.20f, 1.f));
	}
	return FReply::Handled();
}

FReply SGorgeousRPCInspectorWindow::OnClearClicked()
{
	FGorgeousRPCDebugTracker::Get().Reset();
	SelectedItem.Reset();
	LastBuiltDetailGuid = FGuid();
	ClearDetailPanels();
	RefreshList();
	return FReply::Handled();
}
