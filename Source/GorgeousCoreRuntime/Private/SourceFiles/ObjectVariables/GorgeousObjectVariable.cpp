// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
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
#include "ObjectVariables/Networking/GorgeousRootNetworkStackSubsystem.h"
#include "GorgeousCoreUtilitiesMinimalShared.h"
#include "AutoReplication/ObjectVariables/GorgeousRPC_OV.h"
#include "AutoReplication/GorgeousAutoReplicationCoordinator.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
#include "ModuleCore/GorgeousAutoReplicationSettings.h"
#include "ModuleCore/GorgeousObjectVariableRootSettings.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Profiling/GorgeousProfiling.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Engine/ActorChannel.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/StructuredArchive.h"
#include "Serialization/Formatters/BinaryArchiveFormatter.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/Field.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UnrealType.h"
#include "Misc/ScopeLock.h"
#include "HAL/ThreadSafeCounter.h"
#include "Stats/Stats.h"
#include "Net/UnrealNetwork.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousObjectVariable Implementation
//=============================================================================

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
	, bDeliveredInitialNotify(false)
	, bShadowInitialized(false)
	, bHasValidatedRepNotifySignature(false)
	, bIsRepNotifySignatureValid(true)
{
}

UGorgeousObjectVariable::FReplicatedPropertyDeclaration::~FReplicatedPropertyDeclaration()
{
	ResetShadowState();
}

