#pragma once

#include "CoreMinimal.h"

/**
 * Utility for creating and verifying encrypted license files for Gorgeous Core plugin licensing.
 */
struct FGorgeousCoreLicenseHelper
{
	/**
	 * Creates an encrypted license file in the GorgeousCore plugin folder with multiple license strings.
	 * @param LicenseStrings - The strings to store (e.g., user info, custom data).
	 * @param ProjectId - The project ID to use as the AES key.
	 * @return True if file was created successfully.
	 */
	static bool CreateEncryptedLicenseFile(const TArray<FString>& LicenseStrings, const FString& ProjectId);

	/**
	 * Backward compatible: Creates an encrypted license file with a single license string.
	 */
	static bool CreateEncryptedLicenseFile(const FString& LicenseString, const FString& ProjectId);

	/**
	 * Decrypts and reads the license file, returning all license strings.
	 * @param ProjectId - The project ID to use as the AES key.
	 * @param OutLicenseStrings - The decrypted license strings.
	 * @param OutDateTime - The date/time the file was created.
	 * @return True if decryption and read was successful.
	 */
	static bool ReadAndDecryptLicenseFile(const FString& ProjectId, TArray<FString>& OutLicenseStrings, FDateTime& OutDateTime);

	/**
	 * Backward compatible: Decrypts and reads the license file, returning only the first license string.
	 */
	static bool ReadAndDecryptLicenseFile(const FString& ProjectId, FString& OutLicenseString, FDateTime& OutDateTime);

	/**
	 * Appends a new entry (with timestamp) to the encrypted license file.
	 * @param EntryString - The string to store (e.g., feature, user, event).
	 * @param ProjectId - The project ID to use as the AES key.
	 * @return True if the entry was added successfully.
	 */
	static bool AddLicenseEntry(const FString& EntryString, const FString& ProjectId);

	/**
	 * Reads and decrypts all license entries (timestamp + string) from the license file.
	 * @param ProjectId - The project ID to use as the AES key.
	 * @param OutEntries - Array of (timestamp, string) pairs.
	 * @return True if read and decrypt was successful.
	 */
	static bool ReadAllLicenseEntries(const FString& ProjectId, TArray<TPair<FDateTime, FString>>& OutEntries);
};
