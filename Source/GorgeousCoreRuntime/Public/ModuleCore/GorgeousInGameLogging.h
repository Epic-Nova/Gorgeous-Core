// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once
#include "Blueprint/UserWidget.h"
#include "UObject/Interface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ModuleCore/GorgeousCoreRuntimeUtilitiesEnums.h"
#include "ModuleCore/GorgeousCoreRuntimeUtilitiesStructures.h"
#include "GorgeousInGameLogging.generated.h"

UINTERFACE(BlueprintType)
class GORGEOUSCORERUNTIME_API UGorgeousLogWidgetReceiver : public UInterface
{
    GENERATED_BODY()
};
class GORGEOUSCORERUNTIME_API IGorgeousLogWidgetReceiver
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Logging")
    void ReceiveGorgeousLogMessage(const FString& Message, FName LoggingKey, EGorgeousLoggingImportance Importance);
};

/** Default bounded in-game log panel; Blueprint widgets may implement the same receiver interface. */
UCLASS(Blueprintable)
class GORGEOUSCORERUNTIME_API UGorgeousInGameLogWidget : public UUserWidget, public IGorgeousLogWidgetReceiver
{
    GENERATED_BODY()
public:
    virtual void ReceiveGorgeousLogMessage_Implementation(const FString& Message, FName LoggingKey, EGorgeousLoggingImportance Importance) override;
    UPROPERTY(BlueprintReadOnly, Transient, Category="Gorgeous Core|Logging")
    TArray<FString> Messages;
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
    TSharedPtr<class SVerticalBox> MessageBox;
    void RefreshMessages();
};

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousInGameLogSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
private:
    void ReceiveLogEntry(const FGorgeousLogEntry& Entry);
    FDelegateHandle LogHandle;
    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> LogWidget;
};
