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
#include "Helpers/GorgeousPluginHelper.h"
#include "LibraryWizard/GorgeousUpdateManager.h"
#include "NudgeHelper/GorgeousNudgeDeveloperSettings.h"
#include "NudgeHelper/SGorgeousNudgeCarousel.h"
#include "Containers/Ticker.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "HttpModule.h"
#include "Interfaces/IPluginManager.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Widgets/SWindow.h"

//=============================================================================
// Helpers
//=============================================================================
namespace
{
	const TCHAR* AcknowledgedNudgesField = TEXT("AcknowledgedNudges");
	const TCHAR* NudgeFeedCacheField = TEXT("NudgeFeedCache");
	const TCHAR* NudgeFeedCacheSchemaVersionField = TEXT("NudgeFeedCacheSchemaVersion");
	const TCHAR* NudgeFeedCacheUpdatedAtField = TEXT("NudgeFeedCacheUpdatedAt");
	const TCHAR* NudgeFeedEndpoint = TEXT("https://api.gorgeous.simsalabim.studio/news");
	const FVector2D DefaultCarouselSize(1100.0f, 760.0f);

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
		return false;
	}

	bool ParseNudgeFeed(const TSharedPtr<FJsonObject>& FeedRoot, TArray<FGorgeousNudgeEntry>& OutEntries)
	{
		if (!FeedRoot.IsValid()) return false;
		double SchemaVersion = 0.0;
		if (!FeedRoot->TryGetNumberField(TEXT("schemaVersion"), SchemaVersion) || SchemaVersion < 1.0) return false;

		const TArray<TSharedPtr<FJsonValue>>* JsonEntries = nullptr;
		if (!FeedRoot->TryGetArrayField(TEXT("entries"), JsonEntries) || !JsonEntries) return false;
		for (const TSharedPtr<FJsonValue>& JsonValue : *JsonEntries)
		{
			const TSharedPtr<FJsonObject> JsonEntry = JsonValue.IsValid() ? JsonValue->AsObject() : nullptr;
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
				else if (ContentType.Equals(TEXT("url"), ESearchCase::IgnoreCase) && Entry.ActionUrl.IsEmpty())
				{
					Entry.ActionUrl = GetJsonString(*ContentObject, TEXT("url"));
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
}

//=============================================================================
// UGorgeousNudgeSubsystem Implementation
//=============================================================================

void UGorgeousNudgeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadAcknowledgements();
	DetectNewGorgeousPlugins();
	LoadCachedNudgeFeed();
	CheckForNudgesNow();
	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UGorgeousNudgeSubsystem::Tick), 1.0f);
}

void UGorgeousNudgeSubsystem::Deinitialize()
{
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
	}
	if (CarouselWindow.IsValid())
	{
		CarouselWindow->RequestDestroyWindow();
	}
	Super::Deinitialize();
}

