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
#include "Components/ActorComponent.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")

//<=============================--- Includes ---=============================>
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GeneralSystem_AC.generated.h"

class UGeneralSystemConfiguration_PDA;
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: General System
| Functional Name: UGeneralSystem_AC
| Parent Class: UActorComponent
| Class Suffix: _AC
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Provides runtime functionality for General System.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType, Blueprintable,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Anatomy/#ugeneralsystem_ac",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/"
		)
)
class GORGEOUSCORERUNTIME_API UGeneralSystem_AC : public UActorComponent
{
    GENERATED_BODY()
public:
    /** The system asset that defines the project settings schema for this component. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General Core System|Project Settings")
    TSoftObjectPtr<UGeneralSystemConfiguration_PDA> ProjectSettingsConfiguration;

    /** Resolves project overrides over the asset defaults. Use Get Instanced Struct Value to read the typed struct. */
    UFUNCTION(BlueprintCallable, Category = "General Core System|Project Settings")
    FInstancedStruct GetSystemProjectSettings() const;

protected:
    virtual void OnRegister() override;
};
