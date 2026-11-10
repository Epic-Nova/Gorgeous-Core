// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Misc/ScopeExit.h"
#include "HAL/ThreadSafeCounter.h"
#include "Helpers/GorgeousLoggingHelper.h"
#include "ModuleCore/GorgeousLoggingDeveloperSettings.h"
#include "MessageLogModule.h"
#include "IMessageLogListing.h"

namespace
{
    class FGorgeousAuditOutputCapture final : public FOutputDevice
    {
    public:
        explicit FGorgeousAuditOutputCapture(FString InMarker) : Marker(MoveTemp(InMarker)) {}
        FString Marker;
        FThreadSafeCounter Calls;
        virtual bool CanBeUsedOnAnyThread() const override { return true; }
        virtual bool CanBeUsedOnMultipleThreads() const override { return true; }
        virtual void Serialize(const TCHAR* Text, ELogVerbosity::Type, const FName&) override
        { if (FCString::Strstr(Text, *Marker)) Calls.Increment(); }
    };
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousLoggingOutputOnceTest,
    "GorgeousCore.Limitations.Logging.OneOutputOneListingOneFeed",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousLoggingOutputOnceTest::RunTest(const FString&)
{
    if (!TestNotNull(TEXT("Output redirector available"), GLog)) return false;
    const auto Snapshot = GorgeousLogging::GetLoggingSettingsSnapshot();
    const auto* Settings = GetDefault<UGorgeousLoggingDeveloperSettings>();
    if (!TestTrue(TEXT("Test exercises default enabled Message Log and output mirror"),
        Settings && Settings->bEnableGorgeousMessageLog && Snapshot.bEnableGorgeousMessageLog && Snapshot.bMirrorToOutputLog)) return false;
    const FString Marker = TEXT("GORGEOUS_AUDIT_SINGLE_EMISSION_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    // The formatted entry includes its key; use a distinct key so body occurrences count records.
    const FName Key(*(TEXT("GorgeousAuditSingleEmissionKey_") + FGuid::NewGuid().ToString(EGuidFormats::Digits)));
    auto& MessageLog = FModuleManager::LoadModuleChecked<FMessageLogModule>(TEXT("MessageLog"));
    const auto Listing = MessageLog.GetLogListing(Settings->MessageLogListingName);
    FGorgeousAuditOutputCapture Capture(Marker);
    FThreadSafeCounter FeedCalls;
    const FDelegateHandle FeedHandle = GetGorgeousLogEntryDelegate().AddLambda([&](const FGorgeousLogEntry& Entry)
    { if (Entry.LoggingKey == Key) FeedCalls.Increment(); });
    GLog->FlushThreadedLogs();
    GLog->AddOutputDevice(&Capture);
    ON_SCOPE_EXIT { GLog->RemoveOutputDevice(&Capture); GetGorgeousLogEntryDelegate().Remove(FeedHandle); };
    GorgeousLogging::LogMessage_Internal(FText::FromString(Marker), Logging_Information, Key.ToString(), 0.f, false, true);
    GLog->FlushThreadedLogs();
    TestEqual(TEXT("Public logging call physically emits once to Output Log"), Capture.Calls.GetValue(), 1);
    TestEqual(TEXT("Independent runtime subscriber receives exactly one entry"), FeedCalls.GetValue(), 1);
    const FString Messages = Listing->GetAllMessagesAsString();
    const int32 First = Messages.Find(Marker);
    TestTrue(TEXT("Message Log UI still receives the same entry"), First != INDEX_NONE);
    if (First != INDEX_NONE)
        TestEqual(TEXT("Message Log UI contains the marker once"), Messages.Find(Marker, ESearchCase::CaseSensitive, ESearchDir::FromStart, First + Marker.Len()), INDEX_NONE);
    return true;
}
#endif