void UGorgeousObjectVariable::FReplicatedPropertyDeclaration::InitializeShadowState(const FProperty* Property, const void* InitialData)
{
	if (!Property)
	{
		return;
	}

	if (!bShadowInitialized)
	{
		const int32 ShadowSize = Property->GetElementSize() * Property->ArrayDim;
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

bool UGorgeousObjectVariable::ValidateVariableAssignment(const FName PropertyName, const FProperty* ValueProperty, const void* ValueAddress) const
{
	return true;
}

namespace GorgeousObjectVariable_Private
{
	static constexpr int32 MaxTrackedRPCResults = 8;
	static const FName GorgeousRPCResultsDisplayName(TEXT("GorgeousRPCResults"));
	static TWeakObjectPtr<UGorgeousObjectVariable> CachedRPCResultsParent;
	static const FName ValuePropertyName(TEXT("Value"));

	static bool SerializePropertyValue(const FProperty* Property, void* ValuePtr, const EGorgeousReplicationMode Mode, UPackageMap* PackageMap, TArray<uint8>& OutBytes)
	{
		if (!Property || !ValuePtr)
		{
			return false;
		}

		OutBytes.Reset();

		switch (Mode)
		{
		case EGorgeousReplicationMode::EProperty:
		{
			FMemoryWriter Writer(OutBytes, true);
			FBinaryArchiveFormatter Formatter(Writer);
			FStructuredArchive Archive(Formatter);
			FStructuredArchive::FSlot Slot = Archive.Open();
			Property->SerializeItem(Slot, ValuePtr, nullptr);
			Archive.Close();
			return !Writer.IsError();
		}
		case EGorgeousReplicationMode::ENetSerialize:
		{
			FMemoryWriter Writer(OutBytes, true);
			const bool bNetSerialized = Property->NetSerializeItem(Writer, PackageMap, ValuePtr);
			return bNetSerialized && !Writer.IsError();
		}
		case EGorgeousReplicationMode::ECustomPayload:
		default:
			return false;
		}
	}

	static bool DeserializePropertyValue(FProperty* Property, void* ValuePtr, const EGorgeousReplicationMode Mode, UPackageMap* PackageMap, const TArray<uint8>& InBytes)
	{
		if (!Property || !ValuePtr || InBytes.Num() == 0)
		{
			return false;
		}

		FMemoryReader Reader(InBytes, true);

		switch (Mode)
		{
		case EGorgeousReplicationMode::EProperty:
		{
			FBinaryArchiveFormatter Formatter(Reader);
			FStructuredArchive Archive(Formatter);
			FStructuredArchive::FSlot Slot = Archive.Open();
			Property->SerializeItem(Slot, ValuePtr, nullptr);
			Archive.Close();
			return !Reader.IsError();
		}
		case EGorgeousReplicationMode::ENetSerialize:
			return Property->NetSerializeItem(Reader, PackageMap, ValuePtr) && !Reader.IsError();
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

	static bool CopyArgumentToProperty(UGorgeousObjectVariable* ArgumentVariable, FProperty* ParameterProperty, uint8* ParameterData)
	{
		if (!ArgumentVariable || !ParameterProperty || !ParameterData)
		{
			return false;
		}

		FProperty* SourceProperty = ArgumentVariable->GetClass()->FindPropertyByName(ValuePropertyName);
		if (!SourceProperty)
		{
			UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("%s does not expose a Value property and cannot be used as a AutoReplication RPC argument."), *ArgumentVariable->GetName());
			return false;
		}

		void* SourceValuePtr = SourceProperty->ContainerPtrToValuePtr<void>(ArgumentVariable);
		void* DestValuePtr = ParameterProperty->ContainerPtrToValuePtr<void>(ParameterData);

		if (SourceProperty->SameType(ParameterProperty))
		{
			ParameterProperty->CopyCompleteValue(DestValuePtr, SourceValuePtr);
			return true;
		}

		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("AutoReplication RPC argument %s (property type %s) is incompatible with parameter %s (type %s)."),
			*ArgumentVariable->GetName(), *SourceProperty->GetClass()->GetName(), *ParameterProperty->GetName(), *ParameterProperty->GetClass()->GetName());
		return false;
	}

	namespace Snapshot
	{
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

	static bool InvokeNativeHandler(UObject* TargetObject, UGorgeousObjectVariable* InvocationContext, const FGorgeousQueuedRPC& QueuedRPC, UGorgeousObjectVariable** OutReturnVariable)
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
			return true;
		}

		TArray<uint8> ParameterStorage;
		ParameterStorage.SetNumZeroed(HandlerFunction->ParmsSize);
		HandlerFunction->InitializeStruct(ParameterStorage.GetData());

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
			const TObjectPtr<UGorgeousObjectVariable>* ArgumentVariablePtr = &QueuedRPC.Payload.Arguments.FindByPredicate(
				[&](const FGorgeousRPCArgumentContainer& ArgumentContainer)
				{
					return ArgumentContainer.ArgumentName == Property->GetFName();
				})->ArgumentValue;
			
			UGorgeousObjectVariable* ArgumentVariable = (ArgumentVariablePtr && ArgumentVariablePtr->Get()) ? ArgumentVariablePtr->Get() : nullptr;

			if (!ArgumentVariable)
			{
				UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("AutoReplication RPC %s expected parameter %s but it was not provided."),
					*QueuedRPC.Payload.HandlerName.ToString(), *Property->GetName());
				bAllParametersFilled = false;
				break;
			}

			if (!CopyArgumentToProperty(ArgumentVariable, Property, ParameterStorage.GetData()))
			{
				UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("Failed to apply argument %s to parameter %s for handler %s."),
					*ArgumentVariable->GetName(), *Property->GetName(), *QueuedRPC.Payload.HandlerName.ToString());
				bAllParametersFilled = false;
				break;
			}

			++ParameterIndex;
		}

		if (bAllParametersFilled)
		{
			TargetObject->ProcessEvent(HandlerFunction, ParameterStorage.GetData());
		}

		HandlerFunction->DestroyStruct(ParameterStorage.GetData());

		if (OutReturnVariable)
		{
			*OutReturnVariable = nullptr;
		}

		return bAllParametersFilled;
	}
}