bool UGorgeousNudgeSubsystem::Tick(float DeltaTime)
{
	const UGorgeousNudgeDeveloperSettings* Settings = UGorgeousNudgeDeveloperSettings::Get();
	if (!Settings || !Settings->bEnableAutomaticNudges)
	{
		return true;
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
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(NudgeFeedEndpoint);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->OnProcessRequestComplete().BindLambda([WeakSubsystem = TWeakObjectPtr<UGorgeousNudgeSubsystem>(this)](FHttpRequestPtr, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		UGorgeousNudgeSubsystem* Subsystem = WeakSubsystem.Get();
		if (!Subsystem || !bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode())) return;

		TSharedPtr<FJsonObject> FeedRoot;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (!FJsonSerializer::Deserialize(Reader, FeedRoot) || !FeedRoot.IsValid()) return;

		TArray<FGorgeousNudgeEntry> ParsedEntries;
		if (!ParseNudgeFeed(FeedRoot, ParsedEntries)) return;
		Subsystem->SaveNudgeFeedCache(FeedRoot);

		TArray<FGorgeousNudgeEntry> ReadyEntries;
		for (FGorgeousNudgeEntry& Entry : ParsedEntries)
		{
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
	Request->ProcessRequest();
}

void UGorgeousNudgeSubsystem::FetchMarkdownNudge(FGorgeousNudgeEntry Entry)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Entry.MarkdownUrl);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Accept"), TEXT("text/markdown, text/plain"));
	Request->OnProcessRequestComplete().BindLambda([WeakSubsystem = TWeakObjectPtr<UGorgeousNudgeSubsystem>(this), Entry = MoveTemp(Entry)](FHttpRequestPtr, FHttpResponsePtr Response, bool bWasSuccessful) mutable
	{
		UGorgeousNudgeSubsystem* Subsystem = WeakSubsystem.Get();
		if (!Subsystem) return;
		if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
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
	Request->ProcessRequest();
}

void UGorgeousNudgeSubsystem::HandlePluginUpdateCheckCompleted(bool bUpdatesAvailable)
{
	if (!bUpdatesAvailable) return;
	UGorgeousPluginHelper* PluginHelper = UGorgeousPluginHelper::GetSingleton();
	if (!PluginHelper) return;

	TArray<FGorgeousNudgeEntry> Updates;
	for (const FGorgeousPluginUpdateCacheEntry& Update : PluginHelper->GetPluginUpdateCache())
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
		Entry.PluginDownloadToken = PluginHelper->GetPluginUpdateDownloadToken(FName(*Update.PluginName));
		Updates.Add(MoveTemp(Entry));
	}
	RegisterNudges(Updates);
}

void UGorgeousNudgeSubsystem::RegisterNudges(const TArray<FGorgeousNudgeEntry>& Entries)
{
	for (const FGorgeousNudgeEntry& Entry : Entries)
	{
		VisibleNudges.RemoveAll([&Entry](const FGorgeousNudgeEntry& Existing) { return Existing.Id == Entry.Id; });
		if (!Entry.Id.IsEmpty() && IsApplicable(Entry) && !WasAcknowledged(Entry.Id))
		{
			VisibleNudges.Add(Entry);
		}
	}
	if (UGorgeousNudgeDeveloperSettings::Get()->bEnableAutomaticNudges && VisibleNudges.Num() > 0)
	{
		ShowNudgeCarousel();
	}
}

bool UGorgeousNudgeSubsystem::IsApplicable(const FGorgeousNudgeEntry& Entry) const
{
	if (Entry.Scope == EGorgeousNudgeScope::EcosystemNews)
	{
		return true;
	}
	if (Entry.Scope == EGorgeousNudgeScope::GeneralSystemUpdate)
	{
		if (const UGorgeousPluginHelper* PluginHelper = UGorgeousPluginHelper::GetSingleton())
		{
			return PluginHelper->GetRecordedInstalledSystems().ContainsByPredicate([&Entry](const FString& SystemId)
			{
				return SystemId.Equals(Entry.TargetId, ESearchCase::IgnoreCase);
			});
		}
		return false;
	}
	if (Entry.TargetId.IsEmpty())
	{
		return false;
	}
	if (Entry.Scope == EGorgeousNudgeScope::NewPluginTutorial && !UGorgeousNudgeDeveloperSettings::Get()->bShowTutorialForNewPlugins)
	{
		return false;
	}
	const UGorgeousPluginHelper* PluginHelper = UGorgeousPluginHelper::GetSingleton();
	const TSet<FName> KnownGorgeousPlugins = PluginHelper ? PluginHelper->GetKnownGorgeousPlugins() : TSet<FName>();
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(Entry.TargetId);
	return Plugin.IsValid() && (KnownGorgeousPlugins.Contains(FName(*Entry.TargetId)) || Entry.TargetId.StartsWith(TEXT("Gorgeous"), ESearchCase::IgnoreCase));
}

void UGorgeousNudgeSubsystem::ShowNudgeCarousel()
{
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
	CarouselWindow = SNew(SWindow)
		.Title(NSLOCTEXT("GorgeousNudges", "WindowTitle", "Gorgeous Updates & News"))
		.ClientSize(DefaultCarouselSize)
		.SizingRule(ESizingRule::UserSized)
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		[
			SNew(SGorgeousNudgeCarousel).NudgeSubsystem(this)
		];
	ResizeCarouselForEntry(VisibleNudges[0]);
	FSlateApplication::Get().AddWindow(CarouselWindow.ToSharedRef());
	// A popup is considered delivered when it is shown, not only when its link is opened.
	for (const FGorgeousNudgeEntry& Entry : VisibleNudges)
	{
		AcknowledgedNudgeIds.Add(Entry.Id);
	}
	SaveAcknowledgements();
}

void UGorgeousNudgeSubsystem::ResizeCarouselForEntry(const FGorgeousNudgeEntry& Entry)
{
	if (!CarouselWindow.IsValid()) return;

	FVector2D RequestedSize = DefaultCarouselSize;
	if (!Entry.MarkdownContent.IsEmpty())
	{
		if (Entry.PreferredWindowSize.X > 0.0f) RequestedSize.X = Entry.PreferredWindowSize.X;
		if (Entry.PreferredWindowSize.Y > 0.0f) RequestedSize.Y = Entry.PreferredWindowSize.Y;
	}
	RequestedSize.X = FMath::Clamp(RequestedSize.X, 720.0f, 1600.0f);
	RequestedSize.Y = FMath::Clamp(RequestedSize.Y, 520.0f, 1100.0f);
	CarouselWindow->Resize(RequestedSize);
}

void UGorgeousNudgeSubsystem::AcknowledgeNudge(const FString& NudgeId)
{
	AcknowledgedNudgeIds.Add(NudgeId);
	VisibleNudges.RemoveAll([&NudgeId](const FGorgeousNudgeEntry& Entry) { return Entry.Id == NudgeId; });
	SaveAcknowledgements();
}

bool UGorgeousNudgeSubsystem::WasAcknowledged(const FString& NudgeId) const
{
	return AcknowledgedNudgeIds.Contains(NudgeId);
}

void UGorgeousNudgeSubsystem::LoadAcknowledgements()
{
	FString Contents;
	const FString Path = UGorgeousPluginHelper::GetGorgeousPersistentDataFilePath();
	if (!FFileHelper::LoadFileToString(Contents, *Path)) return;
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Contents);
	if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
	{
		const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
		if (Root->TryGetArrayField(AcknowledgedNudgesField, Values) && Values)
		{
			for (const TSharedPtr<FJsonValue>& Value : *Values) AcknowledgedNudgeIds.Add(Value->AsString());
		}
	}
}

void UGorgeousNudgeSubsystem::LoadCachedNudgeFeed()
{
	FString Contents;
	const FString Path = UGorgeousPluginHelper::GetGorgeousPersistentDataFilePath();
	if (!FFileHelper::LoadFileToString(Contents, *Path)) return;

	TSharedPtr<FJsonObject> PersistentData;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Contents);
	if (!FJsonSerializer::Deserialize(Reader, PersistentData) || !PersistentData.IsValid()) return;

	const TArray<TSharedPtr<FJsonValue>>* CachedEntries = nullptr;
	if (!PersistentData->TryGetArrayField(NudgeFeedCacheField, CachedEntries) || !CachedEntries) return;

	TSharedPtr<FJsonObject> FeedRoot = MakeShared<FJsonObject>();
	double SchemaVersion = 1.0;
	PersistentData->TryGetNumberField(NudgeFeedCacheSchemaVersionField, SchemaVersion);
	FeedRoot->SetNumberField(TEXT("schemaVersion"), SchemaVersion);
	FeedRoot->SetArrayField(TEXT("entries"), *CachedEntries);

	TArray<FGorgeousNudgeEntry> ParsedEntries;
	if (!ParseNudgeFeed(FeedRoot, ParsedEntries)) return;

	TArray<FGorgeousNudgeEntry> ReadyEntries;
	for (FGorgeousNudgeEntry& Entry : ParsedEntries)
	{
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

	const FString Path = UGorgeousPluginHelper::GetGorgeousPersistentDataFilePath();
	FString Contents;
	TSharedPtr<FJsonObject> PersistentData = MakeShared<FJsonObject>();
	if (FFileHelper::LoadFileToString(Contents, *Path))
	{
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Contents);
		FJsonSerializer::Deserialize(Reader, PersistentData);
		if (!PersistentData.IsValid()) PersistentData = MakeShared<FJsonObject>();
	}

	double SchemaVersion = 1.0;
	FeedRoot->TryGetNumberField(TEXT("schemaVersion"), SchemaVersion);
	PersistentData->SetNumberField(NudgeFeedCacheSchemaVersionField, SchemaVersion);
	PersistentData->SetStringField(NudgeFeedCacheUpdatedAtField, FDateTime::UtcNow().ToIso8601());
	PersistentData->SetArrayField(NudgeFeedCacheField, *Entries);

	FString Output;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
	FJsonSerializer::Serialize(PersistentData.ToSharedRef(), Writer);
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
	FFileHelper::SaveStringToFile(Output, *Path);
}

