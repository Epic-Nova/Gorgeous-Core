// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#include "ObjectVariables/GorgeousObjectVariable.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "ObjectVariables/GorgeousObjectVariableTrunk.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "AutoReplication/Interfaces/GorgeousAutoReplicationSpawnHook_I.h"
#include "ObjectVariables/GorgeousRootNetworkStackSubsystem.h"
#include "GorgeousCoreUtilitiesMinimalShared.h"
#include "AutoReplication/ObjectVariables/GorgeousRPC_OV.h"
#include "AutoReplication/GorgeousAutoReplicationCoordinator.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
#include "AutoReplication/GorgeousAutoReplicationRPCTransporter.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRelayComponent.h"
#include "ModuleCore/GorgeousAutoReplicationSettings.h"
#include "ModuleCore/GorgeousObjectVariableRootSettings.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include "Helpers/Macros/GorgeousProfilingHelperMacros.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Engine/ActorChannel.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BitWriter.h"
#include "Serialization/BitReader.h"
#include "Serialization/StructuredArchive.h"
#include "Serialization/Formatters/BinaryArchiveFormatter.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/Field.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UnrealType.h"
#include "Misc/ScopeLock.h"
#include "Containers/Ticker.h"
#include "GameFramework/PlayerState.h"
#include "HAL/ThreadSafeCounter.h"
#include "Stats/Stats.h"
#include "Net/UnrealNetwork.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousObjectVariable Implementation
//=============================================================================

FSimpleMulticastDelegate UGorgeousObjectVariable::OnVariableTreeChanged;

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousObjectVariable, Log, All);

DECLARE_STATS_GROUP(TEXT("Gorgeous Object Variables"), STATGROUP_GorgeousObjectVariables, STATCAT_Advanced);
DECLARE_DWORD_COUNTER_STAT(TEXT("Created"), STAT_GOV_Created, STATGROUP_GorgeousObjectVariables);
DECLARE_DWORD_COUNTER_STAT(TEXT("Destroyed"), STAT_GOV_Destroyed, STATGROUP_GorgeousObjectVariables);
DECLARE_DWORD_COUNTER_STAT(TEXT("Alive"), STAT_GOV_Alive, STATGROUP_GorgeousObjectVariables);

namespace
{
	FThreadSafeCounter GObjectVariableAliveCounter;
}

FGorgeousObjectVariableRootConfiguration::FGorgeousObjectVariableRootConfiguration()
	: PreferredRootName(NAME_None)
{
}

FName FGorgeousObjectVariableRootConfiguration::ResolvePreferredRootName() const
{
	if (!PreferredRootName.IsNone())
	{
		return PreferredRootName;
	}

	return UGorgeousRootObjectVariable::GetDefaultRootName();
}

FGorgeousRootNetworkAccessConfig::FGorgeousRootNetworkAccessConfig()
	: bExposeThroughRootNetworkStack(false)
	, AccessPolicy(EGorgeousObjectVariableAccessPolicy::Everyone)
	, ReplicationChannel(NAME_None)
{
}

UGorgeousObjectVariable::FReplicatedPropertyDeclaration::FReplicatedPropertyDeclaration()
	: PropertyName(NAME_None)
	, CachedProperty(nullptr)
	, Mode(EGorgeousReplicationMode::EProperty)
	, bSendInitialState(true)
	, ReplicationCondition(COND_None)
	, RepNotifyFunction(NAME_None)
	, RepNotifyPolicy(EGorgeousRepNotifyPolicy::OnChanged)
	, bFireInitialNotify(true)
	, bInitializeNullReferences(false)
	, bDeliveredInitialNotify(false)
	, bShadowInitialized(false)
	, bChangeShadowInitialized(false)
	, bHasValidatedRepNotifySignature(false)
	, bIsRepNotifySignatureValid(true)
{
}

UGorgeousObjectVariable::FReplicatedPropertyDeclaration::~FReplicatedPropertyDeclaration()
{
	ResetShadowState();
	ResetChangeShadow();
}

void UGorgeousObjectVariable::FReplicatedPropertyDeclaration::InitializeShadowState(const FProperty* Property, const void* InitialData)
{
	if (!Property)
	{
		return;
	}

	if (!bShadowInitialized)
	{
		GORGEOUS_55_HIGHER(
			const int32 ShadowSize = Property->GetElementSize() * Property->ArrayDim;
		)
		GORGEOUS_54_LOWER(
			const int32 ShadowSize = Property->GetSize() * Property->ArrayDim;
		)
		RepNotifyShadow.SetNumZeroed(ShadowSize > 0 ? ShadowSize : 1);
		Property->InitializeValue(RepNotifyShadow.GetData());
		bShadowInitialized = true;
	}

	if (InitialData)
	{
		Property->CopyCompleteValue(RepNotifyShadow.GetData(), InitialData);
	}
}

void UGorgeousObjectVariable::FReplicatedPropertyDeclaration::ResetShadowState()
{
	if (bShadowInitialized && CachedProperty && RepNotifyShadow.Num() > 0)
	{
		CachedProperty->DestroyValue(RepNotifyShadow.GetData());
	}
	RepNotifyShadow.Reset();
	bShadowInitialized = false;
}

void UGorgeousObjectVariable::FReplicatedPropertyDeclaration::InitializeChangeShadow(const FProperty* Property, const void* InitialData)
{
	if (!Property)
	{
		return;
	}

	if (!bChangeShadowInitialized)
	{
		GORGEOUS_55_HIGHER(
			const int32 ShadowSize = Property->GetElementSize() * Property->ArrayDim;
		)
		GORGEOUS_54_LOWER(
			const int32 ShadowSize = Property->GetSize() * Property->ArrayDim;
		)
		ChangeShadow.SetNumZeroed(ShadowSize > 0 ? ShadowSize : 1);
		Property->InitializeValue(ChangeShadow.GetData());
		bChangeShadowInitialized = true;
	}

	if (InitialData)
	{
		Property->CopyCompleteValue(ChangeShadow.GetData(), InitialData);
	}
}

void UGorgeousObjectVariable::FReplicatedPropertyDeclaration::ResetChangeShadow()
{
	if (bChangeShadowInitialized && CachedProperty && ChangeShadow.Num() > 0)
	{
		CachedProperty->DestroyValue(ChangeShadow.GetData());
	}
	ChangeShadow.Reset();
	bChangeShadowInitialized = false;
}

bool UGorgeousObjectVariable::ValidateVariableAssignment(const FName PropertyName, const FProperty* ValueProperty, const void* ValueAddress) const
{
	return true;
}

namespace GorgeousObjectVariable_Private
{
	static constexpr int32 MaxTrackedRPCResults = 8;
	static const FName GorgeousRPCResultsDisplayName(TEXT("GorgeousRPCResults"));
	static TWeakObjectPtr<UGorgeousObjectVariable> CachedRPCResultsParent;

	// Forward-declare the Snapshot helpers so they are visible to SerializePropertyValue /
	// DeserializePropertyValue which are defined before the full namespace Snapshot block.
	namespace Snapshot
	{
		static bool  ShouldCaptureProperty(const FProperty* Property);
		static void  WriteByteArray(FArchive& Ar, const TArray<uint8>& Data);
		static void  ReadByteArray(FArchive& Ar, TArray<uint8>& Data);
	}

