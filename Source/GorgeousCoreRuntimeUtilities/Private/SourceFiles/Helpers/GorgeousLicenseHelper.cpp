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
#include "Helpers/GorgeousLicenseHelper.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "Misc/Base64.h"
#include "Interfaces/IPluginManager.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// FGorgeousCoreLicenseHelper Implementation
//=============================================================================

namespace
{
	FString GetLicenseFilePath()
	{
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("GorgeousCore"));
		return FPaths::Combine(Plugin.IsValid() ? Plugin->GetBaseDir() : FPaths::ProjectSavedDir(), TEXT("License.lic"));
	}

	// Helper to get 32-byte AES key from ProjectId (pads or truncates as needed)
	FAES::FAESKey MakeAESKey(const FString& ProjectId)
	{
		FAES::FAESKey Key;
		const FTCHARToUTF8 Converter(*ProjectId);
		FMemory::Memset(Key.Key, 0, 32);
		const int32 CopyLen = FMath::Min(Converter.Length(), 32);
		FMemory::Memcpy(Key.Key, Converter.Get(), CopyLen);
		return Key;
	}
}

bool FGorgeousCoreLicenseHelper::CreateEncryptedLicenseFile(const TArray<FString>& LicenseStrings, const FString& ProjectId)
{
	const FDateTime Now = FDateTime::UtcNow();
	const FString DateTimeString = Now.ToIso8601();
	FString Combined = DateTimeString;
	for (const FString& Str : LicenseStrings)
	{
		Combined += TEXT("\n") + Str;
	}

	TArray<uint8> PlainBytes;
	const FTCHARToUTF8 Converter(*Combined);
	PlainBytes.Append((uint8*)Converter.Get(), Converter.Length());

	const FAES::FAESKey Key = MakeAESKey(ProjectId);
	// Pad to AES block size (16 bytes)
	const int32 Pad = 16 - (PlainBytes.Num() % 16);
	for (int32 i = 0; i < Pad; ++i) PlainBytes.Add(static_cast<uint8>(Pad));

	FAES::EncryptData(PlainBytes.GetData(), PlainBytes.Num(), Key);

	FString Encoded = FBase64::Encode(PlainBytes);
	return FFileHelper::SaveStringToFile(Encoded, *GetLicenseFilePath());
}

bool FGorgeousCoreLicenseHelper::CreateEncryptedLicenseFile(const FString& LicenseString, const FString& ProjectId)
{
	TArray<FString> Single;
	Single.Add(LicenseString);
	return CreateEncryptedLicenseFile(Single, ProjectId);
}

bool FGorgeousCoreLicenseHelper::ReadAndDecryptLicenseFile(const FString& ProjectId, TArray<FString>& OutLicenseStrings, FDateTime& OutDateTime)
{
	FString Encoded;
	if (!FFileHelper::LoadFileToString(Encoded, *GetLicenseFilePath()))
	{
		return false;
	}

	TArray<uint8> EncryptedBytes;
	if (!FBase64::Decode(Encoded, EncryptedBytes) || EncryptedBytes.IsEmpty() || EncryptedBytes.Num() % 16 != 0)
	{
		return false;
	}

	const FAES::FAESKey Key = MakeAESKey(ProjectId);
	FAES::DecryptData(EncryptedBytes.GetData(), EncryptedBytes.Num(), Key);

	// Remove padding
	if (EncryptedBytes.Num() == 0) return false;
	const uint8 Pad = EncryptedBytes.Last();
	if (Pad == 0 || Pad > 16 || Pad > EncryptedBytes.Num()) return false;
	for (int32 Index = EncryptedBytes.Num() - Pad; Index < EncryptedBytes.Num(); ++Index)
	{
		if (EncryptedBytes[Index] != Pad) return false;
	}
	EncryptedBytes.SetNum(EncryptedBytes.Num() - Pad);

	const FUTF8ToTCHAR Decoded(reinterpret_cast<const ANSICHAR*>(EncryptedBytes.GetData()), EncryptedBytes.Num());
	const FString Combined(Decoded.Length(), Decoded.Get());
	TArray<FString> Lines;
	Combined.ParseIntoArrayLines(Lines);
	if (Lines.Num() < 1) return false;
	const bool bHasHeader = FDateTime::ParseIso8601(*Lines[0], OutDateTime);
	OutLicenseStrings.Empty();
	for (int32 i = bHasHeader ? 1 : 0; i < Lines.Num(); ++i)
	{
		int32 Separator;
		FDateTime EntryTimestamp;
		if (Lines[i].FindChar('|', Separator) && FDateTime::ParseIso8601(*Lines[i].Left(Separator), EntryTimestamp))
		{
			if (!bHasHeader && i == 0) { OutDateTime = EntryTimestamp; }
			OutLicenseStrings.Add(Lines[i].Mid(Separator + 1));
		}
		else if (bHasHeader) { OutLicenseStrings.Add(Lines[i]); }
		else { return false; }
	}
	return true;
}

