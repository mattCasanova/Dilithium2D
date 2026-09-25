#include "core/LogFormat.hpp"

#include <dilithium/core/Assert.hpp>

#include <cstdio>

namespace dilithium {

std::string_view logPrefix(LogLevel level) {
    switch (level) {
    case LogLevel::Debug:
        return "Captain's log:";
    case LogLevel::Info:
        return "Fascinating:";
    case LogLevel::Warning:
        return "Yellow alert:";
    case LogLevel::Error:
        return "Red alert:";
    }
    ILLOGICAL("unknown LogLevel");
}

std::string formatLogLine(LogLevel level, std::string_view message) {
    return std::format("{} {}\n", logPrefix(level), message);
}

void writeLog(LogLevel level, std::string_view message) {
    const std::string line = formatLogLine(level, message);
    std::fwrite(line.data(), 1, line.size(), stderr);
}

} // namespace dilithium
