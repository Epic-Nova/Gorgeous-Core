// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Insight Matrix (Runtime)                   |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#include "InsightMatrix/GorgeousInsightFunctionalTest.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"

AGorgeousInsightFunctionalTest::AGorgeousInsightFunctionalTest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BeaconMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeaconMesh"));
	if (BeaconMesh)
	{
		BeaconMesh->SetupAttachment(RootComponent);
		BeaconMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	}

	ResultWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ResultWidget"));
	if (ResultWidget)
	{
		ResultWidget->SetupAttachment(BeaconMesh);
		ResultWidget->SetWidgetSpace(EWidgetSpace::Screen);
		ResultWidget->SetDrawAtDesiredSize(true);
		ResultWidget->SetVisibility(false);
	}
}

void AGorgeousInsightFunctionalTest::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateBeaconColor(FLinearColor::Gray);
}

void AGorgeousInsightFunctionalTest::StartTest()
{
	UpdateBeaconColor(FLinearColor::Yellow);
	Super::StartTest();
}

void AGorgeousInsightFunctionalTest::FinishTest(EFunctionalTestResult TestResult, const FString& Message)
{
	Super::FinishTest(TestResult, Message);

	ReportToInsightMatrix(TestResult, Message);

	if (TestResult == EFunctionalTestResult::Succeeded)
	{
		UpdateBeaconColor(FLinearColor::Green);
	}
	else
	{
		UpdateBeaconColor(FLinearColor::Red);
		if (ResultWidget)
		{
			ResultWidget->SetVisibility(true);
		}
	}
}

void AGorgeousInsightFunctionalTest::ReportToInsightMatrix(EFunctionalTestResult TestResult, const FString& Message)
{
	LastResult.bSuccess = (TestResult == EFunctionalTestResult::Succeeded);
	LastResult.Errors.Empty();
	LastResult.Notes.Empty();

	if (LastResult.bSuccess)
	{
		LastResult.AddNote(Message);
	}
	else
	{
		LastResult.AddError(Message);
	}
}

void AGorgeousInsightFunctionalTest::UpdateBeaconColor(FLinearColor NewColor)
{
	if (BeaconMesh && BeaconMesh->GetStaticMesh())
	{
		UMaterialInstanceDynamic* DynMaterial = BeaconMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (DynMaterial)
		{
			DynMaterial->SetVectorParameterValue(FName(TEXT("BaseColor")), NewColor);
			DynMaterial->SetVectorParameterValue(FName(TEXT("EmissiveColor")), NewColor * 5.0f);
		}
	}
}
