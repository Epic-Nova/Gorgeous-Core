import re

path = '/media/aerien/Projects/SimsalabimStudio/Plugins/GorgeousThings/GorgeousShowcase/Plugins/Gorgeous-Core/Source/GorgeousCoreEditor/Private/SourceFiles/LibraryWizard/GorgeousUpdateManager.cpp'
with open(path, 'r') as f:
    content = f.read()

# Make sure we include GenericPlatformMisc
if '#include "GenericPlatform/GenericPlatformMisc.h"' not in content:
    content = content.replace('#include "SourceFiles/LibraryWizard/GorgeousUpdateManager.h"', '#include "SourceFiles/LibraryWizard/GorgeousUpdateManager.h"\n#include "GenericPlatform/GenericPlatformMisc.h"')

# Replace individual file hashing
old_file_hash = """                FSHA256 SHA256Gen;
                const int64 FileSize = FileReader->TotalSize();
                TArray<uint8> FileData;
                
                FileData.SetNumUninitialized(FileSize);
                FileReader->Serialize(FileData.GetData(), FileSize);
                SHA256Gen.Update(FileData.GetData(), FileSize);
                
                uint8 HashBytes[32];
                SHA256Gen.Final();
                SHA256Gen.GetHash(HashBytes);
                FinalFileHashes.Add(BytesToHex(HashBytes, 32).ToLower());"""

new_file_hash = """                const int64 FileSize = FileReader->TotalSize();
                TArray<uint8> FileData;
                
                FileData.SetNumUninitialized(FileSize);
                FileReader->Serialize(FileData.GetData(), FileSize);

                FSHA256Signature Signature;
                if (FPlatformMisc::GetSHA256Signature(FileData.GetData(), FileSize, Signature))
                {
                    FinalFileHashes.Add(Signature.ToString().ToLower());
                }"""

# Replace final combining hash
old_final_hash = """    // Now has all hashes to a single has
    FString FinalFileHash;
    FSHA256 SHA256Gen;
    for (const FString& Hash : FinalFileHashes)
    {
        SHA256Gen.Update(reinterpret_cast<const uint8*>(TCHAR_TO_UTF8(*Hash)), Hash.Len());
    }
    uint8 HashBytes[32];
    SHA256Gen.Final();
    SHA256Gen.GetHash(HashBytes);
    FinalFileHash = BytesToHex(HashBytes, 32).ToLower();"""

new_final_hash = """    // Now hash all hashes to a single hash
    FString CombinedHashes;
    for (const FString& Hash : FinalFileHashes)
    {
        CombinedHashes += Hash;
    }
    
    FTCHARToUTF8 Utf8String(*CombinedHashes);
    FSHA256Signature FinalSignature;
    if (FPlatformMisc::GetSHA256Signature(Utf8String.Get(), Utf8String.Length(), FinalSignature))
    {
        return FinalSignature.ToString().ToLower();
    }
    return TEXT("");"""

content = content.replace(old_file_hash, new_file_hash)
content = content.replace(old_final_hash, new_final_hash)
content = content.replace("    return FinalFileHash;\n}", "}")

with open(path, 'w') as f:
    f.write(content)
