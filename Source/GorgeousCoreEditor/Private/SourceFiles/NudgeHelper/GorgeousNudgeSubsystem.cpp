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
#include "NudgeHelper/GorgeousNudgeSubsystem.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Helpers/GorgeousPluginHelper.h"
#include "Helpers/GorgeousPersistentData.h"
#include "Helpers/GorgeousLoggingHelper.h"
#include "LibraryWizard/SGorgeousLibraryView.h"
#include "LibraryWizard/GorgeousUpdateManager.h"
#include "NudgeHelper/GorgeousNudgeDeveloperSettings.h"
#include "NudgeHelper/SGorgeousNudgeCarousel.h"
#include "LibraryWizard/GorgeousApiEndpoints.h"
#include "LibraryWizard/GorgeousNetworkConsent.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "SWebBrowser.h"
#include "Algo/RandomShuffle.h"
#include "Animation/AnimSequenceBase.h"
#include "Containers/Ticker.h"
#include "Engine/Blueprint.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "HAL/PlatformProcess.h"
#include "HttpModule.h"
#include "Interfaces/IPluginManager.h"
#include "Interfaces/IHttpResponse.h"
#include "Materials/MaterialInterface.h"
#include "Dom/JsonObject.h"
#include "Misc/AutomationTest.h"
#include "Misc/MessageDialog.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/App.h"
#include "HAL/PlatformMisc.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Sound/SoundBase.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SWindow.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// Helpers
//=============================================================================
namespace
{
	const FVector2D DefaultCarouselSize(1100.0f, 760.0f);
	constexpr float ContextTipChance = 0.25f;
	constexpr float ContextTipRequestInterval = 120.0f;

	/**
	 * Finds an asset in a foreground toolkit tab in the active editor window.
	 *
	 * @return The most recently activated asset in that window, if any.
	 */
	UObject* FindActiveTipAsset()
	{
		auto* AssetEditors = GEditor ? GEditor->GetEditorSubsystem<UAssetEditorSubsystem>() : nullptr;
		if (!AssetEditors) return nullptr;

		auto& Slate = FSlateApplication::Get();
		const auto ActiveWindow = Slate.GetActiveTopLevelRegularWindow();
		if (!ActiveWindow) return nullptr;

		UObject* ActiveAsset = nullptr;
		double LastActivation = -1.0;
		for (UObject* Asset : AssetEditors->GetAllEditedAssets())
		{
			if (!IsValid(Asset)) continue;
			IAssetEditorInstance* Editor = AssetEditors->FindEditorForAsset(Asset, false);
			if (!Editor) continue;
			const auto TabManager = Editor->GetAssociatedTabManager();
			const auto Tab = TabManager ? TabManager->GetOwnerTab() : nullptr;
			if (!Tab || !Tab->IsForeground() || Slate.FindWidgetWindow(Tab.ToSharedRef()) != ActiveWindow) continue;

			const double Activation = Editor->GetLastActivationTime();
			if (Activation <= LastActivation) continue;
			LastActivation = Activation;
			ActiveAsset = Asset;
		}
		return ActiveAsset;
	}

	/**
	 * Maps an asset to a fixed request category without using asset or project identifiers.
	 *
	 * @param Asset The local asset whose editor is active.
	 * @return The allowlisted category, or null for an unsupported asset.
	 */
	const TCHAR* GetContextAssetType(const UObject& Asset)
	{
		if (Asset.IsA<UBlueprint>()) return TEXT("blueprint");
		if (Asset.IsA<UMaterialInterface>()) return TEXT("material");
		if (Asset.IsA<UTexture>()) return TEXT("texture");
		if (Asset.IsA<UStaticMesh>()) return TEXT("static_mesh");
		if (Asset.IsA<USkeletalMesh>()) return TEXT("skeletal_mesh");
		if (Asset.IsA<UAnimSequenceBase>()) return TEXT("animation");
		if (Asset.IsA<USoundBase>()) return TEXT("audio");
		return nullptr;
	}

	FString GetJsonString(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName)
	{
		FString Value;
		Object->TryGetStringField(FieldName, Value);
		return Value;
	}

	FString EscapeHtml(FString Value)
	{
		Value.ReplaceInline(TEXT("&"), TEXT("&amp;"));
		Value.ReplaceInline(TEXT("<"), TEXT("&lt;"));
		Value.ReplaceInline(TEXT(">"), TEXT("&gt;"));
		Value.ReplaceInline(TEXT("\""), TEXT("&quot;"));
		return Value;
	}

	FString BuildMarkdownHtml(const FString& Markdown)
	{
		TArray<FString> Lines;
		Markdown.ParseIntoArrayLines(Lines, false);
		FString Body;
		bool bInList = false;
		bool bInCodeBlock = false;
		for (const FString& Line : Lines)
		{
			const FString Trimmed = Line.TrimStartAndEnd();
			if (Trimmed.StartsWith(TEXT("```")))
			{
				if (bInList)
				{
					Body += TEXT("</ul>");
					bInList = false;
				}
				Body += bInCodeBlock ? TEXT("</code></pre>") : TEXT("<pre><code>");
				bInCodeBlock = !bInCodeBlock;
				continue;
			}
			if (bInCodeBlock)
			{
				Body += EscapeHtml(Line) + TEXT("\n");
				continue;
			}
			if (Trimmed.IsEmpty())
			{
				if (bInList)
				{
					Body += TEXT("</ul>");
					bInList = false;
				}
				continue;
			}
			if (Trimmed.StartsWith(TEXT("- ")) || Trimmed.StartsWith(TEXT("* ")))
			{
				if (!bInList)
				{
					Body += TEXT("<ul>");
					bInList = true;
				}
				Body += FString::Printf(TEXT("<li>%s</li>"), *EscapeHtml(Trimmed.Mid(2)));
				continue;
			}
			if (bInList)
			{
				Body += TEXT("</ul>");
				bInList = false;
			}

			int32 HeadingLevel = 0;
			while (HeadingLevel < Trimmed.Len() && Trimmed[HeadingLevel] == TCHAR('#'))
			{
				++HeadingLevel;
			}
			if (HeadingLevel > 0 && HeadingLevel <= 3 && Trimmed.Len() > HeadingLevel && Trimmed[HeadingLevel] == TCHAR(' '))
			{
				Body += FString::Printf(TEXT("<h%d>%s</h%d>"), HeadingLevel, *EscapeHtml(Trimmed.Mid(HeadingLevel + 1)), HeadingLevel);
			}
			else if (Trimmed.StartsWith(TEXT("> ")))
			{
				Body += FString::Printf(TEXT("<blockquote>%s</blockquote>"), *EscapeHtml(Trimmed.Mid(2)));
			}
			else
			{
				Body += FString::Printf(TEXT("<p>%s</p>"), *EscapeHtml(Trimmed));
			}
		}
		if (bInList) Body += TEXT("</ul>");
		if (bInCodeBlock) Body += TEXT("</code></pre>");

		return FString::Printf(TEXT("<html><head><style>body{margin:0;background:radial-gradient(circle at 85%% 4%%,#0a4053 0,#041922 42%%,#020b10 100%%);color:#dceff5;font-family:Segoe UI,Arial,sans-serif}.article{box-sizing:border-box;max-width:980px;min-height:100vh;margin:0 auto;padding:64px 72px}h1{font-size:42px;letter-spacing:-.5px;color:#f4fcff;margin:0 0 24px}h2{font-size:29px;color:#53c8f1;margin:42px 0 14px}h3{font-size:21px;color:#a9e8fc;margin:30px 0 10px}p,li{font-size:17px;line-height:1.7;color:#b4d2dc}ul{padding-left:24px}blockquote{margin:26px 0;padding:14px 20px;border-left:3px solid #21acd8;background:rgba(11,77,101,.3);color:#aee6f7}pre{padding:18px;overflow:auto;border:1px solid rgba(72,187,223,.22);border-radius:12px;background:#021117;color:#8ee1fb}</style></head><body><main class='article'>%s</main></body></html>"), *Body);
	}