UGorgeousObjectVariable::UGorgeousObjectVariable(): 
	ReplicationMode(EGorgeousObjectVariableReplicationMode::EFullAutoReplication),
	bUseSharedNetworkStack(false),
	VariableRegistry(TArray<TObjectPtr<UGorgeousObjectVariable>>() ),
	bPersistent(false),
	bSupportsNetworking(true),
	bReplicates(false),
	Parent(nullptr),
	AutoReplicationEntryKey(NAME_None),
	AutoReplicationReplicationIndex(INDEX_NONE),
	bLegacyReplicationRegistered(false),
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
		GT_E_LOG_MESSAGE("You are trying to register a object variable without a valid class, check if the class is valid!", "GT.ObjectVariables.Registration.Invalid_Class");
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
		GT_I_LOG_MESSAGE("No parent were specified, therefore the resolved root object variable will be used as the parent", "GT.ObjectVariables.No_Parent");
	}
	
	UGorgeousObjectVariable* NewObjectVariable = NewObject<UGorgeousObjectVariable>(InParent, Class);
	NewObjectVariable->AddToRoot();
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
	InParent->RegisterWithRegistry(NewObjectVariable);
	NewObjectVariable->SetDisplayName(DisplayNameOverride);
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
		GT_E_LOG_MESSAGE("Failed to create new transactional instance", "GT.ObjectVariables.Transactional.Failed");
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
		GT_I_LOG_MESSAGE("No parent were specified, therefore the resolved root object variable will be used as the parent", "GT.ObjectVariables.No_Parent");
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

		GT_S_LOG_MESSAGE("Created new transactional instance.", "GT.ObjectVariables.Transactional.Success");
		return NewInstance;
	}
	GT_E_LOG_MESSAGE("Failed to create new transactional instance", "GT.ObjectVariables.Transactional.Failed");
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
		UE_LOG(LogGorgeousObjectVariable, Verbose, TEXT("%s cannot disable the root network stack because the configured root enforces networking."), *GetName());
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

void UGorgeousObjectVariable::EnsureSharedNetworkStackOwner(UObject* FallbackOwner)
{
	if (!FallbackOwner)
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

	AutoReplicationOwner = FallbackOwner;
	SetCachedOwner(FallbackOwner);

	if (SupportsLegacyReplication() && bReplicates)
	{
		RegisterLegacyReplication(FallbackOwner);
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

void UGorgeousObjectVariable::RegisterWithRegistry(UGorgeousObjectVariable* NewObjectVariable)
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
		VariableRegistry.Add(NewObjectVariable);
		UGorgeousRootObjectVariable::TrackRegisteredVariable(NewObjectVariable);
	}
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
		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("Registration cancelled for %s because another unique instance already exists."), *Candidate->GetDisplayNameOrFallback());
		return false;
	}
}