	static bool SerializePropertyValue(UGorgeousObjectVariable* Context, const FProperty* Property, void* ValuePtr, const EGorgeousReplicationMode Mode, UPackageMap* PackageMap, TArray<uint8>& OutBytes, bool bDeepInitialize = false)
	{
		if (!Property || !ValuePtr)
		{
			return false;
		}

		OutBytes.Reset();

		auto DeepSerializeObject = [&](UObject* Obj, FArchive& Ar)
		{
			// 1. Write the standard name string for resolving existing objects
			FObjectAndNameAsStringProxyArchive Proxy(Ar, true);
			Proxy << Obj;

			if (bDeepInitialize)
			{
				bool bHasDeepData = false;
				UGorgeousObjectVariable* AsOV = Cast<UGorgeousObjectVariable>(Obj);
				// We only deep-initialize for OVs (for now) and we skip Actors/Components
				if (Obj && AsOV && !Obj->IsA<AActor>() && !Obj->IsA<UActorComponent>())
				{
					bHasDeepData = true;
				}

				Ar << bHasDeepData;
				if (bHasDeepData)
				{
					TArray<uint8> Snapshot;
					Context->BuildRPCResultSnapshot(AsOV, Snapshot);
					Snapshot::WriteByteArray(Ar, Snapshot);
				}
			}
		};

		switch (Mode)
		{
		case EGorgeousReplicationMode::EProperty:
		{
			FMemoryWriter Writer(OutBytes, true);
			if (bDeepInitialize)
			{
				if (const FObjectProperty* ObjProp = CastField<FObjectProperty>(Property))
				{
					UObject* Obj = ObjProp->GetObjectPropertyValue(ValuePtr);
					DeepSerializeObject(Obj, Writer);
					return !Writer.IsError();
				}
				else if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(Property))
				{
					if (const FObjectProperty* InnerObjProp = CastField<FObjectProperty>(ArrayProp->Inner))
					{
						FScriptArrayHelper Helper(ArrayProp, ValuePtr);
						int32 Count = Helper.Num();
						Writer << Count;
						for (int32 i = 0; i < Count; ++i)
						{
							UObject* Obj = InnerObjProp->GetObjectPropertyValue(Helper.GetRawPtr(i));
							DeepSerializeObject(Obj, Writer);
						}
						return !Writer.IsError();
					}
				}
				else if (const FMapProperty* MapProp = CastField<FMapProperty>(Property))
				{
					const FObjectProperty* ValueObjProp = CastField<FObjectProperty>(MapProp->ValueProp);
					if (ValueObjProp)
					{
						FScriptMapHelper Helper(MapProp, ValuePtr);
						int32 Count = Helper.Num();
						Writer << Count;
						for (int32 i = 0; i < Count; ++i)
						{
							if (Helper.IsValidIndex(i))
							{
								// Serialize Key (normal structured path)
								FBinaryArchiveFormatter Formatter(Writer);
								FStructuredArchive StructAr(Formatter);
								MapProp->KeyProp->SerializeItem(StructAr.Open(), Helper.GetKeyPtr(i));
								StructAr.Close();

								// Serialize Value (Deep)
								UObject* Obj = ValueObjProp->GetObjectPropertyValue(Helper.GetValuePtr(i));
								DeepSerializeObject(Obj, Writer);
							}
						}
						return !Writer.IsError();
					}
				}
			}

			FObjectAndNameAsStringProxyArchive ProxyWriter(Writer, true);
			FBinaryArchiveFormatter Formatter(ProxyWriter);
			FStructuredArchive Archive(Formatter);
			FStructuredArchive::FSlot Slot = Archive.Open();
			Property->SerializeItem(Slot, ValuePtr, nullptr);
			Archive.Close();
			return !ProxyWriter.IsError();
		}
		case EGorgeousReplicationMode::ENetSerialize:
		{
			// ... existing fallback ...
			const bool bIsCollection =
				CastField<FArrayProperty>(Property) != nullptr ||
				CastField<FMapProperty>(Property) != nullptr ||
				CastField<FSetProperty>(Property) != nullptr;
			if (bIsCollection)
			{
				return SerializePropertyValue(Context, Property, ValuePtr, EGorgeousReplicationMode::EProperty, PackageMap, OutBytes, bDeepInitialize);
			}
			FBitWriter BitWriter(0, true);
			const bool bNetSerialized = Property->NetSerializeItem(BitWriter, PackageMap, ValuePtr);
			if (!bNetSerialized || BitWriter.IsError())
			{
				return false;
			}
			const uint32 NumBits = (uint32)BitWriter.GetNumBits();
			OutBytes.Append(reinterpret_cast<const uint8*>(&NumBits), sizeof(uint32));
			OutBytes.Append(*BitWriter.GetBuffer());
			return true;
		}
		case EGorgeousReplicationMode::ECustomPayload:
		default:
			return false;
		}
	}

	static bool DeserializePropertyValue(UGorgeousObjectVariable* Context, FProperty* Property, void* ValuePtr, const EGorgeousReplicationMode Mode, UPackageMap* PackageMap, const TArray<uint8>& InBytes, bool bDeepInitialize = false)
	{
		if (!Property || !ValuePtr || InBytes.Num() == 0)
		{
			return false;
		}

		auto DeepDeserializeObject = [&](UObject*& OutObj, FArchive& Ar)
		{
			FObjectAndNameAsStringProxyArchive Proxy(Ar, true);
			Proxy << OutObj;

			if (bDeepInitialize)
			{
				bool bHasDeepData = false;
				Ar << bHasDeepData;
				if (bHasDeepData)
				{
					TArray<uint8> Snapshot;
					Snapshot::ReadByteArray(Ar, Snapshot);

					if (!OutObj && Snapshot.Num() > 0)
					{
						OutObj = Context->DeserializeOVFromRPCArgumentBytes(Snapshot);
						if (OutObj)
						{
							if (OutObj->GetClass()->ImplementsInterface(UGorgeousAutoReplicationSpawnHook_I::StaticClass()))
							{
								IGorgeousAutoReplicationSpawnHook_I::Execute_OnSpawnedThroughAutoReplication(OutObj);
							}
						}
					}
				}
			}
		};

		switch (Mode)
		{
		case EGorgeousReplicationMode::EProperty:
		{
			FMemoryReader Reader(InBytes, true);
			if (bDeepInitialize)
			{
				if (const FObjectProperty* ObjProp = CastField<FObjectProperty>(Property))
				{
					UObject* Obj = nullptr;
					DeepDeserializeObject(Obj, Reader);
					ObjProp->SetObjectPropertyValue(ValuePtr, Obj);
					return !Reader.IsError();
				}
				else if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(Property))
				{
					if (const FObjectProperty* InnerObjProp = CastField<FObjectProperty>(ArrayProp->Inner))
					{
						FScriptArrayHelper Helper(ArrayProp, ValuePtr);
						int32 Count = 0;
						Reader << Count;
						Helper.Resize(Count);
						for (int32 i = 0; i < Count; ++i)
						{
							UObject* Obj = nullptr;
							DeepDeserializeObject(Obj, Reader);
							InnerObjProp->SetObjectPropertyValue(Helper.GetRawPtr(i), Obj);
						}
						return !Reader.IsError();
					}
				}
				else if (const FMapProperty* MapProp = CastField<FMapProperty>(Property))
				{
					const FObjectProperty* ValueObjProp = CastField<FObjectProperty>(MapProp->ValueProp);
					if (ValueObjProp)
					{
						FScriptMapHelper Helper(MapProp, ValuePtr);
						int32 Count = 0;
						Reader << Count;
						Helper.EmptyValues();
						for (int32 i = 0; i < Count; ++i)
						{
							int32 NewIdx = Helper.AddDefaultValue_Invalid_NeedsRehash();

							// Deserialize Key
							FBinaryArchiveFormatter Formatter(Reader);
							FStructuredArchive StructAr(Formatter);
							MapProp->KeyProp->SerializeItem(StructAr.Open(), Helper.GetKeyPtr(NewIdx));
							StructAr.Close();

							// Deserialize Value (Deep)
							UObject* Obj = nullptr;
							DeepDeserializeObject(Obj, Reader);
							ValueObjProp->SetObjectPropertyValue(Helper.GetValuePtr(NewIdx), Obj);
						}
						Helper.Rehash();
						return !Reader.IsError();
					}
				}
			}

			FObjectAndNameAsStringProxyArchive ProxyReader(Reader, true);
			FBinaryArchiveFormatter Formatter(ProxyReader);
			FStructuredArchive Archive(Formatter);
			FStructuredArchive::FSlot Slot = Archive.Open();
			Property->SerializeItem(Slot, ValuePtr, nullptr);
			Archive.Close();
			return !ProxyReader.IsError();
		}
		case EGorgeousReplicationMode::ENetSerialize:
		{
			const bool bIsCollection =
				CastField<FArrayProperty>(Property) != nullptr ||
				CastField<FMapProperty>(Property) != nullptr ||
				CastField<FSetProperty>(Property) != nullptr;
			if (bIsCollection)
			{
				return DeserializePropertyValue(Context, Property, ValuePtr, EGorgeousReplicationMode::EProperty, PackageMap, InBytes, bDeepInitialize);
			}
			// ... existing bit reader path ...
			if (InBytes.Num() < (int32)sizeof(uint32)) return false;
			uint32 NumBits = 0;
			FMemory::Memcpy(&NumBits, InBytes.GetData(), sizeof(uint32));
			uint8* PayloadData = const_cast<uint8*>(InBytes.GetData()) + sizeof(uint32);
			FBitReader BitReader(PayloadData, (int64)NumBits);
			return Property->NetSerializeItem(BitReader, PackageMap, ValuePtr) && !BitReader.IsError();
		}
		case EGorgeousReplicationMode::ECustomPayload:
		default:
			return false;
		}
	}

	static FString NormalizeLabel(const FString& Input)
	{
		if (Input.IsEmpty())
		{
			return Input;
		}

		FString WorkingCopy = Input;
		WorkingCopy.TrimStartAndEndInline();
		TArray<FString> Parts;
		WorkingCopy.ParseIntoArray(Parts, TEXT("-"), true);
		if (Parts.Num() == 0)
		{
			Parts.Add(WorkingCopy);
		}
		for (FString& Part : Parts)
		{
			Part.ToLowerInline();
			if (!Part.IsEmpty())
			{
				Part[0] = TChar<TCHAR>::ToUpper(Part[0]);
			}
		}

		return FString::Join(Parts, TEXT("-"));
	}

	static FString GenerateRandomDisplayName()
	{
		static const TCHAR* FirstNamePool[] = {
			TEXT("Hinata"), TEXT("Kaito"), TEXT("Yukihiro"), TEXT("Ashura"), TEXT("Ryoma"), TEXT("Haruki"), TEXT("Renji"), TEXT("Yuki"), TEXT("Sakura"), TEXT("Takumi"),
			TEXT("Akira"), TEXT("Misaki"), TEXT("Haruko"), TEXT("Kazuya"), TEXT("Nanami"), TEXT("Yukiko"), TEXT("Mirai"), TEXT("Akiha"), TEXT("Kaze"), TEXT("Aoi")
		};

		static const TCHAR* SecondNamePool[] = {
			TEXT("Realm"), TEXT("Zenith"), TEXT("Vista"), TEXT("Empire"), TEXT("Triumph"), TEXT("Regency"), TEXT("Ascendance"), TEXT("Meadow"), TEXT("Elysium"),
			TEXT("Stage"), TEXT("Haven"), TEXT("Sanctum"), TEXT("Harbor"), TEXT("Oasis"), TEXT("Reflection"), TEXT("Playground"), TEXT("Workshop"), TEXT("Canvas"),
			TEXT("Sanctuary"), TEXT("Innovation")
		};

		const int32 FirstIndex = FMath::RandHelper(UE_ARRAY_COUNT(FirstNamePool));
		const int32 SecondIndex = FMath::RandHelper(UE_ARRAY_COUNT(SecondNamePool));
		return FString::Printf(TEXT("%ss-%s"), FirstNamePool[FirstIndex], SecondNamePool[SecondIndex]);
	}

	/**
	 * Copy the serialized bytes stored in @p Arg into the handler's pre-allocated
	 * parameter slot described by @p ParameterProperty / @p ParameterData.
	 *
	 * Three paths:
	 *   1. Struct types   – FStructuredArchive deserialization via SerializeTaggedProperties.
	 *   2. String / Text  – FMemoryReader + SerializeItem (heap-allocated, needs archive).
	 *   3. All others     – raw memcpy (POD scalar types: int, float, double, bool, FName …).
	 */
	static bool CopyArgumentToProperty(
		const FGorgeousRPCArgumentContainer& Arg,
		FProperty* ParameterProperty,
		uint8* ParameterData)
	{
		if (Arg.ValueBytes.IsEmpty() || !ParameterProperty || !ParameterData)
		{
			return false;
		}

		// Verify the stored property class matches the handler parameter.
		if (Arg.PropertyClassName != ParameterProperty->GetClass()->GetFName())
		{
			GT_W_LOG("GT.ObjectVariables.RPC.ArgumentTypeMismatch",
				TEXT("AutoReplication RPC argument '%s': stored type '%s' is incompatible with "
				     "handler parameter '%s' (type '%s')."),
				*Arg.ArgumentName.ToString(), *Arg.PropertyClassName.ToString(),
				*ParameterProperty->GetName(), *ParameterProperty->GetClass()->GetName());
			return false;
		}

		void* DestPtr = ParameterProperty->ContainerPtrToValuePtr<void>(ParameterData);

		// ── Path 1: Struct ────────────────────────────────────────────────────────────
		if (const FStructProperty* AsStruct = CastField<FStructProperty>(ParameterProperty))
		{
			if (!AsStruct->Struct || AsStruct->Struct->GetFName() != Arg.StructTypeName)
			{
				GT_W_LOG("GT.ObjectVariables.RPC.ArgumentStructMismatch",
					TEXT("AutoReplication RPC argument '%s': stored struct '%s' != parameter struct '%s'."),
					*Arg.ArgumentName.ToString(), *Arg.StructTypeName.ToString(),
					AsStruct->Struct ? *AsStruct->Struct->GetName() : TEXT("<null>"));
				return false;
			}
			// The struct value was serialized via FObjectAndNameAsStringProxyArchive – deserialize the same way
			// so that UObject references (FSoftObjectPtr, FSoftObjectPath, etc.) round-trip correctly.
			FMemoryReader Reader(Arg.ValueBytes, true);
			FObjectAndNameAsStringProxyArchive ProxyReader(Reader, true);
			AsStruct->Struct->SerializeTaggedProperties(ProxyReader, static_cast<uint8*>(DestPtr), AsStruct->Struct, nullptr);
			return true;
		}

		// ── Path 2: FString / FText (heap-allocated, archive round-trip) ──────────────
		if (CastField<FStrProperty>(ParameterProperty) || CastField<FTextProperty>(ParameterProperty))
		{
			FMemoryReader Reader(Arg.ValueBytes);
			ParameterProperty->SerializeItem(FStructuredArchiveFromArchive(Reader).GetSlot(), DestPtr);
			return true;
		}

		// ── Path X: FObjectProperty – UGorgeousObjectVariable snapshot reconstruction ─
		if (const FObjectProperty* AsObject = CastField<FObjectProperty>(ParameterProperty))
		{
			UGorgeousObjectVariable* ReconstructedOV = UGorgeousObjectVariable::DeserializeOVFromRPCArgumentBytes(Arg.ValueBytes);
			if (!ReconstructedOV)
			{
				GT_W_LOG("GT.ObjectVariables.RPC.ArgumentOVDeserializeFailed",
					TEXT("AutoReplication RPC argument '%s': failed to reconstruct UGorgeousObjectVariable from snapshot bytes."),
					*Arg.ArgumentName.ToString());
				return false;
			}
			if (AsObject->PropertyClass && !ReconstructedOV->GetClass()->IsChildOf(AsObject->PropertyClass))
			{
				GT_W_LOG("GT.ObjectVariables.RPC.ArgumentOVClassMismatch",
					TEXT("AutoReplication RPC argument '%s': reconstructed OV class '%s' is not compatible with handler parameter type '%s'."),
					*Arg.ArgumentName.ToString(), *ReconstructedOV->GetClass()->GetName(),
					*AsObject->PropertyClass->GetName());
				return false;
			}
			AsObject->SetObjectPropertyValue(DestPtr, ReconstructedOV);
			return true;
		}

		// ── Path 3: POD / scalar – raw memcpy ─────────────────────────────────────────
		GORGEOUS_55_HIGHER(const int32 PropSize = ParameterProperty->GetElementSize();)
		GORGEOUS_54_LOWER(const int32 PropSize  = ParameterProperty->GetSize();)
		if (Arg.ValueBytes.Num() != PropSize)
		{
			GT_W_LOG("GT.ObjectVariables.RPC.ArgumentSizeMismatch",
				TEXT("AutoReplication RPC argument '%s': stored size %d != parameter size %d."),
				*Arg.ArgumentName.ToString(), Arg.ValueBytes.Num(), PropSize);
			return false;
		}
		FMemory::Memcpy(DestPtr, Arg.ValueBytes.GetData(), PropSize);
		return true;
	}

	namespace Snapshot
	{
		// Constants and definitions of the helpers forward-declared at the top of this namespace.
		constexpr uint32 SnapshotVersion = 1;
		static constexpr uint64 BlockedPropertyFlags = CPF_Transient | CPF_DuplicateTransient | CPF_TextExportTransient | CPF_NonPIEDuplicateTransient | CPF_DisableEditOnInstance | CPF_EditConst;

		static bool ShouldCaptureProperty(const FProperty* Property)
		{
			if (!Property)
			{
				return false;
			}

			if (!Property->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible | CPF_BlueprintReadOnly))
			{
				return false;
			}

			if (Property->HasAnyPropertyFlags(BlockedPropertyFlags))
			{
				return false;
			}

			const UStruct* OwnerStruct = Property->GetOwnerStruct();
			if (!OwnerStruct)
			{
				return false;
			}

			if (OwnerStruct == UGorgeousObjectVariable::StaticClass())
			{
				return false;
			}

			if (OwnerStruct == UGorgeousRPC_OV::StaticClass())
			{
				static const FName CachedResultsName(TEXT("CachedResults"));
				static const FName CachedResultMapName(TEXT("CachedResultMap"));
				static const FName HasCachedResultName(TEXT("bHasCachedResult"));
				const FName PropertyName = Property->GetFName();
				if (PropertyName == CachedResultsName
					|| PropertyName == CachedResultMapName
					|| PropertyName == HasCachedResultName)
				{
					return false;
				}
			}

			return true;
		}

		static void WriteByteArray(FArchive& Ar, const TArray<uint8>& Data)
		{
			int32 Num = Data.Num();
			Ar << Num;
			if (Num > 0)
			{
				Ar.Serialize(const_cast<uint8*>(Data.GetData()), Num);
			}
		}

		static void ReadByteArray(FArchive& Ar, TArray<uint8>& Data)
		{
			int32 Num = 0;
			Ar << Num;
			if (Num > 0)
			{
				Data.SetNum(Num);
				Ar.Serialize(Data.GetData(), Num);
			}
			else
			{
				Data.Reset();
			}
		}
	}

	static bool InvokeNativeHandler(UObject* TargetObject, const FGorgeousQueuedRPC& QueuedRPC, UGorgeousObjectVariable** OutReturnVariable, bool* OutIsDeferred = nullptr)
	{
		if (!TargetObject || QueuedRPC.Payload.HandlerName.IsNone())
		{
			return false;
		}

		UFunction* HandlerFunction = TargetObject->GetClass()->FindFunctionByName(QueuedRPC.Payload.HandlerName);
		if (!HandlerFunction)
		{
			return false;
		}

		if (HandlerFunction->ParmsSize == 0)
		{
			TargetObject->ProcessEvent(HandlerFunction, nullptr);
			if (OutReturnVariable)
			{
				*OutReturnVariable = nullptr;
			}
			if (OutIsDeferred)
			{
				*OutIsDeferred = false;
			}
			return true;
		}

		TArray<uint8> ParameterStorage;
		ParameterStorage.SetNumZeroed(HandlerFunction->ParmsSize);
		HandlerFunction->InitializeStruct(ParameterStorage.GetData());

		// ── QueuedRPC parameter detection ─────────────────────────────────────────────────
		// If any in-parameter is an FStructProperty wrapping FGorgeousQueuedRPC, the runtime
		// auto-injects it with the current RPC descriptor.
		FStructProperty* QueuedRPCProperty = nullptr;
		{
			UScriptStruct* QueuedRPCType = FGorgeousQueuedRPC::StaticStruct();
			for (FProperty* P = HandlerFunction->PropertyLink; P; P = P->PropertyLinkNext)
			{
				if (!P->HasAnyPropertyFlags(CPF_Parm) || P->HasAnyPropertyFlags(CPF_ReturnParm))
					continue;
				// Allow const reference parameters (CPF_OutParm + CPF_ConstParm)
				if (P->HasAnyPropertyFlags(CPF_OutParm) && !P->HasAnyPropertyFlags(CPF_ConstParm))
					continue;
				FStructProperty* AsStructProp = CastField<FStructProperty>(P);
				if (!AsStructProp || AsStructProp->Struct != QueuedRPCType)
					continue;

				void* QueuedRPCPtr = AsStructProp->ContainerPtrToValuePtr<void>(ParameterStorage.GetData());
				FGorgeousQueuedRPC* QueuedRPCParam = reinterpret_cast<FGorgeousQueuedRPC*>(QueuedRPCPtr);
				*QueuedRPCParam = QueuedRPC;
				QueuedRPCProperty = AsStructProp;
				break;
			}
		}

		// ── Return OV parameter detection ─────────────────────────────────────────────────
		// If the FIRST in-parameter is an FObjectProperty wrapping a UGorgeousObjectVariable
		// subclass and the caller has NOT supplied a matching payload argument, the runtime
		// constructs a fresh instance of that type and injects it before calling the handler.
		// After ProcessEvent the (possibly populated) OV is returned via *OutReturnVariable
		// as the explicit "return value OV" of this RPC call.
		FObjectProperty* ReturnOVProperty = nullptr;
		UGorgeousObjectVariable* ConstructedReturnOV = nullptr;
		{
			for (FProperty* P = HandlerFunction->PropertyLink; P; P = P->PropertyLinkNext)
			{
				if (!P->HasAnyPropertyFlags(CPF_Parm) || P->HasAnyPropertyFlags(CPF_ReturnParm))
					continue;
				if (P->HasAnyPropertyFlags(CPF_OutParm) && !P->HasAnyPropertyFlags(CPF_ConstParm))
					continue;
				// Inspect the FIRST in-parm only
				FObjectProperty* AsObjProp = CastField<FObjectProperty>(P);
				if (!AsObjProp || !AsObjProp->PropertyClass ||
					!AsObjProp->PropertyClass->IsChildOf(UGorgeousObjectVariable::StaticClass()))
				{
					break;
				}
				// Only activate when the caller did NOT supply a matching payload argument
				const bool bInPayload = QueuedRPC.Payload.Arguments.ContainsByPredicate(
					[P](const FGorgeousRPCArgumentContainer& C)
					{
						return C.ArgumentName == P->GetFName();
					});
				if (bInPayload)
					break;
				// Use GetTransientPackage() as Outer, NOT TargetObject.
				// If we used TargetObject (a PIE-world OV), then after PIE ends the
				// return OV — stored in CachedResults.TargetVariable (TObjectPtr) on the
				// async action — would anchor the full Outer chain all the way up to the
				// PIE UWorld, causing EndPlayMap to assert "still referenced".
				// GetTransientPackage() breaks that chain: the return OV lives in the
				// transient package and cannot anchor any PIE actors or worlds.
				ConstructedReturnOV = NewObject<UGorgeousObjectVariable>(GetTransientPackage(), AsObjProp->PropertyClass);
				if (!ConstructedReturnOV)
					break;
				ReturnOVProperty = AsObjProp;
				ReturnOVProperty->SetObjectPropertyValue_InContainer(ParameterStorage.GetData(), ConstructedReturnOV);
				break;
			}
		}

		bool bAllParametersFilled = true;
		int32 ParameterIndex = 0;
		for (FProperty* Property = HandlerFunction->PropertyLink; Property; Property = Property->PropertyLinkNext)
		{
			if (!Property->HasAnyPropertyFlags(CPF_Parm) || Property->HasAnyPropertyFlags(CPF_ReturnParm))
			{
				continue;
			}

			if (Property->HasAnyPropertyFlags(CPF_OutParm) && !Property->HasAnyPropertyFlags(CPF_ConstParm))
			{
				continue;
			}
			// Skip parameters that were already injected during the detection passes above
			if ((ReturnOVProperty && Property == ReturnOVProperty) ||
				(QueuedRPCProperty && Property == QueuedRPCProperty))
			{
				++ParameterIndex;
				continue;
			}

			const FGorgeousRPCArgumentContainer* ArgumentContainer = QueuedRPC.Payload.Arguments.FindByPredicate(
				[&](const FGorgeousRPCArgumentContainer& Container)
				{
					return Container.ArgumentName == Property->GetFName();
				});

			if (!ArgumentContainer || ArgumentContainer->ValueBytes.IsEmpty())
			{
				// Build a comma-separated list of argument names that ARE in the payload so the
				// developer can see exactly what they need to rename their arguments to.
				TArray<FString> AvailableNames;
				for (const FGorgeousRPCArgumentContainer& Arg : QueuedRPC.Payload.Arguments)
				{
					AvailableNames.Add(Arg.ArgumentName.ToString());
				}
				const FString AvailableList = AvailableNames.Num() > 0
					? FString::Join(AvailableNames, TEXT(", "))
					: TEXT("<none>");

				GT_W_LOG("GT.ObjectVariables.RPC.MissingParam",
					TEXT("AutoReplication RPC %s expected parameter '%s' but no matching argument was provided. "
					     "Argument names in payload: [%s]. "
					     "Argument names must match the handler function parameter names exactly (case-sensitive)."),
					*QueuedRPC.Payload.HandlerName.ToString(), *Property->GetName(), *AvailableList);
				bAllParametersFilled = false;
				break;
			}

			if (!CopyArgumentToProperty(*ArgumentContainer, Property, ParameterStorage.GetData()))
			{
				GT_W_LOG("GT.ObjectVariables.RPC.ApplyArgumentFailed",
					TEXT("Failed to copy argument '%s' to parameter '%s' for handler '%s'."),
					*ArgumentContainer->ArgumentName.ToString(), *Property->GetName(),
					*QueuedRPC.Payload.HandlerName.ToString());
				bAllParametersFilled = false;
				break;
			}

			++ParameterIndex;
		}

		if (bAllParametersFilled)
		{
			TargetObject->ProcessEvent(HandlerFunction, ParameterStorage.GetData());
			// ── Extract the handler-written return OV after ProcessEvent ──
			// The handler may have modified the pre-constructed OV (or replaced the pointer).
			if (ReturnOVProperty)
			{
				UGorgeousObjectVariable* PostCallOV = Cast<UGorgeousObjectVariable>(
					ReturnOVProperty->GetObjectPropertyValue_InContainer(ParameterStorage.GetData()));
				ConstructedReturnOV = PostCallOV ? PostCallOV : ConstructedReturnOV;
			}
		}
		else
		{
			// Failed to fill params — the pre-constructed OV was never seen by the handler
			ConstructedReturnOV = nullptr;
		}

		HandlerFunction->DestroyStruct(ParameterStorage.GetData());

		if (OutReturnVariable)
		{
			*OutReturnVariable = ConstructedReturnOV;
		}

		if (OutIsDeferred)
		{
			*OutIsDeferred = (QueuedRPCProperty != nullptr) && bAllParametersFilled;
		}

		return bAllParametersFilled;
	}
}

UGorgeousObjectVariable::UGorgeousObjectVariable(): 
	bPersistent(false),
	bSupportsNetworking(true),
	ReplicationMode(EGorgeousObjectVariableReplicationMode::EFullAutoReplication),
	bUseSharedNetworkStack(false),
	bReplicates(false),
	Parent(nullptr),
	ServerPropertyPollingIntervalSeconds(0.0f),
	AutoReplicationEntryKey(NAME_None),
	AutoReplicationReplicationIndex(INDEX_NONE),
	bLegacyReplicationRegistered(false),
	bAutoReplicationActivated(false),
	ClientPropertyPollingIntervalSeconds(0.0f),
	bRemovedFromRegistry(false)
{
	bReplicationActivationGuard = false;

	if (const UGorgeousAutoReplicationSettings* Settings = UGorgeousAutoReplicationSettings::Get())
	{
		AutoReplicationConfig = Settings->DefaultStreamConfig;
	}
}

UGorgeousObjectVariable::~UGorgeousObjectVariable()
{
}