void UGorgeousNudgeSubsystem::DetectNewGorgeousPlugins()
{
	const UGorgeousPluginHelper* PluginHelper = UGorgeousPluginHelper::GetSingleton();
	const TSet<FName> KnownGorgeousPlugins = PluginHelper ? PluginHelper->GetKnownGorgeousPlugins() : TSet<FName>();
	const FString Path = UGorgeousPluginHelper::GetGorgeousPersistentDataFilePath();
	FString Contents;
	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	const bool bHasPersistentData = FFileHelper::LoadFileToString(Contents, *Path);
	if (bHasPersistentData)
	{
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Contents);
		FJsonSerializer::Deserialize(Reader, Root);
		if (!Root.IsValid()) Root = MakeShared<FJsonObject>();
	}

	TSet<FString> PreviouslyKnown;
	const TArray<TSharedPtr<FJsonValue>>* SavedPlugins = nullptr;
	const bool bHasSnapshot = Root->TryGetArrayField(TEXT("KnownNudgePlugins"), SavedPlugins) && SavedPlugins;
	if (bHasSnapshot)
	{
		for (const TSharedPtr<FJsonValue>& Value : *SavedPlugins) PreviouslyKnown.Add(Value->AsString());
	}

	TArray<TSharedPtr<FJsonValue>> CurrentPlugins;
	TArray<FGorgeousNudgeEntry> Tutorials;
	for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetEnabledPlugins())
	{
		const FString PluginName = Plugin->GetName();
		const bool bIsKnownGorgeousPlugin = KnownGorgeousPlugins.Contains(FName(*PluginName));
		if (!bIsKnownGorgeousPlugin && !PluginName.StartsWith(TEXT("Gorgeous"), ESearchCase::IgnoreCase)) continue;
		CurrentPlugins.Add(MakeShared<FJsonValueString>(PluginName));
		if (bHasSnapshot && !PreviouslyKnown.Contains(PluginName))
		{
			FGorgeousNudgeEntry Entry;
			Entry.Id = FString::Printf(TEXT("plugin-tutorial:%s:%s"), *PluginName, *Plugin->GetDescriptor().VersionName);
			Entry.Scope = EGorgeousNudgeScope::NewPluginTutorial;
			Entry.TargetId = PluginName;
			Entry.Title = FText::Format(NSLOCTEXT("GorgeousNudges", "TutorialTitle", "Getting started with {0}"), FText::FromString(PluginName));
			Entry.Description = NSLOCTEXT("GorgeousNudges", "TutorialDescription", "A Gorgeous Cast is available to help you get started.");
			Entry.ActionLabel = TEXT("Watch Gorgeous Cast");
			Entry.ActionUrl = FString::Printf(TEXT("https://gorgeous.simsalabim.studio/casts?plugin=%s"), *FGenericPlatformHttp::UrlEncode(PluginName));
			Tutorials.Add(MoveTemp(Entry));
		}
	}
	Root->SetArrayField(TEXT("KnownNudgePlugins"), CurrentPlugins);
	FString Output;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
	FFileHelper::SaveStringToFile(Output, *Path);
	if (!Tutorials.IsEmpty()) RegisterNudges(Tutorials);
}

void UGorgeousNudgeSubsystem::SaveAcknowledgements() const
{
	const FString Path = UGorgeousPluginHelper::GetGorgeousPersistentDataFilePath();
	FString Contents;
	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	if (FFileHelper::LoadFileToString(Contents, *Path))
	{
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Contents);
		FJsonSerializer::Deserialize(Reader, Root);
		if (!Root.IsValid()) Root = MakeShared<FJsonObject>();
	}
	TArray<TSharedPtr<FJsonValue>> Values;
	for (const FString& Id : AcknowledgedNudgeIds) Values.Add(MakeShared<FJsonValueString>(Id));
	Root->SetArrayField(AcknowledgedNudgesField, Values);
	FString Output;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
	FFileHelper::SaveStringToFile(Output, *Path);
}
