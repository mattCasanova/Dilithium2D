#include "StressSchedule.hpp"

#include <catch2/catch_test_macros.hpp>

#include <set>
#include <utility>

using stress::Action;
using stress::stepFor;

TEST_CASE("most frames do nothing", "[stress]") {
    STATIC_CHECK(stepFor(0).action == Action::None);
    STATIC_CHECK(stepFor(1).action == Action::None);
    STATIC_CHECK(stepFor(19).action == Action::None);
    STATIC_CHECK(stepFor(21).action == Action::None);
}

TEST_CASE("every 20th frame resizes, walking through every size", "[stress]") {
    std::set<std::pair<int, int>> sizes;
    for (int frame = 20; frame <= 160; frame += 20) {
        const auto step = stepFor(frame);
        REQUIRE(step.action == Action::Resize);
        CHECK(step.width > 0);
        CHECK(step.height > 0);
        sizes.insert({step.width, step.height});
    }
    CHECK(sizes.size() == 8);
}

TEST_CASE("every 97th frame minimizes", "[stress]") {
    STATIC_CHECK(stepFor(97).action == Action::Minimize);
    STATIC_CHECK(stepFor(194).action == Action::Minimize);
    STATIC_CHECK(stepFor(98).action == Action::None);
}

TEST_CASE("minimize wins over a resize due on the same frame", "[stress]") {
    STATIC_CHECK(stepFor(20 * 97).action == Action::Minimize); // 1940: due for both
}

TEST_CASE("a 600-frame run resizes 30 times and minimizes 6", "[stress]") {
    int resizes = 0;
    int minimizes = 0;
    for (int frame = 1; frame <= 600; ++frame) {
        const Action action = stepFor(frame).action;
        resizes += action == Action::Resize ? 1 : 0;
        minimizes += action == Action::Minimize ? 1 : 0;
    }
    CHECK(resizes == 30);
    CHECK(minimizes == 6);
}
