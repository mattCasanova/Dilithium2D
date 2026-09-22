#pragma once

#include <format>
#include <string_view>
#include <utility>

namespace dilithium {

enum class LogLevel { Debug, Info, Warning, Error };

/// Writes one line to stderr, prefixed `Captain's log:`, `Fascinating:`, `Yellow alert:` or `Red alert:`. Keeps no
/// state, and each line goes out in one write, so lines from different threads never interleave.
void writeLog(LogLevel level, std::string_view message);

/// Debug builds only (Mach 5's `M5DEBUG_PRINT`): release builds skip the formatting and the write. The arguments
/// are still evaluated, so keep them cheap.
template <typename... Args>
void logDebug([[maybe_unused]] std::format_string<Args...> format, [[maybe_unused]] Args&&... args) {
#if defined(DILITHIUM_DEBUG)
    writeLog(LogLevel::Debug, std::format(format, std::forward<Args>(args)...));
#endif
}

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
