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
#include "Helpers/GorgeousPersistentData.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousPersistentDataJson.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "../../../../Shared/GorgeousPersistentDataFile.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// Helpers
//=============================================================================

namespace GorgeousPersistentDataJson
{
	TSharedRef<FJsonObject> ObjectField(FJsonObject& Parent, const FString& Name)
	{
		const TSharedPtr<FJsonObject>* Existing = nullptr;
		if (Parent.TryGetObjectField(Name, Existing) && Existing && Existing->IsValid()) return Existing->ToSharedRef();
		const TSharedRef<FJsonObject> Created = MakeShared<FJsonObject>();
		Parent.SetObjectField(Name, Created);
		return Created;
	}

	TArray<FString> ReadInstalledSystems(const FJsonObject& Data)
	{
		TArray<FString> Result;
		const TSharedPtr<FJsonObject>* Validation = nullptr;
		const TArray<TSharedPtr<FJsonValue>>* Entries = nullptr;
		if (Data.TryGetObjectField(TEXT("SystemValidation"), Validation) && Validation && Validation->IsValid() &&
			(*Validation)->TryGetArrayField(TEXT("InstalledSystems"), Entries))
			for (const TSharedPtr<FJsonValue>& Entry : *Entries)
			{
				FString Value;
				if (Entry.IsValid() && Entry->TryGetString(Value)) Result.AddUnique(Value);
			}
		return Result;
	}

	void WriteInstalledSystems(FJsonObject& Data, const TArray<FString>& Systems)
	{
		TArray<TSharedPtr<FJsonValue>> Values;
		for (const FString& System : Systems) Values.Add(MakeShared<FJsonValueString>(System));
		const TSharedRef<FJsonObject> Validation = ObjectField(Data, TEXT("SystemValidation"));
		Validation->SetArrayField(TEXT("InstalledSystems"), Values);
		Validation->SetNumberField(TEXT("InstalledPluginsDiscoveryVersion"), 1);
	}

	bool BackfillInstalledPlugins(FJsonObject& Data, const TArray<FString>& Plugins)
	{
		const TSharedPtr<FJsonObject>* Validation = nullptr;
		const TArray<TSharedPtr<FJsonValue>>* Existing = nullptr;
		double Version = 0;
		if (!Data.TryGetObjectField(TEXT("SystemValidation"), Validation) || !Validation || !Validation->IsValid() ||
			!(*Validation)->TryGetArrayField(TEXT("InstalledSystems"), Existing)) return false;
		if ((*Validation)->TryGetNumberField(TEXT("InstalledPluginsDiscoveryVersion"), Version) && Version >= 1) return false;
		TArray<FString> Systems = ReadInstalledSystems(Data);
		if (Systems.IsEmpty()) return false; // A first recording still establishes the whole baseline.
		for (const FString& Plugin : Plugins) Systems.AddUnique(Plugin);
		WriteInstalledSystems(Data, Systems);
		return true;
	}

}

namespace GorgeousPersistentDataJson
{
	bool ComputeBinaryDirectoryChecksum(const FString& Directory, FString& OutChecksum)
	{
		OutChecksum.Reset();
		if (!FPaths::DirectoryExists(Directory)) return false;
		TArray<FString> FoundFiles;
#if PLATFORM_WINDOWS
		IFileManager::Get().FindFilesRecursive(FoundFiles, *Directory, TEXT("*.dll"), true, false);
#elif PLATFORM_MAC
		IFileManager::Get().FindFilesRecursive(FoundFiles, *Directory, TEXT("*.dylib"), true, false);
#else
		IFileManager::Get().FindFilesRecursive(FoundFiles, *Directory, TEXT("*.so"), true, false);
#endif
		// An empty folder is not a verified binary distribution.
		if (FoundFiles.IsEmpty()) return false;
		FoundFiles.Sort();
		FString CombinedHashes;
		for (const FString& FilePath : FoundFiles)
		{
			const FMD5Hash Hash = FMD5Hash::HashFile(*FilePath);
			if (!Hash.IsValid()) return false;
			CombinedHashes += LexToString(Hash);
		}
		FMD5 FinalMD5;
		FinalMD5.Update(reinterpret_cast<const uint8*>(TCHAR_TO_UTF8(*CombinedHashes)), CombinedHashes.Len());
		FMD5Hash FinalHash;
		FinalHash.Set(FinalMD5);
		OutChecksum = LexToString(FinalHash);
		return true;
	}
}

/**
 * Hashes the discovered plugin descriptor without changing the metadata document.
 *
 * @param Plugin The discovered plugin whose descriptor is fingerprinted.
 * @return The descriptor digest, or an empty string when it cannot be verified.
 */
static FString PluginDescriptorFingerprint(const TSharedPtr<IPlugin>& Plugin)
{
	if (!Plugin.IsValid()) return FString();
	const FMD5Hash Hash = FMD5Hash::HashFile(*Plugin->GetDescriptorFileName());
	return Hash.IsValid() ? LexToString(Hash) : FString();
}

