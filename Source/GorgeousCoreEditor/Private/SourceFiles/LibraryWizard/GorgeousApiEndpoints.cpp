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
#include "LibraryWizard/GorgeousApiEndpoints.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "LibraryWizard/GorgeousLibrarySettings.h"
#include "Helpers/Macros/GorgeousConnectionHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Internationalization/Regex.h"
#include "Misc/AutomationTest.h"
//<-------------------------------------------------------------------------->

namespace
{
    /**
     * Extracts the HTTP origin without accepting credentials or ambiguous authority separators.
     *
     * @param URL The absolute URL to inspect.
     * @return Its scheme and authority, or an empty string for an invalid URL.
     */
    FString GetURLOrigin(const FString& URL)
    {
        for (const TCHAR Character : URL)
        {
            if (Character <= TEXT(' ') || Character == 127 || Character == TEXT('\\')) return FString();
        }
        static const FRegexPattern Pattern(TEXT("(?i)^https?://(?:[a-z0-9.-]+|\\[[a-f0-9:]+\\])(?::[0-9]{1,5})?(?=[/?#]|$)"));
        FRegexMatcher Matcher(Pattern, URL);
        return Matcher.FindNext() ? URL.Left(Matcher.GetMatchEnding()) : FString();
    }

    /**
     * Combines the configured API host with the protocol selected by Force Dev Mode.
     *
     * @param bForceDevMode Whether to use HTTP.
     * @param ApiOrigin The configured hostname and optional port.
     * @return A validated origin without a trailing slash.
     */
    FString ResolveOrigin(bool bForceDevMode, FString ApiOrigin)
    {
        ApiOrigin.TrimStartAndEndInline();
        if (!ApiOrigin.RemoveFromStart(TEXT("https://"), ESearchCase::IgnoreCase))
        {
            ApiOrigin.RemoveFromStart(TEXT("http://"), ESearchCase::IgnoreCase);
        }
        ApiOrigin.RemoveFromEnd(TEXT("/"));

        const TCHAR* Scheme = bForceDevMode ? TEXT("http://") : TEXT("https://");
        const FString Candidate = Scheme + ApiOrigin;
        if (GetURLOrigin(Candidate).Equals(Candidate, ESearchCase::IgnoreCase)) return Candidate;
        return Scheme + FString(TEXT(GORGEOUS_API_DEFAULT_HOST));
    }

    /**
     * Checks whether an HTTP URL belongs to the current developer origin.
     *
     * @param URL The candidate URL.
     * @param DeveloperOrigin The allowed development origin, or empty when disabled.
     * @return Whether the transport policy permits the URL.
     */
    bool IsAllowedURLForOrigin(const FString& URL, const FString& DeveloperOrigin)
    {
        const FString Origin = GetURLOrigin(URL);
        if (Origin.StartsWith(TEXT("https://"), ESearchCase::IgnoreCase)) return true;
        return !Origin.IsEmpty() && Origin.Equals(DeveloperOrigin, ESearchCase::IgnoreCase);
    }
}

FString GorgeousApiEndpoints::GetEndpoint(const FString& Path)
{
    const auto* Settings = GetDefault<UGorgeousLibrarySettings>();
    return ResolveOrigin(Settings->bForceDevMode, Settings->ApiOrigin) + Path;
}

FString GorgeousApiEndpoints::GetV1Endpoint(const FString& Path)
{
    return GetEndpoint(TEXT("/api/v1") + Path);
}

bool GorgeousApiEndpoints::IsAllowedURL(const FString& URL, bool bAllowDeveloperOrigin)
{
    const auto* Settings = GetDefault<UGorgeousLibrarySettings>();
    const FString DeveloperOrigin = bAllowDeveloperOrigin && Settings->bForceDevMode
        ? ResolveOrigin(true, Settings->ApiOrigin) : FString();
    return IsAllowedURLForOrigin(URL, DeveloperOrigin);
}

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousApiEndpointPolicyTest, "GorgeousCore.Editor.OnlineServices.EndpointPolicy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousApiEndpointPolicyTest::RunTest(const FString&)
{
    const FString DeveloperOrigin = TEXT("http://127.0.0.1:8181");
    TestEqual(TEXT("Normal mode uses HTTPS with the configured host and port"),
        ResolveOrigin(false, DeveloperOrigin), FString(TEXT("https://127.0.0.1:8181")));
    TestEqual(TEXT("Development defaults to the normal API host with HTTP"),
        ResolveOrigin(true, FString()), FString(TEXT("http://api.gorgeous.simsalabim.studio")));
    TestEqual(TEXT("Development uses HTTP even when an HTTPS origin was pasted"),
        ResolveOrigin(true, TEXT("https://127.0.0.1:8181/")), DeveloperOrigin);
    TestEqual(TEXT("An invalid override uses the default HTTP origin"),
        ResolveOrigin(true, TEXT("http://localhost@other.invalid/api/v1")), ResolveOrigin(true, FString()));
    TestTrue(TEXT("Production accepts HTTPS"), IsAllowedURLForOrigin(TEXT("https://example.invalid/api/v1/news"), FString()));
    TestFalse(TEXT("Production rejects cleartext"), IsAllowedURLForOrigin(DeveloperOrigin + TEXT("/api/v1/news"), FString()));
    TestTrue(TEXT("Development allows HTTP at the configured origin"), IsAllowedURLForOrigin(DeveloperOrigin + TEXT("/api/v1/news"), DeveloperOrigin));
    TestFalse(TEXT("Development does not allow cleartext to a different port"), IsAllowedURLForOrigin(TEXT("http://127.0.0.1:8182/news"), DeveloperOrigin));
    TestFalse(TEXT("Development rejects userinfo"), IsAllowedURLForOrigin(DeveloperOrigin + TEXT("@other.invalid/news"), DeveloperOrigin));
    TestFalse(TEXT("Development rejects backslash authority separators"), IsAllowedURLForOrigin(DeveloperOrigin + TEXT("\\other.invalid/news"), DeveloperOrigin));
    TestFalse(TEXT("HTTPS rejects embedded line breaks"), IsAllowedURLForOrigin(TEXT("https://example.invalid/news\r\n"), DeveloperOrigin));
    return true;
}
#endif