	bool ParseNudgeScope(const FString& Type, EGorgeousNudgeScope& OutScope)
	{
		if (Type.Equals(TEXT("plugin_update"), ESearchCase::IgnoreCase))
		{
			OutScope = EGorgeousNudgeScope::PluginUpdate;
			return true;
		}
		if (Type.Equals(TEXT("general_system_update"), ESearchCase::IgnoreCase))
		{
			OutScope = EGorgeousNudgeScope::GeneralSystemUpdate;
			return true;
		}
		if (Type.Equals(TEXT("ecosystem_news"), ESearchCase::IgnoreCase))
		{
			OutScope = EGorgeousNudgeScope::EcosystemNews;
			return true;
		}
		if (Type.Equals(TEXT("new_plugin_tutorial"), ESearchCase::IgnoreCase))
		{
			OutScope = EGorgeousNudgeScope::NewPluginTutorial;
			return true;
		}
		if (Type.Equals(TEXT("tutorial_available"), ESearchCase::IgnoreCase))
		{
			OutScope = EGorgeousNudgeScope::TutorialAvailable;
			return true;
		}
		if (Type.Equals(TEXT("new_general_system"), ESearchCase::IgnoreCase))
		{
			OutScope = EGorgeousNudgeScope::NewGeneralSystem;
			return true;
		}
		if (Type.Equals(TEXT("common_plugin_tip"), ESearchCase::IgnoreCase))
		{
			OutScope = EGorgeousNudgeScope::CommonPluginTip;
			return true;
		}
		return false;
	}
}

//=============================================================================
// UGorgeousNudgeSubsystem Implementation
//=============================================================================

bool UGorgeousNudgeSubsystem::ParseNudgeFeed(const TSharedPtr<FJsonObject>& FeedRoot, TArray<FGorgeousNudgeEntry>& OutEntries)
{
	if (!FeedRoot.IsValid()) return false;
	double SchemaVersion = 0.0;
	if (!FeedRoot->TryGetNumberField(TEXT("schemaVersion"), SchemaVersion) || SchemaVersion != 1.0) return false;

	const TArray<TSharedPtr<FJsonValue>>* JsonEntries = nullptr;
	if (!FeedRoot->TryGetArrayField(TEXT("entries"), JsonEntries) || !JsonEntries) return false;
	for (const TSharedPtr<FJsonValue>& JsonValue : *JsonEntries)
	{
		const TSharedPtr<FJsonObject> JsonEntry = JsonValue.IsValid() && JsonValue->Type == EJson::Object ? JsonValue->AsObject() : nullptr;
		if (!JsonEntry.IsValid()) continue;

		FGorgeousNudgeEntry Entry;
		Entry.Id = GetJsonString(JsonEntry, TEXT("id"));
		if (Entry.Id.IsEmpty() || !ParseNudgeScope(GetJsonString(JsonEntry, TEXT("type")), Entry.Scope)) continue;
		Entry.TargetId = GetJsonString(JsonEntry, TEXT("targetId"));
		Entry.Version = GetJsonString(JsonEntry, TEXT("version"));
		Entry.Title = FText::FromString(GetJsonString(JsonEntry, TEXT("title")));
		Entry.Description = FText::FromString(GetJsonString(JsonEntry, TEXT("description")));

		const TSharedPtr<FJsonObject>* ActionObject = nullptr;
		if (JsonEntry->TryGetObjectField(TEXT("action"), ActionObject) && ActionObject && ActionObject->IsValid())
		{
			Entry.ActionLabel = GetJsonString(*ActionObject, TEXT("label"));
			Entry.ActionUrl = GetJsonString(*ActionObject, TEXT("url"));
			Entry.bUsePluginDownloader = GetJsonString(*ActionObject, TEXT("kind")).Equals(TEXT("download_plugin"), ESearchCase::IgnoreCase);
			Entry.PluginDownloadToken = GetJsonString(*ActionObject, TEXT("downloadToken"));
		}

		const TSharedPtr<FJsonObject>* ContentObject = nullptr;
		if (JsonEntry->TryGetObjectField(TEXT("content"), ContentObject) && ContentObject && ContentObject->IsValid())
		{
			const FString ContentType = GetJsonString(*ContentObject, TEXT("type"));
			if (ContentType.Equals(TEXT("markdown"), ESearchCase::IgnoreCase) || ContentType.Equals(TEXT("markdown_url"), ESearchCase::IgnoreCase) || ContentType.Equals(TEXT("markdownUrl"), ESearchCase::IgnoreCase))
			{
				Entry.MarkdownContent = GetJsonString(*ContentObject, TEXT("markdown"));
				Entry.MarkdownUrl = GetJsonString(*ContentObject, TEXT("markdownUrl"));
				if (!Entry.MarkdownContent.IsEmpty()) Entry.HtmlPage = BuildMarkdownHtml(Entry.MarkdownContent);
			}
			else if (ContentType.Equals(TEXT("url"), ESearchCase::IgnoreCase))
			{
				Entry.ContentUrl = GetJsonString(*ContentObject, TEXT("url"));
			}
		}

		const TSharedPtr<FJsonObject>* WindowObject = nullptr;
		if (JsonEntry->TryGetObjectField(TEXT("window"), WindowObject) && WindowObject && WindowObject->IsValid())
		{
			double Width = 0.0;
			double Height = 0.0;
			(*WindowObject)->TryGetNumberField(TEXT("width"), Width);
			(*WindowObject)->TryGetNumberField(TEXT("height"), Height);
			Entry.PreferredWindowSize = FVector2D(Width, Height);
		}
		OutEntries.Add(MoveTemp(Entry));
	}
	return true;
}


void UGorgeousNudgeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bStopping = false;
	LoadAcknowledgements();
	DetectNewGorgeousPlugins();
	LoadCachedNudgeFeed();
	if (const UGorgeousNudgeDeveloperSettings* Settings = UGorgeousNudgeDeveloperSettings::Get(); Settings && Settings->bEnableAutomaticNudges)
	{
		CheckForNudgesNow();
	}
	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UGorgeousNudgeSubsystem::Tick), 1.0f);
}

void UGorgeousNudgeSubsystem::Deinitialize()
{
	bStopping = true;
	if (FeedRequest.IsValid())
	{
		FeedRequest->OnProcessRequestComplete().Unbind();
		FeedRequest->CancelRequest();
		FeedRequest.Reset();
	}
	if (ContextTipRequest.IsValid())
	{
		ContextTipRequest->OnProcessRequestComplete().Unbind();
		ContextTipRequest->CancelRequest();
		ContextTipRequest.Reset();
	}
	ContextAsset.Reset();
	PendingContextTip.Reset();
	for (auto& Pair : MarkdownRequests)
	{
		Pair.Value->OnProcessRequestComplete().Unbind();
		Pair.Value->CancelRequest();
	}
	MarkdownRequests.Empty();
	const TArray<TSharedPtr<SWindow>> ClosingTutorials = MoveTemp(TutorialWindows);
	for (const TSharedPtr<SWindow>& Window : ClosingTutorials)
	{
		if (Window.IsValid()) Window->RequestDestroyWindow();
	}
	PendingNotifications.Empty();
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
	}
	if (CarouselWindow.IsValid())
	{
		CarouselWindow->RequestDestroyWindow();
		CarouselWindow.Reset();
	}
	Super::Deinitialize();
}

bool UGorgeousNudgeSubsystem::Tick(float DeltaTime)
{
	SecondsSinceInitialization += DeltaTime;
	SecondsSinceLastNotification += DeltaTime;
	if (VisibleNudges.ContainsByPredicate([this](const FGorgeousNudgeEntry& Entry) { return !IsApplicable(Entry); }))
	{
		if (CarouselWindow.IsValid()) CarouselWindow->RequestDestroyWindow();
		VisibleNudges.RemoveAll([this](const FGorgeousNudgeEntry& Entry) { return !IsApplicable(Entry); });
	}
	if (!bGeneralSystemsDetectionCompleted && SecondsSinceInitialization >= 3.0f)
	{
		DetectNewGorgeousGeneralSystems();
		bGeneralSystemsDetectionCompleted = true;
	}

	const UGorgeousNudgeDeveloperSettings* Settings = UGorgeousNudgeDeveloperSettings::Get();
	if (Settings && Settings->bEnableAutomaticNudges && IsNetworkAllowed() && bAutomaticCarouselQueued && SecondsSinceInitialization >= 10.0f && FSlateApplication::IsInitialized() && FSlateApplication::Get().GetActiveTopLevelWindow().IsValid())
	{
		bAutomaticCarouselQueued = false;
		ShowNudgeCarousel();
	}

	ConsiderContextTip();
	if (!Settings || !Settings->bEnableAutomaticNudges)
	{
		return true;
	}
	if (SecondsSinceInitialization >= 10.0f && SecondsSinceLastNotification >= 12.0f && DeliverNextNotification())
	{
		SecondsSinceLastNotification = 0.0f;
	}
	SecondsSinceLastCheck += DeltaTime;
	if (SecondsSinceLastCheck >= Settings->CheckIntervalSeconds)
	{
		SecondsSinceLastCheck = 0.0f;
		CheckForNudgesNow();
	}
	return true;
}

void UGorgeousNudgeSubsystem::CheckForNudgesNow()
{
	FetchNudgeFeed();
}

void UGorgeousNudgeSubsystem::FetchNudgeFeed()
{
	if (bStopping || !IsNetworkAllowed() || FeedRequest.IsValid()) return;
	FString Endpoint = GorgeousApiEndpoints::GetV1Endpoint(TEXT("/news"));
#if WITH_DEV_AUTOMATION_TESTS
	if (!TestFeedEndpoint.IsEmpty()) Endpoint = TestFeedEndpoint;
#endif
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	FeedRequest = Request;
	Request->SetURL(Endpoint);
	Request->SetTimeout(20.0f);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->OnProcessRequestComplete().BindLambda([WeakSubsystem = TWeakObjectPtr<UGorgeousNudgeSubsystem>(this)](FHttpRequestPtr HttpRequest, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		UGorgeousNudgeSubsystem* Subsystem = WeakSubsystem.Get();
		if (Subsystem) Subsystem->FeedRequest.Reset();
		if (!Subsystem || !Subsystem->IsNetworkAllowed() || Subsystem->bStopping || !bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode())) return;

#if WITH_DEV_AUTOMATION_TESTS
		const bool bUsesTestEndpoint = Subsystem->TestFeedEndpoint == HttpRequest->GetURL()
			&& Subsystem->IsLoopbackTestEndpoint(Response->GetEffectiveURL());
		if (!bUsesTestEndpoint && !GorgeousApiEndpoints::IsAllowedURL(Response->GetEffectiveURL())) return;
#else
		if (!GorgeousApiEndpoints::IsAllowedURL(Response->GetEffectiveURL())) return;
#endif

		TSharedPtr<FJsonObject> FeedRoot;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (!FJsonSerializer::Deserialize(Reader, FeedRoot) || !FeedRoot.IsValid()) return;

		TArray<FGorgeousNudgeEntry> ParsedEntries;
		if (!ParseNudgeFeed(FeedRoot, ParsedEntries)) return;
		Subsystem->SaveNudgeFeedCache(FeedRoot);

		TArray<FGorgeousNudgeEntry> ReadyEntries;
		for (FGorgeousNudgeEntry& Entry : ParsedEntries)
		{
			if (!Subsystem->IsApplicable(Entry) || Subsystem->WasAcknowledged(Entry.Id) || (UsesToast(Entry) && !Subsystem->IsNotificationEnabled(Entry))) continue;
			if (Entry.MarkdownContent.IsEmpty() && !Entry.MarkdownUrl.IsEmpty())
			{
				Subsystem->FetchMarkdownNudge(MoveTemp(Entry));
			}
			else
			{
				ReadyEntries.Add(MoveTemp(Entry));
			}
		}
		Subsystem->RegisterNudges(ReadyEntries);
	});
	if (!Request->ProcessRequest()) FeedRequest.Reset();
}

