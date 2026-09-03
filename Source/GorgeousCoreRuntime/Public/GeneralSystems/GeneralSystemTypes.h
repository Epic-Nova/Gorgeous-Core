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
#pragma once

//<=============================--- Includes ---============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GeneralSystemTypes.generated.h"
//<-------------------------------------------------------------------------->


USTRUCT(BlueprintType)
struct FGorgeousDamageContext_S
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<AActor> DamagedActor;

    UPROPERTY(BlueprintReadWrite)
    float BaseDamage;

    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<AController> EventInstigator;

    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<AActor> DamageCauser;

    UPROPERTY(BlueprintReadWrite)
    TSubclassOf<UDamageType> DamageTypeClass;

    UPROPERTY(BlueprintReadWrite)
    FVector HitLocation;

    UPROPERTY(BlueprintReadWrite)
    FVector ShotDirection;

    UPROPERTY(BlueprintReadWrite)
    FGameplayTagContainer DamageTags;

    UPROPERTY(BlueprintReadWrite)
    TMap<FName, float> NumericParameters;

    UPROPERTY(BlueprintReadWrite)
    TMap<FName, TObjectPtr<UObject>> ObjectParameters;
};

UINTERFACE(BlueprintType)
class UGorgeousDamageTypeHandler_I : public UInterface
{
    GENERATED_BODY()
};

class IGorgeousDamageTypeHandler_I
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|General Systems")
    void HandleDamage(
        UPARAM(ref) FGorgeousDamageContext_S& DamageContext
    );
};
