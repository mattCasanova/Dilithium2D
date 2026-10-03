#include <dilithium/core/Assert.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {

bool isPositive(int value) {
    return value > 0;
}

bool countedTrue(int& calls) {
    ++calls;
    return true;
}

} // namespace

// A failing DILITHIUM_ASSERT or DILITHIUM_UNREACHABLE aborts in debug, which a test cannot catch; those paths were
// checked by hand.

TEST_CASE("DILITHIUM_ASSERT lets a true condition through", "[assert]") {
    // Both are used only inside DILITHIUM_ASSERT: the release build proves they still count as used under -Werror.
    const int spriteCount = 40;
    DILITHIUM_ASSERT(spriteCount > 0, "there are sprites to draw");
    DILITHIUM_ASSERT(isPositive(spriteCount), "helper functions count as used too");
    SUCCEED();
}

TEST_CASE("DILITHIUM_ASSERT runs its condition once in debug and never in release", "[assert]") {
    int calls = 0;
    DILITHIUM_ASSERT(countedTrue(calls), "counted");
#ifdef DILITHIUM_DEBUG
    CHECK(calls == 1);
#else
    CHECK(calls == 0);
#endif
}

TEST_CASE("DILITHIUM_ASSERT is one expression, safe in a brace-less if", "[assert]") {
    const bool takeFirstBranch = false;
    bool tookElse = false;
    // The unbraced if/else is the point: the macro must not swallow the else.
    // NOLINTBEGIN(readability-braces-around-statements)
    if (takeFirstBranch)
        DILITHIUM_ASSERT(true, "never reached");
    else
        tookElse = true;
    // NOLINTEND(readability-braces-around-statements)
    CHECK(tookElse);
}

#ifndef DILITHIUM_DEBUG
// Only this block uses them; the linter checks the debug build, where the block does not exist.
#include <dilithium/gfx/Color.hpp>

#include <limits>
#include <stdexcept>

TEST_CASE("DILITHIUM_UNREACHABLE throws logic_error in release", "[assert]") {
    CHECK_THROWS_AS(DILITHIUM_UNREACHABLE("a branch that cannot happen"), std::logic_error);
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