UGorgeousObjectVariable* UGorgeousObjectVariable::NewObjectVariable(const TSubclassOf<UGorgeousObjectVariable> Class, FGuid& Identifier, UGorgeousObjectVariable* InParent, const bool bShouldPersist, const FString& DisplayNameOverride)
{
	GORGEOUS_PROFILE_SCOPE(GOV_NewObjectVariable);
	if (!Class && Class.Get() == nullptr)
	{
		GT_E_LOG("GT.ObjectVariables.Registration.Invalid_Class", TEXT("You are trying to register a object variable without a valid class, check if the class is valid!"));
		return nullptr;
	}

	if (!InParent)
	{
		FName DesiredRootName = NAME_None;
		if (const UGorgeousObjectVariable* ClassDefaultObject = Class ? Cast<UGorgeousObjectVariable>(Class->GetDefaultObject()) : nullptr)
		{
			DesiredRootName = ClassDefaultObject->GetConfiguredRootName();
		}
		InParent = UGorgeousRootObjectVariable::GetRootObjectVariable(DesiredRootName);
		
		GT_I_LOG("GT.ObjectVariables.Registration.No_Parent",
			TEXT("No parent were specified for object variable with class %s, therefore the resolved root object variable will be used as the parent"),
			*Class->GetName());
	}
	
	UGorgeousObjectVariable* NewObjectVariable = NewObject<UGorgeousObjectVariable>(InParent, Class);
	NewObjectVariable->SetFallbackOwner(InParent);
	
	INC_DWORD_STAT(STAT_GOV_Created);
	INC_DWORD_STAT(STAT_GOV_Alive);
	const int32 AliveCount = GObjectVariableAliveCounter.Increment();
	GORGEOUS_CSV_CUSTOM_STAT_SET(ObjectVariablesAlive, AliveCount);
	GORGEOUS_CSV_CUSTOM_STAT_ACCUMULATE(ObjectVariablesCreated, 1);
	
	const FGuid NewObjectVariableIdentifier = FGuid::NewGuid();
	Identifier = NewObjectVariableIdentifier;
	NewObjectVariable->UniqueIdentifier = NewObjectVariableIdentifier;
	NewObjectVariable->Parent = InParent;
	NewObjectVariable->bPersistent = bShouldPersist;
	// Priority: DisplayNameOverride (explicit) → random name (generated when empty) → GUID (unreachable here, kept as final safety net).
	// SetDisplayName must run BEFORE RegisterWithRegistry so the key is derived from DisplayName, not an empty string.
	NewObjectVariable->SetDisplayName(DisplayNameOverride);
	InParent->RegisterWithRegistry(NewObjectVariable);
	GORGEOUS_TRACE_BOOKMARK(TEXT("ObjectVariable.Create %s (%s)"), *NewObjectVariable->GetName(), *NewObjectVariable->GetClass()->GetName());

	GT_S_LOG("GT.ObjectVariables.Registration.Successful",
		TEXT("Successfully registered object variable with name %s & identifier: %s where the parent is: %s (%s)"),
		*NewObjectVariable->GetDisplayNameOrFallback(),
		*Identifier.ToString(),
		*InParent->GetName(),
		*InParent->UniqueIdentifier.ToString());
	
	return NewObjectVariable;
}

UGorgeousObjectVariable* UGorgeousObjectVariable::InstantiateTransactionalObjectVariable(
	const TSubclassOf<UGorgeousObjectVariable> Class, UGorgeousObjectVariable* InParent, UObject* Outer)
{
	GORGEOUS_PROFILE_SCOPE(GOV_InstantiateTransactional);
	if (!Class)
	{
		GT_E_LOG("GT.ObjectVariables.Transactional.Invalid_Class", TEXT("You are trying to create a transactional object variable without a valid class, check if the class is valid!"));
		return nullptr;
	}

	if (!InParent)
	{
		FName DesiredRootName = NAME_None;
		if (const UGorgeousObjectVariable* ClassDefaultObject = Class ? Cast<UGorgeousObjectVariable>(Class->GetDefaultObject()) : nullptr)
		{
			DesiredRootName = ClassDefaultObject->GetConfiguredRootName();
		}
		InParent = UGorgeousRootObjectVariable::GetRootObjectVariable(DesiredRootName);
		
		GT_I_LOG("GT.ObjectVariables.Transactional.No_Parent",
			TEXT("No parent were specified for transactional object variable with class %s, therefore the resolved root object variable will be used as the parent"),
			*Class->GetName());
	}
	
	UGorgeousObjectVariable* NewInstance = NewObject<UGorgeousObjectVariable>(InParent, Class, NAME_None, RF_Transactional);
	NewInstance->UniqueIdentifier = FGuid::NewGuid();
	NewInstance->Parent = InParent;
	NewInstance->SetDisplayName(Class ? Class->GetName() : FString());
	INC_DWORD_STAT(STAT_GOV_Created);
	INC_DWORD_STAT(STAT_GOV_Alive);
	const int32 AliveCount = GObjectVariableAliveCounter.Increment();
	GORGEOUS_CSV_CUSTOM_STAT_SET(ObjectVariablesAlive, AliveCount);
	GORGEOUS_CSV_CUSTOM_STAT_ACCUMULATE(ObjectVariablesCreated, 1);
	
	if (NewInstance)
	{
		Modify();

		GT_S_LOG("GT.ObjectVariables.Registration.Transactional",
			TEXT("Created new transactional instance of class %s with name %s under parent %s (%s)"),
			*Class->GetName(),
			*NewInstance->GetDisplayNameOrFallback(),
			*InParent->GetName(),
			*InParent->UniqueIdentifier.ToString());
		return NewInstance;
	}
	GT_E_LOG("GT.ObjectVariables.Transactional.Failed", TEXT("Failed to create new transactional instance of class %s under parent %s (%s)"), *Class->GetName(), *InParent->GetName(), *InParent->UniqueIdentifier.ToString());
	return nullptr;
}

bool UGorgeousObjectVariable::SerializeToPayload(FGorgeousObjectVariableSerializedPayload& OutPayload) const
{
	UGorgeousObjectVariable* MutableThis = const_cast<UGorgeousObjectVariable*>(this);
	MutableThis->PreSerializeToPayload(OutPayload);
	return FGorgeousObjectVariableSerialization::WriteObjectToPayload(*MutableThis, OutPayload);
}

bool UGorgeousObjectVariable::DeserializeFromPayload(const FGorgeousObjectVariableSerializedPayload& InPayload)
{
	if (!InPayload.VariableClass)
	{
		return false;
	}

	if (!GetClass()->IsChildOf(InPayload.VariableClass.Get()))
	{
		return false;
	}

	if (!FGorgeousObjectVariableSerialization::LoadObjectFromPayload(*this, InPayload))
	{
		return false;
	}

	if (InPayload.VariableIdentifier.IsValid())
	{
		UniqueIdentifier = InPayload.VariableIdentifier;
	}

	PostDeserializeFromPayload(InPayload);
	return true;
}

void UGorgeousObjectVariable::PreSerializeToPayload(FGorgeousObjectVariableSerializedPayload& OutPayload) const
{
	if (!OutPayload.VariableIdentifier.IsValid())
	{
		OutPayload.VariableIdentifier = UniqueIdentifier.IsValid() ? UniqueIdentifier : FGuid::NewGuid();
	}

	if (!OutPayload.VariableClass)
	{
		OutPayload.VariableClass = GetClass();
	}

#if WITH_EDITORONLY_DATA
	OutPayload.CachedPinConfiguration = PinConfiguration;
	OutPayload.SelectedContainerType = PinConfiguration.ContainerType;
#endif
}

void UGorgeousObjectVariable::PostDeserializeFromPayload(const FGorgeousObjectVariableSerializedPayload& InPayload)
{
#if WITH_EDITORONLY_DATA
	if (InPayload.CachedPinConfiguration.PinCategory != NAME_None)
	{
		PinConfiguration = InPayload.CachedPinConfiguration;
	}
#endif
}

void UGorgeousObjectVariable::InvokeInstancedFunctionality(const FGuid NewUniqueIdentifier)
{
	if (NewUniqueIdentifier.IsValid())
	{
		ApplyReplicatedIdentifier(NewUniqueIdentifier);
	}
}

void UGorgeousObjectVariable::ApplyReplicatedIdentifier(const FGuid& InIdentifier)
{
	if (!InIdentifier.IsValid())
	{
		return;
	}

	const bool bAlreadyRegistered = UGorgeousRootObjectVariable::IsVariableRegistered(this);
	if (bAlreadyRegistered && UniqueIdentifier == InIdentifier)
	{
		return;
	}

	if (bAlreadyRegistered && UniqueIdentifier.IsValid() && UniqueIdentifier != InIdentifier)
	{
		UGorgeousRootObjectVariable::RemoveVariableFromRegistry(this);
	}

	UniqueIdentifier = InIdentifier;

	if (!Parent)
	{
		const FName DesiredRootName = GetConfiguredRootName();
		Parent = UGorgeousRootObjectVariable::GetRootObjectVariable(DesiredRootName);
		if (!Parent)
		{
			Parent = UGorgeousRootObjectVariable::GetRootObjectVariable(NAME_None);
		}
	}

	if (Parent)
	{
		Parent->RegisterWithRegistry(this);
	}
	else
	{
		UGorgeousRootObjectVariable::TrackRegisteredVariable(this);
	}
}

void UGorgeousObjectVariable::BeginDestroy()
{
	GORGEOUS_PROFILE_SCOPE(GOV_BeginDestroy);
	StopClientPropertyPolling();
	StopServerPropertyPolling();
	if (!HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		INC_DWORD_STAT(STAT_GOV_Destroyed);
		DEC_DWORD_STAT(STAT_GOV_Alive);
		const int32 AliveCount = GObjectVariableAliveCounter.Decrement();
		GORGEOUS_CSV_CUSTOM_STAT_SET(ObjectVariablesAlive, AliveCount);
		GORGEOUS_CSV_CUSTOM_STAT_ACCUMULATE(ObjectVariablesDestroyed, 1);
		GORGEOUS_TRACE_BOOKMARK(TEXT("ObjectVariable.Destroy %s (%s)"), *GetName(), *GetClass()->GetName());
		EnsureRemovedFromRegistry();
	}

	if (SupportsAutoReplicationFeatures())
	{
		if (UWorld* TargetWorld = GetWorld())
		{
			FGorgeousAutoReplicationCoordinator::Get(TargetWorld).UnregisterObjectVariable(this);
		}
	}

	UnregisterLegacyReplication();
	UGorgeousRootObjectVariable::ReleaseDisplayName(this);
	Super::BeginDestroy();
}

void UGorgeousObjectVariable::EnsureRemovedFromRegistry()
{
	if (bRemovedFromRegistry)
	{
		return;
	}

	bRemovedFromRegistry = true;

	if (!UGorgeousRootObjectVariable::IsVariableRegistered(this))
	{
		return;
	}

	UGorgeousRootObjectVariable::RemoveVariableFromRegistry(this);
}

void UGorgeousObjectVariable::SetParent(UGorgeousObjectVariable* NewParent)
{
	Parent = NewParent;

	if (!Parent)
	{
		return;
	}

	const FName DesiredName = GetFName();
	const FName UniqueName = MakeUniqueObjectName(Parent, GetClass(), DesiredName);
	Rename(*UniqueName.ToString(), Parent, REN_DontCreateRedirectors);
}

void UGorgeousObjectVariable::SetSharedNetworkStackEnabled(const bool bEnable)
{
	SetSharedNetworkStackEnabledInternal(bEnable, true);
}

void UGorgeousObjectVariable::SetRootNetworkStackEnabled(const bool bEnable)
{
	SetRootNetworkStackEnabledInternal(bEnable, true);
}

void UGorgeousObjectVariable::SetSharedNetworkStackEnabledInternal(const bool bEnable, const bool bNotifyCoordinator)
{
	if (!bSupportsNetworking)
	{
		if (bUseSharedNetworkStack)
		{
			bUseSharedNetworkStack = false;
		}
		return;
	}

	if (bUseSharedNetworkStack == bEnable)
	{
		return;
	}

	bUseSharedNetworkStack = bEnable;
	if (bUseSharedNetworkStack)
	{
		RootNetworkConfig = FGorgeousRootNetworkAccessConfig();
	}

	if (bNotifyCoordinator)
	{
		UpdateAutoReplicationState(bReplicates);
	}
}

void UGorgeousObjectVariable::SetRootNetworkStackEnabledInternal(const bool bEnable, const bool bNotifyCoordinator)
{
	if (!bSupportsNetworking)
	{
		if (RootNetworkConfig.bExposeThroughRootNetworkStack)
		{
			RootNetworkConfig = FGorgeousRootNetworkAccessConfig();
		}
		return;
	}

	const bool bRootForced = DoesRootEnforceNetworking();
	const bool bDesiredState = bRootForced || bEnable;
	if (bRootForced && !bEnable)
	{
		GT_I_LOG("GT.ObjectVariables.RootStack.Forced", TEXT("%s cannot disable the root network stack because the configured root enforces networking."), *GetName());
	}

	if (RootNetworkConfig.bExposeThroughRootNetworkStack == bDesiredState)
	{
		return;
	}

	RootNetworkConfig.bExposeThroughRootNetworkStack = bDesiredState;
	if (bDesiredState && bUseSharedNetworkStack)
	{
		bUseSharedNetworkStack = false;
	}
	else if (!bDesiredState)
	{
		RootNetworkConfig.ReplicationChannel = NAME_None;
		RootNetworkConfig.AccessPolicy = EGorgeousObjectVariableAccessPolicy::Everyone;
	}

	if (bNotifyCoordinator)
	{
		UpdateAutoReplicationState(bReplicates);
	}
}

void UGorgeousObjectVariable::SetAutoReplicationRespectAccessPolicy(const bool bEnable)
{
	if (!SupportsAutoReplicationFeatures())
	{
		return;
	}

	if (AutoReplicationConfig.bRespectAccessPolicy == bEnable)
	{
		return;
	}

	AutoReplicationConfig.bRespectAccessPolicy = bEnable;
	UpdateAutoReplicationState(bReplicates);
}

void UGorgeousObjectVariable::SetNetworkAccessPolicy(const EGorgeousObjectVariableAccessPolicy NewPolicy, const bool bApplyToSharedStack)
{
	if (!bSupportsNetworking)
	{
		return;
	}

	bool bConfigChanged = false;
	if (bApplyToSharedStack)
	{
		if (!bUseSharedNetworkStack)
		{
			SetSharedNetworkStackEnabledInternal(true, false);
			bConfigChanged = true;
		}

		if (RootNetworkConfig.AccessPolicy != NewPolicy)
		{
			RootNetworkConfig.AccessPolicy = NewPolicy;
			bConfigChanged = true;
		}
	}
	else
	{
		if (!RootNetworkConfig.bExposeThroughRootNetworkStack)
		{
			SetRootNetworkStackEnabledInternal(true, false);
			bConfigChanged = true;
		}

		if (RootNetworkConfig.AccessPolicy != NewPolicy)
		{
			RootNetworkConfig.AccessPolicy = NewPolicy;
			bConfigChanged = true;
		}
	}

	if (bConfigChanged)
	{
		UpdateAutoReplicationState(bReplicates);
	}
}

void UGorgeousObjectVariable::EnsureSharedNetworkStackOwner(UObject* NewOwner)
{
	if (!NewOwner)
	{
		return;
	}

	const bool bUsesSharedStack = bUseSharedNetworkStack;
	const bool bUsesRootStack = ShouldUseRootNetworkStack();
	if (!bUsesSharedStack && !bUsesRootStack)
	{
		return;
	}

	if (AutoReplicationOwner.IsValid())
	{
		return;
	}

	AutoReplicationOwner = NewOwner;
	SetFallbackOwner(NewOwner);

	if (SupportsLegacyReplication() && bReplicates)
	{
		RegisterLegacyReplication(NewOwner);
	}
}

void UGorgeousObjectVariable::SetDisplayName(const FString& InDisplayName)
{
	FString CandidateLabel = InDisplayName;
	if (CandidateLabel.IsEmpty())
	{
		CandidateLabel = GorgeousObjectVariable_Private::GenerateRandomDisplayName();
	}

	CandidateLabel = GorgeousObjectVariable_Private::NormalizeLabel(CandidateLabel);
	if (UGorgeousRootObjectVariable* Root = UGorgeousRootObjectVariable::GetRootObjectVariable())
	{
		Root->ReleaseDisplayName(this);
		CandidateLabel = Root->ReserveDisplayName(this, CandidateLabel);
	}

	DisplayName = CandidateLabel;
	NotifyDisplayNameChanged();
}

void UGorgeousObjectVariable::NotifyDisplayNameChanged()
{
	if (!SupportsAutoReplicationFeatures() || !bReplicates)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (World->GetNetMode() == NM_Client)
		{
			return;
		}

		FGorgeousAutoReplicationCoordinator::Get(World).MarkStreamDirty(this);
	}
}

void UGorgeousObjectVariable::OnRep_DisplayName(const FString& PreviousDisplayName)
{
	if (UGorgeousRootObjectVariable* Root = UGorgeousRootObjectVariable::GetRootObjectVariable())
	{
		Root->ReleaseDisplayName(this);
		if (!DisplayName.IsEmpty())
		{
			DisplayName = Root->ReserveDisplayName(this, DisplayName);
		}
	}
}

bool UGorgeousObjectVariable::TryGetReplicatedRPCResult(const FGuid& RequestGuid, UGorgeousObjectVariable*& OutResult)
{
	OutResult = nullptr;
	if (!RequestGuid.IsValid())
	{
		return false;
	}

	if (const TWeakObjectPtr<UGorgeousObjectVariable>* Existing = CachedRPCResultInstances.Find(RequestGuid))
	{
		OutResult = Existing->Get();
	}

	if (!OutResult)
	{
		if (const FGorgeousAutoReplicationRPCResultDescriptor* Descriptor = CachedRPCResultDescriptors.Find(RequestGuid))
		{
			OutResult = UGorgeousRootObjectVariable::FindVariableByIdentifier(Descriptor->ResultIdentifier);
			if (OutResult)
			{
				CachedRPCResultInstances.FindOrAdd(RequestGuid) = OutResult;
			}
		}
	}

	return OutResult != nullptr;
}

