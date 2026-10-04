#include <dilithium/math/Bezier.hpp>
#include <dilithium/math/Math.hpp>
#include <dilithium/math/Types.hpp>

#include <catch2/catch_test_macros.hpp>
#include <glm/geometric.hpp>

#include <type_traits>

using dilithium::Mat4;
using dilithium::Vec2;
using dilithium::Vec3;
using dilithium::Vec4;
using namespace dilithium::math;

TEST_CASE("the aliases are glm's types, with the operators that come with them", "[types]") {
    STATIC_CHECK(std::is_same_v<Vec2, glm::vec2>);
    STATIC_CHECK(std::is_same_v<Mat4, glm::mat4>);
    STATIC_CHECK(sizeof(Vec2) == 2 * sizeof(float));
    STATIC_CHECK(sizeof(Mat4) == 16 * sizeof(float));

    const Vec2 a{1.0f, 2.0f};
    const Vec2 b{3.0f, 4.0f};
    CHECK(a + b == Vec2{4.0f, 6.0f});
    CHECK(a * 2.0f == Vec2{2.0f, 4.0f});
    CHECK(glm::dot(a, b) == 11.0f);
    CHECK(Mat4{1.0f} * Vec4{a, 0.0f, 1.0f} == Vec4{1.0f, 2.0f, 0.0f, 1.0f}); // identity leaves a point alone
}

TEST_CASE("isNearlyEqual compares every component", "[types]") {
    // Run-time checks: glm's aligned types (the price of intrinsics) cannot be built in a constant expression.
    CHECK(isNearlyEqual(Vec2{1.0f, 2.0f}, Vec2{1.0f, 2.0f}));
    CHECK(isNearlyEqual(Vec2{1.0f, 2.0f}, Vec2{1.0f, 2.0f + (kEpsilon / 2.0f)}));
    CHECK_FALSE(isNearlyEqual(Vec2{1.0f, 2.0f}, Vec2{1.0f, 2.1f}));
    CHECK_FALSE(isNearlyEqual(Vec3{1.0f, 2.0f, 3.0f}, Vec3{1.0f, 2.0f, 3.1f}));
    CHECK_FALSE(isNearlyEqual(Vec4{1.0f, 2.0f, 3.0f, 4.0f}, Vec4{1.0f, 2.0f, 3.0f, 4.1f}));
    CHECK(isNearlyEqual(Vec4{1.0f, 2.0f, 3.0f, 4.0f}, Vec4{1.0f, 2.0f, 3.0f, 4.1f}, 0.2f));
}

TEST_CASE("a quadratic Bezier starts at p0, ends at p2, and is pulled toward p1", "[types][bezier]") {
    const Vec2 p0{0.0f, 0.0f};
    const Vec2 p1{1.0f, 2.0f};
    const Vec2 p2{2.0f, 0.0f};
    CHECK(quadraticBezier(p0, p1, p2, 0.0f) == p0);
    CHECK(quadraticBezier(p0, p1, p2, 1.0f) == p2);
    CHECK(isNearlyEqual(quadraticBezier(p0, p1, p2, 0.5f), Vec2{1.0f, 1.0f})); // halfway up to p1, not all the way
}

TEST_CASE("a cubic Bezier starts at p0, ends at p3, and is symmetric for symmetric controls", "[types][bezier]") {
    const Vec2 p0{0.0f, 0.0f};
    const Vec2 p1{0.0f, 1.0f};
    const Vec2 p2{1.0f, 1.0f};
    const Vec2 p3{1.0f, 0.0f};
    CHECK(cubicBezier(p0, p1, p2, p3, 0.0f) == p0);
    CHECK(cubicBezier(p0, p1, p2, p3, 1.0f) == p3);
    CHECK(isNearlyEqual(cubicBezier(p0, p1, p2, p3, 0.5f), Vec2{0.5f, 0.75f}));
}
