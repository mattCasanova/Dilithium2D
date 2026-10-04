#include <dilithium/math/Shapes.hpp>
#include <dilithium/math/Types.hpp>

#include <catch2/catch_test_macros.hpp>

using dilithium::AABB;
using dilithium::Circle;
using dilithium::LineSegment;
using dilithium::Vec2;

TEST_CASE("shapes default to nothing: zero size at the origin", "[shapes]") {
    CHECK(Circle{}.radius == 0.0f);
    CHECK(AABB{}.width == 0.0f);
    CHECK(LineSegment{}.start == Vec2{0.0f, 0.0f});
}

TEST_CASE("a box's corners come from its center and size", "[shapes]") {
    const AABB box{.center = {10.0f, 20.0f}, .width = 4.0f, .height = 6.0f};
    CHECK(box.halfWidth() == 2.0f);
    CHECK(box.halfHeight() == 3.0f);
    CHECK(box.min() == Vec2{8.0f, 17.0f});
    CHECK(box.max() == Vec2{12.0f, 23.0f});
}
