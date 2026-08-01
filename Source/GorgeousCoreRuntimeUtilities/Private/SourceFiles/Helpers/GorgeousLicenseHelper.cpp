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
//<-------------------------------------------------------------------------->

//=============================================================================
// FGorgeousCoreLicenseHelper Implementation
//=============================================================================

namespace
{
	FString GetLicenseFilePath()
	{
		return FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("GorgeousThings/GorgeousCore/License.lic"));
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
	if (!FBase64::Decode(Encoded, EncryptedBytes))
	{
		return false;
	}

	const FAES::FAESKey Key = MakeAESKey(ProjectId);
	FAES::DecryptData(EncryptedBytes.GetData(), EncryptedBytes.Num(), Key);

	// Remove padding
	if (EncryptedBytes.Num() == 0) return false;
	const uint8 Pad = EncryptedBytes.Last();
	if (Pad > 16) return false;
	EncryptedBytes.SetNum(EncryptedBytes.Num() - Pad);

	const FString Combined = FString(UTF8_TO_TCHAR(EncryptedBytes.GetData()));
	TArray<FString> Lines;
	Combined.ParseIntoArrayLines(Lines);
	if (Lines.Num() < 1) return false;
	if (!FDateTime::ParseIso8601(*Lines[0], OutDateTime)) return false;
	OutLicenseStrings.Empty();
	for (int32 i = 1; i < Lines.Num(); ++i)
	{
		OutLicenseStrings.Add(Lines[i]);
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
	ReadAllLicenseEntries(ProjectId, Entries); // ignore result, file may not exist yet

	FDateTime Now = FDateTime::UtcNow();
	Entries.Add(TPair<FDateTime, FString>(Now, EntryString));

	TArray<FString> Lines;
	for (const auto& Pair : Entries)
	{
		Lines.Add(Pair.Key.ToIso8601() + TEXT("|") + Pair.Value);
	}
	const FString Combined = FString::Join(Lines, TEXT("\n"));

	TArray<uint8> PlainBytes;
	const FTCHARToUTF8 Converter(*Combined);
	PlainBytes.Append((uint8*)Converter.Get(), Converter.Length());

	const FAES::FAESKey Key = MakeAESKey(ProjectId);
	const int32 Pad = 16 - (PlainBytes.Num() % 16);
	for (int32 i = 0; i < Pad; ++i) PlainBytes.Add(static_cast<uint8>(Pad));

	FAES::EncryptData(PlainBytes.GetData(), PlainBytes.Num(), Key);

	const FString Encoded = FBase64::Encode(PlainBytes);
	return FFileHelper::SaveStringToFile(Encoded, *GetLicenseFilePath());
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
	if (!FBase64::Decode(Encoded, EncryptedBytes))
	{
		return false;
	}

	const FAES::FAESKey Key = MakeAESKey(ProjectId);
	FAES::DecryptData(EncryptedBytes.GetData(), EncryptedBytes.Num(), Key);

	if (EncryptedBytes.Num() == 0) return true;
	const uint8 Pad = EncryptedBytes.Last();
	if (Pad > 16) return false;
	EncryptedBytes.SetNum(EncryptedBytes.Num() - Pad);

	const FString Combined = FString(UTF8_TO_TCHAR(EncryptedBytes.GetData()));
	TArray<FString> Lines;
	Combined.ParseIntoArrayLines(Lines);
	for (const FString& Line : Lines)
	{
		if (int32 SepIdx; Line.FindChar('|', SepIdx))
		{
			FString DateStr = Line.Left(SepIdx);
			FString ValueStr = Line.Mid(SepIdx + 1);
			FDateTime DT;
			if (FDateTime::ParseIso8601(*DateStr, DT))
			{
				OutEntries.Add(TPair<FDateTime, FString>(DT, ValueStr));
			}
		}
	}
	return true;
}