void UGorgeousObjectVariable::ClearReplicatedRPCResult(const FGuid& RequestGuid)
{
	if (!RequestGuid.IsValid())
	{
		return;
	}

	CachedRPCResultDescriptors.Remove(RequestGuid);
	CachedRPCResultInstances.Remove(RequestGuid);

	UWorld* World = GetWorld();
	const bool bIsAuthority = World && World->GetNetMode() != NM_Client;
	if (!bIsAuthority)
	{
		return;
	}

	auto RemoveByGuid = [&RequestGuid](const FGorgeousAutoReplicationRPCResultDescriptor& Descriptor)
	{
		return Descriptor.RequestGuid == RequestGuid;
	};

	const int32 RemovedOwner = OwnerScopedRPCResultDescriptors.RemoveAll(RemoveByGuid);
	const int32 RemovedBroadcast = BroadcastRPCResultDescriptors.RemoveAll(RemoveByGuid);
	if (RemovedOwner > 0 || RemovedBroadcast > 0)
	{
		MarkRPCResultDescriptorsDirty();
	}
}

void UGorgeousObjectVariable::OnRep_BroadcastRPCResultDescriptors()
{
	HandleReplicatedRPCResultDescriptors(BroadcastRPCResultDescriptors);
}

void UGorgeousObjectVariable::OnRep_OwnerScopedRPCResultDescriptors()
{
	HandleReplicatedRPCResultDescriptors(OwnerScopedRPCResultDescriptors);
}

FString UGorgeousObjectVariable::GetDisplayNameOrFallback() const
{
	return DisplayName.IsEmpty() ? GetName() : DisplayName;
}

void UGorgeousObjectVariable::RegisterWithRegistry(UGorgeousObjectVariable* NewObjectVariable, FName RegistryKey)
{
	if (!NewObjectVariable)
	{
		return;
	}

	if (!HandleUniqueRegistrationPolicy(NewObjectVariable))
	{
		return;
	}

	if (!UGorgeousRootObjectVariable::IsVariableRegistered(NewObjectVariable))
	{
		// Derive a stable map key following the priority chain:
		//   1. Explicit RegistryKey passed by the caller.
		//   2. DisplayName — set before this call to either the user-supplied DisplayNameOverride or a
		//      randomly generated name (see SetDisplayName), so this branch is taken in virtually all cases.
		//   3. GUID string — final safety net for any code path that bypasses SetDisplayName.
		FName Key = RegistryKey;
		if (Key == NAME_None)
		{
			Key = !NewObjectVariable->DisplayName.IsEmpty()
				? FName(*NewObjectVariable->DisplayName)
				: FName(*NewObjectVariable->UniqueIdentifier.ToString());
		}
		// Avoid key collision: append a GUID fragment only for genuine live-variable collisions.
		// If the existing slot is null/invalid (stale GC'd entry), replace it directly.
		if (const TObjectPtr<UGorgeousObjectVariable>* ExistingSlot = VariableRegistry.Find(Key))
		{
			if (*ExistingSlot && IsValid(ExistingSlot->Get()))
			{
				// Genuine collision with a live variable — append GUID fragment
				Key = FName(*FString::Printf(TEXT("%s_%s"), *Key.ToString(),
					*NewObjectVariable->UniqueIdentifier.ToString().Left(8)));
			}
			// else: stale / null slot — replace in-place without suffix
		}
		VariableRegistry.Add(Key, NewObjectVariable);
		UGorgeousRootObjectVariable::TrackRegisteredVariable(NewObjectVariable);
		OnVariableTreeChanged.Broadcast();
	}
}

UGorgeousObjectVariable* UGorgeousObjectVariable::FindInRegistry(FName Key, const TSubclassOf<UGorgeousObjectVariable> Class,
	const EFindInRegistryMatchCase MatchCase) const
{
	auto PassesClassFilter = [&Class](UGorgeousObjectVariable* Variable) -> bool
	{
		return !Class || Variable->IsA(Class);
	};

	// Exact is a direct TMap lookup — O(1)
	if (MatchCase == EFindInRegistryMatchCase::Exact)
	{
		const TObjectPtr<UGorgeousObjectVariable>* Found = VariableRegistry.Find(Key);
		if (!Found || !(*Found))
		{
			return nullptr;
		}
		UGorgeousObjectVariable* Variable = Found->Get();
		return PassesClassFilter(Variable) ? Variable : nullptr;
	}

	// Contains / StartsWith / EndsWith — iterate all keys, case-insensitive
	const FString KeyStr = Key.ToString();
	for (const auto& Pair : VariableRegistry)
	{
		if (!Pair.Value || !IsValid(Pair.Value.Get()))
		{
			continue;
		}

		const FString EntryKey = Pair.Key.ToString();
		bool bMatches = false;
		switch (MatchCase)
		{
		case EFindInRegistryMatchCase::Contains:
			bMatches = EntryKey.Contains(KeyStr, ESearchCase::IgnoreCase);
			break;
		case EFindInRegistryMatchCase::StartsWith:
			bMatches = EntryKey.StartsWith(KeyStr, ESearchCase::IgnoreCase);
			break;
		case EFindInRegistryMatchCase::EndsWith:
			bMatches = EntryKey.EndsWith(KeyStr, ESearchCase::IgnoreCase);
			break;
		default:
			break;
		}

		if (bMatches)
		{
			UGorgeousObjectVariable* Variable = Pair.Value.Get();
			if (PassesClassFilter(Variable))
			{
				return Variable;
			}
		}
	}
	return nullptr;
}

bool UGorgeousObjectVariable::HandleUniqueRegistrationPolicy(UGorgeousObjectVariable* Candidate)
{
	if (!Candidate || !Candidate->bUnique)
	{
		return true;
	}

	const TArray<UGorgeousObjectVariable*> Registry = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry(Candidate->GetConfiguredRootName());
	TArray<UGorgeousObjectVariable*> Collisions;
	for (UGorgeousObjectVariable* Entry : Registry)
	{
		if (!Entry || Entry == Candidate)
		{
			continue;
		}

		if (Entry->GetClass() == Candidate->GetClass() && UGorgeousRootObjectVariable::IsVariableRegistered(Entry))
		{
			Collisions.Add(Entry);
		}
	}

	if (Collisions.IsEmpty())
	{
		return true;
	}

	switch (Candidate->UniqueRegistrationPolicy)
	{
	case EGorgeousObjectVariableUniqueRegistrationPolicy::RemoveAllExisting:
		for (UGorgeousObjectVariable* Collision : Collisions)
		{
			if (!Collision)
			{
				continue;
			}

			UGorgeousRootObjectVariable::RemoveVariableFromRegistry(Collision);
		}
		return true;

	case EGorgeousObjectVariableUniqueRegistrationPolicy::CancelRegistration:
	default:
		GT_W_LOG("GT.ObjectVariables.Registration.UniqueCollision", TEXT("Registration cancelled for %s because another unique instance already exists."), *Candidate->GetDisplayNameOrFallback());
		return false;
	}
}

void UGorgeousObjectVariable::ActivateReplication(const FGorgeousAutoReplicationContext& Context)
{
	if (!bSupportsNetworking)
	{
		GT_W_LOG("GT.ObjectVariables.Replication.Disabled", TEXT("%s cannot activate replication because networking is disabled."), *GetName());
		return;
	}

	if (bReplicationActivationGuard)
	{
		GT_W_LOG("GT.ObjectVariables.Replication.AlreadyActive", TEXT("Replication activation already in progress for %s."), *GetName());
		return;
	}

	RegisteredReplicatedProperties.Reset();
	RegisteredRPCBindings.Reset();
	ActiveReplicationContext = Context;

	if (const UGorgeousAutoReplicationSettings* Settings = UGorgeousAutoReplicationSettings::Get())
	{
		if (const FGorgeousAutoReplicationStreamConfig* Override = Settings->StreamOverrides.Find(Context.EntryKey))
		{
			AutoReplicationConfig = *Override;
		}
	}

	TGuardValue<bool> ActivationGuard(bReplicationActivationGuard, true);

	if (SupportsAutoReplicationFeatures())
	{
		FGorgeousReplicatedPropertyConfig DisplayNameConfig;
		DisplayNameConfig.RepNotifyFunction = GET_FUNCTION_NAME_CHECKED(UGorgeousObjectVariable, OnRep_DisplayName);
		DisplayNameConfig.RepNotifyPolicy = EGorgeousRepNotifyPolicy::OnChanged;
		DisplayNameConfig.bFireInitialNotify = true;
		RegisterReplicatedProperty(GET_MEMBER_NAME_CHECKED(UGorgeousObjectVariable, DisplayName), EGorgeousReplicationMode::EProperty, true, DisplayNameConfig);

		FGorgeousReplicatedPropertyConfig BroadcastResultConfig;
		BroadcastResultConfig.RepNotifyFunction = GET_FUNCTION_NAME_CHECKED(UGorgeousObjectVariable, OnRep_BroadcastRPCResultDescriptors);
		BroadcastResultConfig.RepNotifyPolicy = EGorgeousRepNotifyPolicy::Always;
		RegisterReplicatedProperty(GET_MEMBER_NAME_CHECKED(UGorgeousObjectVariable, BroadcastRPCResultDescriptors), EGorgeousReplicationMode::EProperty, true, BroadcastResultConfig);

		FGorgeousReplicatedPropertyConfig OwnerResultConfig = BroadcastResultConfig;
		OwnerResultConfig.RepNotifyFunction = GET_FUNCTION_NAME_CHECKED(UGorgeousObjectVariable, OnRep_OwnerScopedRPCResultDescriptors);
		OwnerResultConfig.ReplicationCondition = COND_OwnerOnly;
		RegisterReplicatedProperty(GET_MEMBER_NAME_CHECKED(UGorgeousObjectVariable, OwnerScopedRPCResultDescriptors), EGorgeousReplicationMode::EProperty, true, OwnerResultConfig);
	}

	OnReplicationActivated(Context);
	bAutoReplicationActivated = true;
	ActiveReplicationContext = FGorgeousAutoReplicationContext();
	RefreshClientChangeShadows();
	StartClientPropertyPolling();
	StartServerPropertyPolling();

	if (SupportsAutoReplicationFeatures() && Context.OwningObject)
	{
		if (UWorld* TargetWorld = Context.OwningObject->GetWorld())
		{
			FGorgeousAutoReplicationCoordinator::Get(TargetWorld).RegisterObjectVariable(this, AutoReplicationConfig);
		}
	}

	if (SupportsLegacyReplication())
	{
		RegisterLegacyReplication(Context.OwningObject);
	}
}

void UGorgeousObjectVariable::RegisterReplicatedProperty(const FName PropertyName, const EGorgeousReplicationMode Mode, const bool bSendInitialState, const FGorgeousReplicatedPropertyConfig& AdvancedConfig)
{
	/*if (!EnsureReplicationActivation(TEXT("RegisterReplicatedProperty")))
	{
		return;
	}*/

	if (PropertyName.IsNone())
	{
		GT_W_LOG("GT.ObjectVariables.Replication.PropertyUnnamed", TEXT("Attempted to register an unnamed property on %s."), *GetName());
		return;
	}

	FProperty* Property = FindFProperty<FProperty>(GetClass(), PropertyName);
	if (!Property)
	{
		GT_W_LOG("GT.ObjectVariables.Replication.PropertyMissing", TEXT("Property %s does not exist on %s."), *PropertyName.ToString(), *GetName());
		return;
	}

	// Properties that contain UObject references cannot be serialized via a plain FMemoryWriter
	// (EProperty mode) because FMemoryArchive asserts when asked to serialize object pointers.
	// We now use FObjectAndNameAsStringProxyArchive in the EProperty path, so the auto-promotion to
	// ENetSerialize is no longer needed.  Collection types (FArrayProperty, FMapProperty,
	// FSetProperty) must NOT be promoted to ENetSerialize because FArrayProperty::NetSerializeItem is
	// unconditionally deprecated (checkf(false)) in UE5.7+ — promoting them would cause a hard crash.
	const EGorgeousReplicationMode ResolvedMode = Mode;

	FReplicatedPropertyDeclaration& Declaration = RegisteredReplicatedProperties.AddDefaulted_GetRef();
	Declaration.PropertyName = PropertyName;
	Declaration.CachedProperty = Property;
	Declaration.Mode = ResolvedMode;
	Declaration.bSendInitialState = bSendInitialState;
	Declaration.ReplicationCondition = AdvancedConfig.ReplicationCondition;
	Declaration.RepNotifyFunction = AdvancedConfig.RepNotifyFunction;
	Declaration.RepNotifyPolicy = AdvancedConfig.RepNotifyPolicy;
	Declaration.bFireInitialNotify = AdvancedConfig.bFireInitialNotify;
	Declaration.bInitializeNullReferences = AdvancedConfig.bInitializeNullReferences;
	Declaration.bDeliveredInitialNotify = false;

	if (Declaration.HasRepNotify())
	{
		if (!FindFunction(Declaration.RepNotifyFunction))
		{
			GT_W_LOG("GT.ObjectVariables.Replication.RepNotifyMissing", TEXT("RepNotify target %s was not found on %s while registering property %s. Notify will be disabled."),
				*Declaration.RepNotifyFunction.ToString(), *GetName(), *PropertyName.ToString());
			Declaration.RepNotifyFunction = NAME_None;
		}
		else
		{
			Declaration.InitializeShadowState(Property, nullptr);
		}
	}

	if (Declaration.CachedProperty)
	{
		void* CurrentValue = Declaration.CachedProperty->ContainerPtrToValuePtr<void>(this);
		Declaration.InitializeChangeShadow(Declaration.CachedProperty, CurrentValue);
	}

	GT_I_LOG("GT.ObjectVariables.Replication.PropertyRegistered", TEXT("Registered replicated property %s on %s."), *PropertyName.ToString(), *GetName());
}

bool UGorgeousObjectVariable::BuildAutoReplicationPropertyPayload(const FGorgeousAutoReplicationConditionContext& ConditionContext, FGorgeousAutoReplicationPropertyPayload& OutPayload) const
{
	OutPayload.Reset();
	if (ConditionContext.StreamGuid.IsValid())
	{
		OutPayload.StreamGuid = ConditionContext.StreamGuid;
	}

	if (RegisteredReplicatedProperties.Num() == 0)
	{
		return false;
	}

	bool bSerializedAny = false;
	for (const FReplicatedPropertyDeclaration& Declaration : RegisteredReplicatedProperties)
	{
		if (!Declaration.CachedProperty)
		{
			continue;
		}

		if (ConditionContext.bIsInitialState && !Declaration.bSendInitialState)
		{
			continue;
		}

		if (!ConditionContext.PassesLifetimeCondition(Declaration.ReplicationCondition))
		{
			continue;
		}

		void* PropertyData = Declaration.CachedProperty->ContainerPtrToValuePtr<void>(const_cast<UGorgeousObjectVariable*>(this));
		if (!PropertyData)
		{
			continue;
		}

		FGorgeousAutoReplicationPropertyValue SerializedValue;
		SerializedValue.PropertyName = Declaration.PropertyName;
		SerializedValue.Mode = Declaration.Mode;
		SerializedValue.ReplicationCondition = Declaration.ReplicationCondition;
		SerializedValue.bIsInitialState = ConditionContext.bIsInitialState ? 1 : 0;

		if (Declaration.Mode == EGorgeousReplicationMode::ECustomPayload)
		{
			if (!const_cast<UGorgeousObjectVariable*>(this)->BuildCustomAutoReplicationPayload(Declaration.PropertyName, SerializedValue.Payload, ConditionContext.bIsInitialState))
			{
				GT_I_LOG("GT.ObjectVariables.Payload.CustomSkipped", TEXT("Custom AutoReplication payload for property %s on %s was skipped because no handler provided data."),
					*Declaration.PropertyName.ToString(), *GetName());
				continue;
			}
		}
		else if (!GorgeousObjectVariable_Private::SerializePropertyValue(const_cast<UGorgeousObjectVariable*>(this), Declaration.CachedProperty, PropertyData, Declaration.Mode, ConditionContext.PackageMap, SerializedValue.Payload, Declaration.bInitializeNullReferences))
		{
			GT_W_LOG("GT.ObjectVariables.Payload.SerializeFailed", TEXT("Failed to serialize auto-replicated property %s on %s."), *Declaration.PropertyName.ToString(), *GetName());
			continue;
		}

		OutPayload.Properties.Add(MoveTemp(SerializedValue));
		bSerializedAny = true;
	}

	if (!bSerializedAny)
	{
		OutPayload.Reset();
	}

	return bSerializedAny;
}

