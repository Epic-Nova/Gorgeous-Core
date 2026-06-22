import re

path = '/media/aerien/Projects/SimsalabimStudio/Plugins/GorgeousThings/GorgeousShowcase/Plugins/Gorgeous-Core/Source/GorgeousCoreEditor/Private/SourceFiles/LibraryWizard/GorgeousUpdateManager.cpp'
with open(path, 'r') as f:
    content = f.read()

# Replace MD5 with SHA256
old_hash_logic = """                FMD5 MD5Gen;
                const int64 FileSize = FileReader->TotalSize();
                TArray<uint8> FileData;
                
                FileData.SetNumUninitialized(FileSize);
                FileReader->Serialize(FileData.GetData(), FileSize);
                MD5Gen.Update(FileData.GetData(), FileSize);
                
                FMD5Hash Hash;
                Hash.Set(MD5Gen);
                FinalFileHashes.Add(LexToString(Hash));"""

new_hash_logic = """                FSHA256 SHA256Gen;
                const int64 FileSize = FileReader->TotalSize();
                TArray<uint8> FileData;
                
                FileData.SetNumUninitialized(FileSize);
                FileReader->Serialize(FileData.GetData(), FileSize);
                SHA256Gen.Update(FileData.GetData(), FileSize);
                
                uint8 HashBytes[32];
                SHA256Gen.Final();
                SHA256Gen.GetHash(HashBytes);
                FinalFileHashes.Add(BytesToHex(HashBytes, 32).ToLower());"""

old_final_logic = """    // Now has all hashes to a single has
    FString FinalFileHash;
    FMD5 MD5Gen;
    for (const FString& Hash : FinalFileHashes)
    {
        MD5Gen.Update(reinterpret_cast<const uint8*>(TCHAR_TO_UTF8(*Hash)), Hash.Len());
    }
    FMD5Hash Hash;
    Hash.Set(MD5Gen);
    FinalFileHash = LexToString(Hash);"""

new_final_logic = """    // Now has all hashes to a single has
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

if "FMD5 MD5Gen;" in content:
    content = content.replace(old_hash_logic, new_hash_logic)
    content = content.replace(old_final_logic, new_final_logic)
    
with open(path, 'w') as f:
    f.write(content)
