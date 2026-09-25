#pragma once

#include <format>
#include <string_view>
#include <utility>

namespace dilithium {

enum class LogLevel { Debug, Info, Warning, Error };

/// Writes one line to stderr, prefixed `debug:`, `info:`, `warning:` or `error:`. Keeps no state, and each line goes
/// out in one write, so lines from different threads never interleave.
void writeLog(LogLevel level, std::string_view message);

namespace detail {

/// Behind `DILITHIUM_LOG_DEBUG`. Call the macro, not this.
template <typename... Args>
void logDebug(std::format_string<Args...> format, Args&&... args) {
    writeLog(LogLevel::Debug, std::format(format, std::forward<Args>(args)...));
}

} // namespace detail

template <typename... Args>
void logInfo(std::format_string<Args...> format, Args&&... args) {
    writeLog(LogLevel::Info, std::format(format, std::forward<Args>(args)...));
}

template <typename... Args>
void logWarning(std::format_string<Args...> format, Args&&... args) {
    writeLog(LogLevel::Warning, std::format(format, std::forward<Args>(args)...));
}

template <typename... Args>
void logError(std::format_string<Args...> format, Args&&... args) {
    writeLog(LogLevel::Error, std::format(format, std::forward<Args>(args)...));
}

} // namespace dilithium

/// A log line for debug builds only (Mach 5's `M5DEBUG_PRINT`), printed as `debug: …`. Release still compiles the
/// call, so the format string is checked and anything the arguments name counts as used, but never runs it: an
/// expensive argument costs nothing outside debug.
#if defined(DILITHIUM_DEBUG)
#define DILITHIUM_LOG_DEBUG(...) ::dilithium::detail::logDebug(__VA_ARGS__)
#else
#define DILITHIUM_LOG_DEBUG(...) static_cast<void>(false && (::dilithium::detail::logDebug(__VA_ARGS__), true))
#endif
