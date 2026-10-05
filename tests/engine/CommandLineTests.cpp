#include <dilithium/engine/CommandLine.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string_view>

using dilithium::CommandLine;

namespace {

constexpr std::array<const char*, 5> kArgv{"game", "--frames", "600", "--verbose", "--level"};

} // namespace

TEST_CASE("getAll() keeps every argument in order, the program name first", "[commandline]") {
    const CommandLine line(static_cast<int>(kArgv.size()), kArgv.data());
    REQUIRE(line.getAll().size() == 5);
    CHECK(line.getAll()[0] == "game");
    CHECK(line.getAll()[4] == "--level");
}

TEST_CASE("has() finds a flag after the program name only", "[commandline]") {
    const CommandLine line(static_cast<int>(kArgv.size()), kArgv.data());
    CHECK(line.has("--verbose"));
    CHECK(line.has("--frames"));
    CHECK_FALSE(line.has("--fullscreen"));
    CHECK_FALSE(line.has("game")); // the program name is not a flag
}

TEST_CASE("getValue() is the argument after the flag, or nothing", "[commandline]") {
    const CommandLine line(static_cast<int>(kArgv.size()), kArgv.data());
    CHECK(line.getValue("--frames") == std::string_view{"600"});
    CHECK(line.getValue("--verbose") == std::string_view{"--level"}); // the caller decides what counts as a value
    CHECK_FALSE(line.getValue("--level").has_value());                // last, so no value
    CHECK_FALSE(line.getValue("--missing").has_value());
}

TEST_CASE("a program with no arguments has only its name", "[commandline]") {
    constexpr std::array<const char*, 1> kNameOnly{"game"};
    const CommandLine line(1, kNameOnly.data());
    CHECK(line.getAll().size() == 1);
    CHECK_FALSE(line.has("--frames"));
}
