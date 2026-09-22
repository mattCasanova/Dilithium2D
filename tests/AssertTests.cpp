#include <dilithium/Assert.hpp>
#include <dilithium/Color.hpp>

#include <catch2/catch_test_macros.hpp>

#include <limits>
#include <stdexcept>

namespace {

bool isPositive(int value) {
    return value > 0;
}

bool countedTrue(int& calls) {
    ++calls;
    return true;
}

} // namespace

// A failing LOGICAL or ILLOGICAL aborts in debug, which a test cannot catch; those paths were checked by hand.

TEST_CASE("LOGICAL lets a true condition through", "[assert]") {
    // Both are used only inside LOGICAL: the release build proves they still count as used under -Werror.
    const int shields = 40;
    LOGICAL(shields > 0, "shields are up");
    LOGICAL(isPositive(shields), "helper functions count as used too");
    SUCCEED();
}

TEST_CASE("LOGICAL runs its condition once in debug and never in release", "[assert]") {
    int calls = 0;
    LOGICAL(countedTrue(calls), "counted");
#if defined(DILITHIUM_DEBUG)
    CHECK(calls == 1);
#else
    CHECK(calls == 0);
#endif
}

TEST_CASE("LOGICAL is one expression, safe in a brace-less if", "[assert]") {
    const bool redAlert = false;
    bool tookElse = false;
    if (redAlert)
        LOGICAL(true, "never reached");
    else
        tookElse = true;
    CHECK(tookElse);
}

#if !defined(DILITHIUM_DEBUG)

TEST_CASE("ILLOGICAL throws logic_error in release", "[assert]") {
    CHECK_THROWS_AS(ILLOGICAL("the needs of the many"), std::logic_error);
}

TEST_CASE("fromHSV rejects non-finite input in release", "[assert][color]") {
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float inf = std::numeric_limits<float>::infinity();
    CHECK_THROWS_AS(dilithium::Color::fromHSV(nan, 1.0f, 1.0f), std::logic_error);
    CHECK_THROWS_AS(dilithium::Color::fromHSV(0.0f, inf, 1.0f), std::logic_error);
    CHECK_THROWS_AS(dilithium::Color::fromHSV(0.0f, 1.0f, nan), std::logic_error);
    CHECK_THROWS_AS(dilithium::Color::fromHSV(0.0f, 1.0f, 1.0f, -inf), std::logic_error);
}

#endif
