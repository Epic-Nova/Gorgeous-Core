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

#include "ObjectVariables/GorgeousObjectVariableTrunk.h"

#include "ObjectVariables/GorgeousObjectVariable.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "UObject/Field.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/Crc.h"

const FName GorgeousObjectVariableTrunkTags::PropertyAccessor(TEXT("GorgeousObjectVariableTrunk"));

namespace
{
FGorgeousObjectVariableTrunk* ResolveTrunkInternal(UObject* Outer, const FName TrunkPropertyName)
{
	if (!Outer || TrunkPropertyName.IsNone())
	{
		return nullptr;
	}

	FProperty* CandidateProperty = Outer->GetClass()->FindPropertyByName(TrunkPropertyName);
	if (!CandidateProperty)
	{
		return nullptr;
	}

	if (FStructProperty* StructProperty = CastField<FStructProperty>(CandidateProperty))
	{
		if (StructProperty->Struct == FGorgeousObjectVariableTrunk::StaticStruct())
		{
			return StructProperty->ContainerPtrToValuePtr<FGorgeousObjectVariableTrunk>(Outer);
		}
	}

	return nullptr;
}
}

namespace GorgeousObjectVariableTrunk
{
	class FObjectVariableWriter : public FObjectAndNameAsStringProxyArchive
	{
	public:
		explicit FObjectVariableWriter(FArchive& InInnerArchive)
			: FObjectAndNameAsStringProxyArchive(InInnerArchive, true)
		{
			ArIsSaveGame = true;
			ArNoDelta = true;
		}
	};

	class FObjectVariableReader : public FObjectAndNameAsStringProxyArchive
	{
	public:
		explicit FObjectVariableReader(FArchive& InInnerArchive)
			: FObjectAndNameAsStringProxyArchive(InInnerArchive, true)
		{
			ArIsSaveGame = true;
			ArNoDelta = true;
		}
	};
}

FGorgeousObjectVariableTrunk* GorgeousObjectVariableTrunkUtils::ResolveTrunk(UObject* Outer, const FName TrunkPropertyName)
{
	return ResolveTrunkInternal(Outer, TrunkPropertyName);
}

const FGorgeousObjectVariableTrunk* GorgeousObjectVariableTrunkUtils::ResolveTrunk(const UObject* Outer, const FName TrunkPropertyName)
{
	return ResolveTrunkInternal(const_cast<UObject*>(Outer), TrunkPropertyName);
}

FGorgeousObjectVariableSerializedPayload::FGorgeousObjectVariableSerializedPayload()
	: VariableIdentifier(FGuid())
	, VariableClass(nullptr)
	, SelectedContainerType(EObjectVariableContainerType_E::ESingle)
	, bShowInnerProperties(false)
	, PayloadRevision(0)
	, PayloadHash(0)
{
}

bool FGorgeousObjectVariableSerializedPayload::IsValid() const
{
	return VariableClass != nullptr && SerializedBytes.Num() > 0;
}

void FGorgeousObjectVariableSerializedPayload::Touch()
{
	++PayloadRevision;
	PayloadHash = FGorgeousObjectVariableSerialization::ComputePayloadHash(SerializedBytes);
}

void FGorgeousObjectVariableSerializedPayload::ResetSerializedState()
{
	SerializedBytes.Reset();
	PayloadRevision = 0;
	PayloadHash = 0;
}

void FGorgeousObjectVariableSerializedPayload::ApplyMetadataFromClass(TSubclassOf<UGorgeousObjectVariable> InClass)
{
	VariableClass = InClass;
#if WITH_EDITORONLY_DATA
	if (InClass)
	{
		if (const UGorgeousObjectVariable* CDO = InClass->GetDefaultObject<UGorgeousObjectVariable>())
		{
			CachedPinConfiguration = CDO->GetObjectVariablePinConfiguration();
			SelectedContainerType = CachedPinConfiguration.ContainerType;
		}
	}
#endif
}

bool FGorgeousObjectVariableSerialization::WriteObjectToPayload(UGorgeousObjectVariable& Source, FGorgeousObjectVariableSerializedPayload& OutPayload)
{
	OutPayload.VariableClass = Source.GetClass();

	if (!OutPayload.VariableIdentifier.IsValid())
	{
		OutPayload.VariableIdentifier = Source.UniqueIdentifier.IsValid() ? Source.UniqueIdentifier : FGuid::NewGuid();
	}

#if WITH_EDITOR && WITH_EDITORONLY_DATA
	OutPayload.CachedPinConfiguration = Source.GetObjectVariablePinConfiguration();
	OutPayload.SelectedContainerType = OutPayload.CachedPinConfiguration.ContainerType;
#else
	OutPayload.SelectedContainerType = EObjectVariableContainerType_E::ESingle;
#endif

	TArray<uint8> Buffer;
	Buffer.Reserve(1024);

	FMemoryWriter MemoryWriter(Buffer, true);
	GorgeousObjectVariableTrunk::FObjectVariableWriter WriterProxy(MemoryWriter);
	Source.Serialize(WriterProxy);

	OutPayload.SerializedBytes = MoveTemp(Buffer);
	OutPayload.Touch();

	return true;
}

