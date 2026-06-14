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

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "Commandlets/Commandlet.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousDataSchemaMappingBatchCommandlet.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Runs schema migrations in batch mode from command line.
 *
 * Example:
 * UnrealEditor-Cmd.exe <Project> -run=GorgeousDataSchemaMappingBatch
 *     -Schema=/Game/Path/DA_Schema.DA_Schema
 *     -Sources=/Game/Data/ItemA.ItemA,/Game/Data/ItemB.ItemB
 *     -DryRun
 */
UCLASS()
class GORGEOUSCOREEDITOR_API UGorgeousDataSchemaMappingBatchCommandlet final : public UCommandlet
{
	GENERATED_BODY()

public:
	UGorgeousDataSchemaMappingBatchCommandlet();
	virtual int32 Main(const FString& Params) override;

private:
	static void ParseListArgument(const FString& RawList, TArray<FString>& OutValues);
};