bool UGorgeousNudgeSubsystem::CanUseContextTips() const
{
	const auto* Settings = UGorgeousNudgeDeveloperSettings::Get();
	return !bStopping && Settings && Settings->bContextAwareTips && Settings->bEnableAutomaticNudges
		&& !IsRunningCommandlet() && !FApp::IsUnattended() && !GIsRunningUnattendedScript && !GIsAutomationTesting
		&& GEditor && !GEditor->PlayWorld && FSlateApplication::IsInitialized()
		&& FSlateApplication::Get().IsActive() && IsNetworkAllowed();
}

void UGorgeousNudgeSubsystem::ConsiderContextTip()
{
	const bool bCanUseTips = CanUseContextTips();
	if (!bCanUseTips)
	{
		ContextAsset.Reset();
		bSampledContextAsset = false;
		PendingContextTip.Reset();
		if (ContextTipRequest)
		{
			auto Request = MoveTemp(ContextTipRequest);
			Request->OnProcessRequestComplete().Unbind();
			Request->CancelRequest();
		}
		return;
	}

	UObject* Asset = FindActiveTipAsset();
	if (ContextAsset.Get() != Asset)
	{
		ContextAsset = Asset;
		bSampledContextAsset = false;
		PendingContextTip.Reset();
	}
	if (!Asset || bSampledContextAsset || ContextTipRequest || SecondsSinceInitialization < 10.0f
		|| SecondsSinceInitialization < NextContextTipRequest) return;

	bSampledContextAsset = true;
	const TCHAR* AssetType = GetContextAssetType(*Asset);
	if (!AssetType || FMath::FRand() >= ContextTipChance) return;

	NextContextTipRequest = SecondsSinceInitialization + ContextTipRequestInterval;
	FetchContextTip(Asset, AssetType);
}

void UGorgeousNudgeSubsystem::FetchContextTip(UObject* Asset, const FString& AssetType)
{
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	ContextTipRequest = Request;
	Request->SetURL(GorgeousApiEndpoints::GetV1Endpoint(TEXT("/news")));
	Request->SetVerb(TEXT("GET"));
	Request->SetTimeout(15.0f);
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->SetHeader(TEXT("X-Gorgeous-Asset-Type"), AssetType);
	Request->OnProcessRequestComplete().BindWeakLambda(this,
		[this, SampledAsset = TWeakObjectPtr<UObject>(Asset), AssetType](FHttpRequestPtr Completed, FHttpResponsePtr Response, bool bSucceeded)
		{
			if (ContextTipRequest != Completed) return;
			ContextTipRequest.Reset();
			if (!CanUseContextTips() || !SampledAsset.IsValid() || ContextAsset != SampledAsset
				|| SampledAsset.Get() != FindActiveTipAsset() || !bSucceeded || !Response.IsValid()
				|| !EHttpResponseCodes::IsOk(Response->GetResponseCode())
				|| !GorgeousApiEndpoints::IsAllowedURL(Response->GetEffectiveURL())) return;

			TSharedPtr<FJsonObject> Feed;
			const auto Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
			if (!FJsonSerializer::Deserialize(Reader, Feed) || !Feed.IsValid()
				|| GetJsonString(Feed, TEXT("assetType")) != AssetType) return;

			TArray<FGorgeousNudgeEntry> Tips;
			if (!ParseNudgeFeed(Feed, Tips)) return;
			Tips.RemoveAll([this](const FGorgeousNudgeEntry& Entry)
			{
				return Entry.Scope != EGorgeousNudgeScope::CommonPluginTip || WasAcknowledged(Entry.Id) || !IsApplicable(Entry);
			});
			if (!Tips.IsEmpty()) PendingContextTip = MoveTemp(Tips[FMath::RandHelper(Tips.Num())]);
		});
	if (!Request->ProcessRequest()) ContextTipRequest.Reset();
}

void UGorgeousNudgeSubsystem::FetchMarkdownNudge(FGorgeousNudgeEntry Entry)
{
	if (bStopping || !IsNetworkAllowed() || !IsApplicable(Entry) || WasAcknowledged(Entry.Id)
		|| (UsesToast(Entry) && !IsNotificationEnabled(Entry)) || MarkdownRequests.Contains(Entry.Id)
		|| !GorgeousApiEndpoints::IsAllowedURL(Entry.MarkdownUrl)) return;
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	const FString EntryId = Entry.Id;
	MarkdownRequests.Add(EntryId, Request);
	Request->SetTimeout(20.0f);
	Request->SetURL(Entry.MarkdownUrl);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Accept"), TEXT("text/markdown, text/plain"));
	Request->OnProcessRequestComplete().BindLambda([WeakSubsystem = TWeakObjectPtr<UGorgeousNudgeSubsystem>(this), Entry = MoveTemp(Entry)](FHttpRequestPtr, FHttpResponsePtr Response, bool bWasSuccessful) mutable
	{
		UGorgeousNudgeSubsystem* Subsystem = WeakSubsystem.Get();
		if (Subsystem) Subsystem->MarkdownRequests.Remove(Entry.Id);
		if (!Subsystem || Subsystem->bStopping || !Subsystem->IsNetworkAllowed()) return;
		if (bWasSuccessful && Response.IsValid() && GorgeousApiEndpoints::IsAllowedURL(Response->GetEffectiveURL())
			&& EHttpResponseCodes::IsOk(Response->GetResponseCode()))
		{
			Entry.MarkdownContent = Response->GetContentAsString();
			Entry.HtmlPage = BuildMarkdownHtml(Entry.MarkdownContent);
		}
		else
		{
			Entry.Description = NSLOCTEXT("GorgeousNudges", "MarkdownUnavailable", "This article could not be loaded. Please try again when the endpoint is reachable.");
		}
		TArray<FGorgeousNudgeEntry> ResolvedEntries;
		ResolvedEntries.Add(MoveTemp(Entry));
		Subsystem->RegisterNudges(ResolvedEntries);
	});
	if (!Request->ProcessRequest()) MarkdownRequests.Remove(EntryId);
}

