// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Profiling Helpers (Runtime)                |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "HAL/PreprocessorHelpers.h"

#if __has_include("ProfilingDebugging/CpuProfilerTrace.h")
#include "ProfilingDebugging/CpuProfilerTrace.h"
#endif

#if __has_include("ProfilingDebugging/CsvProfiler.h")
#include "ProfilingDebugging/CsvProfiler.h"
#endif

#if __has_include("Trace/Trace.h")
#include "Trace/Trace.h"
#endif

#ifndef TRACE_CPUPROFILER_EVENT_SCOPE
#define TRACE_CPUPROFILER_EVENT_SCOPE(Name)
#endif

#ifndef TRACE_BOOKMARK
#define TRACE_BOOKMARK(...)
#endif

#ifndef CSV_SCOPED_TIMING_STAT
#define CSV_SCOPED_TIMING_STAT(Category, StatName)
#endif

#ifndef CSV_CUSTOM_STAT
#define CSV_CUSTOM_STAT(Category, StatName, Value, Op)
#endif

#if defined(CSV_PROFILER) && CSV_PROFILER
CSV_DECLARE_CATEGORY_MODULE_EXTERN(GORGEOUSCORERUNTIMEUTILITIES_API, Gorgeous);

#define GORGEOUS_PROFILE_SCOPE(StatName) \
	TRACE_CPUPROFILER_EVENT_SCOPE(StatName); \
	TRACE_CSV_PROFILER_INLINE_STAT(#StatName, CSV_CATEGORY_INDEX(Gorgeous)); \
	FScopedCsvStat PREPROCESSOR_JOIN(_GorgeousCsvStat_, __LINE__)(#StatName, CSV_CATEGORY_INDEX(Gorgeous), "CSV_" #StatName)

#define GORGEOUS_CSV_CUSTOM_STAT_SET(StatName, Value) \
	TRACE_CSV_PROFILER_INLINE_STAT(#StatName, CSV_CATEGORY_INDEX(Gorgeous)); \
	FCsvProfiler::RecordCustomStat(#StatName, CSV_CATEGORY_INDEX(Gorgeous), Value, ECsvCustomStatOp::Set)

#define GORGEOUS_CSV_CUSTOM_STAT_ACCUMULATE(StatName, Value) \
	TRACE_CSV_PROFILER_INLINE_STAT(#StatName, CSV_CATEGORY_INDEX(Gorgeous)); \
	FCsvProfiler::RecordCustomStat(#StatName, CSV_CATEGORY_INDEX(Gorgeous), Value, ECsvCustomStatOp::Accumulate)
#else
#define GORGEOUS_PROFILE_SCOPE(StatName) \
	TRACE_CPUPROFILER_EVENT_SCOPE(StatName)

#define GORGEOUS_CSV_CUSTOM_STAT_SET(StatName, Value)
#define GORGEOUS_CSV_CUSTOM_STAT_ACCUMULATE(StatName, Value)
#endif

#define GORGEOUS_TRACE_BOOKMARK(Format, ...) \
	TRACE_BOOKMARK(Format, ##__VA_ARGS__)