namespace
{
    // The stored field that records the first resource-protection banner display.
    constexpr const TCHAR* ResourceProtectionBannerShownKey = TEXT("ResourceProtectionBannerShown");

    /**
     * Reads an existing JSON owner object without creating or replacing fields.
     *
     * @param Parent The JSON object that may contain the requested field.
     * @param Field The name of the existing object field to read.
     * @return The stored object, or an empty pointer when it does not exist.
     */
    TSharedPtr<FJsonObject> FindPersistentObject(const TSharedPtr<FJsonObject>& Parent, const FString& Field)
    {
        const TSharedPtr<FJsonObject>* Found = nullptr;
        return Parent.IsValid() && Parent->TryGetObjectField(Field, Found) && Found ? *Found : nullptr;
    }
}

//=============================================================================
// FGorgeousPersistentData Implementation
//=============================================================================

bool FGorgeousPersistentData::bAbortBinaryChecksumSave = false;

FGorgeousPersistentData::FGorgeousPersistentData(FString InFilePath)
    : FilePath(InFilePath.IsEmpty() ? GetDefaultFilePath() : MoveTemp(InFilePath))
{
}

FGorgeousPersistentData::~FGorgeousPersistentData() = default;

FGorgeousPersistentData& FGorgeousPersistentData::Get()
{
    static FGorgeousPersistentData Instance;
    return Instance;
}

FString FGorgeousPersistentData::GetDefaultFilePath()
{
    return GorgeousPersistentDataFile::DefaultPath();
}

void FGorgeousPersistentData::MarkDirty(const FString& Root, const FString& Child)
{
    DirtyFields.FindOrAdd(Root).Add(Child);
}

void FGorgeousPersistentData::LoadPersistentData()
{
    if (bLoadedPersistentData) return;
    bLoadedPersistentData = true;
    const auto Result = GorgeousPersistentDataFile::ReadObject(FilePath, PersistentDataObject);
    if (Result == GorgeousPersistentDataFile::EReadResult::Missing)
    {
        PersistentDataObject = MakeShared<FJsonObject>();
        const auto Validation = GorgeousPersistentDataJson::ObjectField(*PersistentDataObject, TEXT("SystemValidation"));
        Validation->SetNumberField(TEXT("ValidationCount"), 0);
        Validation->SetNumberField(TEXT("ValidationInterval"), 10);
        Validation->SetBoolField(TEXT("HasRunInitialValidation"), false);
        PersistentDataObject->SetArrayField(TEXT("CircularDependencyNotifications"), {});
        MarkDirty(TEXT("SystemValidation"));
        MarkDirty(TEXT("CircularDependencyNotifications"));
        SavePersistentData();
        return;
    }
    if (Result == GorgeousPersistentDataFile::EReadResult::Loaded)
    {
        const TArray<TSharedPtr<FJsonValue>>* Pairs = nullptr;
        if (PersistentDataObject->TryGetArrayField(TEXT("CircularDependencyNotifications"), Pairs))
            for (const auto& Value : *Pairs)
            {
                FString Pair;
                if (Value.IsValid() && Value->TryGetString(Pair)) NotifiedCircularDependencyPairs.Add(Pair);
            }
        GT_I_LOG("GT.Core.PluginHelper", TEXT("Loaded persistent Gorgeous data from disk (%d circular dependency records)"), NotifiedCircularDependencyPairs.Num());
        return;
    }
    bPersistentDataLoadFailed = true;
    if (Result == GorgeousPersistentDataFile::EReadResult::ParseFailed)
        GT_E_LOG("GT.Core.PluginHelper.PersistentData.ParseFailed", TEXT("Could not parse Gorgeous persistent data at %s. The existing file is preserved; restore valid JSON, then restart the editor or game before saving metadata."), *FilePath);
    else
        GT_E_LOG("GT.Core.PluginHelper.PersistentData.ReadFailed", TEXT("Could not read Gorgeous persistent data at %s. Check file permissions, then restart the editor or game; metadata writes are disabled to preserve the existing file."), *FilePath);
}

