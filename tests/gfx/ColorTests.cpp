#include <dilithium/gfx/Color.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
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

TEST_CASE("fromHex reads 0xRRGGBBAA, the stylesheet order", "[color]") {
    STATIC_CHECK(dilithium::Color::fromHex(0xFF0000FF) == dilithium::colors::kRed);
    STATIC_CHECK(dilithium::Color::fromHex(0x00FF00FF) == dilithium::colors::kGreen);
    STATIC_CHECK(dilithium::Color::fromHex(0x0000FFFF) == dilithium::colors::kBlue);
    STATIC_CHECK(dilithium::Color::fromHex(0xFFFFFFFF) == dilithium::colors::kWhite);
    STATIC_CHECK(dilithium::Color::fromHex(0x00000000) == dilithium::colors::kTransparent);
    STATIC_CHECK(dilithium::Color::fromHex(0x000000FF) == dilithium::colors::kBlack);
}

TEST_CASE("bytes round-trip through a Color, and halves round to the nearest byte", "[color]") {
    const dilithium::Color grey = dilithium::Color::fromRGBA8(10, 20, 30, 40);
    CHECK(grey.toRGBA8() == std::array<uint8_t, 4>{10, 20, 30, 40});
    CHECK(dilithium::colors::kWhite.toRGBA8() == std::array<uint8_t, 4>{255, 255, 255, 255});
    CHECK(dilithium::Color{0.5f, 0.5f, 0.5f, 0.5f}.toRGBA8() == std::array<uint8_t, 4>{128, 128, 128, 128});
    CHECK(dilithium::Color::fromRGBA8(7, 8, 9).a == 1.0f); // alpha defaults to opaque
}

TEST_CASE("toRGBA8 clamps components outside 0..1", "[color]") {
    CHECK(dilithium::Color{2.0f, -1.0f, 1.0f, 1.0f}.toRGBA8() == std::array<uint8_t, 4>{255, 0, 255, 255});
}
