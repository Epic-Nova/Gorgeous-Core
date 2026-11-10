#include "LibraryWizard/GorgeousNetworkConsent.h"
#include "LibraryWizard/GorgeousLibrarySettings.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SHyperlink.h"

bool GorgeousNetworkConsent::IsSafeURL(const FString& URL)
{
    return URL.StartsWith(TEXT("https://"), ESearchCase::IgnoreCase) &&
        !URL.Contains(TEXT("\r")) && !URL.Contains(TEXT("\n"));
}

bool GorgeousNetworkConsent::IsAllowed(bool bPrompt)
{
    if (FPlatformMisc::GetEnvironmentVariable(TEXT("GORGEOUS_OFFLINE")) == TEXT("1") ||
        FParse::Param(FCommandLine::Get(), TEXT("GorgeousOffline"))) return false;
    if (FPlatformMisc::GetEnvironmentVariable(TEXT("GORGEOUS_ALLOW_NETWORK")) == TEXT("1") ||
        FParse::Param(FCommandLine::Get(), TEXT("AllowGorgeousNetwork"))) return true;
    if (IsRunningCommandlet() || FApp::IsUnattended()) return false;
    auto* Settings = GetMutableDefault<UGorgeousLibrarySettings>();
    if (Settings->bAllowGorgeousNetwork) return true;
    if (!bPrompt || Settings->bNetworkConsentDecided || !FSlateApplication::IsInitialized()) return false;

    TSharedRef<SWindow> Window = SNew(SWindow)
        .Title(NSLOCTEXT("GorgeousNetwork", "Title", "Gorgeous API access"))
        .ClientSize(FVector2D(560, 210)).SupportsMinimize(false).SupportsMaximize(false);
    Window->SetContent(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(16)[SNew(STextBlock).AutoWrapText(true).Text(
            NSLOCTEXT("GorgeousNetwork", "Explanation", "Allow Gorgeous Core to contact the Gorgeous API for news, the system catalog, update checks and downloads? Update checks send plugin names, versions and source checksums. This choice applies to this project and Windows user. You can change it in Gorgeous Library settings."))]
        + SVerticalBox::Slot().AutoHeight().Padding(16, 4)[SNew(SHyperlink)
            .Text(NSLOCTEXT("GorgeousNetwork", "Privacy", "Read the privacy policy"))
            .OnNavigate_Lambda([] { FPlatformProcess::LaunchURL(TEXT("https://epicnova.net/legal/privacy-policy/long"), nullptr, nullptr); })]
        + SVerticalBox::Slot().AutoHeight().Padding(16)[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(NSLOCTEXT("GorgeousNetwork", "Deny", "Stay offline"))
                .OnClicked_Lambda([Settings, Window] { Settings->bAllowGorgeousNetwork=false; Settings->bNetworkConsentDecided=true; Settings->SaveConfig(); Window->RequestDestroyWindow(); return FReply::Handled(); })]
            + SHorizontalBox::Slot().AutoWidth().Padding(12, 0)[SNew(SButton).Text(NSLOCTEXT("GorgeousNetwork", "Allow", "Allow API access"))
                .OnClicked_Lambda([Settings, Window] { Settings->bAllowGorgeousNetwork=true; Settings->bNetworkConsentDecided=true; Settings->SaveConfig(); Window->RequestDestroyWindow(); return FReply::Handled(); })]]);
    FSlateApplication::Get().AddModalWindow(Window, FSlateApplication::Get().GetActiveTopLevelWindow());
    return Settings->bAllowGorgeousNetwork;
}
