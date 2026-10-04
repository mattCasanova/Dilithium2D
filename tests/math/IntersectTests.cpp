#include <dilithium/math/Intersect.hpp>
#include <dilithium/math/Shapes.hpp>
#include <dilithium/math/Types.hpp>

#include <catch2/catch_test_macros.hpp>

using dilithium::AABB;
using dilithium::Circle;
using dilithium::LineSegment;
using dilithium::Vec2;
using namespace dilithium::intersect;

namespace {

// Functions, not globals: glm's aligned types have no constexpr constructor, and a global's initializer runs
// before main, where a throw cannot be caught.
Circle unit() {
    return {.center = {0.0f, 0.0f}, .radius = 1.0f};
}
AABB box() { // x in [-2, 2], y in [-1, 1]
    return {.center = {0.0f, 0.0f}, .width = 4.0f, .height = 2.0f};
}
LineSegment across() {
    return {.start = {-1.0f, 0.0f}, .end = {1.0f, 0.0f}};
}

} // namespace

TEST_CASE("point in circle: inside, on the edge, outside", "[intersect]") {
    CHECK(pointCircle({0.5f, 0.0f}, unit()));
    CHECK(pointCircle({1.0f, 0.0f}, unit())); // touching counts
    CHECK_FALSE(pointCircle({1.1f, 0.0f}, unit()));
    CHECK(pointCircle({0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f)); // a zero circle is its center
    CHECK_FALSE(pointCircle({0.1f, 0.0f}, {0.0f, 0.0f}, 0.0f));
}

TEST_CASE("point in box: inside, on an edge or corner, outside", "[intersect]") {
    CHECK(pointBox({1.0f, 0.5f}, box()));
    CHECK(pointBox({2.0f, 1.0f}, box())); // the corner counts
    CHECK(pointBox({-2.0f, 0.0f}, box()));
    CHECK_FALSE(pointBox({2.1f, 0.0f}, box()));
    CHECK_FALSE(pointBox({0.0f, 1.1f}, box()));
}

TEST_CASE("point on segment: on it, at an end, beside it, beyond it", "[intersect]") {
    CHECK(pointSegment({0.0f, 0.0f}, across()));
    CHECK(pointSegment({1.0f, 0.0f}, across())); // an end counts
    CHECK(pointSegment({-1.0f, 0.0f}, across()));
    CHECK_FALSE(pointSegment({0.0f, 0.01f}, across())); // beside, more than epsilon off
    CHECK_FALSE(pointSegment({1.5f, 0.0f}, across()));  // in line, but beyond the end
    CHECK(pointSegment({0.5f, 0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f}));
}

TEST_CASE("point on a zero-length segment is its one point", "[intersect]") {
    CHECK(pointSegment({2.0f, 3.0f}, {2.0f, 3.0f}, {2.0f, 3.0f}));
    CHECK_FALSE(pointSegment({2.0f, 3.1f}, {2.0f, 3.0f}, {2.0f, 3.0f}));
}

TEST_CASE("circle and circle: apart, touching, overlapping, one inside the other", "[intersect]") {
    CHECK_FALSE(circleCircle(unit(), {.center = {3.0f, 0.0f}, .radius = 1.0f}));
    CHECK(circleCircle(unit(), {.center = {2.0f, 0.0f}, .radius = 1.0f})); // touching counts
    CHECK(circleCircle(unit(), {.center = {1.0f, 0.0f}, .radius = 1.0f}));
    CHECK(circleCircle(unit(), {.center = {0.1f, 0.0f}, .radius = 0.2f})); // contained
}

TEST_CASE("circle and box: center inside, overlapping an edge, touching a corner, clear of a corner", "[intersect]") {
    CHECK(circleBox({.center = {1.0f, 0.5f}, .radius = 0.1f}, box())); // center inside
    CHECK(circleBox({.center = {2.5f, 0.0f}, .radius = 1.0f}, box())); // over the right edge
    CHECK(circleBox({.center = {3.0f, 0.0f}, .radius = 1.0f}, box())); // touching the right edge counts
    CHECK_FALSE(circleBox({.center = {3.1f, 0.0f}, .radius = 1.0f}, box()));
    // Near the corner (2, 1): a circle whose edge reaches the corner exactly, and one that misses it diagonally
    // even though it would overlap the box's bounding extents in each axis alone.
    CHECK(circleBox({.center = {3.0f, 2.0f}, .radius = 1.4143f}, box()));
    CHECK_FALSE(circleBox({.center = {3.0f, 2.0f}, .radius = 1.0f}, box()));
}

TEST_CASE("circle and segment: crossing, nearest to an end, nearest to the middle, missing", "[intersect]") {
    CHECK(circleSegment(unit(), {.start = {-5.0f, 0.0f}, .end = {5.0f, 0.0f}}));  // runs through it
    CHECK(circleSegment(unit(), {.start = {1.0f, 0.0f}, .end = {5.0f, 0.0f}}));   // starts on the edge
    CHECK(circleSegment(unit(), {.start = {-5.0f, 0.0f}, .end = {-1.0f, 0.0f}})); // ends on the edge
    CHECK(circleSegment(unit(), {.start = {-5.0f, 1.0f}, .end = {5.0f, 1.0f}}));  // grazes the top
    CHECK_FALSE(circleSegment(unit(), {.start = {-5.0f, 1.1f}, .end = {5.0f, 1.1f}}));
    CHECK_FALSE(circleSegment(unit(), {.start = {2.0f, 0.0f}, .end = {5.0f, 0.0f}})); // in line, but short
}

TEST_CASE("box and box: apart, touching, overlapping, contained", "[intersect]") {
    CHECK_FALSE(boxBox(box(), {.center = {5.0f, 0.0f}, .width = 1.0f, .height = 1.0f}));
    CHECK(boxBox(box(), {.center = {2.5f, 0.0f}, .width = 1.0f, .height = 1.0f}));        // edges touch at x = 2
    CHECK(boxBox(box(), {.center = {2.0f, 1.0f}, .width = 1.0f, .height = 1.0f}));        // overlapping the corner
    CHECK(boxBox(box(), {.center = {0.0f, 0.0f}, .width = 1.0f, .height = 1.0f}));        // contained
    CHECK_FALSE(boxBox(box(), {.center = {0.0f, 2.0f}, .width = 10.0f, .height = 1.0f})); // wide but above
}
