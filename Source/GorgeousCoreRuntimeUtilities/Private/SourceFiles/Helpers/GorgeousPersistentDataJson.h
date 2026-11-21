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

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

// Provides owned-field JSON operations for the storage service and regression tests.
namespace GorgeousPersistentDataJson
{
    /**
     * Returns an existing object field or creates a correctly typed owner object.
     *
     * @param Parent The owning JSON object.
     * @param Name The object field to find or create.
     * @return The existing object field or its newly created replacement.
     */
    TSharedRef<FJsonObject> ObjectField(FJsonObject& Parent, const FString& Name);

    /**
     * Reads the installed-system baseline without changing the document.
     *
     * @param Data The internal persistent document operated on.
     * @return The recorded system baseline without duplicates.
     */
    TArray<FString> ReadInstalledSystems(const FJsonObject& Data);

    /**
     * Replaces the baseline and its migration marker, preserving sibling fields.
     *
     * @param Data The internal persistent document operated on.
     * @param Systems The current installed-system or known-system baseline.
     */
    void WriteInstalledSystems(FJsonObject& Data, const TArray<FString>& Systems);

    /**
     * Adds legacy plugin entries once without re-baselining later installations.
     *
     * @param Data The internal persistent document operated on.
     * @param Plugins The plugin identifiers supplied by discovery.
     * @return True when a legacy baseline was backfilled.
     */
    bool BackfillInstalledPlugins(FJsonObject& Data, const TArray<FString>& Plugins);

    /**
     * Retains the binary digest algorithm consumed by the early startup gate.
     *
     * @param Directory The platform binary directory to hash.
     * @param OutChecksum Receives the digest, or an empty string when binaries cannot be verified.
     * @return True when built platform binaries produced a digest.
     */
    bool ComputeBinaryDirectoryChecksum(const FString& Directory, FString& OutChecksum);
}