bool UGorgeousObjectVariable::ApplyAutoReplicationPropertyPayload(const FGorgeousAutoReplicationPropertyPayload& Payload, UPackageMap* PackageMap, bool bSyncChangeShadow)
{
	if (Payload.IsEmpty())
	{
		return false;
	}

	bool bAppliedAny = false;
	for (const FGorgeousAutoReplicationPropertyValue& SerializedProperty : Payload.Properties)
	{
		FReplicatedPropertyDeclaration* Declaration = FindReplicatedDeclarationByName(SerializedProperty.PropertyName);
		if (!Declaration || !Declaration->CachedProperty)
		{
			GT_I_LOG("GT.ObjectVariables.Payload.UnknownProperty", TEXT("Received AutoReplication payload for unknown property %s on %s."), *SerializedProperty.PropertyName.ToString(), *GetName());
			continue;
		}

		if (Declaration->Mode == EGorgeousReplicationMode::ECustomPayload)
		{
			const bool bHandled = ApplyCustomAutoReplicationPayload(SerializedProperty.PropertyName, SerializedProperty.Payload, SerializedProperty.bIsInitialState != 0);
			if (!bHandled)
			{
				GT_W_LOG("GT.ObjectVariables.Payload.CustomApplyFailed", TEXT("Custom AutoReplication payload for property %s on %s was not applied."), *SerializedProperty.PropertyName.ToString(), *GetName());
				continue;
			}
			bAppliedAny = true;
			continue;
		}

		if (SerializedProperty.Payload.Num() == 0)
		{
			continue;
		}

		void* PropertyData = Declaration->CachedProperty->ContainerPtrToValuePtr<void>(this);
		if (!PropertyData)
		{
			continue;
		}

		if (!GorgeousObjectVariable_Private::DeserializePropertyValue(this, Declaration->CachedProperty, PropertyData, Declaration->Mode, PackageMap, SerializedProperty.Payload, Declaration->bInitializeNullReferences))
		{
			GT_W_LOG("GT.ObjectVariables.Payload.DeserializeFailed", TEXT("Failed to deserialize AutoReplication payload for property %s on %s."), *SerializedProperty.PropertyName.ToString(), *GetName());
			continue;
		}

		// On the CLIENT: sync the change shadow to the freshly applied value.  Without
		// this, any in-flight mutation the polling ticker had already stamped into the
		// shadow would leave shadow != live, causing the poller to re-detect a spurious
		// dirty state and repeat the send indefinitely.  (Correction-loop fix.)
		//
		// On the SERVER (C2S relay path): intentionally skip the shadow update so that
		// the next HandleServerPropertyPollingTick detects the relay-applied value as
		// dirty and fans it out to all connected clients via the normal S2C pipeline.
		// Without this, shadow == live after apply and the polling tick sees no change,
		// preventing the multicast / S2C fan-out leg of C2S and C2MC scenarios.
		if (bSyncChangeShadow)
		{
			UpdateChangeShadowForProperty(*Declaration);
		}

		bAppliedAny = true;
	}

	return bAppliedAny;
}

bool UGorgeousObjectVariable::BuildCustomAutoReplicationPayload_Implementation(FName PropertyName, TArray<uint8>& OutPayload, bool bIsInitialState)
{
	OutPayload.Reset();
	return false;
}

bool UGorgeousObjectVariable::ApplyCustomAutoReplicationPayload_Implementation(FName PropertyName, const TArray<uint8>& Payload, bool bIsInitialState)
{
	return false;
}

void UGorgeousObjectVariable::OnReplicationActivated_Implementation(const FGorgeousAutoReplicationContext& Context)
{
	// Default: no additional properties to register.
	// Subclasses (or the UE_SETUP_OBJECT_VARIABLE_AUTO_REPLICATION macro) override this
	// to call RegisterReplicatedProperty() for their custom fields.
}

void UGorgeousObjectVariable::BindRPCHandler(const FName RPCName, const EGorgeousAutoReplicationRPCType Reliability)
{
	if (!EnsureReplicationActivation(TEXT("BindRPCHandler")))
	{
		return;
	}

	if (RPCName.IsNone())
	{
		GT_W_LOG("GT.ObjectVariables.RPC.BindUnnamed", TEXT("Attempted to bind an unnamed RPC handler on %s."), *GetName());
		return;
	}

	FRPCBindingDeclaration& Binding = RegisteredRPCBindings.AddDefaulted_GetRef();
	Binding.RPCName = RPCName;
	Binding.Reliability = Reliability;

	GT_I_LOG("GT.ObjectVariables.RPC.HandlerRegistered", TEXT("Registered AutoReplication RPC handler %s on %s."), *RPCName.ToString(), *GetName());
}

bool UGorgeousObjectVariable::ExecuteAutoReplicationRPC(const FGorgeousQueuedRPC& QueuedRPC, UGorgeousObjectVariable** OutReturnVariable, bool* OutIsDeferred)
{
	if (!SupportsAutoReplicationFeatures())
	{
		GT_W_LOG("GT.ObjectVariables.RPC.ExecuteManual", TEXT("%s is configured for manual networking and cannot execute AutoReplication RPCs."), *GetName());
		return false;
	}

	if (QueuedRPC.Payload.HandlerName.IsNone())
	{
		GT_W_LOG("GT.ObjectVariables.RPC.MissingHandler", TEXT("%s received AutoReplication RPC for key %s without a handler name."), *GetName(), *QueuedRPC.Key.ToString());
		return false;
	}

	const FRPCBindingDeclaration* Binding = RegisteredRPCBindings.FindByPredicate([&QueuedRPC](const FRPCBindingDeclaration& Declaration)
	{
		return Declaration.RPCName == QueuedRPC.Payload.HandlerName;
	});

	if (!Binding)
	{
		GT_I_LOG("GT.ObjectVariables.RPC.NoBinding", TEXT("%s executing AutoReplication RPC %s for key %s without an explicit binding."), *GetName(), *QueuedRPC.Payload.HandlerName.ToString(), *QueuedRPC.Key.ToString());
	}
	else if (Binding->Reliability != QueuedRPC.Type)
	{
		const UEnum* EnumClass = StaticEnum<EGorgeousAutoReplicationRPCType>();
		const FString Expected = EnumClass ? EnumClass->GetNameStringByValue(static_cast<int64>(Binding->Reliability)) : TEXT("<unknown>");
		const FString Received = EnumClass ? EnumClass->GetNameStringByValue(static_cast<int64>(QueuedRPC.Type)) : TEXT("<unknown>");
		GT_I_LOG("GT.ObjectVariables.RPC.ReliabilityMismatch", TEXT("AutoReplication RPC %s on %s expected %s but received %s. Continuing execution."), *QueuedRPC.Payload.HandlerName.ToString(), *GetName(), *Expected, *Received);
	}

	UGorgeousObjectVariable* NativeReturnOV = nullptr;
	bool bNativeIsDeferred = false;
	const bool bExecutedNativeHandler = InvokeNativeAutoReplicationRPCHandler(QueuedRPC, &NativeReturnOV, &bNativeIsDeferred);
	if (OutIsDeferred)
	{
		*OutIsDeferred = bNativeIsDeferred;
	}
	if (!bExecutedNativeHandler)
	{
		GT_I_LOG("GT.ObjectVariables.RPC.NoNativeHandler", TEXT("AutoReplication RPC %s on %s did not match a native function. Blueprint event will be fired instead."),
			*QueuedRPC.Payload.HandlerName.ToString(), *GetName());
	}

	OnAutoReplicationRPCPayload.Broadcast(QueuedRPC, this);
	HandleAutoReplicationRPCPayload(QueuedRPC);
	UGorgeousRPC_OV* ResultContainer = bExecutedNativeHandler ? CreateAutoRPCResultContainer() : nullptr;
	if (ResultContainer)
	{
		RegisterReplicatedRPCResult(QueuedRPC, ResultContainer);
	}
	if (OutReturnVariable)
	{
		// When the handler wrote an explicit return OV (first-param pattern), expose that OV
		// directly so that ExecuteOnVariable → EmitResult → NotifyRequestCompleted all carry
		// the handler-populated value as TargetVariable (matching the EOwner path behaviour).
		// ResultContainer is still used internally for RegisterReplicatedRPCResult above.
		*OutReturnVariable = NativeReturnOV ? static_cast<UGorgeousObjectVariable*>(NativeReturnOV)
		                                    : static_cast<UGorgeousObjectVariable*>(ResultContainer);
	}

	// When the handler wrote an explicit return OV via its first parameter, use that OV as
	// TargetVariable so callers retrieve the handler-populated value via GetCachedTargetVariable().
	// The UGorgeousRPC_OV wrapper (ResultContainer) is still created for the async machinery.
	UGorgeousObjectVariable* EffectiveTargetVariable = NativeReturnOV
		? static_cast<UGorgeousObjectVariable*>(NativeReturnOV)
		: (ResultContainer ? static_cast<UGorgeousObjectVariable*>(ResultContainer) : this);

	FGorgeousAutoReplicationRPCResult Result;
	Result.QueuedRPC = QueuedRPC;
	Result.TargetKind = EGorgeousAutoReplicationTargetKind::EObjectVariable;
	Result.TargetVariable = EffectiveTargetVariable;
	Result.TargetOwner = nullptr;
	Result.TargetVariableIdentifier = EffectiveTargetVariable
		? IGorgeousObjectVariableInteraction_I::Execute_GetUniqueIdentifierForObjectVariable(EffectiveTargetVariable)
		: GetUniqueIdentifierForObjectVariable();
	if (ResultContainer)
	{
		ResultContainer->CaptureResult(Result);
	}
	// NOTE: Do NOT call NotifyRequestCompleted here.
	// The mixin's EmitResult lambda calls it after ExecuteAutoReplicationRPC returns for
	// the EObjectVariable/EAuto paths.  Calling it here too produces a duplicate
	// per-responder callback: TotalReceivedResponders > TotalExpectedResponders → test fail.
	if (UWorld* TargetWorld = GetWorld())
	{
		FGorgeousAutoReplicationCoordinator::Get(TargetWorld).NotifyRPCBroadcast(QueuedRPC, this);
	}
	return true;
}

bool UGorgeousObjectVariable::InvokeNativeAutoReplicationRPCHandler(const FGorgeousQueuedRPC& QueuedRPC, UGorgeousObjectVariable** OutReturnVariable, bool* OutIsDeferred)
{
	// Thread OutReturnVariable and OutIsDeferred through so InvokeNativeHandler can return the
	// handler-written return OV (first-param OV pattern) and signal deferred mode when the
	// handler signature includes FGorgeousAutoReplicationRPCHandlerContext.
	const bool bHandled = GorgeousObjectVariable_Private::InvokeNativeHandler(this, QueuedRPC, OutReturnVariable, OutIsDeferred);
	return bHandled;
}

FName UGorgeousObjectVariable::GetConfiguredRootName() const
{
	return RootConfiguration.ResolvePreferredRootName();
}

EGorgeousObjectVariableAccessPolicy UGorgeousObjectVariable::GetEffectiveAccessPolicy() const
{
	if (RootNetworkConfig.AccessPolicy == EGorgeousObjectVariableAccessPolicy::Everyone && AutoReplicationConfig.bRespectAccessPolicy)
	{
		return ResolveRespectAccessPolicy(const_cast<UGorgeousObjectVariable*>(this));
	}

	return RootNetworkConfig.AccessPolicy;
}

FName UGorgeousObjectVariable::GetEffectiveNetworkChannel() const
{
	if (!ShouldUseRootNetworkStack())
	{
		return NAME_None;
	}

	FName ChannelName = NAME_None;
	if (DoesRootEnforceNetworking() || RootNetworkConfig.bExposeThroughRootNetworkStack || bUseSharedNetworkStack)
	{
		ChannelName = RootNetworkConfig.ReplicationChannel;
	}

	if (ChannelName.IsNone())
	{
		ChannelName = UGorgeousRootNetworkStackSubsystem::GetDefaultChannelName();
	}

	return ChannelName;
}

bool UGorgeousObjectVariable::CanControllerAccessVariable_Implementation(AGorgeousPlayerController* Controller, FName PropertyName) const
{
	// PropertyName is intentionally unused in the default implementation — the base policy grants
	// access to all properties on this OV when the controller is the owner.  Subclasses or Blueprint
	// overrides can inspect PropertyName to implement per-property access granularity.
	return Controller && Controller == ResolveOwningPlayerController();
}

EGorgeousObjectVariableAccessPolicy UGorgeousObjectVariable::ResolveRespectAccessPolicy_Implementation(UGorgeousObjectVariable* Variable) const
{
	return EGorgeousObjectVariableAccessPolicy::Everyone;
}

bool UGorgeousObjectVariable::ShouldUseRootNetworkStack() const
{
	if (!bSupportsNetworking)
	{
		return false;
	}

	const bool bRootEnforced = DoesRootEnforceNetworking();
	const bool bExplicitRootExpose = RootNetworkConfig.bExposeThroughRootNetworkStack && DoesConfiguredRootSupportNetworking();
	const bool bSharedStackActive = bUseSharedNetworkStack;
	const bool bAutoStreamRespect = AutoReplicationConfig.bRespectAccessPolicy;
	const bool bAccessPolicyRequiresStack = GetEffectiveAccessPolicy() != EGorgeousObjectVariableAccessPolicy::Everyone; // Non-default policies must never bypass the stack

	return bRootEnforced
		|| bExplicitRootExpose
		|| bSharedStackActive
		|| bAutoStreamRespect
		|| bAccessPolicyRequiresStack;
}

bool UGorgeousObjectVariable::DoesRootEnforceNetworking() const
{
	const UGorgeousObjectVariableRootSettings* RootSettings = UGorgeousObjectVariableRootSettings::Get();
	if (!RootSettings)
	{
		return false;
	}

	const FGorgeousObjectVariableRootEntry* Entry = RootSettings->FindRootEntry(GetConfiguredRootName());
	return Entry && Entry->bSupportsNetworking && Entry->bEnforceNetworking;
}

bool UGorgeousObjectVariable::DoesConfiguredRootSupportNetworking() const
{
	const UGorgeousObjectVariableRootSettings* RootSettings = UGorgeousObjectVariableRootSettings::Get();
	if (!RootSettings)
	{
		return true;
	}

	const FGorgeousObjectVariableRootEntry* Entry = RootSettings->FindRootEntry(GetConfiguredRootName());
	return !Entry || Entry->bSupportsNetworking;
}

bool UGorgeousObjectVariable::EvaluateAccessPolicyForController(AController* Controller, FName PropertyName) const
{
	if (!ShouldUseRootNetworkStack())
	{
		return true;
	}

	switch (GetEffectiveAccessPolicy())
	{
	case EGorgeousObjectVariableAccessPolicy::Everyone:
		return true;
	case EGorgeousObjectVariableAccessPolicy::OwningControllerOnly:
		// OwningControllerOnly is always a whole-OV gate; PropertyName has no effect here.
		return Controller && Controller == ResolveOwningPlayerController();
	case EGorgeousObjectVariableAccessPolicy::Custom:
	default:
		return CanControllerAccessVariable(Cast<AGorgeousPlayerController>(Controller), PropertyName);
	}
}

AGorgeousPlayerController* UGorgeousObjectVariable::ResolveOwningPlayerController() const
{
	if (!AutoReplicationOwner.IsValid())
	{
		return nullptr;
	}

	if (AGorgeousPlayerController* AsPlacer = Cast<AGorgeousPlayerController>(AutoReplicationOwner.Get()))
	{
		return AsPlacer;
	}

	if (AActor* OwnerActor = Cast<AActor>(AutoReplicationOwner.Get()))
	{
		if (AGorgeousPlayerController* DirectController = Cast<AGorgeousPlayerController>(OwnerActor))
		{
			return DirectController;
		}

		if (AGorgeousPlayerController* InstigatorController = Cast<AGorgeousPlayerController>(OwnerActor->GetInstigatorController()))
		{
			return InstigatorController;
		}

		if (APawn* OwnerPawn = Cast<APawn>(OwnerActor))
		{
			return Cast<AGorgeousPlayerController>(OwnerPawn->GetController());
		}
	}

	return nullptr;
}

bool UGorgeousObjectVariable::InvokeNativeAutoReplicationRPCHandlerOnObject(UObject* Target, const FGorgeousQueuedRPC& QueuedRPC, UGorgeousObjectVariable** OutReturnVariable, bool* OutIsDeferred)
{
	UGorgeousObjectVariable* NativeReturnOV = nullptr;
	const bool bHandled = GorgeousObjectVariable_Private::InvokeNativeHandler(Target, QueuedRPC, &NativeReturnOV, OutIsDeferred);
	if (bHandled)
	{
		if (NativeReturnOV)
		{
			// Handler declared a return OV as its first parameter; the backend injected and the
			// handler may have populated it.  Pass it directly so the caller sets TargetVariable
			// to the handler-written OV instead of a generic UGorgeousRPC_OV wrapper.
			if (OutReturnVariable)
			{
				*OutReturnVariable = NativeReturnOV;
			}
		}
		else
		{
			UGorgeousRPC_OV* ResultContainer = CreateStandaloneRPCResultContainer(Target);
			if (OutReturnVariable)
			{
				*OutReturnVariable = ResultContainer;
			}
		}
	}
	else if (OutReturnVariable)
	{
		*OutReturnVariable = nullptr;
	}
	return bHandled;
}

UGorgeousObjectVariable* UGorgeousObjectVariable::GetOrCreateRPCResultParent()
{
	if (GorgeousObjectVariable_Private::CachedRPCResultsParent.IsValid())
	{
		return GorgeousObjectVariable_Private::CachedRPCResultsParent.Get();
	}

	if (UGorgeousObjectVariable* Existing = UGorgeousRootObjectVariable::FindVariableByDisplayName(GorgeousObjectVariable_Private::GorgeousRPCResultsDisplayName))
	{
		GorgeousObjectVariable_Private::CachedRPCResultsParent = Existing;
		return Existing;
	}

	UGorgeousRootObjectVariable* Root = UGorgeousRootObjectVariable::GetRootObjectVariable();
	if (!Root)
	{
		return nullptr;
	}

	FGuid Identifier;
	// UGorgeousObjectVariable is abstract — use the concrete UGorgeousRPC_OV as the grouping parent.
	UGorgeousObjectVariable* NewParent = Root->NewObjectVariable(UGorgeousRPC_OV::StaticClass(), Identifier, Root, true, GorgeousObjectVariable_Private::GorgeousRPCResultsDisplayName.ToString());
	if (NewParent)
	{
		NewParent->RootNetworkConfig.bExposeThroughRootNetworkStack = true;
		NewParent->RootNetworkConfig.AccessPolicy = EGorgeousObjectVariableAccessPolicy::Everyone;
		NewParent->RootNetworkConfig.ReplicationChannel = GorgeousObjectVariable_Private::GorgeousRPCResultsDisplayName;
		NewParent->SetNetworkingEnabled(true);
	}

	GorgeousObjectVariable_Private::CachedRPCResultsParent = NewParent;
	return NewParent;
}