void FGorgeousPersistentData::SavePersistentData()
{
    LoadPersistentData();
    if (bPersistentDataLoadFailed || !PersistentDataObject.IsValid() || DirtyFields.IsEmpty()) return;
    TSharedPtr<FJsonObject> Latest;
    const auto Result = GorgeousPersistentDataFile::ReadObject(FilePath, Latest);
    if (Result == GorgeousPersistentDataFile::EReadResult::ReadFailed || Result == GorgeousPersistentDataFile::EReadResult::ParseFailed)
    {
        bPersistentDataLoadFailed = true;
        if (Result == GorgeousPersistentDataFile::EReadResult::ParseFailed)
            GT_E_LOG("GT.Core.PluginHelper.PersistentData.ParseFailed", TEXT("Could not parse Gorgeous persistent data at %s. The existing file is preserved; restore valid JSON, then restart the editor or game before saving metadata."), *FilePath);
        else
            GT_E_LOG("GT.Core.PluginHelper.PersistentData.ReadFailed", TEXT("Could not read Gorgeous persistent data at %s. Check file permissions, then restart the editor or game; metadata writes are disabled to preserve the existing file."), *FilePath);
        return;
    }
    if (!Latest.IsValid()) Latest = MakeShared<FJsonObject>();
    for (const auto& Root : DirtyFields)
    {
        if (Root.Value.Contains(FString()))
        {
            const auto Value = PersistentDataObject->TryGetField(Root.Key);
            if (Value.IsValid()) Latest->SetField(Root.Key, Value);
            else Latest->RemoveField(Root.Key);
        }
        else
        {
            const auto Source = FindPersistentObject(PersistentDataObject, Root.Key);
            const auto Target = GorgeousPersistentDataJson::ObjectField(*Latest, Root.Key);
            for (const FString& Child : Root.Value)
            {
                const auto Value = Source.IsValid() ? Source->TryGetField(Child) : nullptr;
                if (Value.IsValid()) Target->SetField(Child, Value);
                else Target->RemoveField(Child);
            }
        }
    }
    FString Output;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
    if (!FJsonSerializer::Serialize(Latest.ToSharedRef(), Writer))
    {
        GT_E_LOG("GT.Core.PluginHelper.PersistentData.SerializeFailed", TEXT("Could not serialize Gorgeous persistent data for %s. The existing file has not been replaced."), *FilePath);
        return;
    }
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(FilePath), true);
    if (!FFileHelper::SaveStringToFile(Output, *FilePath))
    {
        GT_E_LOG("GT.Core.PluginHelper.PersistentData.WriteFailed", TEXT("Could not save Gorgeous persistent data to %s. Check directory permissions and available disk space."), *FilePath);
        return;
    }
    PersistentDataObject = MoveTemp(Latest);
    DirtyFields.Reset();
    GT_I_LOG("GT.Core.PluginHelper", TEXT("Saved persistent Gorgeous data to disk"));
}

TArray<FString> FGorgeousPersistentData::GetRecordedInstalledSystems(const TArray<FString>& InstalledPlugins) const
{
    auto* Mutable = const_cast<FGorgeousPersistentData*>(this);
    Mutable->LoadPersistentData();
    if (PersistentDataObject.IsValid() && GorgeousPersistentDataJson::BackfillInstalledPlugins(*PersistentDataObject, InstalledPlugins))
    {
        Mutable->MarkDirty(TEXT("SystemValidation"), TEXT("InstalledSystems"));
        Mutable->MarkDirty(TEXT("SystemValidation"), TEXT("InstalledPluginsDiscoveryVersion"));
        Mutable->SavePersistentData();
    }
    return PersistentDataObject.IsValid() ? GorgeousPersistentDataJson::ReadInstalledSystems(*PersistentDataObject) : TArray<FString>();
}

void FGorgeousPersistentData::RecordInstalledSystems(const TArray<FString>& Systems)
{
    LoadPersistentData();
    if (!PersistentDataObject.IsValid()) PersistentDataObject = MakeShared<FJsonObject>();
    GorgeousPersistentDataJson::WriteInstalledSystems(*PersistentDataObject, Systems);
    MarkDirty(TEXT("SystemValidation"), TEXT("InstalledSystems"));
    MarkDirty(TEXT("SystemValidation"), TEXT("InstalledPluginsDiscoveryVersion"));
    SavePersistentData();
}

void FGorgeousPersistentData::GenerateAndSaveBinaryChecksums(const TSet<FName>& InstalledPlugins)
{
    if (bAbortBinaryChecksumSave)
    {
        GT_W_LOG("GT.Core.PluginHelper", TEXT("Binary checksum generation aborted due to previous validation errors."));
        return;
    }
    LoadPersistentData();
    if (!PersistentDataObject.IsValid()) PersistentDataObject = MakeShared<FJsonObject>();
    const auto Checksums = GorgeousPersistentDataJson::ObjectField(*PersistentDataObject, TEXT("VerifiedBinaryChecksums"));
    const auto Descriptors = GorgeousPersistentDataJson::ObjectField(*PersistentDataObject, TEXT("VerifiedBinaryChecksumDescriptors"));
    int32 VerifiedCount = 0;
    for (const FName& Name : InstalledPlugins)
    {
        FString Checksum;
        const FString Key = Name.ToString();
        const FString Descriptor = PluginDescriptorFingerprint(IPluginManager::Get().FindPlugin(Key));
        if (!Descriptor.IsEmpty() && TryComputePluginBinaryChecksum(Name, Checksum))
        {
            Checksums->SetStringField(Key, Checksum);
            Descriptors->SetStringField(Key, Descriptor);
            ++VerifiedCount;
        }
        else
        {
            Checksums->RemoveField(Key);
            Descriptors->RemoveField(Key);
        }
        MarkDirty(TEXT("VerifiedBinaryChecksums"), Key);
        MarkDirty(TEXT("VerifiedBinaryChecksumDescriptors"), Key);
    }
    SavePersistentData();
    GT_I_LOG("GT.Core.PluginHelper", TEXT("Saved verified binary checksums for %d installed Gorgeous plugins."), VerifiedCount);
}