void UGorgeousNudgeSubsystem::HandlePluginUpdateCheckCompleted(bool bUpdatesAvailable)
{
	if (!bUpdatesAvailable) return;

	TArray<FGorgeousNudgeEntry> Updates;
	for (const FGorgeousPluginUpdateCacheEntry& Update : GetPersistentData().GetPluginUpdateCache())
	{
		FGorgeousNudgeEntry Entry;
		Entry.Id = FString::Printf(TEXT("plugin-update:%s:%s"), *Update.PluginName, *Update.AvailableVersion);
		Entry.Scope = EGorgeousNudgeScope::PluginUpdate;
		Entry.TargetId = Update.PluginName;
		Entry.Version = Update.AvailableVersion;
		Entry.Title = FText::Format(NSLOCTEXT("GorgeousNudges", "PluginUpdateTitle", "Update available: {0}"), FText::FromString(Update.PluginName));
		Entry.Description = FText::Format(NSLOCTEXT("GorgeousNudges", "PluginUpdateDescription", "Version {0} is available."), FText::FromString(Update.AvailableVersion));
		Entry.ActionLabel = TEXT("Download update");
		Entry.ActionUrl = Update.ChangelogUrl;
		Entry.bUsePluginDownloader = true;
		Entry.PluginDownloadToken = GetPersistentData().GetPluginUpdateDownloadToken(FName(*Update.PluginName));
		Updates.Add(MoveTemp(Entry));
	}
	RegisterNudges(Updates);
}

void UGorgeousNudgeSubsystem::RegisterNudges(const TArray<FGorgeousNudgeEntry>& Entries)
{
	if (bStopping) return;
	for (const FGorgeousNudgeEntry& Entry : Entries)
	{
		VisibleNudges.RemoveAll([&Entry](const FGorgeousNudgeEntry& Existing) { return Existing.Id == Entry.Id; });
		PendingNotifications.RemoveAll([&Entry](const FGorgeousNudgeEntry& Existing) { return Existing.Id == Entry.Id; });
		if (Entry.Id.IsEmpty() || !IsApplicable(Entry) || WasAcknowledged(Entry.Id)) continue;
		if (UsesToast(Entry))
		{
			PendingNotifications.Add(Entry);
		}
		else
		{
			VisibleNudges.Add(Entry);
		}
	}
	QueueAutomaticCarousel();
}

void UGorgeousNudgeSubsystem::QueueAutomaticCarousel()
{
	const UGorgeousNudgeDeveloperSettings* Settings = UGorgeousNudgeDeveloperSettings::Get();
	if (IsNetworkAllowed() && Settings != nullptr && Settings->bEnableAutomaticNudges && !VisibleNudges.IsEmpty())
	{
		bAutomaticCarouselQueued = true;
	}
}

bool UGorgeousNudgeSubsystem::IsApplicable(const FGorgeousNudgeEntry& Entry) const
{
	const UGorgeousNudgeDeveloperSettings* Settings = UGorgeousNudgeDeveloperSettings::Get();
	if (!Settings) return false;
	if (Entry.Scope == EGorgeousNudgeScope::PluginUpdate && !Settings->bShowPluginUpdates) return false;
	if (Entry.Scope == EGorgeousNudgeScope::GeneralSystemUpdate && !Settings->bShowGeneralSystemUpdates) return false;
	if (Entry.Scope == EGorgeousNudgeScope::EcosystemNews) return Settings->bShowEcosystemNews;
	const UGorgeousPluginHelper* Helper = UGorgeousPluginHelper::GetSingleton();
	const TArray<FString> InstalledSystems = GetPersistentData().GetRecordedInstalledSystems(Helper ? Helper->GetInstalledPluginSystemEntries() : TArray<FString>());
	const bool bSystemInstalled = InstalledSystems.ContainsByPredicate([&Entry](const FString& Id)
	{
		return Id.Equals(Entry.TargetId, ESearchCase::IgnoreCase);
	});
	if (Entry.Scope == EGorgeousNudgeScope::GeneralSystemUpdate) return bSystemInstalled;
	if (Entry.Scope == EGorgeousNudgeScope::NewGeneralSystem) return !Entry.TargetId.IsEmpty() && !bSystemInstalled;
	if (Entry.Scope == EGorgeousNudgeScope::TutorialAvailable && Entry.TargetId.IsEmpty()) return true;
	if (Entry.TargetId.IsEmpty()) return false;
	if (Entry.Scope == EGorgeousNudgeScope::NewPluginTutorial && !UGorgeousNudgeDeveloperSettings::Get()->bShowTutorialForNewPlugins) return false;
	const TSet<FName> KnownPlugins = Helper ? Helper->GetKnownGorgeousPlugins() : TSet<FName>();
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(Entry.TargetId);
	const bool bPluginInstalled = Plugin.IsValid() && Plugin->IsEnabled()
		&& (KnownPlugins.Contains(FName(*Entry.TargetId)) || Entry.TargetId.StartsWith(TEXT("Gorgeous"), ESearchCase::IgnoreCase));
	return bPluginInstalled || (Entry.Scope == EGorgeousNudgeScope::TutorialAvailable && bSystemInstalled);
}

bool UGorgeousNudgeSubsystem::UsesToast(const FGorgeousNudgeEntry& Entry)
{
	return Entry.Scope == EGorgeousNudgeScope::TutorialAvailable || Entry.Scope == EGorgeousNudgeScope::NewGeneralSystem
		|| Entry.Scope == EGorgeousNudgeScope::CommonPluginTip;
}

bool UGorgeousNudgeSubsystem::IsNotificationEnabled(const FGorgeousNudgeEntry& Entry) const
{
	const UGorgeousNudgeDeveloperSettings* Settings = UGorgeousNudgeDeveloperSettings::Get();
	if (!Settings || !Settings->bEnableAutomaticNudges || !IsNetworkAllowed()) return false;
	switch (Entry.Scope)
	{
	case EGorgeousNudgeScope::TutorialAvailable: return Settings->bShowNewTutorialsWhenAvailable;
	case EGorgeousNudgeScope::NewGeneralSystem: return Settings->bNotifyNewSystems;
	case EGorgeousNudgeScope::CommonPluginTip: return Settings->bShowCommonPluginTips;
	default: return false;
	}
}

