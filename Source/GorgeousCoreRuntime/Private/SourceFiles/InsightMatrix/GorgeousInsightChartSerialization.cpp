// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "InsightMatrix/GorgeousInsightChartSerialization.h"
#include "JsonObjectConverter.h"

TSharedRef<FJsonObject> GorgeousInsightCharts::ToJson(const FGorgeousInsightChartDefinition& Chart)
{
	TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	FJsonObjectConverter::UStructToJsonObject(FGorgeousInsightChartDefinition::StaticStruct(), &Chart, Json, 0, CPF_Transient);
	if (Chart.InstancedPayload.IsValid())
	{
		Json->SetStringField(TEXT("instancedPayloadType"), Chart.InstancedPayload.GetScriptStruct()->GetPathName());
		TSharedRef<FJsonObject> PayloadJson = MakeShared<FJsonObject>();
		FJsonObjectConverter::UStructToJsonObject(Chart.InstancedPayload.GetScriptStruct(), Chart.InstancedPayload.GetMemory(), PayloadJson, 0, CPF_Transient);
		Json->SetObjectField(TEXT("instancedPayloadData"), PayloadJson);
	}
	return Json;
}

bool GorgeousInsightCharts::FromJson(const TSharedRef<FJsonObject>& Json, FGorgeousInsightChartDefinition& Chart)
{
	if (!FJsonObjectConverter::JsonObjectToUStruct(Json, FGorgeousInsightChartDefinition::StaticStruct(), &Chart, 0, CPF_Transient)) return false;
	FString TypePath;
	const TSharedPtr<FJsonObject>* PayloadJson = nullptr;
	if (Json->TryGetStringField(TEXT("instancedPayloadType"), TypePath) && Json->TryGetObjectField(TEXT("instancedPayloadData"), PayloadJson))
	{
		// A saved chart never loads packages or arbitrary classes. Its owning plugin registers the payload type.
		UScriptStruct* Type = FindObject<UScriptStruct>(nullptr, *TypePath);
		if (!Type || !PayloadJson || !PayloadJson->IsValid()) return false;
		Chart.InstancedPayload.InitializeAs(Type);
		if (!FJsonObjectConverter::JsonObjectToUStruct(PayloadJson->ToSharedRef(), Type, Chart.InstancedPayload.GetMutableMemory(), 0, CPF_Transient)) return false;
	}
	return true;
}

TArray<TSharedPtr<FJsonValue>> GorgeousInsightCharts::ToJsonArray(const TArray<FGorgeousInsightChartDefinition>& Charts)
{
	TArray<TSharedPtr<FJsonValue>> Result;
	for (const FGorgeousInsightChartDefinition& Chart : Charts) Result.Add(MakeShared<FJsonValueObject>(ToJson(Chart)));
	return Result;
}

TArray<FGorgeousInsightChartDefinition> GorgeousInsightCharts::FromJsonArray(const TArray<TSharedPtr<FJsonValue>>& Json)
{
	TArray<FGorgeousInsightChartDefinition> Result;
	for (const TSharedPtr<FJsonValue>& Value : Json)
	{
		if (!Value.IsValid() || Value->Type != EJson::Object) continue;
		FGorgeousInsightChartDefinition Chart;
		if (FromJson(Value->AsObject().ToSharedRef(), Chart)) Result.Add(MoveTemp(Chart));
	}
	return Result;
}
