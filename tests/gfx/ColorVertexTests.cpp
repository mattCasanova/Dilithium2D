#include "gfx/renderers/ColorVertex.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>

using dilithium::ColorVertex;

TEST_CASE("a color vertex is six packed floats: two for position, four for color", "[gfx]") {
    STATIC_CHECK(sizeof(ColorVertex) == 24);
    STATIC_CHECK(offsetof(ColorVertex, position) == 0);
    STATIC_CHECK(offsetof(ColorVertex, color) == 8);
    STATIC_CHECK(dilithium::kColorVertexColorOffset == 8);
}
