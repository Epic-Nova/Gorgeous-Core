// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "DetailCustomizations/GorgeousFeedbackEffectCustomization.h"
#include "FeedbackEffects/GorgeousFeedbackEffect.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void FGorgeousFeedbackEffectCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    TArray<TWeakObjectPtr<UObject>> Objects;
    DetailBuilder.GetObjectsBeingCustomized(Objects);
    TArray<TWeakObjectPtr<UGorgeousFeedbackEffect>> Effects;
    for (const auto& Object : Objects) { if (auto* Effect = Cast<UGorgeousFeedbackEffect>(Object.Get())) { Effects.Add(Effect); } }
    if (Effects.IsEmpty()) { return; }
    const TWeakObjectPtr<UGorgeousFeedbackEffect> Primary = Effects[0];
    auto& Category = DetailBuilder.EditCategory(TEXT("Feedback Preview"), FText::GetEmpty(), ECategoryPriority::Important);
    Category.AddCustomRow(NSLOCTEXT("GorgeousFeedback", "PreviewSearch", "Feedback preview category color"))
    .WholeRowContent()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0,4)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0,0,8,0)
            [SNew(SColorBlock).Size(FVector2D(24,24)).Color_Lambda([Primary] { return Primary.IsValid() ? Primary->EditorColor : FLinearColor::White; })]
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [SNew(STextBlock).Text_Lambda([Primary]
            {
                return Primary.IsValid() && !Primary->EditorCategory.IsEmpty() ? Primary->EditorCategory
                    : NSLOCTEXT("GorgeousFeedback", "Uncategorized", "Uncategorized feedback");
            })]
        ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0,0,8,0)
            [SNew(SButton).Text(NSLOCTEXT("GorgeousFeedback", "PlayPreview", "Preview"))
                .IsEnabled_Lambda([Primary] { return Primary.IsValid() && Primary->SupportsPreview(); })
                .OnClicked_Lambda([Effects] { for (const auto& Effect : Effects) { if (Effect.IsValid()) { Effect->Preview(); } } return FReply::Handled(); })]
            + SHorizontalBox::Slot().AutoWidth()
            [SNew(SButton).Text(NSLOCTEXT("GorgeousFeedback", "StopPreview", "Stop"))
                .OnClicked_Lambda([Effects] { for (const auto& Effect : Effects) { if (Effect.IsValid()) { Effect->StopPreview(); } } return FReply::Handled(); })]
        ]
    ];
}
