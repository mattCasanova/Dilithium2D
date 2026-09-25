#include <dilithium/gfx/Color.hpp>

#include <catch2/catch_test_macros.hpp>

#include <format>
#include <string>

using dilithium::Color;

template <>
struct Catch::StringMaker<Color> {
    static std::string convert(const Color& c) { return std::format("Color({}, {}, {}, {})", c.r, c.g, c.b, c.a); }
};

TEST_CASE("fromHSV hits the primaries and secondaries exactly", "[color]") {
    CHECK(Color::fromHSV(0.0f, 1.0f, 1.0f) == Color{1.0f, 0.0f, 0.0f, 1.0f});
    CHECK(Color::fromHSV(60.0f, 1.0f, 1.0f) == Color{1.0f, 1.0f, 0.0f, 1.0f});
    CHECK(Color::fromHSV(120.0f, 1.0f, 1.0f) == Color{0.0f, 1.0f, 0.0f, 1.0f});
    CHECK(Color::fromHSV(180.0f, 1.0f, 1.0f) == Color{0.0f, 1.0f, 1.0f, 1.0f});
    CHECK(Color::fromHSV(240.0f, 1.0f, 1.0f) == Color{0.0f, 0.0f, 1.0f, 1.0f});
    CHECK(Color::fromHSV(300.0f, 1.0f, 1.0f) == Color{1.0f, 0.0f, 1.0f, 1.0f});
}

TEST_CASE("fromHSV blends between sectors", "[color]") {
    CHECK(Color::fromHSV(30.0f, 1.0f, 1.0f) == Color{1.0f, 0.5f, 0.0f, 1.0f});
}

TEST_CASE("fromHSV wraps the hue", "[color]") {
    CHECK(Color::fromHSV(360.0f, 1.0f, 1.0f) == Color::fromHSV(0.0f, 1.0f, 1.0f));
    CHECK(Color::fromHSV(480.0f, 1.0f, 1.0f) == Color::fromHSV(120.0f, 1.0f, 1.0f));
    CHECK(Color::fromHSV(-120.0f, 1.0f, 1.0f) == Color::fromHSV(240.0f, 1.0f, 1.0f));
    CHECK(Color::fromHSV(-1e-8f, 1.0f, 1.0f) == Color::fromHSV(0.0f, 1.0f, 1.0f));
}

TEST_CASE("fromHSV with no saturation is grey, with no value is black", "[color]") {
    CHECK(Color::fromHSV(200.0f, 0.0f, 0.25f) == Color{0.25f, 0.25f, 0.25f, 1.0f});
    CHECK(Color::fromHSV(200.0f, 1.0f, 0.0f) == Color{0.0f, 0.0f, 0.0f, 1.0f});
}

TEST_CASE("fromHSV clamps saturation, value and alpha to 0...1", "[color]") {
    CHECK(Color::fromHSV(0.0f, 2.0f, 2.0f, 2.0f) == Color{1.0f, 0.0f, 0.0f, 1.0f});
    CHECK(Color::fromHSV(0.0f, -1.0f, 1.0f, -1.0f) == Color{1.0f, 1.0f, 1.0f, 0.0f});
}
