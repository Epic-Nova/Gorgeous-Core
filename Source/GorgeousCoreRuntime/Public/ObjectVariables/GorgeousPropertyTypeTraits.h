#pragma once
#include "CoreMinimal.h"
#include "UObject/UnrealType.h"

// FProperty::IsA<TProperty<T,FProperty>> only tests FProperty, not T.
// These traits validate the complete reflected type before touching its storage.
template<typename T> struct TGorgeousPropertyTypeTraits { static bool Matches(const FProperty*) { return false; } };
template<> struct TGorgeousPropertyTypeTraits<bool> { static bool Matches(const FProperty* P) { return CastField<FBoolProperty>(P) != nullptr; } };
template<> struct TGorgeousPropertyTypeTraits<uint8> { static bool Matches(const FProperty* P) { return CastField<FByteProperty>(P) != nullptr; } };
template<> struct TGorgeousPropertyTypeTraits<int32> { static bool Matches(const FProperty* P) { return CastField<FIntProperty>(P) != nullptr; } };
template<> struct TGorgeousPropertyTypeTraits<int64> { static bool Matches(const FProperty* P) { return CastField<FInt64Property>(P) != nullptr; } };
template<> struct TGorgeousPropertyTypeTraits<float> { static bool Matches(const FProperty* P) { return CastField<FFloatProperty>(P) != nullptr; } };
template<> struct TGorgeousPropertyTypeTraits<double> { static bool Matches(const FProperty* P) { return CastField<FDoubleProperty>(P) != nullptr; } };
template<> struct TGorgeousPropertyTypeTraits<FName> { static bool Matches(const FProperty* P) { return CastField<FNameProperty>(P) != nullptr; } };
template<> struct TGorgeousPropertyTypeTraits<FString> { static bool Matches(const FProperty* P) { return CastField<FStrProperty>(P) != nullptr; } };
template<> struct TGorgeousPropertyTypeTraits<FText> { static bool Matches(const FProperty* P) { return CastField<FTextProperty>(P) != nullptr; } };
template<> struct TGorgeousPropertyTypeTraits<FRotator> { static bool Matches(const FProperty* P) { const FStructProperty* S=CastField<FStructProperty>(P); return S && S->Struct == TBaseStructure<FRotator>::Get(); } };
template<> struct TGorgeousPropertyTypeTraits<FObjectVariableMultiMapValue> { static bool Matches(const FProperty* P) { const FStructProperty* S=CastField<FStructProperty>(P); return S && S->Struct == FObjectVariableMultiMapValue::StaticStruct(); } };
template<typename T> struct TGorgeousPropertyTypeTraits<T*> { static bool Matches(const FProperty* P) { const FObjectProperty* O=CastField<FObjectProperty>(P); return O && O->PropertyClass == T::StaticClass(); } };
template<typename T> struct TGorgeousPropertyTypeTraits<TSoftObjectPtr<T>> { static bool Matches(const FProperty* P) { const FSoftObjectProperty* O=CastField<FSoftObjectProperty>(P); return O && O->PropertyClass == T::StaticClass(); } };
template<typename T> struct TGorgeousPropertyTypeTraits<TSoftClassPtr<T>> { static bool Matches(const FProperty* P) { const FSoftClassProperty* O=CastField<FSoftClassProperty>(P); return O && O->MetaClass == T::StaticClass(); } };
template<typename T> struct TGorgeousPropertyTypeTraits<TArray<T>> { static bool Matches(const FProperty* P) { const FArrayProperty* A=CastField<FArrayProperty>(P); return A && TGorgeousPropertyTypeTraits<T>::Matches(A->Inner); } };
template<typename T> struct TGorgeousPropertyTypeTraits<TSet<T>> { static bool Matches(const FProperty* P) { const FSetProperty* A=CastField<FSetProperty>(P); return A && TGorgeousPropertyTypeTraits<T>::Matches(A->ElementProp); } };
template<typename K,typename V> struct TGorgeousPropertyTypeTraits<TMap<K,V>> { static bool Matches(const FProperty* P) { const FMapProperty* A=CastField<FMapProperty>(P); return A && TGorgeousPropertyTypeTraits<K>::Matches(A->KeyProp) && TGorgeousPropertyTypeTraits<V>::Matches(A->ValueProp); } };
