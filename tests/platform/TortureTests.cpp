#include "platform/Torture.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <set>
#include <utility>

using dilithium::TortureAction;
using dilithium::tortureStep;

TEST_CASE("most frames do nothing", "[torture]") {
    CHECK(tortureStep(0).action == TortureAction::None);
    CHECK(tortureStep(1).action == TortureAction::None);
    CHECK(tortureStep(19).action == TortureAction::None);
    CHECK(tortureStep(21).action == TortureAction::None);
}

TEST_CASE("every 20th frame resizes, walking through different sizes", "[torture]") {
    std::set<std::pair<int, int>> sizes;
    for (uint64_t frame = 20; frame <= 160; frame += 20) {
        const auto step = tortureStep(frame);
        REQUIRE(step.action == TortureAction::Resize);
        CHECK(step.width > 0);
        CHECK(step.height > 0);
        sizes.insert({step.width, step.height});
    }
    CHECK(sizes.size() == 8);
}

TEST_CASE("every 97th frame minimizes, and 10 frames later restores", "[torture]") {
    CHECK(tortureStep(97).action == TortureAction::Minimize);
    CHECK(tortureStep(107).action == TortureAction::Restore);
    CHECK(tortureStep(194).action == TortureAction::Minimize);
    CHECK(tortureStep(204).action == TortureAction::Restore);
}

TEST_CASE("no restore before the first minimize", "[torture]") {
    CHECK(tortureStep(10).action == TortureAction::None);
}

TEST_CASE("minimize and restore win over a resize due on the same frame", "[torture]") {
    CHECK(tortureStep(20 * 97).action == TortureAction::Minimize); // 1940: due for both
}

TEST_CASE("a 600-frame torture run minimizes and restores six times", "[torture]") {
    int minimizes = 0;
    int restores = 0;
    for (uint64_t frame = 1; frame <= 600; ++frame) {
        const auto action = tortureStep(frame).action;
        minimizes += action == TortureAction::Minimize ? 1 : 0;
        restores += action == TortureAction::Restore ? 1 : 0;
    }
    CHECK(minimizes == 6);
    CHECK(restores == 6);
}
