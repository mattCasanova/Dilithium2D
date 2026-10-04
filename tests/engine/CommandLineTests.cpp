#include <dilithium/engine/CommandLine.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string_view>

using dilithium::CommandLine;

namespace {

constexpr std::array<const char*, 5> kArgv{"game", "--frames", "600", "--torture", "--level"};

} // namespace

TEST_CASE("all() keeps every argument in order, the program name first", "[commandline]") {
    const CommandLine line(static_cast<int>(kArgv.size()), kArgv.data());
    REQUIRE(line.all().size() == 5);
    CHECK(line.all()[0] == "game");
    CHECK(line.all()[4] == "--level");
}

TEST_CASE("has() finds a flag after the program name only", "[commandline]") {
    const CommandLine line(static_cast<int>(kArgv.size()), kArgv.data());
    CHECK(line.has("--torture"));
    CHECK(line.has("--frames"));
    CHECK_FALSE(line.has("--fullscreen"));
    CHECK_FALSE(line.has("game")); // the program name is not a flag
}

TEST_CASE("value() is the argument after the flag, or nothing", "[commandline]") {
    const CommandLine line(static_cast<int>(kArgv.size()), kArgv.data());
    CHECK(line.value("--frames") == std::string_view{"600"});
    CHECK(line.value("--torture") == std::string_view{"--level"}); // the caller decides what counts as a value
    CHECK_FALSE(line.value("--level").has_value());                // last, so no value
    CHECK_FALSE(line.value("--missing").has_value());
}

TEST_CASE("a program with no arguments has only its name", "[commandline]") {
    constexpr std::array<const char*, 1> kNameOnly{"game"};
    const CommandLine line(1, kNameOnly.data());
    CHECK(line.all().size() == 1);
    CHECK_FALSE(line.has("--frames"));
}
