#include "core/LogFormat.hpp"

#include <catch2/catch_test_macros.hpp>

using dilithium::formatLogLine;
using dilithium::LogLevel;

TEST_CASE("each log level has its own prefix", "[log]") {
    CHECK(formatLogLine(LogLevel::Debug, "stardate 41153.7") == "Captain's log: stardate 41153.7\n");
    CHECK(formatLogLine(LogLevel::Info, "warp core online") == "Fascinating: warp core online\n");
    CHECK(formatLogLine(LogLevel::Warning, "shields at 40%") == "Yellow alert: shields at 40%\n");
    CHECK(formatLogLine(LogLevel::Error, "warp core breach") == "Red alert: warp core breach\n");
}

TEST_CASE("a log line always ends in exactly one newline", "[log]") {
    CHECK(formatLogLine(LogLevel::Info, "") == "Fascinating: \n");
}

namespace {

int counted(int& calls) {
    ++calls;
    return calls;
}

} // namespace

TEST_CASE("CAPTAINS_LOG runs its arguments in debug and never in release", "[log]") {
    // Used only in the log: the release build proves it still counts as used under -Werror.
    const int torpedoes = 4;
    CAPTAINS_LOG("torpedoes: {}", torpedoes);

    int calls = 0;
    CAPTAINS_LOG("counted: {}", counted(calls));
#if defined(DILITHIUM_DEBUG)
    CHECK(calls == 1);
#else
    CHECK(calls == 0);
#endif
}
