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
#include "GameplayTagsManager.h"
#include "GameplayTagsSettings.h"
#include "Logging/TokenizedMessage.h"
#include "Async/Async.h"
#include "HAL/Event.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousLoggingMetadataTest,
    "GorgeousCore.Logging.Metadata.SourceAndDescription",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousLoggingMetadataTest::RunTest(const FString&)
{
    const auto CheckKey = [&](const FString& Key)
    {
        FString NodeDescription;
        TArray<FName> TagSources;
        const FName KeyName(*Key);
        bool bExplicit = false, bRestricted = false, bAllowChildren = false;
        const bool bFound = UGameplayTagsManager::Get().GetTagEditorData(
            KeyName, NodeDescription, TagSources, bExplicit, bRestricted, bAllowChildren);
        const FGameplayTagTableRow* Row = nullptr;
        int32 DeclaredLoggingRows = 0;
        for (const FName SourceName : TagSources)
        {
            if (!SourceName.ToString().EndsWith(TEXT("LoggingTags.ini"), ESearchCase::IgnoreCase)) continue;
            const FGameplayTagSource* LoggingSource = UGameplayTagsManager::Get().FindTagSource(SourceName);
            if (!LoggingSource || !LoggingSource->SourceTagList) continue;
            for (const FGameplayTagTableRow& Candidate : LoggingSource->SourceTagList->GameplayTagList)
                if (Candidate.Tag == KeyName)
                {
                    Row = &Candidate;
                    ++DeclaredLoggingRows;
                }
        }
        if (!TestEqual(Key + TEXT(" has exactly one declared logging-INI row"), DeclaredLoggingRows, 1)) return false;
        const FString RawDescription = Row ? Row->DevComment : FString();
        FString Description;
        FString Examples;
        if (!RawDescription.Split(TEXT(" Examples:"), &Description, &Examples, ESearchCase::IgnoreCase))
            Description = RawDescription;
        if (!TestTrue(Key + TEXT(" has a real explicit logging-INI row description"), bFound && bExplicit && Row && !Description.IsEmpty())) return false;
        const auto Snapshot = GorgeousLogging::GetLoggingSettingsSnapshot();
        if (!TestTrue(TEXT("Metadata test has an enabled information-level Message Log"),
            Snapshot.bEnableGorgeousMessageLog && GorgeousLogging::GetImportanceRank(Snapshot.MinMessageLogVerbosity) <= 0)) return false;
        const auto Listing = FModuleManager::LoadModuleChecked<FMessageLogModule>(TEXT("MessageLog")).GetLogListing(Snapshot.MessageLogListingName);
        const FString Marker = TEXT("GORGEOUS_METADATA_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
        FGorgeousLogEntry Captured;
        const FDelegateHandle Handle = GetGorgeousLogEntryDelegate().AddLambda([&](const FGorgeousLogEntry& Entry)
        {
            if (Entry.Message.Contains(Marker)) Captured = Entry;
        });
        ON_SCOPE_EXIT { GetGorgeousLogEntryDelegate().Remove(Handle); };
        const int32 ExpectedLine = __LINE__ + 1;
        GT_I_LOG(*Key, TEXT("[%s] Successfully registered module type: %s"), *Marker, TEXT("Runtime"));
        TestEqual(TEXT("Macro captures the emitting source line"), Captured.SourceLine, ExpectedLine);
        TestTrue(TEXT("Macro captures source rather than the shared helper"), FPaths::GetCleanFilename(Captured.SourceFile) == TEXT("GorgeousLoggingOutputTests.cpp"));
        TestTrue(TEXT("Macro captures its emitting function"), Captured.SourceFunction.Contains(TEXT("FGorgeousLoggingMetadataTest")));
        TestEqual(TEXT("Default macro uses its configured display duration"), Captured.Duration, float(GT_DURATION));
        TestEqual(TEXT("Default macro respects information-level screen configuration"), Captured.bPrintToScreen, bool(GT_PRINT_TO_SCREEN_ON_INFO && Snapshot.bShowOnScreen));
        TestEqual(TEXT("Default macro respects configured Output Log routing"), Captured.bPrintToLog, bool(GT_PRINT_TO_LOG && Snapshot.bMirrorToOutputLog));
        TestTrue(TEXT("Default macro supplies no world context"), !Captured.WorldContextObject.IsValid());
        bool bPlainBodyFound = false;
        int32 SourceLinkCount = 0;
        for (const auto& Message : Listing->GetFilteredMessages())
        {
            if (!Message->ToText().ToString().Contains(Marker)) continue;
            for (const auto& Token : Message->GetMessageTokens())
            {
                const FString Label = Token->ToText().ToString();
                if (Label.Contains(Marker))
                {
                    bPlainBodyFound = Token->GetType() == EMessageToken::Text;
                    TestFalse(TEXT("The message body is not a hyperlink"), Token->GetOnMessageTokenActivated().IsBound());
                }
                if (Token->GetType() != EMessageToken::Action) continue;
                TestEqual(TEXT("The only action on a plain macro entry is the compact source link"), Label, FString(TEXT("Source")));
                ++SourceLinkCount;
                const auto Action = StaticCastSharedRef<FActionToken>(Token);
                const FString Tooltip = Action->GetActionDescription().ToString();
                TestTrue(TEXT("Hover shows the concrete formatted event"), Tooltip.Contains(
                    FString::Printf(TEXT("[%s] Successfully registered module type: Runtime"), *Marker)));
                TestFalse(TEXT("Hover omits examples from other calls"), Tooltip.Contains(TEXT("Examples:")));
                TestFalse(TEXT("Hover omits unresolved example arguments"), Tooltip.Contains(TEXT("%s")));
                TestTrue(TEXT("Source hover exposes its INI description"), Tooltip.Contains(Description));
                TestTrue(TEXT("Source hover exposes its logging key"), Tooltip.Contains(Key));
                TestTrue(TEXT("Source hover carries the complete file and line"), Tooltip.Contains(
                    FString::Printf(TEXT("%s:%d"), *Captured.SourceFile, ExpectedLine)));
                TestTrue(TEXT("Source hover carries the function context"), Tooltip.Contains(Captured.SourceFunction));
                if (!NodeDescription.IsEmpty() && NodeDescription != Description)
                    TestFalse(TEXT("Inherited native-child metadata does not replace the exact logging description"), Tooltip.Contains(NodeDescription));
                TestTrue(TEXT("Source hyperlink has a reusable navigation action"), Action->CanExecuteAction());
                TestTrue(TEXT("The row's double-click destination is the same source link"),
                    Message->GetMessageLink().Get() == &Token.Get() && Token->GetOnMessageTokenActivated().IsBound());
            }
        }
        TestTrue(TEXT("The log body is displayed as plain text"), bPlainBodyFound);
        TestEqual(TEXT("Exactly one source hyperlink is present"), SourceLinkCount, 1);

        const FString BlueprintMarker = Marker + TEXT("_NO_CPP_ORIGIN");
        GorgeousLogging::LogMessage_Internal(FText::FromString(BlueprintMarker), Logging_Information, Key, 0.f, false, true);
        TestTrue(TEXT("Direct/Blueprint API does not invent a C++ source location"), Captured.SourceFile.IsEmpty() && Captured.SourceLine == 0);
        bool bDetailsFound = false;
        for (const auto& Message : Listing->GetFilteredMessages())
        {
            if (!Message->ToText().ToString().Contains(BlueprintMarker)) continue;
            for (const auto& Token : Message->GetMessageTokens())
                if (Token->GetType() == EMessageToken::Action)
                {
                    const auto Action = StaticCastSharedRef<FActionToken>(Token);
                    bDetailsFound |= Token->ToText().ToString() == TEXT("Copy details")
                        && Action->GetActionDescription().ToString().Contains(Description);
                }
        }
        TestTrue(TEXT("Source-less entries still expose their registered metadata"), bDetailsFound);
        return true;
    };
    // GT.UI has a native descendant whose comment can populate its implicit
    // parent before the explicit INI row. Both origins must use the exact row.
    const bool bChooserValid = CheckKey(TEXT("GT.Core.ConditionalObjectChoosers.Invalid_Mode"));
    const bool bParentValid = CheckKey(TEXT("GT.UI"));
    const bool bPluginHelperValid = CheckKey(TEXT("GT.Core.PluginHelper"));
    return bChooserValid && bParentValid && bPluginHelperValid;
}

namespace
{
    class FGorgeousVerbosityCapture final : public FOutputDevice
    {
    public:
        explicit FGorgeousVerbosityCapture(FString InMarker) : Marker(MoveTemp(InMarker)) {}
        FString Marker;
        TArray<ELogVerbosity::Type> Levels;
        virtual void Serialize(const TCHAR* Text, ELogVerbosity::Type Verbosity, const FName&) override
        {
            if (FCString::Strstr(Text, *Marker))
                Levels.Add(ELogVerbosity::Type(Verbosity & ELogVerbosity::VerbosityMask));
        }
    };
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousLoggingVerbosityTest,
    "GorgeousCore.Logging.Verbosity.NativeLevels",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousLoggingVerbosityTest::RunTest(const FString&)
{
    if (!TestNotNull(TEXT("Output redirector available"), GLog)) return false;
    const FString Marker = TEXT("GORGEOUS_VERBOSITY_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    FGorgeousVerbosityCapture Capture(Marker);
    const auto PreviousVerbosity = LogGorgeousThings.GetVerbosity();
    LogGorgeousThings.SetVerbosity(ELogVerbosity::VeryVerbose);
    GLog->FlushThreadedLogs();
    GLog->AddOutputDevice(&Capture);
    ON_SCOPE_EXIT { GLog->RemoveOutputDevice(&Capture); LogGorgeousThings.SetVerbosity(PreviousVerbosity); };
    const FString Key = TEXT("GT.Core.ConditionalObjectChoosers.Invalid_Mode");
    TArray<EGorgeousLoggingImportance> FeedLevels;
    const auto Handle = GetGorgeousLogEntryDelegate().AddLambda([&](const FGorgeousLogEntry& Entry)
    {
        if (Entry.Message.Contains(Marker)) FeedLevels.Add(Entry.Importance);
    });
    ON_SCOPE_EXIT { GetGorgeousLogEntryDelegate().Remove(Handle); };
    GT_D_LOG(*Key, TEXT("%s Display"), *Marker);
    GT_V_LOG(*Key, TEXT("%s Verbose"), *Marker);
    GT_VV_LOG(*Key, TEXT("%s VeryVerbose"), *Marker);
    GLog->FlushThreadedLogs();
    TestEqual(TEXT("Each native level physically emits once"), Capture.Levels.Num(), 3);
    if (Capture.Levels.Num() == 3)
    {
        TestEqual(TEXT("Display preserves native verbosity"), Capture.Levels[0], ELogVerbosity::Display);
        TestEqual(TEXT("Verbose preserves native verbosity"), Capture.Levels[1], ELogVerbosity::Verbose);
        TestEqual(TEXT("VeryVerbose preserves native verbosity"), Capture.Levels[2], ELogVerbosity::VeryVerbose);
    }
    TestEqual(TEXT("Runtime subscribers receive every requested output"), FeedLevels.Num(), 3);
    if (FeedLevels.Num() == 3)
    {
        TestEqual(TEXT("Display does not claim success"), FeedLevels[0], Logging_Display);
        TestEqual(TEXT("Verbose keeps its distinct importance"), FeedLevels[1], Logging_Verbose);
        TestEqual(TEXT("VeryVerbose keeps its distinct importance"), FeedLevels[2], Logging_VeryVerbose);
    }
    TestTrue(TEXT("Verbose sorts below informational messages"), GorgeousLogging::GetImportanceRank(Logging_Verbose) < GorgeousLogging::GetImportanceRank(Logging_Information));
    TestTrue(TEXT("Appended enum values do not bypass a warning threshold"), GorgeousLogging::GetImportanceRank(Logging_Display) < GorgeousLogging::GetImportanceRank(Logging_Warning));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousLoggingConcurrentWorkerTest,
    "GorgeousCore.Logging.Threading.ConcurrentWorkerFeed",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousLoggingConcurrentWorkerTest::RunTest(const FString&)
{
    const FString Marker = TEXT("GORGEOUS_CONCURRENT_FEED_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    constexpr int32 WorkerCount = 4;
    constexpr int32 PerWorkerCount = 8;
    FThreadSafeCounter ReadyWorkers, FeedCalls, InvalidOrigins, GameThreadCalls, TemporaryCalls;
    FCriticalSection SeenLock;
    TSet<FString> SeenMessages;
    const auto Observer = GetGorgeousLogEntryDelegate().AddLambda([&](const FGorgeousLogEntry& Entry)
    {
        if (!Entry.Message.Contains(Marker)) return;
        FeedCalls.Increment();
        if (IsInGameThread()) GameThreadCalls.Increment();
        if (Entry.SourceLine <= 0 || Entry.SourceFunction.IsEmpty()
            || FPaths::GetCleanFilename(Entry.SourceFile) != TEXT("GorgeousLoggingOutputTests.cpp"))
            InvalidOrigins.Increment();
        FScopeLock Lock(&SeenLock);
        SeenMessages.Add(Entry.Message);
    });
    ON_SCOPE_EXIT { GetGorgeousLogEntryDelegate().Remove(Observer); };
    FEvent* StartEvent = FPlatformProcess::GetSynchEventFromPool(true);
    ON_SCOPE_EXIT { FPlatformProcess::ReturnSynchEventToPool(StartEvent); };
    TArray<TFuture<void>> Workers;
    for (int32 Worker = 0; Worker < WorkerCount; ++Worker)
    {
        Workers.Add(Async(EAsyncExecution::Thread, [&, Worker]()
        {
            ReadyWorkers.Increment();
            StartEvent->Wait();
            for (int32 Sequence = 0; Sequence < PerWorkerCount; ++Sequence)
            {
                // Exercise binding changes during other workers' real helper broadcasts,
                // not only parallel reads of an otherwise immutable delegate list.
                const auto Temporary = GetGorgeousLogEntryDelegate().AddLambda([&](const FGorgeousLogEntry& Entry)
                {
                    if (Entry.Message.Contains(Marker)) TemporaryCalls.Increment();
                });
                GT_I_LOG(TEXT("GT.Core.ConditionalObjectChoosers.Invalid_Mode"), TEXT("%s worker=%d sequence=%d"), *Marker, Worker, Sequence);
                GetGorgeousLogEntryDelegate().Remove(Temporary);
            }
        }));
    }
    const double Deadline = FPlatformTime::Seconds() + 5.;
    while (ReadyWorkers.GetValue() < WorkerCount && FPlatformTime::Seconds() < Deadline)
        FPlatformProcess::SleepNoStats(.001f);
    const bool bAllReady = ReadyWorkers.GetValue() == WorkerCount;
    StartEvent->Trigger();
    for (auto& Worker : Workers) Worker.Get();
    TestTrue(TEXT("All workers reached the concurrent start barrier"), bAllReady);
    TestEqual(TEXT("The shared feed receives every worker diagnostic once"), FeedCalls.GetValue(), WorkerCount * PerWorkerCount);
    TestEqual(TEXT("All worker records remain distinct"), SeenMessages.Num(), WorkerCount * PerWorkerCount);
    TestEqual(TEXT("Runtime feed keeps the caller's thread contract"), GameThreadCalls.GetValue(), 0);
    TestEqual(TEXT("Worker macro origins remain valid"), InvalidOrigins.GetValue(), 0);
    TestTrue(TEXT("Concurrent temporary subscribers received messages"), TemporaryCalls.GetValue() >= WorkerCount * PerWorkerCount);
    // The editor subscriber owns copied records in its game-thread queue; its normal
    // ticker presents them after this test returns. No Slate work runs on these workers.
    return true;
}

#endif