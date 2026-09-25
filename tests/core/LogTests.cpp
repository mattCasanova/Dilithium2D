#include "core/LogFormat.hpp"

#include <catch2/catch_test_macros.hpp>

using dilithium::formatLogLine;
using dilithium::LogLevel;

TEST_CASE("each log level has its own prefix", "[log]") {
    CHECK(formatLogLine(LogLevel::Debug, "frame 41") == "debug: frame 41\n");
    CHECK(formatLogLine(LogLevel::Info, "device ready") == "info: device ready\n");
    CHECK(formatLogLine(LogLevel::Warning, "frame took 40 ms") == "warning: frame took 40 ms\n");
    CHECK(formatLogLine(LogLevel::Error, "device lost") == "error: device lost\n");
}

TEST_CASE("a log line always ends in exactly one newline", "[log]") {
    CHECK(formatLogLine(LogLevel::Info, "") == "info: \n");
}

namespace {

int counted(int& calls) {
    ++calls;
    return calls;
}

} // namespace

TEST_CASE("DILITHIUM_LOG_DEBUG runs its arguments in debug and never in release", "[log]") {
    // Used only in the log: the release build proves it still counts as used under -Werror.
    const int spriteCount = 4;
    DILITHIUM_LOG_DEBUG("sprites: {}", spriteCount);

    int calls = 0;
    DILITHIUM_LOG_DEBUG("counted: {}", counted(calls));
#if defined(DILITHIUM_DEBUG)
    CHECK(calls == 1);
#else
    CHECK(calls == 0);
#endif
}