void UGorgeousObjectVariable::ActivateReplication(const FGorgeousAutoReplicationContext& Context)
{
	if (!bSupportsNetworking)
	{
		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("%s cannot activate replication because networking is disabled."), *GetName());
		return;
	}

	if (bReplicationActivationGuard)
	{
		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("Replication activation already in progress for %s."), *GetName());
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
	ActiveReplicationContext = FGorgeousAutoReplicationContext();

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
	if (!EnsureReplicationActivation(TEXT("RegisterReplicatedProperty")))
	{
		return;
	}

	if (PropertyName.IsNone())
	{
		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("Attempted to register an unnamed property on %s."), *GetName());
		return;
	}

	FProperty* Property = FindFProperty<FProperty>(GetClass(), PropertyName);
	if (!Property)
	{
		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("Property %s does not exist on %s."), *PropertyName.ToString(), *GetName());
		return;
	}

	FReplicatedPropertyDeclaration& Declaration = RegisteredReplicatedProperties.AddDefaulted_GetRef();
	Declaration.PropertyName = PropertyName;
	Declaration.CachedProperty = Property;
	Declaration.Mode = Mode;
	Declaration.bSendInitialState = bSendInitialState;
	Declaration.ReplicationCondition = AdvancedConfig.ReplicationCondition;
	Declaration.RepNotifyFunction = AdvancedConfig.RepNotifyFunction;
	Declaration.RepNotifyPolicy = AdvancedConfig.RepNotifyPolicy;
	Declaration.bFireInitialNotify = AdvancedConfig.bFireInitialNotify;
	Declaration.bDeliveredInitialNotify = false;

	if (Declaration.HasRepNotify())
	{
		if (!FindFunction(Declaration.RepNotifyFunction))
		{
			UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("RepNotify target %s was not found on %s while registering property %s. Notify will be disabled."),
				*Declaration.RepNotifyFunction.ToString(), *GetName(), *PropertyName.ToString());
			Declaration.RepNotifyFunction = NAME_None;
		}
		else
		{
			Declaration.InitializeShadowState(Property, nullptr);
		}
	}

	UE_LOG(LogGorgeousObjectVariable, Verbose, TEXT("Registered replicated property %s on %s."), *PropertyName.ToString(), *GetName());
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
				UE_LOG(LogGorgeousObjectVariable, Verbose, TEXT("Custom AutoReplication payload for property %s on %s was skipped because no handler provided data."),
					*Declaration.PropertyName.ToString(), *GetName());
				continue;
			}
		}
		else if (!GorgeousObjectVariable_Private::SerializePropertyValue(Declaration.CachedProperty, PropertyData, Declaration.Mode, ConditionContext.PackageMap, SerializedValue.Payload))
		{
			UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("Failed to serialize auto-replicated property %s on %s."), *Declaration.PropertyName.ToString(), *GetName());
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

bool UGorgeousObjectVariable::ApplyAutoReplicationPropertyPayload(const FGorgeousAutoReplicationPropertyPayload& Payload, UPackageMap* PackageMap)
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
			UE_LOG(LogGorgeousObjectVariable, Verbose, TEXT("Received AutoReplication payload for unknown property %s on %s."), *SerializedProperty.PropertyName.ToString(), *GetName());
			continue;
		}

		if (Declaration->Mode == EGorgeousReplicationMode::ECustomPayload)
		{
			const bool bHandled = ApplyCustomAutoReplicationPayload(SerializedProperty.PropertyName, SerializedProperty.Payload, SerializedProperty.bIsInitialState != 0);
			if (!bHandled)
			{
				UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("Custom AutoReplication payload for property %s on %s was not applied."), *SerializedProperty.PropertyName.ToString(), *GetName());
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

		if (!GorgeousObjectVariable_Private::DeserializePropertyValue(Declaration->CachedProperty, PropertyData, Declaration->Mode, PackageMap, SerializedProperty.Payload))
		{
			UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("Failed to deserialize AutoReplication payload for property %s on %s."), *SerializedProperty.PropertyName.ToString(), *GetName());
			continue;
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

void UGorgeousObjectVariable::BindRPCHandler(const FName RPCName, const EGorgeousAutoReplicationRPCType Reliability)
{
	if (!EnsureReplicationActivation(TEXT("BindRPCHandler")))
	{
		return;
	}

	if (RPCName.IsNone())
	{
		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("Attempted to bind an unnamed RPC handler on %s."), *GetName());
		return;
	}

	FRPCBindingDeclaration& Binding = RegisteredRPCBindings.AddDefaulted_GetRef();
	Binding.RPCName = RPCName;
	Binding.Reliability = Reliability;

	UE_LOG(LogGorgeousObjectVariable, Verbose, TEXT("Registered AutoReplication RPC handler %s on %s."), *RPCName.ToString(), *GetName());
}