UGorgeousRPC_OV* UGorgeousObjectVariable::CreateAutoRPCResultContainer()
{
	UGorgeousObjectVariable* ResultsParent = GetOrCreateRPCResultParent();
	if (!ResultsParent)
	{
		return nullptr;
	}

	FGuid Identifier;
	UGorgeousObjectVariable* NewInstance = ResultsParent->NewObjectVariable(UGorgeousRPC_OV::StaticClass(), Identifier, ResultsParent, false, FString());
	UGorgeousRPC_OV* ResultContainer = Cast<UGorgeousRPC_OV>(NewInstance);
	if (!ResultContainer)
	{
		return nullptr;
	}

	ResultContainer->AutoReplicationOwner = AutoReplicationOwner;
	ResultContainer->AutoReplicationEntryKey = AutoReplicationEntryKey;
	ResultContainer->AutoReplicationReplicationIndex = INDEX_NONE;
	ResultContainer->SetNetworkingEnabled(true);
	FGorgeousAutoReplicationContext Context;
	Context.OwningObject = AutoReplicationOwner.Get();
	Context.EntryKey = AutoReplicationEntryKey;
	Context.ReplicationIndex = INDEX_NONE;
	if (Context.OwningObject)
	{
		ResultContainer->ActivateReplication(Context);
	}
	else
	{
		GT_I_LOG("GT.ObjectVariables.RPC.ResultNoOwner", TEXT("%s created an RPC result container without an AutoReplication owner. Networking may be skipped."), *GetName());
	}
	ResultContainer->SetFlags(RF_Transient);
	return ResultContainer;
}

UGorgeousRPC_OV* UGorgeousObjectVariable::CreateStandaloneRPCResultContainer(UObject* OwningContext)
{
	UGorgeousObjectVariable* ResultsParent = GetOrCreateRPCResultParent();
	if (!ResultsParent)
	{
		return nullptr;
	}

	FGuid Identifier;
	UGorgeousObjectVariable* NewInstance = ResultsParent->NewObjectVariable(UGorgeousRPC_OV::StaticClass(), Identifier, ResultsParent, false, FString());
	UGorgeousRPC_OV* ResultContainer = Cast<UGorgeousRPC_OV>(NewInstance);
	if (!ResultContainer)
	{
		return nullptr;
	}

	ResultContainer->AutoReplicationOwner = OwningContext;
	ResultContainer->AutoReplicationEntryKey = NAME_None;
	ResultContainer->AutoReplicationReplicationIndex = INDEX_NONE;
	// Enable networking before ActivateReplication — same reason as CreateRPCResultContainer.
	ResultContainer->SetNetworkingEnabled(true);
	FGorgeousAutoReplicationContext Context;
	Context.OwningObject = OwningContext;
	Context.EntryKey = NAME_None;
	Context.ReplicationIndex = INDEX_NONE;
	if (Context.OwningObject)
	{
		ResultContainer->ActivateReplication(Context);
	}
	ResultContainer->SetFlags(RF_Transient);
	return ResultContainer;
}

bool UGorgeousObjectVariable::BuildRPCResultSnapshot(UGorgeousObjectVariable* ResultContainer, TArray<uint8>& OutSnapshot) const
{
	OutSnapshot.Reset();
	if (!ResultContainer)
	{
		return false;
	}

	FMemoryWriter Writer(OutSnapshot, true);
	Writer.SetIsPersistent(true);
	uint32 Version = GorgeousObjectVariable_Private::Snapshot::SnapshotVersion;
	Writer << Version;
	if (!SerializeRPCSnapshotRecursive(ResultContainer, Writer))
	{
		OutSnapshot.Reset();
		return false;
	}

	return !Writer.IsError();
}

bool UGorgeousObjectVariable::SerializeRPCSnapshotRecursive(UGorgeousObjectVariable* Variable, FArchive& Ar) const
{
	if (!Variable)
	{
		return false;
	}

	FString ClassPathString = Variable->GetClass()->GetPathName();
	Ar << ClassPathString;

	FGuid Identifier = IGorgeousObjectVariableInteraction_I::Execute_GetUniqueIdentifierForObjectVariable(Variable);
	Ar << Identifier;

	FString LocalDisplayName = Variable->GetDisplayName();
	Ar << LocalDisplayName;

	TArray<const FProperty*> SerializableProperties;
	for (FProperty* Property = Variable->GetClass()->PropertyLink; Property; Property = Property->PropertyLinkNext)
	{
		if (GorgeousObjectVariable_Private::Snapshot::ShouldCaptureProperty(Property))
		{
			SerializableProperties.Add(Property);
		}
	}

	int32 PropertyCount = SerializableProperties.Num();
	Ar << PropertyCount;
	for (const FProperty* Property : SerializableProperties)
	{
		FName PropertyName = Property->GetFName();
		Ar << PropertyName;

		TArray<uint8> Payload;
		if (uint8* PropertyData = Property->ContainerPtrToValuePtr<uint8>(Variable))
		{
			// Deep-init is false here — the RPC snapshot path uses its own class-path + property capture,
			// not the auto-replication deep-init format. Context is 'this' (const cast for the helper).
			GorgeousObjectVariable_Private::SerializePropertyValue(const_cast<UGorgeousObjectVariable*>(this), Property, PropertyData, EGorgeousReplicationMode::EProperty, nullptr, Payload, false);
		}
		GorgeousObjectVariable_Private::Snapshot::WriteByteArray(Ar, Payload);
	}

	int32 ChildCount = Variable->VariableRegistry.Num();
	Ar << ChildCount;
	for (auto& [ChildKey, Child] : Variable->VariableRegistry)
	{
		if (!SerializeRPCSnapshotRecursive(Child.Get(), Ar))
		{
			return false;
		}
	}

	return !Ar.IsError();
}

UGorgeousObjectVariable* UGorgeousObjectVariable::InstantiateRPCResultFromDescriptor(const FGorgeousAutoReplicationRPCResultDescriptor& Descriptor)
{
	if (Descriptor.SnapshotPayload.Num() == 0)
	{
		return nullptr;
	}

	UGorgeousObjectVariable* ResultsParent = GetOrCreateRPCResultParent();
	if (!ResultsParent)
	{
		return nullptr;
	}

	TArray<uint8> WorkingCopy = Descriptor.SnapshotPayload;
	FMemoryReader Reader(WorkingCopy, true);
	Reader.SetIsPersistent(true);

	uint32 Version = 0;
	Reader << Version;
	if (Version != GorgeousObjectVariable_Private::Snapshot::SnapshotVersion)
	{
		GT_W_LOG("GT.ObjectVariables.RPC.SnapshotVersionMismatch", TEXT("RPC result snapshot version mismatch (expected %u, received %u)."),
			GorgeousObjectVariable_Private::Snapshot::SnapshotVersion, Version);
		return nullptr;
	}

	UGorgeousObjectVariable* Reconstructed = DeserializeRPCSnapshotRecursive(ResultsParent, Reader);
	if (Reconstructed && Descriptor.ResultIdentifier.IsValid())
	{
		Reconstructed->ApplyReplicatedIdentifier(Descriptor.ResultIdentifier);
	}
	return Reconstructed;
}

UGorgeousObjectVariable* UGorgeousObjectVariable::DeserializeRPCSnapshotRecursive(UGorgeousObjectVariable* InParent, FArchive& Ar)
{
	FString ClassPathString;
	Ar << ClassPathString;

	FGuid Identifier;
	Ar << Identifier;

	FString LocalDisplayName;
	Ar << LocalDisplayName;

	int32 PropertyCount = 0;
	Ar << PropertyCount;

	struct FSerializedProperty
	{
		FName PropertyName;
		TArray<uint8> Payload;
	};

	TArray<FSerializedProperty> SerializedProperties;
	SerializedProperties.Reserve(PropertyCount);
	for (int32 PropertyIndex = 0; PropertyIndex < PropertyCount; ++PropertyIndex)
	{
		FSerializedProperty Entry;
		Ar << Entry.PropertyName;
		GorgeousObjectVariable_Private::Snapshot::ReadByteArray(Ar, Entry.Payload);
		SerializedProperties.Add(MoveTemp(Entry));
	}

	int32 ChildCount = 0;
	Ar << ChildCount;

	UClass* LoadedClass = nullptr;
	if (!ClassPathString.IsEmpty())
	{
		const FSoftObjectPath ClassPath(ClassPathString);
		LoadedClass = Cast<UClass>(ClassPath.TryLoad());
	}

	const bool bCanInstantiate = InParent != nullptr && LoadedClass && LoadedClass->IsChildOf(UGorgeousObjectVariable::StaticClass());
	if (!bCanInstantiate)
	{
		for (int32 ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
		{
			DeserializeRPCSnapshotRecursive(nullptr, Ar);
		}
		return nullptr;
	}

	FGuid TempIdentifier;
	UGorgeousObjectVariable* NewInstance = InParent->NewObjectVariable(LoadedClass, TempIdentifier, InParent, false, LocalDisplayName);
	if (!NewInstance)
	{
		for (int32 ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
		{
			DeserializeRPCSnapshotRecursive(nullptr, Ar);
		}
		return nullptr;
	}

	NewInstance->SetFlags(RF_Transient);
	NewInstance->SetNetworkingEnabled(false);
	if (Identifier.IsValid())
	{
		NewInstance->ApplyReplicatedIdentifier(Identifier);
	}

	for (const FSerializedProperty& SerializedProperty : SerializedProperties)
	{
		if (SerializedProperty.Payload.Num() == 0)
		{
			continue;
		}

		if (FProperty* Property = FindFProperty<FProperty>(LoadedClass, SerializedProperty.PropertyName))
		{
			if (uint8* PropertyData = Property->ContainerPtrToValuePtr<uint8>(NewInstance))
			{
				// Deep-init is false here — the RPC snapshot path handles class instantiation itself;
				// this call only applies property values to an already-created NewInstance.
				GorgeousObjectVariable_Private::DeserializePropertyValue(NewInstance, Property, PropertyData, EGorgeousReplicationMode::EProperty, nullptr, SerializedProperty.Payload, false);
			}
		}
	}

	for (int32 ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
	{
		DeserializeRPCSnapshotRecursive(NewInstance, Ar);
	}

	return NewInstance;
}

bool UGorgeousObjectVariable::SerializeOVToRPCArgumentBytes(UGorgeousObjectVariable* OV, TArray<uint8>& OutBytes)
{
	if (!OV)
	{
		return false;
	}
	// BuildRPCResultSnapshot only uses its parameters and not `this`, so calling it on OV itself
	// is safe and avoids the need for any special context instance.
	return OV->BuildRPCResultSnapshot(OV, OutBytes);
}

UGorgeousObjectVariable* UGorgeousObjectVariable::DeserializeOVFromRPCArgumentBytes(const TArray<uint8>& Bytes)
{
	if (Bytes.IsEmpty())
	{
		return nullptr;
	}

	TArray<uint8> WorkingCopy = Bytes;
	FMemoryReader Reader(WorkingCopy, true);
	Reader.SetIsPersistent(true);

	uint32 Version = 0;
	Reader << Version;
	if (Version != GorgeousObjectVariable_Private::Snapshot::SnapshotVersion)
	{
		GT_W_LOG("GT.ObjectVariables.RPC.ArgumentOVVersionMismatch",
			TEXT("RPC argument OV snapshot version mismatch (expected %u, received %u)."),
			GorgeousObjectVariable_Private::Snapshot::SnapshotVersion, Version);
		return nullptr;
	}

	UGorgeousObjectVariable* ResultsParent = GetOrCreateRPCResultParent();
	if (!ResultsParent)
	{
		GT_W_LOG("GT.ObjectVariables.RPC.ArgumentOVNoResultParent",
			TEXT("DeserializeOVFromRPCArgumentBytes: RPC result parent is unavailable — cannot reconstruct argument OV."));
		return nullptr;
	}

	return ResultsParent->DeserializeRPCSnapshotRecursive(ResultsParent, Reader);
}

void UGorgeousObjectVariable::RegisterReplicatedRPCResult(const FGorgeousQueuedRPC& QueuedRPC, UGorgeousObjectVariable* ResultContainer)
{
	if (!ResultContainer || !QueuedRPC.RequestGuid.IsValid())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return;
	}

	FGorgeousAutoReplicationRPCResultDescriptor Descriptor;
	Descriptor.RequestGuid = QueuedRPC.RequestGuid;
	Descriptor.ResultIdentifier = IGorgeousObjectVariableInteraction_I::Execute_GetUniqueIdentifierForObjectVariable(ResultContainer);
	if (!Descriptor.ResultIdentifier.IsValid())
	{
		return;
	}
	Descriptor.HandlerName = QueuedRPC.Payload.HandlerName;
	Descriptor.ResultClass = ResultContainer->GetClass();
	Descriptor.bReplicateToAllConnections = QueuedRPC.Payload.bReplicateResultToAllConnections;
	if (!BuildRPCResultSnapshot(ResultContainer, Descriptor.SnapshotPayload))
	{
		GT_W_LOG("GT.ObjectVariables.RPC.SnapshotCaptureFailed", TEXT("Failed to capture RPC result snapshot for %s."), *ResultContainer->GetName());
	}
	CachedRPCResultDescriptors.Add(Descriptor.RequestGuid, Descriptor);
	CachedRPCResultInstances.FindOrAdd(Descriptor.RequestGuid) = ResultContainer;

	if (Descriptor.bReplicateToAllConnections)
	{
		BroadcastRPCResultDescriptors.Add(Descriptor);
		TrimRPCResultArray(BroadcastRPCResultDescriptors);
	}
	else
	{
		OwnerScopedRPCResultDescriptors.Add(Descriptor);
		TrimRPCResultArray(OwnerScopedRPCResultDescriptors);
	}

	MarkRPCResultDescriptorsDirty();
}

void UGorgeousObjectVariable::HandleReplicatedRPCResultDescriptors(const TArray<FGorgeousAutoReplicationRPCResultDescriptor>& Descriptors)
{
	for (const FGorgeousAutoReplicationRPCResultDescriptor& Descriptor : Descriptors)
	{
		if (!Descriptor.RequestGuid.IsValid())
		{
			continue;
		}

		CachedRPCResultDescriptors.FindOrAdd(Descriptor.RequestGuid) = Descriptor;
		UGorgeousObjectVariable* ResolvedInstance = nullptr;
		if (Descriptor.ResultIdentifier.IsValid())
		{
			ResolvedInstance = UGorgeousRootObjectVariable::FindVariableByIdentifier(Descriptor.ResultIdentifier);
		}

		if (!ResolvedInstance)
		{
			ResolvedInstance = InstantiateRPCResultFromDescriptor(Descriptor);
			if (!ResolvedInstance && Descriptor.SnapshotPayload.Num() > 0)
			{
				GT_W_LOG("GT.ObjectVariables.RPC.ResultReconstructFailed", TEXT("Failed to reconstruct RPC result for request %s."), *Descriptor.RequestGuid.ToString());
			}
		}

		if (ResolvedInstance)
		{
			CachedRPCResultInstances.FindOrAdd(Descriptor.RequestGuid) = ResolvedInstance;
		}
	}
}

void UGorgeousObjectVariable::TrimRPCResultArray(TArray<FGorgeousAutoReplicationRPCResultDescriptor>& InOutArray)
{
	while (InOutArray.Num() > GorgeousObjectVariable_Private::MaxTrackedRPCResults)
	{
		const FGorgeousAutoReplicationRPCResultDescriptor Removed = InOutArray[0];
		InOutArray.RemoveAt(0);
		CachedRPCResultDescriptors.Remove(Removed.RequestGuid);
		CachedRPCResultInstances.Remove(Removed.RequestGuid);
	}
}

void UGorgeousObjectVariable::MarkRPCResultDescriptorsDirty()
{
	if (!SupportsAutoReplicationFeatures() || !bReplicates)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (World->GetNetMode() == NM_Client)
		{
			return;
		}

		FGorgeousAutoReplicationCoordinator::Get(World).MarkStreamDirty(this);
	}
}

void UGorgeousObjectVariable::SetNetworkingEnabled(const bool bShouldReplicate)
{
	SetIsReplicated(bShouldReplicate);
}

void UGorgeousObjectVariable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGorgeousObjectVariable, DisplayName);
	// Derived classes should rely on RegisteredReplicatedProperties for introspection.
}

