#include <dilithium/math/Easing.hpp>
#include <dilithium/math/Math.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <array>
#include <cmath>

using namespace dilithium::easing;
using Catch::Matchers::WithinAbs;

namespace {

using Curve = float (*)(float);

/// One family: its in, out and in-out forms; whether they leave [0, 1] (elastic and back overshoot on purpose);
/// and whether they only ever rise (a bounce rebounds away from the end between bounces, by design).
struct Family {
    const char* name;
    Curve in;
    Curve out;
    Curve inOut;
    bool overshoots;
    bool monotonic;
};

constexpr std::array<Family, 8> kFamilies{{
    {"quad", inQuad, outQuad, inOutQuad, false, true},
    {"cubic", inCubic, outCubic, inOutCubic, false, true},
    {"quart", inQuart, outQuart, inOutQuart, false, true},
    {"sine", inSine, outSine, inOutSine, false, true},
    {"expo", inExpo, outExpo, inOutExpo, false, true},
    {"bounce", inBounce, outBounce, inOutBounce, false, false},
    {"elastic", inElastic, outElastic, inOutElastic, true, false},
    {"back", inBack, outBack, inOutBack, true, false},
}};

constexpr double kTolerance = 1e-5;

auto nearly(float expected) {
    return WithinAbs(static_cast<double>(expected), kTolerance);
}

constexpr float kSlack = 1e-5f;

/// What a sweep of a curve across [0, 1] found.
struct Sweep {
    float lowest = 0.0f;
    float highest = 0.0f;
    int firstDropStep = 0; ///< the first step whose value fell below the one before, or 0 if none did
};

Sweep sweep(Curve curve) {
    constexpr int kSteps = 100;
    Sweep result{.lowest = curve(0.0f), .highest = curve(0.0f)};
    float previous = result.lowest;
    for (int step = 1; step <= kSteps; ++step) {
        const float value = curve(static_cast<float>(step) / kSteps);
        result.lowest = std::min(result.lowest, value);
        result.highest = std::max(result.highest, value);
        if (result.firstDropStep == 0 && value < previous - kSlack) {
            result.firstDropStep = step;
        }
        previous = value;
    }
    return result;
}

/// The three sweep checks for one family's three curves.
void checkSweeps(const Family& family) {
    for (const Curve curve : {family.in, family.out, family.inOut}) {
        INFO(family.name);
        const Sweep found = sweep(curve);
        CHECK(found.lowest >= -kSlack);
        CHECK(found.highest <= 1.0f + kSlack);
        CHECK((found.firstDropStep == 0) == family.monotonic);
    }
}

} // namespace

TEST_CASE("every curve starts at 0 and ends at 1", "[easing]") {
    for (const Family& family : kFamilies) {
        INFO(family.name);
        for (const Curve curve : {family.in, family.out, family.inOut}) {
            CHECK_THAT(curve(0.0f), nearly(0.0f));
            CHECK_THAT(curve(1.0f), nearly(1.0f));
        }
    }
}

TEST_CASE("out is in, mirrored: out(t) == 1 - in(1 - t)", "[easing]") {
    const float t = GENERATE(0.1f, 0.25f, 0.5f, 0.75f, 0.9f);
    for (const Family& family : kFamilies) {
        INFO(family.name << " at " << t);
        CHECK_THAT(family.out(t), nearly(1.0f - family.in(1.0f - t)));
    }
}

TEST_CASE("in-out passes through the middle at the middle", "[easing]") {
    for (const Family& family : kFamilies) {
        INFO(family.name);
        CHECK_THAT(family.inOut(0.5f), nearly(0.5f));
    }
}

TEST_CASE("the plain curves never leave [0, 1], and the smooth ones never go backwards", "[easing]") {
    for (const Family& family : kFamilies) {
        if (!family.overshoots) {
            checkSweeps(family);
        }
    }
}

TEST_CASE("a bounce rebounds: it reaches the floor early and comes back off it", "[easing]") {
    constexpr float kFirstFloor = 1.0f / 2.75f; // where the first parabola ends
    CHECK_THAT(outBounce(kFirstFloor), nearly(1.0f));
    CHECK(outBounce(0.45f) < 1.0f); // off the floor again
    CHECK(outBounce(0.45f) > 0.7f); // but most of the way there
}

TEST_CASE("elastic and back overshoot on purpose", "[easing]") {
    CHECK(inBack(0.3f) < 0.0f);     // pulls back before going
    CHECK(outBack(0.7f) > 1.0f);    // goes past, then settles
    CHECK(outElastic(0.2f) > 1.0f); // springs past the end
    CHECK(inElastic(0.8f) < 0.0f);
}

TEST_CASE("a few known values", "[easing]") {
    STATIC_CHECK(inQuad(0.5f) == 0.25f);
    STATIC_CHECK(outQuad(0.5f) == 0.75f);
    STATIC_CHECK(inCubic(0.5f) == 0.125f);
    STATIC_CHECK(inQuart(0.5f) == 0.0625f);
    STATIC_CHECK(outBounce(1.0f) == 1.0f);
    CHECK_THAT(inSine(0.5f), nearly(1.0f - std::cos(dilithium::math::kHalfPi / 2.0f)));
    CHECK_THAT(inExpo(0.5f), nearly(std::exp2(-5.0f)));
}