bool UGorgeousNudgeSubsystem::DeliverNextNotification()
{
	if (bStopping || !IsNetworkAllowed()) return false;
#if WITH_DEV_AUTOMATION_TESTS
	const bool bTestPresentation = HasIsolatedTestContext() && static_cast<bool>(TestNotificationPresenter);
#else
	const bool bTestPresentation = false;
#endif
	if (!bTestPresentation && (IsRunningCommandlet() || FApp::IsUnattended() || GIsAutomationTesting || !FSlateApplication::IsInitialized())) return false;
	if (PendingContextTip.IsSet())
	{
		FGorgeousNudgeEntry Tip = MoveTemp(PendingContextTip.GetValue());
		PendingContextTip.Reset();
		if (CanUseContextTips() && ContextAsset.IsValid() && ContextAsset.Get() == FindActiveTipAsset()
			&& !WasAcknowledged(Tip.Id) && IsApplicable(Tip) && PresentNotification(Tip, true)) return true;
	}

	Algo::RandomShuffle(PendingNotifications);
	for (int32 Index = 0; Index < PendingNotifications.Num(); ++Index)
	{
		const FGorgeousNudgeEntry Entry = PendingNotifications[Index];
		if (WasAcknowledged(Entry.Id) || !IsApplicable(Entry) || !IsNotificationEnabled(Entry)) continue;
		if (Entry.Scope == EGorgeousNudgeScope::NewGeneralSystem)
		{
			// News and catalog requests can finish in either order. Keep the nudge queued
			// until its downloadable target can actually be selected in the Library.
			const bool bHasCatalogTarget = GetPersistentData().GetOfflineSystemCache().ContainsByPredicate(
				[&Entry](const FGorgeousOfflineSystemCacheEntry& System)
				{
					return !System.bIsCoreSystem && !System.DownloadUrl.IsEmpty()
						&& System.SystemId.Equals(Entry.TargetId, ESearchCase::IgnoreCase);
				});
			if (!bHasCatalogTarget) continue;
		}
		if (!PresentNotification(Entry, false)) return false;
		PendingNotifications.RemoveAt(Index);
		return true;
	}
	return false;
}

bool UGorgeousNudgeSubsystem::PresentNotification(const FGorgeousNudgeEntry& Entry, bool bContextualTip)
{
	FGorgeousToastParams Params;
	Params.Title = Entry.Title.ToString();
	Params.Message = Entry.Description.ToString();
	Params.ToastIconKind = 0;
	Params.ExpireDuration = 12.0f;
	const FText Action = GetNotificationActionLabel(Entry);
	if (!Action.IsEmpty())
	{
		Params.Buttons.Add(FNotificationButtonInfo(Action, Action,
			FSimpleDelegate::CreateUObject(this, &UGorgeousNudgeSubsystem::ExecuteToastAction, Entry, bContextualTip)));
	}
	bool bPresented = false;
#if WITH_DEV_AUTOMATION_TESTS
	if (HasIsolatedTestContext() && TestNotificationPresenter) bPresented = TestNotificationPresenter(Entry, Action);
	else
#endif
	bPresented = ShowTrackedToastNotification(Params).IsValid();
	if (!bPresented) return false;
	AcknowledgedNudgeIds.Add(Entry.Id);
	SaveAcknowledgements();
	return true;
}

FText UGorgeousNudgeSubsystem::GetNotificationActionLabel(const FGorgeousNudgeEntry& Entry)
{
	if (Entry.Scope == EGorgeousNudgeScope::TutorialAvailable) return NSLOCTEXT("GorgeousNudges", "ShowTutorialWindow", "Show in new Window");
	if (Entry.Scope == EGorgeousNudgeScope::NewGeneralSystem) return NSLOCTEXT("GorgeousNudges", "ShowSystemLibrary", "Show in Library");
	if (Entry.Scope == EGorgeousNudgeScope::CommonPluginTip && GorgeousApiEndpoints::IsAllowedURL(Entry.ActionUrl))
	{
		return NSLOCTEXT("GorgeousNudges", "TipLearnMore", "Learn More");
	}
	return FText::GetEmpty();
}

FString UGorgeousNudgeSubsystem::GetDocumentUrl(const FGorgeousNudgeEntry& Entry)
{
	const FString& Url = Entry.ContentUrl.IsEmpty() ? Entry.ActionUrl : Entry.ContentUrl;
	return GorgeousApiEndpoints::IsAllowedURL(Url) ? Url : FString();
}

FVector2D UGorgeousNudgeSubsystem::ResolveWindowSize(const FGorgeousNudgeEntry& Entry)
{
	FVector2D Size = DefaultCarouselSize;
	if (FMath::IsFinite(Entry.PreferredWindowSize.X) && Entry.PreferredWindowSize.X > 0.0) Size.X = Entry.PreferredWindowSize.X;
	if (FMath::IsFinite(Entry.PreferredWindowSize.Y) && Entry.PreferredWindowSize.Y > 0.0) Size.Y = Entry.PreferredWindowSize.Y;
	Size.X = FMath::Clamp(Size.X, 720.0, 1600.0);
	Size.Y = FMath::Clamp(Size.Y, 520.0, 1100.0);
	return Size;
}

bool UGorgeousNudgeSubsystem::IsNetworkAllowed() const
{
	// Explicit offline always wins, including isolated tests.
	if (FPlatformMisc::GetEnvironmentVariable(TEXT("GORGEOUS_OFFLINE")) == TEXT("1")
		|| FParse::Param(FCommandLine::Get(), TEXT("GorgeousOffline"))) return false;
#if WITH_DEV_AUTOMATION_TESTS
	if (HasIsolatedTestContext() && TestNetworkConsent.IsSet()) return TestNetworkConsent.GetValue();
#endif
	return GorgeousNetworkConsent::IsAllowed();
}

#if WITH_DEV_AUTOMATION_TESTS
bool UGorgeousNudgeSubsystem::HasIsolatedTestContext() const
{
	return (IsRunningCommandlet() || (GIsAutomationTesting && FApp::IsUnattended())) && TestPersistentData
		&& !FPaths::IsSamePath(TestPersistentData->GetFilePath(), FGorgeousPersistentData::GetDefaultFilePath())
		&& IsLoopbackTestEndpoint(TestFeedEndpoint);
}
#endif

void UGorgeousNudgeSubsystem::ExecuteToastAction(FGorgeousNudgeEntry Entry, bool bContextualTip)
{
	if (bStopping || !IsNetworkAllowed() || !IsApplicable(Entry)) return;
	if (bContextualTip ? !CanUseContextTips() : !IsNotificationEnabled(Entry)) return;
#if WITH_DEV_AUTOMATION_TESTS
	if (HasIsolatedTestContext() && TestActionDispatcher)
	{
		TestActionDispatcher(Entry);
		return;
	}
#endif
	if (IsRunningCommandlet() || FApp::IsUnattended() || GIsAutomationTesting) return;
	if (Entry.Scope == EGorgeousNudgeScope::NewGeneralSystem)
	{
		SGorgeousLibraryView::OpenAndFocusSystem(Entry.TargetId);
		return;
	}
	if (Entry.Scope == EGorgeousNudgeScope::CommonPluginTip)
	{
		if (GorgeousApiEndpoints::IsAllowedURL(Entry.ActionUrl)) FPlatformProcess::LaunchURL(*Entry.ActionUrl, nullptr, nullptr);
		return;
	}
	if (Entry.Scope != EGorgeousNudgeScope::TutorialAvailable || !FSlateApplication::IsInitialized()) return;
	const FString Html = Entry.HtmlPage.IsEmpty() ? BuildMarkdownHtml(Entry.Title.ToString() + TEXT("\n\n") + Entry.Description.ToString()) : Entry.HtmlPage;
	const FString DocumentUrl = GetDocumentUrl(Entry);
	const bool bLoadDocumentUrl = Entry.HtmlPage.IsEmpty() && Entry.MarkdownContent.IsEmpty() && !DocumentUrl.IsEmpty();
	TSharedRef<SWebBrowser> Browser = SNew(SWebBrowser)
		.InitialURL(bLoadDocumentUrl ? DocumentUrl : TEXT("about:blank"))
		.ShowControls(false).ShowAddressBar(false)
		.ContentsToLoad(bLoadDocumentUrl ? TOptional<FString>() : TOptional<FString>(Html));
	TSharedRef<SWindow> Window = SNew(SWindow).Title(Entry.Title).ClientSize(ResolveWindowSize(Entry))
		.SupportsMinimize(false).SupportsMaximize(true)[Browser];
	Window->SetOnWindowClosed(FOnWindowClosed::CreateWeakLambda(this, [this](const TSharedRef<SWindow>& Closed)
	{
		TutorialWindows.Remove(Closed);
	}));
	TutorialWindows.Add(Window);
	FSlateApplication::Get().AddWindow(Window);
}