void FGorgeousPersistentData::InvalidatePersistedChecksum(const TSet<FName>& InstalledPlugins)
{
    LoadPersistentData();
    if (!PersistentDataObject.IsValid()) PersistentDataObject = MakeShared<FJsonObject>();
    const auto Checksums = GorgeousPersistentDataJson::ObjectField(*PersistentDataObject, TEXT("VerifiedBinaryChecksums"));
    const auto Descriptors = GorgeousPersistentDataJson::ObjectField(*PersistentDataObject, TEXT("VerifiedBinaryChecksumDescriptors"));
    for (const FName& Name : InstalledPlugins)
    {
        Checksums->RemoveField(Name.ToString());
        Descriptors->RemoveField(Name.ToString());
        MarkDirty(TEXT("VerifiedBinaryChecksums"), Name.ToString());
        MarkDirty(TEXT("VerifiedBinaryChecksumDescriptors"), Name.ToString());
    }
    Checksums->SetStringField(TEXT("GorgeousCore"), TEXT(""));
    Descriptors->RemoveField(TEXT("GorgeousCore"));
    MarkDirty(TEXT("VerifiedBinaryChecksums"), TEXT("GorgeousCore"));
    MarkDirty(TEXT("VerifiedBinaryChecksumDescriptors"), TEXT("GorgeousCore"));
    if (PersistentDataObject->HasField(TEXT("PluginUpdateCache")))
    {
        PersistentDataObject->SetArrayField(TEXT("PluginUpdateCache"), {});
        MarkDirty(TEXT("PluginUpdateCache"));
    }
    SavePersistentData();
    GT_I_LOG("GT.Core.PluginHelper", TEXT("Invalidated persisted Gorgeous plugin binary checksums and plugin update cache."));
}

bool FGorgeousPersistentData::MarkCircularDependencyNotified(const FString& PairKey)
{
    LoadPersistentData();
    if (NotifiedCircularDependencyPairs.Contains(PairKey)) return false;
    NotifiedCircularDependencyPairs.Add(PairKey);
    SetStringSet(TEXT("CircularDependencyNotifications"), NotifiedCircularDependencyPairs);
    return true;
}

bool FGorgeousPersistentData::TryGetStringSet(const FString& Field, TSet<FString>& OutValues) const
{
    const_cast<FGorgeousPersistentData*>(this)->LoadPersistentData();
    OutValues.Reset();
    const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
    if (!PersistentDataObject.IsValid() || !PersistentDataObject->TryGetArrayField(Field, Values)) return false;
    for (const auto& Entry : *Values)
    {
        FString Value;
        if (Entry.IsValid() && Entry->TryGetString(Value)) OutValues.Add(Value);
    }
    return true;
}

void FGorgeousPersistentData::SetStringSet(const FString& Field, const TSet<FString>& Values)
{
    LoadPersistentData();
    if (!PersistentDataObject.IsValid()) PersistentDataObject = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> Entries;
    for (const FString& Value : Values) Entries.Add(MakeShared<FJsonValueString>(Value));
    PersistentDataObject->SetArrayField(Field, Entries);
    MarkDirty(Field);
    SavePersistentData();
}

TSet<FString> FGorgeousPersistentData::GetAcknowledgedNudges() const
{
    TSet<FString> Values;
    TryGetStringSet(TEXT("AcknowledgedNudges"), Values);
    return Values;
}

void FGorgeousPersistentData::SetAcknowledgedNudges(const TSet<FString>& Ids)
{
    SetStringSet(TEXT("AcknowledgedNudges"), Ids);
}

bool FGorgeousPersistentData::TryGetKnownNudgePlugins(TSet<FString>& OutPlugins) const
{
    return TryGetStringSet(TEXT("KnownNudgePlugins"), OutPlugins);
}

void FGorgeousPersistentData::SetKnownNudgePlugins(const TSet<FString>& Plugins)
{
    SetStringSet(TEXT("KnownNudgePlugins"), Plugins);
}

bool FGorgeousPersistentData::TryGetKnownNudgeGeneralSystems(TSet<FString>& OutSystems) const
{
    return TryGetStringSet(TEXT("KnownNudgeGeneralSystems"), OutSystems);
}

void FGorgeousPersistentData::SetKnownNudgeGeneralSystems(const TSet<FString>& Systems)
{
    SetStringSet(TEXT("KnownNudgeGeneralSystems"), Systems);
}

bool FGorgeousPersistentData::TryGetNudgeFeedCache(FString& OutFeedJson) const
{
    const_cast<FGorgeousPersistentData*>(this)->LoadPersistentData();
    OutFeedJson.Reset();
    const TArray<TSharedPtr<FJsonValue>>* Entries = nullptr;
    if (!PersistentDataObject.IsValid() || !PersistentDataObject->TryGetArrayField(TEXT("NudgeFeedCache"), Entries)) return false;
    const auto Feed = MakeShared<FJsonObject>();
    double Version = 1.;
    PersistentDataObject->TryGetNumberField(TEXT("NudgeFeedCacheSchemaVersion"), Version);
    Feed->SetNumberField(TEXT("schemaVersion"), Version);
    Feed->SetArrayField(TEXT("entries"), *Entries);
    return FJsonSerializer::Serialize(Feed, TJsonWriterFactory<>::Create(&OutFeedJson));
}

