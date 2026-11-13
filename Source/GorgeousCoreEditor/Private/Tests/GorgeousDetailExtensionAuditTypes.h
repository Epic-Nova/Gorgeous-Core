// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once
#include "CoreMinimal.h"
#include "DetailExtensions/GorgeousDetailExtension.h"
#include "UObject/UnrealType.h"
#include "GorgeousDetailExtensionAuditTypes.generated.h"

class USkeletalMesh;

USTRUCT()
struct FGorgeousDetailAuditAttachmentKey
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Attachment")
    TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
    UPROPERTY(EditAnywhere, Category="Attachment")
    FName SocketName = NAME_None;
    bool operator==(const FGorgeousDetailAuditAttachmentKey& Other) const
    { return SkeletalMesh == Other.SkeletalMesh && SocketName == Other.SocketName; }
    friend uint32 GetTypeHash(const FGorgeousDetailAuditAttachmentKey& Key)
    { return HashCombine(GetTypeHash(Key.SkeletalMesh.ToSoftObjectPath()), GetTypeHash(Key.SocketName)); }
};

UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class UGorgeousDetailAuditAdapterBase : public UObject
{
    GENERATED_BODY()
};

/** Same nesting/property shape as Inventory's polymorphic WorldInteraction adapter. */
UCLASS(EditInlineNew, DefaultToInstanced)
class UGorgeousDetailAuditWorldAdapter : public UGorgeousDetailAuditAdapterBase
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="World Interaction", meta=(ShowOnlyInnerProperties="true", GorgeousDetailExtension="GorgeousAuditNestedMap"))
    TMap<FGorgeousDetailAuditAttachmentKey, FVector> EquipOffsetsByAttachmentPoint;
};

USTRUCT()
struct FGorgeousDetailAuditNestedOwner
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Instanced, Category="Nested")
    TObjectPtr<UGorgeousDetailAuditAdapterBase> Adapter;
};

UCLASS()
class UGorgeousDetailAuditOwner : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Instanced, Category="Adapters")
    TArray<TObjectPtr<UGorgeousDetailAuditAdapterBase>> Adapters;
    UPROPERTY(EditAnywhere, Category="Nested")
    FGorgeousDetailAuditNestedOwner Nested;
};

UCLASS()
class UGorgeousDetailAuditExtension : public UGorgeousDetailExtension
{
    GENERATED_BODY()
public:
    int32 HeaderCalls = 0;
    int32 ChildrenCalls = 0;
    bool bEveryHandleIsMap = true;
    TSet<TWeakObjectPtr<UObject>> ChildOwners;
    virtual FName GetExtensionName() const override { return TEXT("GorgeousAuditNestedMap"); }
    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> Handle, FDetailWidgetRow& Header) override;
    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> Handle, IDetailChildrenBuilder& Children) override;
    void ResetObservations() { HeaderCalls = 0; ChildrenCalls = 0; bEveryHandleIsMap = true; ChildOwners.Reset(); }
};