bool UGorgeousObjectVariable::RequestAutoReplicationRPC(const EGorgeousAutoReplicationRPCType Type, const FGorgeousRPCPayload& Payload, const FName OverrideKey, UObject* OverrideContext, const EGorgeousAutoReplicationTargetKind TargetKind)
{
	if (!SupportsAutoReplicationFeatures())
	{
		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("%s cannot queue AutoReplication RPC because it is configured for manual networking."), *GetName());
		return false;
	}

	FName ResolvedKey;
	UObject* ResolvedContext = nullptr;
	if (!ResolveAutoReplicationRPCContext(OverrideKey, OverrideContext, ResolvedKey, ResolvedContext))
	{
		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("%s cannot queue AutoReplication RPC because no owning context could be resolved."), *GetName());
		return false;
	}

	return UGorgeousCoreRuntimeGlobals::RequestAutoReplicationRPC(ResolvedContext, ResolvedKey, Type, Payload, TargetKind);
}

UGorgeousAutoReplicationRPCRequestAsyncAction* UGorgeousObjectVariable::RequestAutoReplicationRPCAsync(const EGorgeousAutoReplicationRPCType Type, const FGorgeousRPCPayload& Payload, const FName OverrideKey, UObject* OverrideContext, const EGorgeousAutoReplicationTargetKind TargetKind)
{
	if (!SupportsAutoReplicationFeatures())
	{
		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("%s cannot queue AutoReplication RPC async action because it is configured for manual networking."), *GetName());
		return nullptr;
	}

	FName ResolvedKey;
	UObject* ResolvedContext = nullptr;
	if (!ResolveAutoReplicationRPCContext(OverrideKey, OverrideContext, ResolvedKey, ResolvedContext))
	{
		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("%s cannot queue AutoReplication RPC async action because no owning context could be resolved."), *GetName());
		return nullptr;
	}

	return UGorgeousAutoReplicationRPCRequestAsyncAction::RequestAutoReplicationRPC(ResolvedContext, ResolvedKey, Type, Payload, TargetKind);
}

bool UGorgeousObjectVariable::ExecuteAutoReplicationRPC(const FGorgeousQueuedRPC& QueuedRPC, UGorgeousObjectVariable** OutReturnVariable)
{
	if (!SupportsAutoReplicationFeatures())
	{
		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("%s is configured for manual networking and cannot execute AutoReplication RPCs."), *GetName());
		return false;
	}

	if (QueuedRPC.Payload.HandlerName.IsNone())
	{
		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("%s received AutoReplication RPC for key %s without a handler name."), *GetName(), *QueuedRPC.Key.ToString());
		return false;
	}

	const FRPCBindingDeclaration* Binding = RegisteredRPCBindings.FindByPredicate([&QueuedRPC](const FRPCBindingDeclaration& Declaration)
	{
		return Declaration.RPCName == QueuedRPC.Payload.HandlerName;
	});

	if (!Binding)
	{
		UE_LOG(LogGorgeousObjectVariable, Verbose, TEXT("%s executing AutoReplication RPC %s for key %s without an explicit binding."), *GetName(), *QueuedRPC.Payload.HandlerName.ToString(), *QueuedRPC.Key.ToString());
	}
	else if (Binding->Reliability != QueuedRPC.Type)
	{
		const UEnum* EnumClass = StaticEnum<EGorgeousAutoReplicationRPCType>();
		const FString Expected = EnumClass ? EnumClass->GetNameStringByValue(static_cast<int64>(Binding->Reliability)) : TEXT("<unknown>");
		const FString Received = EnumClass ? EnumClass->GetNameStringByValue(static_cast<int64>(QueuedRPC.Type)) : TEXT("<unknown>");
		UE_LOG(LogGorgeousObjectVariable, Verbose, TEXT("AutoReplication RPC %s on %s expected %s but received %s. Continuing execution."), *QueuedRPC.Payload.HandlerName.ToString(), *GetName(), *Expected, *Received);
	}

	const bool bExecutedNativeHandler = InvokeNativeAutoReplicationRPCHandler(QueuedRPC, nullptr);
	if (!bExecutedNativeHandler)
	{
		UE_LOG(LogGorgeousObjectVariable, Verbose, TEXT("AutoReplication RPC %s on %s did not match a native function. Blueprint event will be fired instead."),
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
		*OutReturnVariable = ResultContainer;
	}

	FGorgeousAutoReplicationRPCResult Result;
	Result.QueuedRPC = QueuedRPC;
	Result.TargetKind = EGorgeousAutoReplicationTargetKind::EObjectVariable;
	Result.TargetVariable = ResultContainer ? static_cast<UGorgeousObjectVariable*>(ResultContainer) : this;
	Result.TargetOwner = nullptr;
	Result.TargetVariableIdentifier = ResultContainer ? ResultContainer->GetUniqueIdentifierForObjectVariable() : GetUniqueIdentifierForObjectVariable();
	if (ResultContainer)
	{
		ResultContainer->CaptureResult(Result);
	}
	UGorgeousAutoReplicationRPCRequestAsyncAction::NotifyRequestCompleted(Result);
	if (UWorld* TargetWorld = GetWorld())
	{
		FGorgeousAutoReplicationCoordinator::Get(TargetWorld).NotifyRPCBroadcast(QueuedRPC, this);
	}
	return true;
}

