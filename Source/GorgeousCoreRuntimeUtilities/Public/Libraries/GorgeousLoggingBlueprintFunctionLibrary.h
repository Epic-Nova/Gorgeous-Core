/*==========================================================================>
|				Gorgeous Core - Core functionality provider					|
| ------------------------------------------------------------------------- |
|		  Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,			|
|			  administrated by Epic Nova. All rights reserved.				|
| ------------------------------------------------------------------------- |
|					Epic Nova is an independent entity,						|
|	    that is has nothing in common with Epic Games in any capacity.		|
<==========================================================================*/
#pragma once

//#include "GorgeousCoreRuntimeUtilitiesMinimal.h" CURSED
#include "GameplayTagContainer.h"
#include "GorgeousCoreRuntimeUtilitiesEnums.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GorgeousLoggingBlueprintFunctionLibrary.generated.h"


UCLASS()
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousLoggingBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging", meta = (WorldContext = "WorldContextObject"))
	static void LogMessage(UPARAM(ref) const FString& Message, EGorgeousLoggingImportancy Importancy, FString LoggingKey, const float Duration = 5.0f, const bool bPrintToScreen = true, const bool bPrintToLog = true, bool bOverrideLoggingIfPresent = true, UObject* WorldContextObject = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging", meta = (WorldContext = "WorldContextObject"))
	static void LogInformationMessage(const FString Message, FString LoggingKey, const float Duration = 5.0f, const bool bPrintToScreen = true, const bool bPrintToLog = true, UObject* WorldContextObject = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging", meta = (WorldContext = "WorldContextObject"))
	static void LogSuccessMessage(const FString Message, FString LoggingKey, const float Duration = 5.0f, const bool bPrintToScreen = true, const bool bPrintToLog = true, UObject* WorldContextObject = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging", meta = (WorldContext = "WorldContextObject"))
	static void LogWarningMessage(const FString Message, FString LoggingKey, const float Duration = 2.0f, UObject* WorldContextObject = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging", meta = (WorldContext = "WorldContextObject"))
	static void LogErrorMessage(const FString Message, FString LoggingKey, const float Duration = 2.0f, UObject* WorldContextObject = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Logging", meta = (WorldContext = "WorldContextObject"))
	static void LogFatalMessage(const FString Message, FString LoggingKey, UObject* WorldContextObject = nullptr);
};

