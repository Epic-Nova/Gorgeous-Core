// Copyright (c) 2026 Simsalabim Studios.
#pragma once
#include "CoreMinimal.h"
#include "Misc/Base64.h"
#include "HAL/PlatformMisc.h"
#pragma push_macro("UI")
#undef UI
#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#include "openssl/evp.h"
THIRD_PARTY_INCLUDES_END
#undef UI
#pragma pop_macro("UI")

/** Detached release signatures. Trust comes only from operator configuration, never the catalog. */
namespace GorgeousReleaseVerification
{
inline bool IsFieldValid(const FString& Value)
{
    if (Value.IsEmpty() || Value.Len()>4096) return false;
    for (TCHAR C : Value) if (C==0 || C=='\r' || C=='\n') return false;
    return true;
}
inline bool MakePayload(const FString& Token, const FString& Version, const FString& Digest, FString& Out)
{
    if (!IsFieldValid(Token) || !IsFieldValid(Version) || Digest.Len()!=64) return false;
    for (TCHAR C : Digest) if (!((C>='0' && C<='9') || (C>='a' && C<='f') || (C>='A' && C<='F'))) return false;
    Out=TEXT("gorgeous-release-v1\n")+Token+TEXT("\n")+Version+TEXT("\n")+Digest.ToLower()+TEXT("\n");
    return true;
}
inline bool DecodeCanonicalBase64(const FString& Encoded, int32 ExpectedBytes, TArray<uint8>& Out)
{
    return Encoded.Len()==((ExpectedBytes+2)/3)*4 && FBase64::Decode(Encoded,Out) && Out.Num()==ExpectedBytes && FBase64::Encode(Out)==Encoded;
}
inline bool Verify(const FString& Token, const FString& Version, const FString& Digest, const FString& Signature, const FString& PublicKey)
{
    FString Payload;
    TArray<uint8> KeyBytes, SignatureBytes;
    if (!MakePayload(Token,Version,Digest,Payload) || !DecodeCanonicalBase64(PublicKey,32,KeyBytes) || !DecodeCanonicalBase64(Signature,64,SignatureBytes)) return false;
    EVP_PKEY* Key=EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519,nullptr,KeyBytes.GetData(),KeyBytes.Num());
    EVP_MD_CTX* Context=EVP_MD_CTX_new();
    FTCHARToUTF8 Bytes(*Payload);
    const bool bValid=Key && Context && EVP_DigestVerifyInit(Context,nullptr,nullptr,nullptr,Key)==1 &&
        EVP_DigestVerify(Context,SignatureBytes.GetData(),SignatureBytes.Num(),reinterpret_cast<const uint8*>(Bytes.Get()),Bytes.Length())==1;
    if (Context) EVP_MD_CTX_free(Context);
    if (Key) EVP_PKEY_free(Key);
    return bValid;
}
inline bool VerifyWithPinnedKey(const FString& Token, const FString& Version, const FString& Digest, const FString& Signature)
{
    return Verify(Token,Version,Digest,Signature,FPlatformMisc::GetEnvironmentVariable(TEXT("GORGEOUS_RELEASE_PUBLIC_KEY")));
}
}