bool FGorgeousObjectVariableSerialization::LoadObjectFromPayload(UGorgeousObjectVariable& Destination, const FGorgeousObjectVariableSerializedPayload& InPayload)
{
	if (InPayload.SerializedBytes.Num() == 0)
	{
		return false;
	}

	TArray<uint8> BufferCopy(InPayload.SerializedBytes);
	FMemoryReader MemoryReader(BufferCopy, true);
	GorgeousObjectVariableTrunk::FObjectVariableReader ReaderProxy(MemoryReader);
	Destination.Serialize(ReaderProxy);

	return true;
}

uint32 FGorgeousObjectVariableSerialization::ComputePayloadHash(const TArray<uint8>& Bytes)
{
	return Bytes.Num() > 0 ? FCrc::MemCrc32(Bytes.GetData(), Bytes.Num()) : 0;
}

FGorgeousObjectVariableTrunk::FGorgeousObjectVariableTrunk()
{
}

const FGorgeousObjectVariableSerializedPayload* FGorgeousObjectVariableTrunk::FindPayload(const FGuid& Identifier) const
{
	return SerializedEntries.Find(Identifier);
}

FGorgeousObjectVariableSerializedPayload* FGorgeousObjectVariableTrunk::FindPayload(const FGuid& Identifier)
{
	return SerializedEntries.Find(Identifier);
}

FGorgeousObjectVariableSerializedPayload& FGorgeousObjectVariableTrunk::UpsertPayload(const FGuid& Identifier)
{
	FGuid Key = Identifier.IsValid() ? Identifier : FGuid::NewGuid();
	FGorgeousObjectVariableSerializedPayload& Payload = SerializedEntries.FindOrAdd(Key);
	Payload.VariableIdentifier = Key;
	return Payload;
}

void FGorgeousObjectVariableTrunk::SetPayload(const FGorgeousObjectVariableSerializedPayload& Payload)
{
	if (!Payload.VariableIdentifier.IsValid())
	{
		return;
	}

	SerializedEntries.Add(Payload.VariableIdentifier, Payload);
	InvalidateCache(Payload.VariableIdentifier);
}

bool FGorgeousObjectVariableTrunk::RemovePayload(const FGuid& Identifier)
{
	if (!Identifier.IsValid())
	{
		return false;
	}

	InvalidateCache(Identifier);
	return SerializedEntries.Remove(Identifier) > 0;
}

UGorgeousObjectVariable* FGorgeousObjectVariableTrunk::GetOrCreateRuntimeInstance(const FGuid& Identifier, UObject* Outer) const
{
	if (!Identifier.IsValid())
	{
		return nullptr;
	}

	const FGorgeousObjectVariableSerializedPayload* Payload = SerializedEntries.Find(Identifier);
	if (!Payload || !Payload->VariableClass)
	{
		return nullptr;
	}

	if (!Outer)
	{
		Outer = GetTransientPackage();
	}

	if (FGorgeousObjectVariableRuntimeCacheEntry* CacheEntry = RuntimeCache.Find(Identifier))
	{
		if (CacheEntry->PayloadRevision == Payload->PayloadRevision)
		{
			if (UGorgeousObjectVariable* CachedInstance = CacheEntry->Instance.Get())
			{
				return CachedInstance;
			}
		}
	}

	UGorgeousObjectVariable* NewInstance = BuildRuntimeInstance(*Payload, Outer);
	if (!NewInstance)
	{
		return nullptr;
	}

	FGorgeousObjectVariableRuntimeCacheEntry& CacheSlot = RuntimeCache.FindOrAdd(Identifier);
	CacheSlot.Instance = NewInstance;
	CacheSlot.PayloadRevision = Payload->PayloadRevision;

	return NewInstance;
}

void FGorgeousObjectVariableTrunk::InvalidateCache(const FGuid& Identifier) const
{
	if (!Identifier.IsValid())
	{
		return;
	}

	RuntimeCache.Remove(Identifier);
}

void FGorgeousObjectVariableTrunk::InvalidateAll() const
{
	RuntimeCache.Reset();
}

UGorgeousObjectVariable* FGorgeousObjectVariableTrunk::BuildRuntimeInstance(const FGorgeousObjectVariableSerializedPayload& Payload, UObject* Outer) const
{
	if (!Payload.VariableClass)
	{
		return nullptr;
	}

	UGorgeousObjectVariable* Instance = NewObject<UGorgeousObjectVariable>(Outer, Payload.VariableClass, NAME_None, RF_Transient);
	if (!Instance)
	{
		return nullptr;
	}

	if (!Instance->DeserializeFromPayload(Payload))
	{
		Instance->MarkAsGarbage();
		return nullptr;
	}

	return Instance;
}