bool UGorgeousObjectVariable::InvokeNativeAutoReplicationRPCHandler(const FGorgeousQueuedRPC& QueuedRPC, UGorgeousObjectVariable** OutReturnVariable)
{
	const bool bHandled = GorgeousObjectVariable_Private::InvokeNativeHandler(this, this, QueuedRPC, nullptr);
	if (OutReturnVariable)
	{
		*OutReturnVariable = nullptr;
	}
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

bool UGorgeousObjectVariable::CanControllerAccessVariable_Implementation(AGorgeousPlayerController* Controller) const
{
	return Controller && Controller == ResolveOwningPlayerController();
}

EGorgeousObjectVariableAccessPolicy UGorgeousObjectVariable::ResolveRespectAccessPolicy_Implementation(UGorgeousObjectVariable* Variable) const
{
	if (!Variable)
	{
		return EGorgeousObjectVariableAccessPolicy::Everyone;
	}

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

bool UGorgeousObjectVariable::EvaluateAccessPolicyForController(AController* Controller) const
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
		return Controller && Controller == ResolveOwningPlayerController();
	case EGorgeousObjectVariableAccessPolicy::Custom:
	default:
		return CanControllerAccessVariable(Cast<AGorgeousPlayerController>(Controller));
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

bool UGorgeousObjectVariable::InvokeNativeAutoReplicationRPCHandlerOnObject(UObject* Target, const FGorgeousQueuedRPC& QueuedRPC, UGorgeousObjectVariable** OutReturnVariable)
{
	UGorgeousRPC_OV* ResultContainer = nullptr;
	const bool bHandled = GorgeousObjectVariable_Private::InvokeNativeHandler(Target, nullptr, QueuedRPC, nullptr);
	if (bHandled)
	{
		ResultContainer = CreateStandaloneRPCResultContainer(Target);
	}
	if (OutReturnVariable)
	{
		*OutReturnVariable = ResultContainer;
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
	UGorgeousObjectVariable* NewParent = Root->NewObjectVariable(UGorgeousObjectVariable::StaticClass(), Identifier, Root, true, GorgeousObjectVariable_Private::GorgeousRPCResultsDisplayName.ToString());
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
		UE_LOG(LogGorgeousObjectVariable, Verbose, TEXT("%s created an RPC result container without an AutoReplication owner. Networking may be skipped."), *GetName());
	}
	ResultContainer->SetNetworkingEnabled(true);
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
	FGorgeousAutoReplicationContext Context;
	Context.OwningObject = OwningContext;
	Context.EntryKey = NAME_None;
	Context.ReplicationIndex = INDEX_NONE;
	if (Context.OwningObject)
	{
		ResultContainer->ActivateReplication(Context);
	}
	ResultContainer->SetNetworkingEnabled(true);
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

	FGuid Identifier = Variable->GetUniqueIdentifierForObjectVariable();
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
			GorgeousObjectVariable_Private::SerializePropertyValue(Property, PropertyData, EGorgeousReplicationMode::EProperty, nullptr, Payload);
		}
		GorgeousObjectVariable_Private::Snapshot::WriteByteArray(Ar, Payload);
	}

	int32 ChildCount = Variable->VariableRegistry.Num();
	Ar << ChildCount;
	for (UGorgeousObjectVariable* Child : Variable->VariableRegistry)
	{
		if (!SerializeRPCSnapshotRecursive(Child, Ar))
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
		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("RPC result snapshot version mismatch (expected %u, received %u)."),
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
				GorgeousObjectVariable_Private::DeserializePropertyValue(Property, PropertyData, EGorgeousReplicationMode::EProperty, nullptr, SerializedProperty.Payload);
			}
		}
	}

	for (int32 ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
	{
		DeserializeRPCSnapshotRecursive(NewInstance, Ar);
	}

	return NewInstance;
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
	Descriptor.ResultIdentifier = ResultContainer->GetUniqueIdentifierForObjectVariable();
	if (!Descriptor.ResultIdentifier.IsValid())
	{
		return;
	}
	Descriptor.HandlerName = QueuedRPC.Payload.HandlerName;
	Descriptor.ResultClass = ResultContainer->GetClass();
	Descriptor.bReplicateToAllConnections = QueuedRPC.Payload.bReplicateResultToAllConnections;
	if (!BuildRPCResultSnapshot(ResultContainer, Descriptor.SnapshotPayload))
	{
		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("Failed to capture RPC result snapshot for %s."), *ResultContainer->GetName());
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
				UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("Failed to reconstruct RPC result for request %s."), *Descriptor.RequestGuid.ToString());
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
}

