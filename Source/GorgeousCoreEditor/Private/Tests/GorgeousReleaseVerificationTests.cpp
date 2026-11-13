// Copyright (c) 2026 Simsalabim Studios.
#if WITH_DEV_AUTOMATION_TESTS
#include "LibraryWizard/GorgeousReleaseVerification.h"
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousReleaseSignatureTest,"GorgeousCore.Limitations.Updates.ReleaseSignatures",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FGorgeousReleaseSignatureTest::RunTest(const FString&)
{
    using namespace GorgeousReleaseVerification;
    TestTrue(TEXT("Go/OpenSSL signature parity"),Verify(TEXT("GorgeousAuditToken"),TEXT("1.2.3"),FString::ChrN(64,'0'),TEXT("w70JlZC/abp8+UXKRkqwE1JB883HyThaSoPk9KYKQr8jvBjMcm6zKjyu+mZPVwfxO/ljUVjny3ApSjhLpTDCCA=="),TEXT("A6EHv/POEL4dcN0Y50vAmWfk1jCbpQ1fHdyGZBJVMbg=")));
    // Ephemeral test-only key; no production private material is read or persisted.
    uint8 Seed[32]; for(int32 I=0;I<32;++I) Seed[I]=uint8(I);
    EVP_PKEY* Key=EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519,nullptr,Seed,32);
    if(!TestNotNull(TEXT("OpenSSL Ed25519 support"),Key)) return false;
    uint8 Public[32]; size_t PublicLength=32;
    const bool bPublic=EVP_PKEY_get_raw_public_key(Key,Public,&PublicLength)==1;
    const FString Digest=TEXT("0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef");
    FString Payload;
    TestTrue(TEXT("Build exact signed payload"),MakePayload(TEXT("test-token"),TEXT("1.2.3"),Digest,Payload));
    TestEqual(TEXT("Payload includes final newline"),Payload,TEXT("gorgeous-release-v1\ntest-token\n1.2.3\n")+Digest+TEXT("\n"));
    FTCHARToUTF8 Bytes(*Payload);
    uint8 Signature[64]; size_t SignatureLength=64;
    EVP_MD_CTX* Context=EVP_MD_CTX_new();
    const bool bSigned=Context && EVP_DigestSignInit(Context,nullptr,nullptr,nullptr,Key)==1 &&
        EVP_DigestSign(Context,Signature,&SignatureLength,reinterpret_cast<const uint8*>(Bytes.Get()),Bytes.Length())==1;
    if(Context) EVP_MD_CTX_free(Context);
    EVP_PKEY_free(Key);
    if(!TestTrue(TEXT("Create in-memory fixture"),bPublic && bSigned && PublicLength==32 && SignatureLength==64)) return false;
    const FString PublicKey=FBase64::Encode(Public,32), Encoded=FBase64::Encode(Signature,64);
    TestTrue(TEXT("Valid pinned release"),Verify(TEXT("test-token"),TEXT("1.2.3"),Digest,Encoded,PublicKey));
    TestTrue(TEXT("Hex digest canonicalization"),Verify(TEXT("test-token"),TEXT("1.2.3"),Digest.ToUpper(),Encoded,PublicKey));
    TestFalse(TEXT("Wrong token rejected"),Verify(TEXT("other-token"),TEXT("1.2.3"),Digest,Encoded,PublicKey));
    TestFalse(TEXT("Wrong version rejected"),Verify(TEXT("test-token"),TEXT("1.2.4"),Digest,Encoded,PublicKey));
    TestFalse(TEXT("Wrong hash rejected"),Verify(TEXT("test-token"),TEXT("1.2.3"),FString::ChrN(64,'0'),Encoded,PublicKey));
    TestFalse(TEXT("Missing trust key fails closed"),Verify(TEXT("test-token"),TEXT("1.2.3"),Digest,Encoded,TEXT("")));
    TestFalse(TEXT("Uncanonical base64 rejected"),Verify(TEXT("test-token"),TEXT("1.2.3"),Digest,Encoded+TEXT("\n"),PublicKey));
    TestFalse(TEXT("Missing signature rejected"),Verify(TEXT("test-token"),TEXT("1.2.3"),Digest,TEXT(""),PublicKey));
    TestFalse(TEXT("Delimiter injection rejected"),MakePayload(TEXT("bad\ntoken"),TEXT("1"),Digest,Payload));
    return true;
}
#endif