bool FGorgeousCoreLicenseHelper::ReadAndDecryptLicenseFile(const FString& ProjectId, FString& OutLicenseString, FDateTime& OutDateTime)
{
	TArray<FString> LicenseStrings;
	if (const bool bResult = ReadAndDecryptLicenseFile(ProjectId, LicenseStrings, OutDateTime); 
		!bResult || LicenseStrings.Num() < 1)
	{
		OutLicenseString = TEXT("");
		return false;
	}
	OutLicenseString = LicenseStrings[0];
	return true;
}

bool FGorgeousCoreLicenseHelper::AddLicenseEntry(const FString& EntryString, const FString& ProjectId)
{
	TArray<TPair<FDateTime, FString>> Entries;
	if (!ReadAllLicenseEntries(ProjectId, Entries)) { return false; } // Preserve unreadable files instead of silently replacing them.

	FDateTime Now = FDateTime::UtcNow();
	Entries.Add(TPair<FDateTime, FString>(Now, EntryString));

	TArray<FString> Lines;
	for (const auto& Pair : Entries)
	{
		Lines.Add(Pair.Key.ToIso8601() + TEXT("|") + Pair.Value);
	}
	return CreateEncryptedLicenseFile(Lines, ProjectId);
}

bool FGorgeousCoreLicenseHelper::ReadAllLicenseEntries(const FString& ProjectId, TArray<TPair<FDateTime, FString>>& OutEntries)
{
	OutEntries.Empty();
	FString Encoded;
	if (!FFileHelper::LoadFileToString(Encoded, *GetLicenseFilePath()))
	{
		return true; // No file = no entries, not an error
	}

	TArray<uint8> EncryptedBytes;
	if (!FBase64::Decode(Encoded, EncryptedBytes) || EncryptedBytes.IsEmpty() || EncryptedBytes.Num() % 16 != 0)
	{
		return false;
	}

	const FAES::FAESKey Key = MakeAESKey(ProjectId);
	FAES::DecryptData(EncryptedBytes.GetData(), EncryptedBytes.Num(), Key);

	if (EncryptedBytes.Num() == 0) return true;
	const uint8 Pad = EncryptedBytes.Last();
	if (Pad == 0 || Pad > 16 || Pad > EncryptedBytes.Num()) return false;
	for (int32 Index = EncryptedBytes.Num() - Pad; Index < EncryptedBytes.Num(); ++Index)
	{
		if (EncryptedBytes[Index] != Pad) return false;
	}
	EncryptedBytes.SetNum(EncryptedBytes.Num() - Pad);

	const FUTF8ToTCHAR Decoded(reinterpret_cast<const ANSICHAR*>(EncryptedBytes.GetData()), EncryptedBytes.Num());
	const FString Combined(Decoded.Length(), Decoded.Get());
	TArray<FString> Lines;
	Combined.ParseIntoArrayLines(Lines);
	FDateTime FileTimestamp;
	const bool bHasHeader = Lines.Num() > 0 && FDateTime::ParseIso8601(*Lines[0], FileTimestamp);
	for (int32 Index = bHasHeader ? 1 : 0; Index < Lines.Num(); ++Index)
	{
		const FString& Line = Lines[Index];
		int32 Separator;
		FDateTime EntryTimestamp;
		if (Line.FindChar('|', Separator) && FDateTime::ParseIso8601(*Line.Left(Separator), EntryTimestamp))
		{
			OutEntries.Emplace(EntryTimestamp, Line.Mid(Separator + 1));
		}
		else if (bHasHeader) { OutEntries.Emplace(FileTimestamp, Line); }
		else { return false; }
	}
	return true;
}