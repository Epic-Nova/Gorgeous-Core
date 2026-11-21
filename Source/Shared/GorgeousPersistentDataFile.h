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
#include "Dom/JsonObject.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
//<-------------------------------------------------------------------------->

// Resolves the metadata file without loading a Gorgeous runtime module.
namespace GorgeousPersistentDataFile
{
    // Identifies a read failure without replacing an existing document.
    enum class EReadResult : uint8
    {
        // The file has not been created.
        Missing,
        // The existing file could not be read.
        ReadFailed,
        // The file did not contain a valid JSON object.
        ParseFailed,
        // A complete JSON object was loaded.
        Loaded
    };

    /**
     * Returns the project-local path used by all persistent-data owners.
     *
     * @return The default project-local metadata path.
     */
    inline FString DefaultPath()
    {
        return FPaths::ProjectSavedDir() / TEXT("GorgeousThings") / TEXT("GorgeousPersistentData.json");
    }

    /**
     * Loads a root object for the storage service or the read-only early gate.
     *
     * @param Path The metadata file path to read.
     * @param OutData Receives the loaded object and remains empty on failure.
     * @return The read outcome, distinguishing absence, read failure and malformed JSON.
     */
    inline EReadResult ReadObject(const FString& Path, TSharedPtr<FJsonObject>& OutData)
    {
        OutData.Reset();
        if (!FPaths::FileExists(Path)) return EReadResult::Missing;
        FString Contents;
        if (!FFileHelper::LoadFileToString(Contents, *Path)) return EReadResult::ReadFailed;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Contents);
        if (!FJsonSerializer::Deserialize(Reader, OutData) || !OutData.IsValid())
        {
            OutData.Reset();
            return EReadResult::ParseFailed;
        }
        return EReadResult::Loaded;
    }
}