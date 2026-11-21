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
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousPersistentData.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class FJsonObject;
//<------------------------------------------------------------->

/**
 * Describes an installable system retained in the offline release catalogue.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousOfflineSystemCacheEntry
{
    GENERATED_BODY()

    // The system identifier recorded by the offline catalogue.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString SystemId;

    // The plugin that receives this installable system.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString TargetPluginName;

    // The display name supplied by the system catalogue.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString DisplayName;

    // The catalogue description shown while offline.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString Description;

    // The available system version from the catalogue.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString Version;

    // The download URL associated with the cached release.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString DownloadUrl;

    // The download token associated with the cached release.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString DownloadToken;

    // The expected SHA-256 digest supplied by the release service.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString ChecksumSha256;

    // The expected Ed25519 release signature.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString SignatureEd25519;

    // The changelog URL associated with the cached release.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString ChangelogUrl;

    // The source paths supplied with the installable system.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    TArray<FString> SourcePaths;

    // The content paths supplied with the installable system.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    TArray<FString> ContentPaths;

    // Whether the cached system belongs to Core.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    bool bIsCoreSystem = false;
};

/**
 * Describes the cached update metadata for one plugin release.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousPluginUpdateCacheEntry
{
    GENERATED_BODY()

    // The plugin whose update metadata is cached.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString PluginName;

    // The plugin version offered by the update service.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString AvailableVersion;

    // The minimum compatible Core version for this release.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString MinimumCoreVersion;

    // The changelog URL associated with the cached release.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString ChangelogUrl;

    // The download token associated with the cached release.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString DownloadToken;

    // The expected SHA-256 digest supplied by the release service.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString ChecksumSha256;

    // The expected Ed25519 release signature.
    UPROPERTY(VisibleAnywhere, Category = "Cache")
    FString SignatureEd25519;
};

/**
 * Owns project-local Gorgeous metadata and merges changed fields into the latest file.
 * Unknown fields remain in the document; unreadable or malformed data is never replaced.
 *
 * @author Nils Bergemann
 */
class GORGEOUSCORERUNTIMEUTILITIES_API FGorgeousPersistentData
{
public:
    // Uses the supplied document path, or the project default when no path is supplied.
    explicit FGorgeousPersistentData(FString InFilePath = FString());

    // Releases the cached document without writing it during destruction.
    ~FGorgeousPersistentData();

    // Prevents copying ownership of a loaded document and its pending changes.
    FGorgeousPersistentData(const FGorgeousPersistentData&) = delete;

    //<============================--- C++ Only ---=============================>
    #pragma region C++ Only

    // Prevents assigning ownership of another service's cached document.
    FGorgeousPersistentData& operator=(const FGorgeousPersistentData&) = delete;

    /**
     * Returns the process-local storage service used by runtime and editor callers.
     *
     * @return The process-local service shared by runtime and editor callers.
     */
    static FGorgeousPersistentData& Get();

    /**
     * Returns the default project path without creating or loading a document.
     *
     * @return The default project-local metadata path.
     */
    static FString GetDefaultFilePath();

    /**
     * Returns this instance's path, including an explicit test override.
     *
     * @return The path owned by this service instance.
     */
    const FString& GetFilePath() const { return FilePath; }

    /**
     * Checks disk existence without causing first-load initialization.
     *
     * @return True when the file already exists.
     */
    bool HasPersistentDataFile() const;

    // Loads once and establishes defaults only when no document exists.
    void LoadPersistentData();

    // Synchronously flushes changed owned fields; fails closed after a read failure.
    void SavePersistentData();

    /**
     * Reads installable-system metadata retained for offline use.
     *
     * @return Cached system entries, or an empty array when unavailable.
     */
    TArray<FGorgeousOfflineSystemCacheEntry> GetOfflineSystemCache() const;

    /**
     * Replaces offline-system metadata while preserving unrelated document fields.
     *
     * @param NewCache The replacement cached release or system entries.
     */
    void SetOfflineSystemCache(const TArray<FGorgeousOfflineSystemCacheEntry>& NewCache);

    /**
     * Reads per-plugin release metadata retained for offline use.
     *
     * @return Cached plugin update entries, or an empty array when unavailable.
     */
    TArray<FGorgeousPluginUpdateCacheEntry> GetPluginUpdateCache() const;

    /**
     * Replaces per-plugin release metadata while preserving unrelated document fields.
     *
     * @param NewCache The replacement cached release or system entries.
     */
    void SetPluginUpdateCache(const TArray<FGorgeousPluginUpdateCacheEntry>& NewCache);