void FGorgeousPersistentData::SetNudgeFeedCache(const FString& FeedJson)
{
    TSharedPtr<FJsonObject> Feed;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(FeedJson), Feed) || !Feed.IsValid()) return;
    const TArray<TSharedPtr<FJsonValue>>* Entries = nullptr;
    if (!Feed->TryGetArrayField(TEXT("entries"), Entries)) return;
    LoadPersistentData();
    if (!PersistentDataObject.IsValid()) PersistentDataObject = MakeShared<FJsonObject>();
    double Version = 1.;
    Feed->TryGetNumberField(TEXT("schemaVersion"), Version);
    PersistentDataObject->SetNumberField(TEXT("NudgeFeedCacheSchemaVersion"), Version);
    PersistentDataObject->SetStringField(TEXT("NudgeFeedCacheUpdatedAt"), FDateTime::UtcNow().ToIso8601());
    PersistentDataObject->SetArrayField(TEXT("NudgeFeedCache"), *Entries);
    MarkDirty(TEXT("NudgeFeedCacheSchemaVersion"));
    MarkDirty(TEXT("NudgeFeedCacheUpdatedAt"));
    MarkDirty(TEXT("NudgeFeedCache"));
    SavePersistentData();
}

bool FGorgeousPersistentData::IsSystemActionPromptDismissed(const FString& Key) const
{
    TSet<FString> Values;
    TryGetStringSet(TEXT("DismissedSystemActionPrompts"), Values);
    return Values.Contains(Key);
}

void FGorgeousPersistentData::DismissSystemActionPrompt(const FString& Key)
{
    TSet<FString> Values;
    TryGetStringSet(TEXT("DismissedSystemActionPrompts"), Values);
    Values.Add(Key);
    SetStringSet(TEXT("DismissedSystemActionPrompts"), Values);
}


bool FGorgeousPersistentData::HasPersistentDataFile() const
{
	return FPaths::FileExists(FilePath);
}

TArray<FGorgeousOfflineSystemCacheEntry> FGorgeousPersistentData::GetOfflineSystemCache() const
{
	const_cast<FGorgeousPersistentData*>(this)->LoadPersistentData();

	TArray<FGorgeousOfflineSystemCacheEntry> CacheEntries;
	if (PersistentDataObject.IsValid())
	{
		const TArray<TSharedPtr<FJsonValue>>* CacheArray;
		if (PersistentDataObject->TryGetArrayField(TEXT("OfflineSystemCache"), CacheArray))
		{
			for (const TSharedPtr<FJsonValue>& Value : *CacheArray)
			{
				const TSharedPtr<FJsonObject> Obj = Value->AsObject();
				if (Obj.IsValid())
				{
					FGorgeousOfflineSystemCacheEntry Entry;
					Entry.SystemId = Obj->GetStringField(TEXT("SystemId"));
					Entry.TargetPluginName = Obj->HasField(TEXT("TargetPluginName")) ? Obj->GetStringField(TEXT("TargetPluginName")) : TEXT("GorgeousCore");
					Entry.DisplayName = Obj->GetStringField(TEXT("DisplayName"));
					Entry.Description = Obj->GetStringField(TEXT("Description"));
					Entry.Version = Obj->GetStringField(TEXT("Version"));
					Entry.DownloadUrl = Obj->GetStringField(TEXT("DownloadUrl"));
                    Obj->TryGetStringField(TEXT("DownloadToken"), Entry.DownloadToken);
                    Obj->TryGetStringField(TEXT("ChecksumSha256"), Entry.ChecksumSha256);
                    Obj->TryGetStringField(TEXT("SignatureEd25519"), Entry.SignatureEd25519);
					Entry.bIsCoreSystem = Obj->GetBoolField(TEXT("bIsCoreSystem"));

					const TArray<TSharedPtr<FJsonValue>>* SourcePathsArray;
					if (Obj->TryGetArrayField(TEXT("SourcePaths"), SourcePathsArray))
					{
						for (const auto& PathVal : *SourcePathsArray)
							Entry.SourcePaths.Add(PathVal->AsString());
					}

					const TArray<TSharedPtr<FJsonValue>>* ContentPathsArray;
					if (Obj->TryGetArrayField(TEXT("ContentPaths"), ContentPathsArray))
					{
						for (const auto& PathVal : *ContentPathsArray)
							Entry.ContentPaths.Add(PathVal->AsString());
					}

					CacheEntries.Add(Entry);
				}
			}
		}
	}
	return CacheEntries;
}

