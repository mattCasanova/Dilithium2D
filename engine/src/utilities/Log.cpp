#include "utilities/LogFormat.hpp"

#include <dilithium/utilities/Assert.hpp>
#include <dilithium/utilities/Log.hpp>

#include <cstdio>
#include <format>
#include <string>
#include <string_view>

namespace dilithium {

std::string_view logPrefix(LogLevel level) {
    switch (level) {
    case LogLevel::Debug:
        return "debug:";
    case LogLevel::Info:
        return "info:";
    case LogLevel::Warning:
        return "warning:";
    case LogLevel::Error:
        return "error:";
    }
    DILITHIUM_UNREACHABLE("unknown LogLevel");
}

std::string formatLogLine(LogLevel level, std::string_view message) {
    return std::format("{} {}\n", logPrefix(level), message);
}

void writeLog(LogLevel level, std::string_view message) {
    const std::string line = formatLogLine(level, message);
    std::fwrite(line.data(), 1, line.size(), stderr);
}

} // namespace dilithium