    /**
     * Tests whether release metadata exists for the requested plugin.
     *
     * @param PluginName The plugin whose stored or current binary metadata is queried.
     * @return True when the plugin has a cached update entry.
     */
    bool HasPluginUpdateAvailable(const FName& PluginName) const;

    /**
     * Reads the cached changelog URL for a plugin.
     *
     * @param PluginName The plugin whose stored or current binary metadata is queried.
     * @return The cached changelog URL, or an empty string.
     */
    FString GetPluginUpdateChangelogUrl(const FName& PluginName) const;

    /**
     * Reads the cached release checksum for a plugin.
     *
     * @param PluginName The plugin whose stored or current binary metadata is queried.
     * @return The cached expected release digest, or an empty string.
     */
    FString GetPluginUpdateChecksumSha256(const FName& PluginName) const;

    /**
     * Reads the cached download token for a plugin.
     *
     * @param PluginName The plugin whose stored or current binary metadata is queried.
     * @return The cached download token, or an empty string.
     */
    FString GetPluginUpdateDownloadToken(const FName& PluginName) const;

    /**
     * Reads whether the initial validation baseline has been recorded.
     *
     * @return True when the initial validation baseline has been recorded.
     */
    bool HasRunInitialValidation() const;

    /**
     * Records whether the initial validation baseline completed.
     *
     * @param bHasRun Whether the initial validation baseline has completed.
     */
    void SetHasRunInitialValidation(bool bHasRun);

    /**
     * Reads the startup validation counter.
     *
     * @return The stored startup counter, defaulting to zero.
     */
    int32 GetSystemValidationCount() const;

    // Increments and saves the startup validation counter.
    void IncrementSystemValidationCount();

    /**
     * Replaces and saves the startup validation counter.
     *
     * @param NewCount The replacement startup validation counter.
     */
    void SetSystemValidationCount(int32 NewCount);

    /**
     * Reads the validation cadence, defaulting to ten startups.
     *
     * @return The stored startup interval, defaulting to ten.
     */
    int32 GetSystemValidationInterval() const;

    /**
     * Replaces and saves the validation cadence.
     *
     * @param NewInterval The startup interval between validation sweeps.
     */
    void SetSystemValidationInterval(int32 NewInterval);

    /**
     * Reads the developer HTTP mode override.
     *
     * @return True when the developer HTTP override is enabled.
     */
    bool GetForceDevMode() const;

    /**
     * Replaces and saves the developer HTTP mode override.
     *
     * @param bForce Whether the developer HTTP mode override is enabled.
     */
    void SetForceDevMode(bool bForce);

    /**
     * Reads the baseline and backfills legacy plugin entries only once.
     *
     * @param InstalledPlugins The installed plugins supplied by discovery for this operation.
     * @return The recorded baseline, including one-time legacy plugin backfill.
     */
    TArray<FString> GetRecordedInstalledSystems(const TArray<FString>& InstalledPlugins) const;

    /**
     * Replaces the baseline without changing unknown validation metadata.
     *
     * @param Systems The current installed-system or known-system baseline.
     */
    void RecordInstalledSystems(const TArray<FString>& Systems);

    /**
     * Computes a platform-binary digest without writing persistent data.
     *
     * @param PluginName The plugin whose stored or current binary metadata is queried.
     * @param OutChecksum Receives the digest, or an empty string when binaries cannot be verified.
     * @return True when compiled binaries produced a valid digest.
     */
    static bool TryComputePluginBinaryChecksum(const FName& PluginName, FString& OutChecksum);

    /**
     * Verifies the digest and descriptor, retaining the legacy Core-only fallback.
     *
     * @param PluginName The plugin whose stored or current binary metadata is queried.
     * @return True when current binaries and descriptor match stored verification.
     */
    bool IsPluginBinaryChecksumVerified(const FName& PluginName) const;

    /**
     * Saves digests for the supplied installed plugins after successful validation.
     *
     * @param InstalledPlugins The installed plugins supplied by discovery for this operation.
     */
    void GenerateAndSaveBinaryChecksums(const TSet<FName>& InstalledPlugins);

    /**
     * Removes known verification records and cached updates using legacy semantics.
     *
     * @param InstalledPlugins The installed plugins supplied by discovery for this operation.
     */
    void InvalidatePersistedChecksum(const TSet<FName>& InstalledPlugins);

    /**
     * Records a circular pair once and reports whether it needs a notification.
     *
     * @param PairKey The canonical sorted plugin-pair identifier.
     * @return True only for the first notification of this pair.
     */
    bool MarkCircularDependencyNotified(const FString& PairKey);