TArray<FGorgeousPluginUpdateCacheEntry> FGorgeousPersistentData::GetPluginUpdateCache() const
{
	const_cast<FGorgeousPersistentData*>(this)->LoadPersistentData();

	TArray<FGorgeousPluginUpdateCacheEntry> CacheEntries;
	if (PersistentDataObject.IsValid())
	{
		const TArray<TSharedPtr<FJsonValue>>* CacheArray;
		if (PersistentDataObject->TryGetArrayField(TEXT("PluginUpdateCache"), CacheArray))
		{
			for (const TSharedPtr<FJsonValue>& Value : *CacheArray)
			{
				const TSharedPtr<FJsonObject> Obj = Value->AsObject();
				if (Obj.IsValid())
				{
					FGorgeousPluginUpdateCacheEntry Entry;
					Entry.PluginName = Obj->GetStringField(TEXT("PluginName"));
					Entry.AvailableVersion = Obj->GetStringField(TEXT("AvailableVersion"));
					Entry.MinimumCoreVersion = Obj->GetStringField(TEXT("MinimumCoreVersion"));
					Entry.ChangelogUrl = Obj->GetStringField(TEXT("ChangelogUrl"));
					Entry.DownloadToken = Obj->GetStringField(TEXT("DownloadToken"));
                    Obj->TryGetStringField(TEXT("ChecksumSha256"), Entry.ChecksumSha256);
                    Obj->TryGetStringField(TEXT("SignatureEd25519"), Entry.SignatureEd25519);
					CacheEntries.Add(Entry);
				}
			}
		}
	}
	return CacheEntries;
}

void FGorgeousPersistentData::SetOfflineSystemCache(const TArray<FGorgeousOfflineSystemCacheEntry>& NewCache)
{
	LoadPersistentData();

	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}

	TArray<TSharedPtr<FJsonValue>> JsonArray;
	for (const FGorgeousOfflineSystemCacheEntry& Entry : NewCache)
	{
		TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
		JsonObj->SetStringField(TEXT("SystemId"), Entry.SystemId);
		JsonObj->SetStringField(TEXT("TargetPluginName"), Entry.TargetPluginName);
		JsonObj->SetStringField(TEXT("DisplayName"), Entry.DisplayName);
		JsonObj->SetStringField(TEXT("Description"), Entry.Description);
		JsonObj->SetStringField(TEXT("Version"), Entry.Version);
		JsonObj->SetStringField(TEXT("DownloadUrl"), Entry.DownloadUrl);
        JsonObj->SetStringField(TEXT("DownloadToken"), Entry.DownloadToken);
        JsonObj->SetStringField(TEXT("ChecksumSha256"), Entry.ChecksumSha256);
        JsonObj->SetStringField(TEXT("SignatureEd25519"), Entry.SignatureEd25519);
		JsonObj->SetStringField(TEXT("ChangelogUrl"), Entry.ChangelogUrl);
		JsonObj->SetBoolField(TEXT("bIsCoreSystem"), Entry.bIsCoreSystem);

		TArray<TSharedPtr<FJsonValue>> SourcePathsArr;
		for (const FString& Path : Entry.SourcePaths) { SourcePathsArr.Add(MakeShared<FJsonValueString>(Path)); }
		JsonObj->SetArrayField(TEXT("SourcePaths"), SourcePathsArr);

		TArray<TSharedPtr<FJsonValue>> ContentPathsArr;
		for (const FString& Path : Entry.ContentPaths) { ContentPathsArr.Add(MakeShared<FJsonValueString>(Path)); }
		JsonObj->SetArrayField(TEXT("ContentPaths"), ContentPathsArr);

		JsonArray.Add(MakeShared<FJsonValueObject>(JsonObj));
	}

	PersistentDataObject->SetArrayField(TEXT("OfflineSystemCache"), JsonArray);
	MarkDirty(TEXT("OfflineSystemCache"));
	SavePersistentData();
}

void FGorgeousPersistentData::SetPluginUpdateCache(const TArray<FGorgeousPluginUpdateCacheEntry>& NewCache)
{
	LoadPersistentData();

	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}

	TArray<TSharedPtr<FJsonValue>> JsonArray;
	for (const FGorgeousPluginUpdateCacheEntry& Entry : NewCache)
	{
		TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
		JsonObj->SetStringField(TEXT("PluginName"), Entry.PluginName);
		JsonObj->SetStringField(TEXT("AvailableVersion"), Entry.AvailableVersion);
		JsonObj->SetStringField(TEXT("MinimumCoreVersion"), Entry.MinimumCoreVersion);
		JsonObj->SetStringField(TEXT("ChangelogUrl"), Entry.ChangelogUrl);
		JsonObj->SetStringField(TEXT("DownloadToken"), Entry.DownloadToken);
        JsonObj->SetStringField(TEXT("ChecksumSha256"), Entry.ChecksumSha256);
        JsonObj->SetStringField(TEXT("SignatureEd25519"), Entry.SignatureEd25519);
		JsonArray.Add(MakeShared<FJsonValueObject>(JsonObj));
	}

	PersistentDataObject->SetArrayField(TEXT("PluginUpdateCache"), JsonArray);
	MarkDirty(TEXT("PluginUpdateCache"));
	SavePersistentData();
}

