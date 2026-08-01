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

#include "InsightMatrix/GorgeousInsightBaselineSettings.h"

FGorgeousInsightBaselineEntry FGorgeousInsightBaselineEntry::FromResult(const FGorgeousInsightTestResult& Result)
{
	FGorgeousInsightBaselineEntry Entry;
	Entry.bSuccess = Result.bSuccess;
	Entry.Errors = Result.Errors;
	Entry.Warnings = Result.Warnings;
	Entry.Notes = Result.Notes;
	Entry.Metrics = Result.Metrics;
	return Entry;
}

FGorgeousInsightTestResult FGorgeousInsightBaselineEntry::ToResult() const
{
	FGorgeousInsightTestResult Result;
	Result.bSuccess = bSuccess;
	Result.Errors = Errors;
	Result.Warnings = Warnings;
	Result.Notes = Notes;
	Result.Metrics = Metrics;
	return Result;
}