FGorgeousPersistentData& UGorgeousNudgeSubsystem::GetPersistentData() const
{
#if WITH_DEV_AUTOMATION_TESTS
	if (TestPersistentData) return *TestPersistentData;
#endif
	return FGorgeousPersistentData::Get();
}

#if WITH_DEV_AUTOMATION_TESTS
bool UGorgeousNudgeSubsystem::IsLoopbackTestEndpoint(const FString& Endpoint)
{
	FString AuthorityAndPath = Endpoint;
	if (!AuthorityAndPath.RemoveFromStart(TEXT("http://127.0.0.1:")) && !AuthorityAndPath.RemoveFromStart(TEXT("http://localhost:"))) return false;
	if (Endpoint.Contains(TEXT("\r")) || Endpoint.Contains(TEXT("\n")) || Endpoint.Contains(TEXT("\\"))) return false;
	int32 Separator = INDEX_NONE;
	if (!AuthorityAndPath.FindChar(TCHAR('/'), Separator) || Separator < 1 || Separator > 5) return false;
	const FString Port = AuthorityAndPath.Left(Separator);
	for (const TCHAR Character : Port)
	{
		if (Character < TCHAR('0') || Character > TCHAR('9')) return false;
	}
	const int32 PortNumber = FCString::Atoi(*Port);
	return PortNumber > 0 && PortNumber <= 65535;
}

void UGorgeousNudgeSubsystem::SetFeedEndpointForTests(const FString& Endpoint)
{
	if (IsLoopbackTestEndpoint(Endpoint)) TestFeedEndpoint = Endpoint;
}
#endif

void UGorgeousNudgeSubsystem::ShowNudgeCarousel()
{
	if (bStopping || IsRunningCommandlet() || FApp::IsUnattended() || GIsAutomationTesting || !IsNetworkAllowed() || !FSlateApplication::IsInitialized()) return;
	VisibleNudges.RemoveAll([this](const FGorgeousNudgeEntry& Entry) { return !IsApplicable(Entry); });
	if (VisibleNudges.IsEmpty())
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			NSLOCTEXT(
				"GorgeousNudges",
				"NoUpdatesOrNews",
				"There are no Gorgeous updates or news to show.\n\n"
				"The Updates & News endpoint is unavailable or has not supplied any applicable entries yet. "
				"This carousel will populate automatically when content becomes available."));
		return;
	}
	if (CarouselWindow.IsValid())
	{
		CarouselWindow->BringToFront();
		return;
	}
	Algo::RandomShuffle(VisibleNudges);
	CarouselWindow = SNew(SWindow)
		.Title(NSLOCTEXT("GorgeousNudges", "WindowTitle", "Gorgeous Updates & News"))
		.ClientSize(DefaultCarouselSize)
		.SizingRule(ESizingRule::UserSized)
		.SupportsMinimize(false)
		.SupportsMaximize(true)
		[
			SNew(SGorgeousNudgeCarousel).NudgeSubsystem(this)
		];
	CarouselWindow->SetOnWindowClosed(FOnWindowClosed::CreateUObject(this, &UGorgeousNudgeSubsystem::HandleCarouselWindowClosed));
	ResizeCarouselForEntry(VisibleNudges[0]);
	FSlateApplication::Get().AddWindow(CarouselWindow.ToSharedRef());
	// A popup is considered delivered when it is shown, not only when its link is opened.
	for (const FGorgeousNudgeEntry& Entry : VisibleNudges)
	{
		AcknowledgedNudgeIds.Add(Entry.Id);
	}
	SaveAcknowledgements();
}

void UGorgeousNudgeSubsystem::HandleCarouselWindowClosed(const TSharedRef<SWindow>& ClosedWindow)
{
	if (CarouselWindow == ClosedWindow)
	{
		CarouselWindow.Reset();
	}

	// The embedded browser may retain native pointer capture while Slate destroys its host window.
	FSlateApplication::Get().ReleaseAllPointerCapture();
}

void UGorgeousNudgeSubsystem::ResizeCarouselForEntry(const FGorgeousNudgeEntry& Entry)
{
	if (!CarouselWindow.IsValid()) return;

	CarouselWindow->Resize(ResolveWindowSize(Entry));
}

void UGorgeousNudgeSubsystem::AcknowledgeNudge(const FString& NudgeId)
{
	AcknowledgedNudgeIds.Add(NudgeId);
	VisibleNudges.RemoveAll([&NudgeId](const FGorgeousNudgeEntry& Entry) { return Entry.Id == NudgeId; });
	PendingNotifications.RemoveAll([&NudgeId](const FGorgeousNudgeEntry& Entry) { return Entry.Id == NudgeId; });
	SaveAcknowledgements();
}

bool UGorgeousNudgeSubsystem::WasAcknowledged(const FString& NudgeId) const
{
	return AcknowledgedNudgeIds.Contains(NudgeId);
}

void UGorgeousNudgeSubsystem::LoadAcknowledgements()
{
	for (const FString& Id : GetPersistentData().GetAcknowledgedNudges())
	{
		AcknowledgedNudgeIds.Add(Id);
	}
}

