#include "engine/RunOptions.hpp"

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string_view>
#include <vector>

using dilithium::parseRunOptions;
using dilithium::RunOptions;

namespace {

RunOptions parse(std::vector<std::string_view> args) {
    return parseRunOptions(args);
}

} // namespace

TEST_CASE("no flags: run until quit, no torture", "[runoptions]") {
    const RunOptions options = parse({"d01_clear_screen"});
    CHECK_FALSE(options.frames.has_value());
    CHECK_FALSE(options.torture);
}

TEST_CASE("reads --frames N and --torture in either order", "[runoptions]") {
    const RunOptions a = parse({"demo", "--frames", "600", "--torture"});
    CHECK(a.frames == 600u);
    CHECK(a.torture);
    const RunOptions b = parse({"demo", "--torture", "--frames", "5"});
    CHECK(b.frames == 5u);
    CHECK(b.torture);
}

TEST_CASE("arguments the engine does not know are left for the game", "[runoptions]") {
    const RunOptions options = parse({"demo", "--level", "3", "--frames", "10", "extra"});
    CHECK(options.frames == 10u);
    CHECK_FALSE(options.torture);
}

TEST_CASE("a --frames with no proper count is an error", "[runoptions]") {
    CHECK_THROWS_AS(parse({"demo", "--frames"}), std::invalid_argument);
    CHECK_THROWS_AS(parse({"demo", "--frames", "0"}), std::invalid_argument);
    CHECK_THROWS_AS(parse({"demo", "--frames", "-5"}), std::invalid_argument);
    CHECK_THROWS_AS(parse({"demo", "--frames", "60x"}), std::invalid_argument);
    CHECK_THROWS_AS(parse({"demo", "--frames", "--torture"}), std::invalid_argument);
}