    /**
     * Reads acknowledged nudge identifiers without exposing the stored document.
     *
     * @return The acknowledged nudge identifiers.
     */
    TSet<FString> GetAcknowledgedNudges() const;

    /**
     * Replaces and saves acknowledged nudge identifiers.
     *
     * @param Ids The acknowledged nudge identifiers to retain.
     */
    void SetAcknowledgedNudges(const TSet<FString>& Ids);

    /**
     * Distinguishes a missing known-plugin snapshot from an empty baseline.
     *
     * @param OutPlugins Receives stored plugin identifiers when a snapshot exists.
     * @return True when a snapshot exists, including a present empty baseline.
     */
    bool TryGetKnownNudgePlugins(TSet<FString>& OutPlugins) const;

    /**
     * Replaces and saves the known-plugin nudge baseline.
     *
     * @param Plugins The plugin identifiers supplied by discovery.
     */
    void SetKnownNudgePlugins(const TSet<FString>& Plugins);

    /**
     * Distinguishes a missing known-system snapshot from an empty baseline.
     *
     * @param OutSystems Receives stored system identifiers when a snapshot exists.
     * @return True when a snapshot exists, including a present empty baseline.
     */
    bool TryGetKnownNudgeGeneralSystems(TSet<FString>& OutSystems) const;

    /**
     * Replaces and saves the known-system nudge baseline.
     *
     * @param Systems The current installed-system or known-system baseline.
     */
    void SetKnownNudgeGeneralSystems(const TSet<FString>& Systems);

    /**
     * Reconstructs only the public feed protocol payload, never the stored root.
     *
     * @param OutFeedJson Receives only the feed protocol payload, not the persistent document.
     * @return True when a cached feed could be reconstructed.
     */
    bool TryGetNudgeFeedCache(FString& OutFeedJson) const;

    /**
     * Caches a validated feed protocol payload and records its update timestamp.
     *
     * @param FeedJson The feed protocol payload containing schemaVersion and entries.
     */
    void SetNudgeFeedCache(const FString& FeedJson);

    /**
     * Checks whether a general-system prompt has been dismissed.
     *
     * @param Key The stable general-system prompt identifier.
     * @return True when the prompt identifier was previously dismissed.
     */
    bool IsSystemActionPromptDismissed(const FString& Key) const;

    /**
     * Records a dismissed prompt without changing other persisted data.
     *
     * @param Key The stable general-system prompt identifier.
     */
    void DismissSystemActionPrompt(const FString& Key);
#if WITH_EDITOR
    /**
     * Reads whether this project has already shown the resource-protection banner.
     *
     * @return True when the banner was recorded as displayed.
     */
    bool HasShownResourceProtectionBanner();

    // Records the resource-protection banner's first display.
    void MarkResourceProtectionBannerShown();
#endif

private:

    /**
     * Marks only the owned field that a setter changes.
     *
     * @param Root The owned root field changed by a setter.
     * @param Child The owned child field, or empty to mark the entire root.
     */
    void MarkDirty(const FString& Root, const FString& Child = FString());

    /**
     * Reads a named string-array field while preserving missing-versus-empty state.
     *
     * @param Field The owned root-array field to read or replace.
     * @param OutValues Receives stored strings when the array field exists.
     * @return True when the named array exists, including an empty array.
     */
    bool TryGetStringSet(const FString& Field, TSet<FString>& OutValues) const;

    /**
     * Writes a named owned string-array field and flushes it.
     *
     * @param Field The owned root-array field to read or replace.
     * @param Values The replacement owned string values.
     */
    void SetStringSet(const FString& Field, const TSet<FString>& Values);
    //<------------------------------------------------------------------------->
    #pragma endregion C++ Only


    //<============================--- Variables ---============================>
    #pragma region Variables
public:

    // A failed validation blocks the delayed checksum writer for this process.
    static bool bAbortBinaryChecksumSave;

private:

    // The path owned by this service instance.
    FString FilePath;

    // Loading is attempted once so malformed files cannot be silently recreated.
    bool bLoadedPersistentData = false;

    // An existing unreadable or malformed document disables all subsequent writes.
    bool bPersistentDataLoadFailed = false;

    // The root object never leaves the persistence implementation.
    TSharedPtr<FJsonObject> PersistentDataObject;

    // Persisted circular pairs already notified in a prior or current session.
    TSet<FString> NotifiedCircularDependencyPairs;

    // Each root records changed child leaves; an empty child means the whole root field.
    TMap<FString, TSet<FString>> DirtyFields;
    //<------------------------------------------------------------------------->
    #pragma endregion Variables
};