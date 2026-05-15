#pragma once
#include <Windows.h>
#include <cstdio>

enum class LogLevel : int {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
};

// LOG_MIN_LEVEL is supplied by build_config as a compile def. Default to
// Trace if the host has somehow forgotten to define it (defensive — the
// root CMakeLists.txt always sets it).
#ifndef LOG_MIN_LEVEL
#define LOG_MIN_LEVEL 0
#endif

extern void LogImpl(LogLevel level, const char* fmt, ...);

// Legacy free-function entry point. Writes the formatted message unprefixed
// to OutputDebugStringA — bypasses both the level tag and LOG_MIN_LEVEL so
// existing callsites continue to fire in every build. New code must use the
// LOG_* macros below.
extern void Log(const char* format, ...);

#define LOG_GMS_DLL_IMPL_(lvl, ...) ::LogImpl((lvl), __VA_ARGS__)

#if LOG_MIN_LEVEL <= 0
#define LOG_TRACE(...) LOG_GMS_DLL_IMPL_(LogLevel::Trace, __VA_ARGS__)
#else
#define LOG_TRACE(...) ((void)0)
#endif

#if LOG_MIN_LEVEL <= 1
#define LOG_DEBUG(...) LOG_GMS_DLL_IMPL_(LogLevel::Debug, __VA_ARGS__)
#else
#define LOG_DEBUG(...) ((void)0)
#endif

#if LOG_MIN_LEVEL <= 2
#define LOG_INFO(...) LOG_GMS_DLL_IMPL_(LogLevel::Info, __VA_ARGS__)
#else
#define LOG_INFO(...) ((void)0)
#endif

#if LOG_MIN_LEVEL <= 3
#define LOG_WARN(...) LOG_GMS_DLL_IMPL_(LogLevel::Warn, __VA_ARGS__)
#else
#define LOG_WARN(...) ((void)0)
#endif

#if LOG_MIN_LEVEL <= 4
#define LOG_ERROR(...) LOG_GMS_DLL_IMPL_(LogLevel::Error, __VA_ARGS__)
#else
#define LOG_ERROR(...) ((void)0)
#endif
