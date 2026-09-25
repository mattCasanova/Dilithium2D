#pragma once

#include <dilithium/core/Log.hpp>

#include <string>
#include <string_view>

namespace dilithium {

/// The prefix for each level: `debug:`, `info:`, `warning:` or `error:`.
std::string_view logPrefix(LogLevel level);

/// One complete log line, newline included, ready for a single write.
std::string formatLogLine(LogLevel level, std::string_view message);

} // namespace dilithium