void UGorgeousNudgeSubsystem::LoadCachedNudgeFeed()
{
	FString FeedJson;
	if (!GetPersistentData().TryGetNudgeFeedCache(FeedJson)) return;

	TSharedPtr<FJsonObject> FeedRoot;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FeedJson);
	if (!FJsonSerializer::Deserialize(Reader, FeedRoot) || !FeedRoot.IsValid()) return;

	TArray<FGorgeousNudgeEntry> ParsedEntries;
	if (!ParseNudgeFeed(FeedRoot, ParsedEntries)) return;

	TArray<FGorgeousNudgeEntry> ReadyEntries;
	for (FGorgeousNudgeEntry& Entry : ParsedEntries)
	{
		if (!IsApplicable(Entry) || WasAcknowledged(Entry.Id) || (UsesToast(Entry) && !IsNotificationEnabled(Entry))) continue;
		if (Entry.MarkdownContent.IsEmpty() && !Entry.MarkdownUrl.IsEmpty())
		{
			FetchMarkdownNudge(MoveTemp(Entry));
		}
		else
		{
			ReadyEntries.Add(MoveTemp(Entry));
		}
	}
	RegisterNudges(ReadyEntries);
}

void UGorgeousNudgeSubsystem::SaveNudgeFeedCache(const TSharedPtr<FJsonObject>& FeedRoot) const
{
	if (!FeedRoot.IsValid()) return;
	const TArray<TSharedPtr<FJsonValue>>* Entries = nullptr;
	if (!FeedRoot->TryGetArrayField(TEXT("entries"), Entries) || !Entries) return;

	// The service receives the feed protocol, never its private persistent document.
	FString FeedJson;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&FeedJson);
	if (FJsonSerializer::Serialize(FeedRoot.ToSharedRef(), Writer))
	{
		GetPersistentData().SetNudgeFeedCache(FeedJson);
	}
}

void UGorgeousNudgeSubsystem::DetectNewGorgeousPlugins()
{
	const UGorgeousPluginHelper* PluginHelper = UGorgeousPluginHelper::GetSingleton();
	const TSet<FName> KnownGorgeousPlugins = PluginHelper ? PluginHelper->GetKnownGorgeousPlugins() : TSet<FName>();
	TSet<FString> PreviouslyKnown;
	const bool bHasSnapshot = GetPersistentData().TryGetKnownNudgePlugins(PreviouslyKnown);

	TSet<FString> CurrentPlugins;
	TArray<FGorgeousNudgeEntry> Tutorials;
	for (const FName& PluginName : KnownGorgeousPlugins)
	{
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName.ToString());
		if (!Plugin.IsValid()) continue;
		const FString PluginDisplayName = PluginName.ToString();
		CurrentPlugins.Add(PluginDisplayName);
		if (bHasSnapshot && !PreviouslyKnown.Contains(PluginDisplayName))
		{
			FGorgeousNudgeEntry Entry;
			Entry.Id = FString::Printf(TEXT("plugin-tutorial:%s:%s"), *PluginDisplayName, *Plugin->GetDescriptor().VersionName);
			Entry.Scope = EGorgeousNudgeScope::NewPluginTutorial;
			Entry.TargetId = PluginDisplayName;
			Entry.Title = FText::Format(NSLOCTEXT("GorgeousNudges", "TutorialTitle", "Getting started with {0}"), FText::FromString(PluginDisplayName));
			Entry.Description = NSLOCTEXT("GorgeousNudges", "TutorialDescription", "A Gorgeous Cast is available to help you get started.");
			Entry.ActionLabel = TEXT("Watch Gorgeous Cast");
			Entry.ActionUrl = FString::Printf(TEXT("https://gorgeous.simsalabim.studio/casts?plugin=%s"), *FGenericPlatformHttp::UrlEncode(PluginDisplayName));
			Tutorials.Add(MoveTemp(Entry));
		}
	}
	GetPersistentData().SetKnownNudgePlugins(CurrentPlugins);
	if (!Tutorials.IsEmpty()) RegisterNudges(Tutorials);
}

void UGorgeousNudgeSubsystem::DetectNewGorgeousGeneralSystems()
{
	TSet<FString> PreviouslyKnown;
	const bool bHasSnapshot = GetPersistentData().TryGetKnownNudgeGeneralSystems(PreviouslyKnown);
	TArray<FGorgeousNudgeEntry> Nudges;
	TSet<FString> CurrentSystemIds;
	if (const UGorgeousPluginHelper* PluginHelper = UGorgeousPluginHelper::GetSingleton())
	{
		for (const FString& SystemId : GetPersistentData().GetRecordedInstalledSystems(PluginHelper->GetInstalledPluginSystemEntries()))
		{
			if (!SystemId.StartsWith(TEXT("Plugin:"), ESearchCase::IgnoreCase))
			{
				CurrentSystemIds.Add(SystemId);
			}
		}
	}

	for (const FString& SystemId : CurrentSystemIds)
	{
		if (bHasSnapshot && !PreviouslyKnown.Contains(SystemId))
		{
			FGorgeousNudgeEntry Entry;
			Entry.Id = FString::Printf(TEXT("system-tutorial:%s"), *SystemId);
			Entry.Scope = EGorgeousNudgeScope::GeneralSystemUpdate;
			Entry.TargetId = SystemId;
			Entry.Title = FText::Format(NSLOCTEXT("GorgeousNudges", "SystemTutorialTitle", "New Gorgeous system: {0}"), FText::FromString(SystemId));
			Entry.Description = NSLOCTEXT("GorgeousNudges", "SystemTutorialDescription", "Learn more about this newly installed Gorgeous system.");
			Entry.ActionLabel = TEXT("Learn more");
			Entry.ActionUrl = FString::Printf(TEXT("https://gorgeous.simsalabim.studio/systems/%s"), *FGenericPlatformHttp::UrlEncode(SystemId));
			Nudges.Add(MoveTemp(Entry));
		}
	}
	GetPersistentData().SetKnownNudgeGeneralSystems(CurrentSystemIds);
	if (!Nudges.IsEmpty()) RegisterNudges(Nudges);
}

void UGorgeousNudgeSubsystem::SaveAcknowledgements() const
{
	GetPersistentData().SetAcknowledgedNudges(AcknowledgedNudgeIds);
}

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousContextAssetCategoryTest,
	"GorgeousCore.Editor.OnlineServices.ContextAssetCategory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGorgeousContextAssetCategoryTest::RunTest(const FString&)
{
	TestEqual(TEXT("A Blueprint reports only its fixed category"),
		FString(GetContextAssetType(*GetDefault<UBlueprint>())), FString(TEXT("blueprint")));
	TestEqual(TEXT("A static mesh reports only its fixed category"),
		FString(GetContextAssetType(*GetDefault<UStaticMesh>())), FString(TEXT("static_mesh")));
	TestNull(TEXT("Unsupported object types supply no context"), GetContextAssetType(*GetDefault<UObject>()));
	return true;
}
#endif