bool FGorgeousPersistentData::HasPluginUpdateAvailable(const FName& PluginName) const
{
	TArray<FGorgeousPluginUpdateCacheEntry> Updates = GetPluginUpdateCache();
	for (const FGorgeousPluginUpdateCacheEntry& Update : Updates)
	{
		if (Update.PluginName == PluginName.ToString())
		{
			return true;
		}
	}
	return false;
}

FString FGorgeousPersistentData::GetPluginUpdateChangelogUrl(const FName& PluginName) const
{
	TArray<FGorgeousPluginUpdateCacheEntry> Updates = GetPluginUpdateCache();
	for (const FGorgeousPluginUpdateCacheEntry& Update : Updates)
	{
		if (Update.PluginName == PluginName.ToString())
		{
			return Update.ChangelogUrl;
		}
	}
	return FString();
}

FString FGorgeousPersistentData::GetPluginUpdateChecksumSha256(const FName& PluginName) const
{
    for (const auto& Entry : GetPluginUpdateCache())
        if (Entry.PluginName == PluginName.ToString()) return Entry.ChecksumSha256;
    return FString();
}

FString FGorgeousPersistentData::GetPluginUpdateDownloadToken(const FName& PluginName) const
{
	TArray<FGorgeousPluginUpdateCacheEntry> Updates = GetPluginUpdateCache();
	for (const FGorgeousPluginUpdateCacheEntry& Update : Updates)
	{
		if (Update.PluginName == PluginName.ToString())
		{
			return Update.DownloadToken;
		}
	}
	return FString();
}

bool FGorgeousPersistentData::HasRunInitialValidation() const
{
	const_cast<FGorgeousPersistentData*>(this)->LoadPersistentData();

	if (PersistentDataObject.IsValid())
	{
		const TSharedPtr<FJsonObject> SystemValidation = FindPersistentObject(PersistentDataObject, TEXT("SystemValidation"));
		if (SystemValidation.IsValid() && SystemValidation->HasField(TEXT("HasRunInitialValidation")))
		{
			return SystemValidation->GetBoolField(TEXT("HasRunInitialValidation"));
		}
	}

	return false;
}

void FGorgeousPersistentData::SetHasRunInitialValidation(bool bHasRun)
{
	LoadPersistentData();

	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}

	TSharedPtr<FJsonObject> SystemValidation = FindPersistentObject(PersistentDataObject, TEXT("SystemValidation"));
	if (!SystemValidation.IsValid())
	{
		SystemValidation = MakeShared<FJsonObject>();
		PersistentDataObject->SetObjectField(TEXT("SystemValidation"), SystemValidation);
	}

	SystemValidation->SetBoolField(TEXT("HasRunInitialValidation"), bHasRun);

	MarkDirty(TEXT("SystemValidation"), TEXT("HasRunInitialValidation"));
	SavePersistentData();
}

int32 FGorgeousPersistentData::GetSystemValidationCount() const
{
	const_cast<FGorgeousPersistentData*>(this)->LoadPersistentData();

	if (PersistentDataObject.IsValid())
	{
		const TSharedPtr<FJsonObject> SystemValidation = FindPersistentObject(PersistentDataObject, TEXT("SystemValidation"));
		if (SystemValidation.IsValid())
		{
			int32 Count = 0;
			SystemValidation->TryGetNumberField(TEXT("ValidationCount"), Count);
			return Count;
		}
	}

	return 0;
}

void FGorgeousPersistentData::IncrementSystemValidationCount()
{
	LoadPersistentData();

	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}

	TSharedPtr<FJsonObject> SystemValidation = FindPersistentObject(PersistentDataObject, TEXT("SystemValidation"));
	if (!SystemValidation.IsValid())
	{
		SystemValidation = MakeShared<FJsonObject>();
		PersistentDataObject->SetObjectField(TEXT("SystemValidation"), SystemValidation);
	}

	int32 CurrentCount = 0;
	SystemValidation->TryGetNumberField(TEXT("ValidationCount"), CurrentCount);
	SystemValidation->SetNumberField(TEXT("ValidationCount"), CurrentCount + 1);

	MarkDirty(TEXT("SystemValidation"), TEXT("ValidationCount"));
	SavePersistentData();
}

void FGorgeousPersistentData::SetSystemValidationCount(int32 NewCount)
{
	LoadPersistentData();

	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}

	TSharedPtr<FJsonObject> SystemValidation = FindPersistentObject(PersistentDataObject, TEXT("SystemValidation"));
	if (!SystemValidation.IsValid())
	{
		SystemValidation = MakeShared<FJsonObject>();
		PersistentDataObject->SetObjectField(TEXT("SystemValidation"), SystemValidation);
	}

	SystemValidation->SetNumberField(TEXT("ValidationCount"), NewCount);

	MarkDirty(TEXT("SystemValidation"), TEXT("ValidationCount"));
	SavePersistentData();
}

int32 FGorgeousPersistentData::GetSystemValidationInterval() const
{
	const_cast<FGorgeousPersistentData*>(this)->LoadPersistentData();

	if (PersistentDataObject.IsValid())
	{
		const TSharedPtr<FJsonObject> SystemValidation = FindPersistentObject(PersistentDataObject, TEXT("SystemValidation"));
		if (SystemValidation.IsValid() && SystemValidation->HasField(TEXT("ValidationInterval")))
		{
			return SystemValidation->GetIntegerField(TEXT("ValidationInterval"));
		}
	}

	return 10; // Default
}

