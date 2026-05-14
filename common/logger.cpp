#include "logger.h"

#include <cstdarg>
#include <cstdio>

namespace {

const char* LevelTag(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "[TRACE]";
        case LogLevel::Debug: return "[DEBUG]";
        case LogLevel::Info:  return "[INFO] ";
        case LogLevel::Warn:  return "[WARN] ";
        case LogLevel::Error: return "[ERROR]";
    }
    return "[?????]";
}

} // namespace

void LogImpl(LogLevel level, const char* fmt, ...) {
    constexpr size_t kBufSz = 1024;
    static_assert(kBufSz >= 4, "buffer must hold the '...' truncation marker");

    char buf[kBufSz];
    const char* tag = LevelTag(level);
    int prefix = std::snprintf(buf, kBufSz, "%s ", tag);
    if (prefix < 0 || static_cast<size_t>(prefix) >= kBufSz) {
        // Prefix alone overflowed — should be impossible at kBufSz = 1024.
        return;
    }

    va_list args;
    va_start(args, fmt);
    int written = std::vsnprintf(buf + prefix, kBufSz - prefix, fmt, args);
    va_end(args);

    if (written >= 0 && static_cast<size_t>(prefix + written) >= kBufSz) {
        buf[kBufSz - 4] = '.';
        buf[kBufSz - 3] = '.';
        buf[kBufSz - 2] = '.';
        buf[kBufSz - 1] = '\0';
    }

    OutputDebugStringA(buf);
}

void Log(const char* format, ...) {
    constexpr size_t kBufSz = 1024;
    char buf[kBufSz];

    va_list args;
    va_start(args, format);
    int written = std::vsnprintf(buf, kBufSz, format, args);
    va_end(args);

    if (written >= 0 && static_cast<size_t>(written) >= kBufSz) {
        buf[kBufSz - 4] = '.';
        buf[kBufSz - 3] = '.';
        buf[kBufSz - 2] = '.';
        buf[kBufSz - 1] = '\0';
    }

    // Two-step prefix prepend would require a second buffer or shifting;
    // simplest is to forward unformatted text to OutputDebugString.
    // To preserve "Log() always fires" semantics regardless of LOG_MIN_LEVEL,
    // call OutputDebugStringA directly here. The tagged variant is
    // accessible via LOG_INFO when callers migrate.
    OutputDebugStringA(buf);
}