void UGorgeousObjectVariable::HandleAutoReplicationRPCPayload_Implementation(const FGorgeousQueuedRPC& QueuedRPC)
{
	UE_LOG(LogGorgeousObjectVariable, Verbose, TEXT("%s received AutoReplication RPC %s for key %s (%d named arguments) but no Blueprint override is provided."),
		*GetName(), *QueuedRPC.Payload.HandlerName.ToString(), *QueuedRPC.Key.ToString(), QueuedRPC.Payload.Arguments.Num());
}

void UGorgeousObjectVariable::ExampleAutoReplicationRPCHandler(double ExampleValue, const FString& ExampleLabel)
{
	UE_LOG(LogGorgeousObjectVariable, Log, TEXT("Example AutoReplication RPC handler invoked on %s | Label: %s | Value: %.2f"), *GetName(), *ExampleLabel, ExampleValue);
}

bool UGorgeousObjectVariable::EnsureReplicationActivation(const TCHAR* SourceFunction) const
{
	if (!bReplicationActivationGuard)
	{
		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("%s can only be invoked inside OnReplicationActivated for %s."), SourceFunction, *GetName());
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
			UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("RepNotify %s on %s expects a parameter incompatible with property %s. Continuing without passing the old value."),
				*Declaration.RepNotifyFunction.ToString(), *GetName(), *Declaration.PropertyName.ToString());
		}
	}

	ProcessEvent(RepFunction, ParameterStorage.GetData());
	RepFunction->DestroyStruct(ParameterStorage.GetData());
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

	const bool bWasReplicating = bReplicates;
	SetIsReplicated(bWantsReplication);

	if (bWasReplicating && bReplicates)
	{
		UpdateAutoReplicationState(true);
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
		UE_LOG(LogGorgeousObjectVariable, Warning, TEXT("%s cannot enable replication%s because networking support is disabled."),
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