void UGorgeousObjectVariable::PreNetReceive()
{
	Super::PreNetReceive();

	if (RegisteredReplicatedProperties.Num() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const ENetMode NetMode = World->GetNetMode();
	if (NetMode == NM_Standalone || NetMode == NM_DedicatedServer)
	{
		return;
	}

	for (FReplicatedPropertyDeclaration& Declaration : RegisteredReplicatedProperties)
	{
		if (!Declaration.HasRepNotify() || !Declaration.CachedProperty)
		{
			continue;
		}

		uint8* CurrentValue = Declaration.CachedProperty->ContainerPtrToValuePtr<uint8>(this);
		if (!CurrentValue)
		{
			continue;
		}

		Declaration.InitializeShadowState(Declaration.CachedProperty, nullptr);
		if (Declaration.RepNotifyShadow.Num() > 0)
		{
			Declaration.CachedProperty->CopyCompleteValue(Declaration.RepNotifyShadow.GetData(), CurrentValue);
		}
	}
}

void UGorgeousObjectVariable::PostNetReceive()
{
	Super::PostNetReceive();
	EvaluateRegisteredRepNotifies(false);
	RefreshClientChangeShadows();
}

void UGorgeousObjectVariable::HandleAutoReplicationRPCPayload_Implementation(const FGorgeousQueuedRPC& QueuedRPC)
{
	GT_I_LOG("GT.ObjectVariables.RPC.NoBlueprintOverride", TEXT("%s received AutoReplication RPC %s for key %s (%d named arguments) but no Blueprint override is provided."),
		*GetName(), *QueuedRPC.Payload.HandlerName.ToString(), *QueuedRPC.Key.ToString(), QueuedRPC.Payload.Arguments.Num());
}

void UGorgeousObjectVariable::ExampleAutoReplicationRPCHandler(double ExampleValue, const FString& ExampleLabel)
{
	GT_I_LOG("GT.ObjectVariables.RPC.ExampleHandler", TEXT("Example AutoReplication RPC handler invoked on %s | Label: %s | Value: %.2f"), *GetName(), *ExampleLabel, ExampleValue);
}

bool UGorgeousObjectVariable::EnsureReplicationActivation(const TCHAR* SourceFunction) const
{
	if (!bReplicationActivationGuard)
	{
		GT_W_LOG("GT.ObjectVariables.Replication.ActivationGuard", TEXT("%s can only be invoked inside OnReplicationActivated for %s."), SourceFunction, *GetName());
		return false;
	}
	return true;
}

FGuid UGorgeousObjectVariable::GetUniqueIdentifierForObjectVariable_Implementation()
{
	return UniqueIdentifier;
}

bool UGorgeousObjectVariable::ResolveAutoReplicationRPCContext(const FName OverrideKey, UObject* OverrideContext, FName& OutKey, UObject*& OutContext) const
{
	OutKey = OverrideKey.IsNone() ? AutoReplicationEntryKey : OverrideKey;
	OutContext = OverrideContext ? OverrideContext : AutoReplicationOwner.Get();
	return !OutKey.IsNone() && OutContext != nullptr;
}

UGorgeousObjectVariable::FReplicatedPropertyDeclaration* UGorgeousObjectVariable::FindReplicatedDeclarationByName(const FName PropertyName)
{
	return const_cast<FReplicatedPropertyDeclaration*>(static_cast<const UGorgeousObjectVariable*>(this)->FindReplicatedDeclarationByName(PropertyName));
}

const UGorgeousObjectVariable::FReplicatedPropertyDeclaration* UGorgeousObjectVariable::FindReplicatedDeclarationByName(const FName PropertyName) const
{
	for (const FReplicatedPropertyDeclaration& Declaration : RegisteredReplicatedProperties)
	{
		if (Declaration.PropertyName == PropertyName)
		{
			return &Declaration;
		}
	}

	return nullptr;
}

void UGorgeousObjectVariable::EvaluateRegisteredRepNotifies(const bool bForceAllNotifies)
{
	if (RegisteredReplicatedProperties.Num() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const ENetMode NetMode = World->GetNetMode();
	if (NetMode == NM_Standalone || NetMode == NM_DedicatedServer)
	{
		return;
	}

	for (FReplicatedPropertyDeclaration& Declaration : RegisteredReplicatedProperties)
	{
		if (!Declaration.HasRepNotify() || !Declaration.CachedProperty)
		{
			continue;
		}

		uint8* CurrentValue = Declaration.CachedProperty->ContainerPtrToValuePtr<uint8>(this);
		if (!CurrentValue)
		{
			continue;
		}

		Declaration.InitializeShadowState(Declaration.CachedProperty, nullptr);
		uint8* OldValueData = Declaration.RepNotifyShadow.Num() > 0 ? Declaration.RepNotifyShadow.GetData() : nullptr;

		bool bShouldNotify = bForceAllNotifies;
		if (!bShouldNotify)
		{
			if (!Declaration.bDeliveredInitialNotify)
			{
				bShouldNotify = Declaration.bFireInitialNotify;
			}
			else if (Declaration.RepNotifyPolicy == EGorgeousRepNotifyPolicy::Always)
			{
				bShouldNotify = true;
			}
			else if (OldValueData)
			{
				bShouldNotify = !Declaration.CachedProperty->Identical(OldValueData, CurrentValue);
			}
		}

		if (bShouldNotify)
		{
			InvokeRepNotify(Declaration, OldValueData);
		}

		Declaration.bDeliveredInitialNotify = true;
		if (Declaration.RepNotifyShadow.Num() > 0)
		{
			Declaration.CachedProperty->CopyCompleteValue(Declaration.RepNotifyShadow.GetData(), CurrentValue);
		}
	}
}

void UGorgeousObjectVariable::InvokeRepNotify(FReplicatedPropertyDeclaration& Declaration, const uint8* OldValueData)
{
	if (!Declaration.HasRepNotify())
	{
		return;
	}

	UFunction* RepFunction = FindFunction(Declaration.RepNotifyFunction);
	if (!RepFunction)
	{
		return;
	}

	if (RepFunction->ParmsSize <= 0)
	{
		ProcessEvent(RepFunction, nullptr);
		Declaration.bHasValidatedRepNotifySignature = true;
		Declaration.bIsRepNotifySignatureValid = true;
		return;
	}

	TArray<uint8> ParameterStorage;
	ParameterStorage.SetNumZeroed(RepFunction->ParmsSize);
	RepFunction->InitializeStruct(ParameterStorage.GetData());

	bool bFilledValue = false;
	for (FProperty* Param = RepFunction->PropertyLink; Param; Param = Param->PropertyLinkNext)
	{
		if (!Param->HasAnyPropertyFlags(CPF_Parm) || Param->HasAnyPropertyFlags(CPF_ReturnParm))
		{
			continue;
		}

		if (Declaration.CachedProperty && Declaration.CachedProperty->SameType(Param))
		{
			if (OldValueData)
			{
				void* DestPtr = Param->ContainerPtrToValuePtr<void>(ParameterStorage.GetData());
				Param->CopyCompleteValue(DestPtr, OldValueData);
			}
			bFilledValue = true;
		}
		break;
	}

	if (!Declaration.bHasValidatedRepNotifySignature)
	{
		Declaration.bHasValidatedRepNotifySignature = true;
		Declaration.bIsRepNotifySignatureValid = bFilledValue;
		if (!bFilledValue && Declaration.CachedProperty)
		{
			GT_W_LOG("GT.ObjectVariables.RepNotify.SignatureMismatch", TEXT("RepNotify %s on %s expects a parameter incompatible with property %s. Continuing without passing the old value."),
				*Declaration.RepNotifyFunction.ToString(), *GetName(), *Declaration.PropertyName.ToString());
		}
	}

	ProcessEvent(RepFunction, ParameterStorage.GetData());
	RepFunction->DestroyStruct(ParameterStorage.GetData());
}

void UGorgeousObjectVariable::TryClientAutoReplicateProperty(const FName PropertyName)
{
	if (PropertyName.IsNone())
	{
		return;
	}

	TSet<FName> DirtyProperties;
	DirtyProperties.Add(PropertyName);
	if (TryClientAutoReplicateProperties(DirtyProperties))
	{
		if (FReplicatedPropertyDeclaration* Declaration = FindReplicatedDeclarationByName(PropertyName))
		{
			UpdateChangeShadowForProperty(*Declaration);
		}
	}
	else
	{
		// The send failed (no relay component, not a client context, networking disabled, etc.).
		// Revert the live property back to the change shadow — the last value that was successfully
		// acknowledged by the server — to prevent a silent local/server divergence.
		// If the shadow is not yet initialized the write was the very first one, so there is nothing
		// to revert to and the value stays as written (it will be sent once the connection is ready).
		if (FReplicatedPropertyDeclaration* Declaration = FindReplicatedDeclarationByName(PropertyName))
		{
			if (Declaration->bChangeShadowInitialized && Declaration->ChangeShadow.Num() > 0 && Declaration->CachedProperty)
			{
				void* PropertyData = Declaration->CachedProperty->ContainerPtrToValuePtr<void>(this);
				if (PropertyData)
				{
					Declaration->CachedProperty->CopyCompleteValue(PropertyData, Declaration->ChangeShadow.GetData());
					GT_I_LOG("GT.ObjectVariables.ClientSync.Reverted", TEXT("%s: Property %s reverted to last server-consistent value because the send dispatch failed."), *GetName(), *PropertyName.ToString());
				}
			}
		}
	}
}

bool UGorgeousObjectVariable::TryClientAutoReplicateProperties(const TSet<FName>& DirtyProperties)
{
	if (DirtyProperties.Num() == 0)
	{
		GT_I_LOG("GT.ObjectVariables.ClientSync.NoDirty", TEXT("%s: No dirty properties to sync."), *GetName());
		return false;
	}

	if (!bSupportsNetworking || !bReplicates)
	{
		GT_I_LOG("GT.ObjectVariables.ClientSync.NetworkingDisabled", TEXT("%s: Networking disabled (bSupportsNetworking=%d, bReplicates=%d)."), *GetName(), bSupportsNetworking, bReplicates);
		return false;
	}

	if (AutoReplicationEntryKey.IsNone())
	{
		GT_I_LOG("GT.ObjectVariables.ClientSync.NoEntryKey", TEXT("%s: AutoReplicationEntryKey is None."), *GetName());
		return false;
	}

	if (!AutoReplicationOwner.IsValid())
	{
		GT_I_LOG("GT.ObjectVariables.ClientSync.NoOwner", TEXT("%s: AutoReplicationOwner is invalid."), *GetName());
		return false;
	}

	AActor* OwnerActor = ResolveReplicationOwnerActor();
	if (!OwnerActor)
	{
		GT_I_LOG("GT.ObjectVariables.ClientSync.NoOwnerActor", TEXT("%s: Could not resolve owner actor."), *GetName());
		return false;
	}

	if (OwnerActor->HasAuthority())
	{
		GT_I_LOG("GT.ObjectVariables.ClientSync.HasAuthority", TEXT("%s: Owner actor %s has authority, skipping client sync."), *GetName(), *OwnerActor->GetName());
		return false;
	}

	// For client-to-server property sync, we need to use the RPC relay component
	// because it exists as a default subobject on both server and client (unlike dynamically created transporters).
	// The relay component is created in the PlayerController constructor, so Server RPCs work properly.
	// If the owner is a PlayerState, we must use its PlayerController instead, because PlayerState
	// doesn't have the right kind of owning connection for Server RPCs to work.
	AActor* RelayActor = OwnerActor;
	if (APlayerState* PS = Cast<APlayerState>(OwnerActor))
	{
		if (APlayerController* PC = PS->GetPlayerController())
		{
			RelayActor = PC;
			GT_I_LOG("GT.ObjectVariables.ClientSync.ResolvedToPC", TEXT("%s: Resolved PlayerState %s to PlayerController %s for relay."), 
				*GetName(), *PS->GetName(), *PC->GetName());
		}
		else
		{
			// PlayerState doesn't have a PlayerController yet (timing during initialization).
			// On a client, we can use the local PlayerController instead.
			if (UWorld* World = OwnerActor->GetWorld())
			{
				if (APlayerController* LocalPC = World->GetFirstPlayerController())
				{
					RelayActor = LocalPC;
					GT_I_LOG("GT.ObjectVariables.ClientSync.FallbackToLocalPC", TEXT("%s: PlayerState %s has no PC yet, using local PlayerController %s for relay."), 
						*GetName(), *PS->GetName(), *LocalPC->GetName());
				}
				else
				{
					GT_W_LOG("GT.ObjectVariables.ClientSync.NoLocalPC", TEXT("%s: PlayerState %s has no PlayerController and no local PC found. Cannot sync."), *GetName(), *PS->GetName());
					return false;
				}
			}
			else
			{
				GT_W_LOG("GT.ObjectVariables.ClientSync.NoWorld", TEXT("%s: PlayerState %s has no World. Cannot sync."), *GetName(), *PS->GetName());
				return false;
			}
		}
	}
	
	UGorgeousAutoReplicationRPCRelayComponent* RelayComponent = RelayActor->FindComponentByClass<UGorgeousAutoReplicationRPCRelayComponent>();
	if (!RelayComponent)
	{
		GT_W_LOG("GT.ObjectVariables.ClientSync.NoRelayComponent", TEXT("%s: No RPC relay component found on owner actor %s."), *GetName(), *OwnerActor->GetName());
		return false;
	}

	GT_I_LOG("GT.ObjectVariables.ClientSync.RelayFound", TEXT("%s: Found relay component on actor %s (Class: %s, Role: %d, HasLocalNetOwner: %d)."),
		*GetName(), *OwnerActor->GetName(), *OwnerActor->GetClass()->GetName(),
		static_cast<int32>(OwnerActor->GetLocalRole()), OwnerActor->HasLocalNetOwner());

	FGorgeousAutoReplicationConditionContext ConditionContext;
	ConditionContext.bIsOwnerConnection = OwnerActor->HasLocalNetOwner();

	// Resolve the PackageMap for this connection so UObject references inside replicated properties
	// (e.g. TArray<UObject*>) are serialized correctly. OVs replicated via the auto-replication mixin
	// bypass the standard UE replication path and therefore do not receive automatic PackageMap injection.
	if (UNetConnection* OwnerConn = OwnerActor->GetNetConnection())
	{
		ConditionContext.PackageMap = OwnerConn->PackageMap;
	}
	else if (UWorld* OVWorld = OwnerActor->GetWorld())
	{
		if (UNetDriver* NetDriver = OVWorld->GetNetDriver())
		{
			ConditionContext.PackageMap = NetDriver->ServerConnection ? NetDriver->ServerConnection->PackageMap : nullptr;
		}
	}

	FGorgeousAutoReplicationPropertyPayload Payload;
	if (!BuildAutoReplicationPropertyPayload(ConditionContext, Payload))
	{
		GT_W_LOG("GT.ObjectVariables.ClientSync.BuildPayloadFailed", TEXT("%s: Failed to build property payload."), *GetName());
		return false;
	}

	Payload.Properties.RemoveAll([&](const FGorgeousAutoReplicationPropertyValue& Value)
	{
		return !DirtyProperties.Contains(Value.PropertyName);
	});

	if (Payload.Properties.Num() == 0)
	{
		GT_I_LOG("GT.ObjectVariables.ClientSync.EmptyPayload", TEXT("%s: Payload contained no matching dirty properties after filtering."), *GetName());
		return false;
	}

	GT_I_LOG("GT.ObjectVariables.ClientSync.Sending", TEXT("%s: Sending %d dirty properties to server via relay for entry %s."), *GetName(), Payload.Properties.Num(), *AutoReplicationEntryKey.ToString());

	FGorgeousAutoReplicationPropertyEnvelope Envelope;
	Envelope.EntryKey = AutoReplicationEntryKey;
	Envelope.Payload = MoveTemp(Payload);

	// Get the mixin from the owner to pass to the relay
	FGorgeousAutoReplicationMixin* TargetMixin = nullptr;
	if (AGorgeousPlayerController* PC = Cast<AGorgeousPlayerController>(OwnerActor))
	{
		TargetMixin = &PC->GetAutoReplicationMixin();
	}

	const bool bResult = RelayComponent->RelayPropertyPayloadToServer(Envelope, TargetMixin);
	if (!bResult)
	{
		GT_W_LOG("GT.ObjectVariables.ClientSync.RelayFailed", TEXT("%s: RelayPropertyPayloadToServer returned false."), *GetName());
	}
	return bResult;
}

bool UGorgeousObjectVariable::ShouldClientPollForPropertyChanges() const
{
	if (!SupportsAutoReplicationFeatures() || !bSupportsNetworking || !bReplicates)
	{
		return false;
	}

	if (RegisteredReplicatedProperties.Num() == 0)
	{
		return false;
	}

	if (AutoReplicationEntryKey.IsNone() || !AutoReplicationOwner.IsValid())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() != NM_Client)
	{
		return false;
	}

	return true;
}

void UGorgeousObjectVariable::StartClientPropertyPolling()
{
	if (!ShouldClientPollForPropertyChanges())
	{
		StopClientPropertyPolling();
		return;
	}

	const float TargetFrequency = AutoReplicationConfig.GetEffectiveUpdateFrequency();
	const float Interval = TargetFrequency > 0.0f ? (1.0f / TargetFrequency) : 0.0f;
	if (ClientPropertyPollingHandle.IsValid() && FMath::IsNearlyEqual(ClientPropertyPollingIntervalSeconds, Interval))
	{
		return;
	}

	StopClientPropertyPolling();
	ClientPropertyPollingIntervalSeconds = Interval;
	ClientPropertyPollingHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UGorgeousObjectVariable::HandleClientPropertyPollingTick),
		ClientPropertyPollingIntervalSeconds);
}

void UGorgeousObjectVariable::StopClientPropertyPolling()
{
	if (ClientPropertyPollingHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(ClientPropertyPollingHandle);
		ClientPropertyPollingHandle.Reset();
	}
}

bool UGorgeousObjectVariable::HandleClientPropertyPollingTick(float DeltaSeconds)
{
	if (!ShouldClientPollForPropertyChanges())
	{
		StopClientPropertyPolling();
		return false;
	}

	TSet<FName> DirtyProperties;
	for (FReplicatedPropertyDeclaration& Declaration : RegisteredReplicatedProperties)
	{
		if (!Declaration.CachedProperty)
		{
			continue;
		}

		void* CurrentValue = Declaration.CachedProperty->ContainerPtrToValuePtr<void>(this);
		if (!CurrentValue)
		{
			continue;
		}

		if (!Declaration.bChangeShadowInitialized)
		{
			Declaration.InitializeChangeShadow(Declaration.CachedProperty, CurrentValue);
			continue;
		}

		if (Declaration.ChangeShadow.Num() == 0)
		{
			continue;
		}

		if (!Declaration.CachedProperty->Identical(Declaration.ChangeShadow.GetData(), CurrentValue))
		{
			DirtyProperties.Add(Declaration.PropertyName);
			GT_I_LOG("GT.ObjectVariables.ClientPoll.DirtyDetected", TEXT("%s: Property %s changed, marking as dirty."), *GetName(), *Declaration.PropertyName.ToString());
		}
	}

	if (DirtyProperties.Num() > 0)
	{
		GT_I_LOG("GT.ObjectVariables.ClientPoll.AttemptSync", TEXT("%s: Attempting to sync %d dirty properties."), *GetName(), DirtyProperties.Num());
		if (TryClientAutoReplicateProperties(DirtyProperties))
		{
			for (FReplicatedPropertyDeclaration& Declaration : RegisteredReplicatedProperties)
			{
				if (DirtyProperties.Contains(Declaration.PropertyName))
				{
					UpdateChangeShadowForProperty(Declaration);
				}
			}
		}
	}

	return true;
}

void UGorgeousObjectVariable::RefreshClientChangeShadows()
{
	if (!ShouldClientPollForPropertyChanges())
	{
		return;
	}

	for (FReplicatedPropertyDeclaration& Declaration : RegisteredReplicatedProperties)
	{
		UpdateChangeShadowForProperty(Declaration);
	}
}

