#pragma once

#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "ModuleCore/GorgeousUsefulSettings.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Text/STextBlock.h"

/** Project protection controls keep their data-loss warning beside the opt-out. */
class FGorgeousProtectionSettingsCustomization final : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance()
    {
        return MakeShared<FGorgeousProtectionSettingsCustomization>();
    }

    virtual void CustomizeDetails(IDetailLayoutBuilder& Builder) override
    {
        IDetailCategoryBuilder& Category=Builder.EditCategory(TEXT("Extension Resource Protection"));
        Category.AddCustomRow(NSLOCTEXT("GorgeousCore", "ProtectionWarningSearch", "Resource protection warning data loss"))
        .WholeRowContent()
        [
            SNew(SVerticalBox)
            +SVerticalBox::Slot().AutoHeight()
            [
                SNew(SBorder)
                .Tag(TEXT("Gorgeous.Protection.Warning"))
                .Padding(8.f)
                .BorderBackgroundColor(FLinearColor(.6f,.2f,.02f,1.f))
                [
                    SNew(STextBlock)
                    .AutoWrapText(true)
                    .Text(NSLOCTEXT("GorgeousCore", "ProtectionDataLossWarning", "Warning: disabling resource protection can permanently remove asset references when required plugins or content are missing. Leave protection enabled unless you understand this risk."))
                ]
            ]
            +SVerticalBox::Slot().AutoHeight().Padding(0.f,4.f)
            [
                SNew(SHyperlink)
                .Tag(TEXT("Gorgeous.Protection.Documentation"))
                .Text(NSLOCTEXT("GorgeousCore", "ProtectionDocumentation", "Read the resource protection documentation"))
                .OnNavigate(FSimpleDelegate::CreateLambda([]()
                {
                    GetMutableDefault<UGorgeousUsefulSettings>()->OpenExtensionResourceProtectionDocumentation();
                }))
            ]
        ];
        Category.AddProperty(GET_MEMBER_NAME_CHECKED(UGorgeousUsefulSettings,bDisableExtensionResourceProtection));
    }
};