void FGorgeousPersistentData::SetSystemValidationInterval(int32 NewInterval)
{
	LoadPersistentData();

	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}

	TSharedPtr<FJsonObject> SystemValidation = FindPersistentObject(PersistentDataObject, TEXT("SystemValidation"));
	if (!SystemValidation.IsValid())
	{
		SystemValidation = MakeShared<FJsonObject>();
		PersistentDataObject->SetObjectField(TEXT("SystemValidation"), SystemValidation);
	}

	SystemValidation->SetNumberField(TEXT("ValidationInterval"), NewInterval);

	MarkDirty(TEXT("SystemValidation"), TEXT("ValidationInterval"));
	SavePersistentData();
}

bool FGorgeousPersistentData::GetForceDevMode() const
{
	const_cast<FGorgeousPersistentData*>(this)->LoadPersistentData();

	if (PersistentDataObject.IsValid())
	{
		const TSharedPtr<FJsonObject> DevSettings = FindPersistentObject(PersistentDataObject, TEXT("DeveloperSettings"));
		if (DevSettings.IsValid() && DevSettings->HasField(TEXT("ForceDevMode")))
		{
			return DevSettings->GetBoolField(TEXT("ForceDevMode"));
		}
	}

	return false; // Default
}

void FGorgeousPersistentData::SetForceDevMode(bool bForce)
{
	LoadPersistentData();

	if (!PersistentDataObject.IsValid())
	{
		PersistentDataObject = MakeShared<FJsonObject>();
	}

	TSharedPtr<FJsonObject> DevSettings = FindPersistentObject(PersistentDataObject, TEXT("DeveloperSettings"));
	if (!DevSettings.IsValid())
	{
		DevSettings = MakeShared<FJsonObject>();
		PersistentDataObject->SetObjectField(TEXT("DeveloperSettings"), DevSettings);
	}

	DevSettings->SetBoolField(TEXT("ForceDevMode"), bForce);

	MarkDirty(TEXT("DeveloperSettings"), TEXT("ForceDevMode"));
	SavePersistentData();
}

#if WITH_EDITOR
bool FGorgeousPersistentData::HasShownResourceProtectionBanner()
{
	LoadPersistentData();
	bool bShown = false;
	return PersistentDataObject.IsValid()
		&& PersistentDataObject->TryGetBoolField(ResourceProtectionBannerShownKey, bShown)
		&& bShown;
}
#endif

#if WITH_EDITOR
void FGorgeousPersistentData::MarkResourceProtectionBannerShown()
{
	if (HasShownResourceProtectionBanner() || !PersistentDataObject.IsValid()) return;
	PersistentDataObject->SetBoolField(ResourceProtectionBannerShownKey, true);
	MarkDirty(ResourceProtectionBannerShownKey);
	SavePersistentData();
}
#endif

bool FGorgeousPersistentData::TryComputePluginBinaryChecksum(const FName& PluginName, FString& OutChecksum)
{
	OutChecksum.Reset();
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName.ToString());
	return Plugin.IsValid() && !Plugin->GetDescriptor().Modules.IsEmpty() &&
		GorgeousPersistentDataJson::ComputeBinaryDirectoryChecksum(
			FPaths::Combine(Plugin->GetBaseDir(), TEXT("Binaries"), FPlatformProcess::GetBinariesSubdirectory()), OutChecksum);
}

bool FGorgeousPersistentData::IsPluginBinaryChecksumVerified(const FName& PluginName) const
{
	const_cast<FGorgeousPersistentData*>(this)->LoadPersistentData();
	if (!PersistentDataObject.IsValid()) return false;
	const TSharedPtr<FJsonObject>* Checksums = nullptr;
	FString Saved, Current;
	if (!PersistentDataObject->TryGetObjectField(TEXT("VerifiedBinaryChecksums"), Checksums) || !Checksums || !Checksums->IsValid() ||
		!(*Checksums)->TryGetStringField(PluginName.ToString(), Saved) || Saved.IsEmpty() ||
		!TryComputePluginBinaryChecksum(PluginName, Current) || Saved != Current) return false;
	const TSharedPtr<FJsonObject>* Descriptors = nullptr;
	FString SavedDescriptor;
	if (!PersistentDataObject->TryGetObjectField(TEXT("VerifiedBinaryChecksumDescriptors"), Descriptors) || !Descriptors || !Descriptors->IsValid() ||
		!(*Descriptors)->TryGetStringField(PluginName.ToString(), SavedDescriptor))
	{
		// Retain the old Core-only cache contract until its first successful backfill.
		return PluginName == FName(TEXT("GorgeousCore"));
	}
	const FString CurrentDescriptor = PluginDescriptorFingerprint(IPluginManager::Get().FindPlugin(PluginName.ToString()));
	return !CurrentDescriptor.IsEmpty() && CurrentDescriptor == SavedDescriptor;
}