void UGorgeousObjectVariable::UpdateChangeShadowForProperty(FReplicatedPropertyDeclaration& Declaration)
{
	if (!Declaration.CachedProperty)
	{
		return;
	}

	void* CurrentValue = Declaration.CachedProperty->ContainerPtrToValuePtr<void>(this);
	if (!CurrentValue)
	{
		return;
	}

	Declaration.InitializeChangeShadow(Declaration.CachedProperty, CurrentValue);
}

bool UGorgeousObjectVariable::ShouldServerPollForPropertyChanges() const
{
	if (!SupportsAutoReplicationFeatures() || !bSupportsNetworking || !bReplicates)
	{
		return false;
	}

	if (RegisteredReplicatedProperties.Num() == 0)
	{
		return false;
	}

	if (AutoReplicationEntryKey.IsNone() || !AutoReplicationOwner.IsValid())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Server-side polling for NM_DedicatedServer, NM_ListenServer, and NM_Standalone (with clients)
	const ENetMode NetMode = World->GetNetMode();
	return NetMode == NM_DedicatedServer || NetMode == NM_ListenServer;
}

void UGorgeousObjectVariable::StartServerPropertyPolling()
{
	if (!ShouldServerPollForPropertyChanges())
	{
		StopServerPropertyPolling();
		return;
	}

	const float TargetFrequency = AutoReplicationConfig.GetEffectiveUpdateFrequency();
	const float Interval = TargetFrequency > 0.0f ? (1.0f / TargetFrequency) : 0.0f;
	if (ServerPropertyPollingHandle.IsValid() && FMath::IsNearlyEqual(ServerPropertyPollingIntervalSeconds, Interval))
	{
		return;
	}

	StopServerPropertyPolling();
	ServerPropertyPollingIntervalSeconds = Interval;
	ServerPropertyPollingHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UGorgeousObjectVariable::HandleServerPropertyPollingTick),
		ServerPropertyPollingIntervalSeconds);
}

void UGorgeousObjectVariable::StopServerPropertyPolling()
{
	if (ServerPropertyPollingHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(ServerPropertyPollingHandle);
		ServerPropertyPollingHandle.Reset();
	}
}

bool UGorgeousObjectVariable::HandleServerPropertyPollingTick(float DeltaSeconds)
{
	if (!ShouldServerPollForPropertyChanges())
	{
		StopServerPropertyPolling();
		return false;
	}

	TSet<FName> DirtyProperties;
	for (FReplicatedPropertyDeclaration& Declaration : RegisteredReplicatedProperties)
	{
		if (!Declaration.CachedProperty)
		{
			continue;
		}

		void* CurrentValue = Declaration.CachedProperty->ContainerPtrToValuePtr<void>(this);
		if (!CurrentValue)
		{
			continue;
		}

		if (!Declaration.bChangeShadowInitialized)
		{
			Declaration.InitializeChangeShadow(Declaration.CachedProperty, CurrentValue);
			continue;
		}

		if (Declaration.ChangeShadow.Num() == 0)
		{
			continue;
		}

		if (!Declaration.CachedProperty->Identical(Declaration.ChangeShadow.GetData(), CurrentValue))
		{
			DirtyProperties.Add(Declaration.PropertyName);
			GT_I_LOG("GT.ObjectVariables.ServerPoll.DirtyDetected", TEXT("%s: Property %s changed on server, marking for replication."), *GetName(), *Declaration.PropertyName.ToString());
		}
	}

	if (DirtyProperties.Num() > 0)
	{
		GT_I_LOG("GT.ObjectVariables.ServerPoll.AttemptReplicate", TEXT("%s: Attempting to replicate %d dirty properties to clients."), *GetName(), DirtyProperties.Num());
		if (TryServerReplicateProperties(DirtyProperties))
		{
			for (FReplicatedPropertyDeclaration& Declaration : RegisteredReplicatedProperties)
			{
				if (DirtyProperties.Contains(Declaration.PropertyName))
				{
					UpdateChangeShadowForProperty(Declaration);
				}
			}
		}
	}

	return true;
}

bool UGorgeousObjectVariable::ShouldSendAutoReplicationPayloadToController(APlayerController* PC, FName PropertyName) const
{
	if (!PC)
	{
		return false;
	}

	// If this OV does not participate in the root network stack there are no access restrictions.
	if (!ShouldUseRootNetworkStack())
	{
		return true;
	}

	UWorld* OVWorld = GetWorld();
	if (!OVWorld)
	{
		return EvaluateAccessPolicyForController(PC, PropertyName);
	}

	if (UGorgeousRootNetworkStackSubsystem* RootNetworkStack = UGorgeousRootNetworkStackSubsystem::Get(OVWorld))
	{
		// When PropertyName is None this is a whole-OV gate: validate channel subscription and
		// whole-OV access policy via the root stack (TryAutoSubscribeController + CanControllerAccess).
		// When PropertyName is set, the channel gate was already passed by the whole-OV call that
		// preceded the per-property loop, so we only re-evaluate the access policy with the property
		// context to allow Blueprint overrides of CanControllerAccessVariable to discriminate per-property.
		if (PropertyName.IsNone())
		{
			RootNetworkStack->TryAutoSubscribeController(this, PC);
			return RootNetworkStack->CanControllerAccess(this, PC);
		}

		// Per-property path: channel already validated by the prior whole-OV gate.
		return EvaluateAccessPolicyForController(PC, PropertyName);
	}

	// Fallback when the subsystem is unavailable (e.g. during tests).
	return EvaluateAccessPolicyForController(PC, PropertyName);
}

bool UGorgeousObjectVariable::TryServerReplicateProperties(const TSet<FName>& DirtyProperties)
{
	if (DirtyProperties.Num() == 0)
	{
		return false;
	}

	if (!bSupportsNetworking || !bReplicates)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Resolve the owner actor once so we can determine per-connection ownership below.
	AActor* OwnerActor = ResolveReplicationOwnerActor();

	// Build and send one payload per client connection. OVs replicated via the auto-replication mixin
	// bypass the standard UE replication path, so they do not receive automatic PackageMap injection.
	// Serializing per-connection ensures that UObject references (e.g. TArray<UObject*>) are mapped
	// through the correct per-connection PackageMap, matching how the standard replication path works.
	int32 ClientsSent = 0;
	int32 TotalPropertiesSent = 0;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			continue;
		}

		// Skip local player on listen server
		if (PC->IsLocalController())
		{
			continue;
		}

		// Enforce access policy before sending — mirrors the access check performed by the transporter
		// path (FGorgeousAutoReplicationMixin::CanControllerReceivePropertyPayload) so that the
		// relay-component polling path is equally protected.
		if (!ShouldSendAutoReplicationPayloadToController(PC))
		{
			GT_I_LOG("GT.ObjectVariables.ServerSync.AccessDenied", TEXT("%s: Skipping property payload for PlayerController %s due to access policy."), *GetName(), *PC->GetName());
			continue;
		}

		// Find the relay component on the player controller
		UGorgeousAutoReplicationRPCRelayComponent* RelayComponent = PC->FindComponentByClass<UGorgeousAutoReplicationRPCRelayComponent>();
		if (!RelayComponent)
		{
			GT_I_LOG("GT.ObjectVariables.ServerSync.NoRelayComponent", TEXT("%s: PlayerController %s has no AutoReplicationRPCRelayComponent."), *GetName(), *PC->GetName());
			continue;
		}

		// Per-connection condition context: inject this client's PackageMap so UObject references inside
		// replicated properties are serialized correctly for its connection. Also determine whether this
		// particular connection is the net owner of the variable's owning actor, for conditional replication.
		UNetConnection* PCConnection = PC->GetNetConnection();
		FGorgeousAutoReplicationConditionContext ConditionContext;
		ConditionContext.PackageMap = PCConnection ? PCConnection->PackageMap : nullptr;
		ConditionContext.bIsOwnerConnection = OwnerActor && PCConnection && (OwnerActor->GetNetConnection() == PCConnection);

		FGorgeousAutoReplicationPropertyPayload Payload;
		if (!BuildAutoReplicationPropertyPayload(ConditionContext, Payload))
		{
			GT_I_LOG("GT.ObjectVariables.ServerSync.BuildPayloadFailed", TEXT("%s: Failed to build property payload for client %s — skipping."), *GetName(), *PC->GetName());
			continue;
		}

		// Filter to only dirty properties, and apply per-property access policy so that Blueprint
		// overrides of CanControllerAccessVariable with a specific PropertyName can suppress
		// individual properties for this connection without blocking the whole payload.
		Payload.Properties.RemoveAll([&](const FGorgeousAutoReplicationPropertyValue& Value)
		{
			if (!DirtyProperties.Contains(Value.PropertyName))
			{
				return true; // not dirty
			}

			// Channel subscription was already validated by ShouldSendAutoReplicationPayloadToController
			// above.  Here we only re-run the access policy with the concrete property name so that
			// Custom-policy CanControllerAccessVariable overrides can gate individual UPROPERTY fields.
			if (!ShouldSendAutoReplicationPayloadToController(PC, Value.PropertyName))
			{
				GT_I_LOG("GT.ObjectVariables.ServerSync.PropertyAccessDenied",
					TEXT("%s: Property %s suppressed for PlayerController %s by per-property access policy."),
					*GetName(), *Value.PropertyName.ToString(), *PC->GetName());
				return true; // remove from payload
			}

			return false; // keep
		});

		if (Payload.Properties.Num() == 0)
		{
			continue;
		}

		// Build the per-client envelope and dispatch
		FGorgeousAutoReplicationPropertyEnvelope Envelope;
		Envelope.EntryKey = AutoReplicationEntryKey;
		Envelope.Payload = Payload;

		// Set the target mixin on the relay so the client can process the payload
		if (AGorgeousPlayerController* GPC = Cast<AGorgeousPlayerController>(PC))
		{
			RelayComponent->SetTargetMixin(&GPC->GetAutoReplicationMixin());
		}

		if (RelayComponent->RelayPropertyPayloadToClient(Envelope))
		{
			ClientsSent++;
			TotalPropertiesSent += Envelope.Payload.Properties.Num();
		}
	}

	// Also mark the stream dirty for Iris backend support
	FGorgeousAutoReplicationCoordinator& Coordinator = FGorgeousAutoReplicationCoordinator::Get(World);
	Coordinator.MarkStreamDirty(this);

	GT_I_LOG("GT.ObjectVariables.ServerSync.Replicated", TEXT("%s: Sent %d properties to %d clients for replication."), *GetName(), TotalPropertiesSent, ClientsSent);
	return ClientsSent > 0;
}

void UGorgeousObjectVariable::SetAutoReplicationBinding(UObject* InOwner, const FName InEntryKey, const int32 InReplicationIndex, const FGorgeousAutoReplicationStreamConfig* StreamOverride, const bool bWantsReplication)
{
	AutoReplicationOwner = InOwner;
	AutoReplicationEntryKey = InEntryKey;
	AutoReplicationReplicationIndex = InReplicationIndex;

	if (StreamOverride)
	{
		AutoReplicationConfig = *StreamOverride;
	}

	if (SupportsAutoReplicationFeatures() && bWantsReplication && !bAutoReplicationActivated && AutoReplicationOwner.IsValid())
	{
		FGorgeousAutoReplicationContext Context;
		Context.OwningObject = AutoReplicationOwner.Get();
		Context.EntryKey = AutoReplicationEntryKey;
		Context.ReplicationIndex = AutoReplicationReplicationIndex;
		ActivateReplication(Context);
	}

	const bool bWasReplicating = bReplicates;
	SetIsReplicated(bWantsReplication);

	if (bWasReplicating && bReplicates)
	{
		UpdateAutoReplicationState(true);
	}

	if (bReplicates)
	{
		StartClientPropertyPolling();
		StartServerPropertyPolling();
	}
	else
	{
		StopClientPropertyPolling();
		StopServerPropertyPolling();
	}
}

void UGorgeousObjectVariable::RegisterLegacyReplication(UObject* BindingContext)
{
	if (!SupportsLegacyReplication() || !bSupportsNetworking || !bReplicates)
	{
		return;
	}

	AActor* OwnerActor = ResolveReplicationOwnerActor(BindingContext);
	if (!OwnerActor)
	{
		return;
	}

	if (bLegacyReplicationRegistered && LegacyReplicationOwner.Get() == OwnerActor)
	{
		return;
	}

	if (LegacyReplicationOwner.IsValid() && LegacyReplicationOwner.Get() != OwnerActor)
	{
		LegacyReplicationOwner->RemoveReplicatedSubObject(this);
		bLegacyReplicationRegistered = false;
	}

	if (OwnerActor->HasAuthority())
	{
		OwnerActor->AddReplicatedSubObject(this);
	}

	LegacyReplicationOwner = OwnerActor;
	bLegacyReplicationRegistered = true;
}

void UGorgeousObjectVariable::UnregisterLegacyReplication()
{
	if (!bLegacyReplicationRegistered)
	{
		return;
	}

	if (LegacyReplicationOwner.IsValid())
	{
		LegacyReplicationOwner->RemoveReplicatedSubObject(this);
	}

	LegacyReplicationOwner.Reset();
	bLegacyReplicationRegistered = false;
}

AActor* UGorgeousObjectVariable::ResolveReplicationOwnerActor(UObject* BindingContext) const
{
	if (BindingContext)
	{
		if (AActor* BindingActor = Cast<AActor>(BindingContext))
		{
			return BindingActor;
		}
	}

	if (AActor* CachedActor = LegacyReplicationOwner.Get())
	{
		return CachedActor;
	}

	if (AActor* AutoRepActor = Cast<AActor>(AutoReplicationOwner.Get()))
	{
		return AutoRepActor;
	}

	return GetTypedOuter<AActor>();
}

bool UGorgeousObjectVariable::IsExecutingOnReplicationOwner() const
{
	if (AActor* OwnerActor = ResolveReplicationOwnerActor())
	{
		return OwnerActor->HasLocalNetOwner();
	}

	return false;
}

bool UGorgeousObjectVariable::HasAuthority() const
{
	UWorld* World = GetWorld();
	return World && World->GetNetMode() != NM_Client;
}

void UGorgeousObjectVariable::SetIsReplicated(bool InIsReplicated)
{
	const bool bRootForcesNetworking = DoesRootEnforceNetworking();
	const bool bEffectiveRequest = InIsReplicated || bRootForcesNetworking;

	if (bRootForcesNetworking && bSupportsNetworking && !RootNetworkConfig.bExposeThroughRootNetworkStack)
	{
		RootNetworkConfig.bExposeThroughRootNetworkStack = true;
	}

	if (bEffectiveRequest && !bSupportsNetworking)
	{
		GT_W_LOG("GT.ObjectVariables.Replication.DisabledSupport", TEXT("%s cannot enable replication%s because networking support is disabled."),
			*GetName(), bRootForcesNetworking ? TEXT(" (required by root)") : TEXT(""));
		return;
	}

	const bool bShouldReplicate = bEffectiveRequest && bSupportsNetworking;
	if (bReplicates == bShouldReplicate)
	{
		return;
	}

	bReplicates = bShouldReplicate;

	UpdateAutoReplicationState(bReplicates);

	if (bReplicates)
	{
		StartClientPropertyPolling();
		StartServerPropertyPolling();
	}
	else
	{
		StopClientPropertyPolling();
		StopServerPropertyPolling();
	}

	if (SupportsLegacyReplication())
	{
		if (bReplicates)
		{
			RegisterLegacyReplication(AutoReplicationOwner.Get());
		}
		else
		{
			UnregisterLegacyReplication();
		}
	}
	else if (!bReplicates)
	{
		UnregisterLegacyReplication();
	}
}

void UGorgeousObjectVariable::UpdateAutoReplicationState(const bool bEnable)
{
	if (!SupportsAutoReplicationFeatures())
	{
		return;
	}

	UWorld* TargetWorld = GetWorld();
	if (!TargetWorld || TargetWorld->GetNetMode() == NM_Client)
	{
		return;
	}

	FGorgeousAutoReplicationCoordinator& Coordinator = FGorgeousAutoReplicationCoordinator::Get(TargetWorld);
	if (bEnable)
	{
		Coordinator.RegisterObjectVariable(this, AutoReplicationConfig);
	}
	else
	{
		Coordinator.UnregisterObjectVariable(this);
	}

	if (UGorgeousRootNetworkStackSubsystem* RootNetworkStack = UGorgeousRootNetworkStackSubsystem::Get(TargetWorld))
	{
		if (bEnable && ShouldUseRootNetworkStack())
		{
			RootNetworkStack->RegisterVariable(this);
		}
		else
		{
			RootNetworkStack->UnregisterVariable(this);
		}
	}
}

int32 UGorgeousObjectVariable::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (AActor* OwnerActor = ResolveReplicationOwnerActor())
	{
		return OwnerActor->GetFunctionCallspace(Function, Stack);
	}

	return Super::GetFunctionCallspace(Function, Stack);
}

bool UGorgeousObjectVariable::CallRemoteFunction(UFunction* Function, void* Parameters, FOutParmRec* OutParms, FFrame* Stack)
{
	if (AActor* OwnerActor = ResolveReplicationOwnerActor())
	{
		if (UWorld* World = OwnerActor->GetWorld())
		{
			if (UNetDriver* NetDriver = World->GetNetDriver())
			{
				NetDriver->ProcessRemoteFunction(OwnerActor, Function, Parameters, OutParms, Stack, this);
				return true;
			}
		}
	}

	return Super::CallRemoteFunction(Function, Parameters, OutParms, Stack);
}

#if WITH_EDITOR
void UGorgeousObjectVariable::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	static const FName SharedStackName = GET_MEMBER_NAME_CHECKED(UGorgeousObjectVariable, bUseSharedNetworkStack);
	static const FName RootExposeName = GET_MEMBER_NAME_CHECKED(FGorgeousRootNetworkAccessConfig, bExposeThroughRootNetworkStack);

	if (PropertyName == SharedStackName)
	{
		SetSharedNetworkStackEnabled(bUseSharedNetworkStack);
	}
	else if (PropertyName == RootExposeName)
	{
		if (RootNetworkConfig.bExposeThroughRootNetworkStack)
		{
			SetSharedNetworkStackEnabled(false);
		}
	}
}
#endif // WITH_EDITOR

