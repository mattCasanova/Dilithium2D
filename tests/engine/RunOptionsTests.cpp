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

TEST_CASE("no flags: run until quit", "[runoptions]") {
    const RunOptions options = parse({"d01_clear_screen"});
    CHECK_FALSE(options.frames.has_value());
}

TEST_CASE("reads --frames N wherever it stands", "[runoptions]") {
    const RunOptions a = parse({"demo", "--frames", "600"});
    CHECK(a.frames == 600u);
    const RunOptions b = parse({"demo", "--verbose", "--frames", "5"});
    CHECK(b.frames == 5u);
}

TEST_CASE("arguments the engine does not know are left for the game", "[runoptions]") {
    const RunOptions options = parse({"demo", "--level", "3", "--frames", "10", "extra"});
    CHECK(options.frames == 10u);
}

TEST_CASE("a --frames with no proper count is an error", "[runoptions]") {
    CHECK_THROWS_AS(parse({"demo", "--frames"}), std::invalid_argument);
    CHECK_THROWS_AS(parse({"demo", "--frames", "0"}), std::invalid_argument);
    CHECK_THROWS_AS(parse({"demo", "--frames", "-5"}), std::invalid_argument);
    CHECK_THROWS_AS(parse({"demo", "--frames", "60x"}), std::invalid_argument);
    CHECK_THROWS_AS(parse({"demo", "--frames", "--verbose"}), std::invalid_argument);
}
