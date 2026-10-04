#pragma once

#include <dilithium/gfx/Color.hpp>
#include <dilithium/math/Types.hpp>

#include <cstddef>
#include <cstdint>

namespace dilithium {

/// One corner of a colored triangle: what `shaders/color.vert` reads at locations 0 and 1. The static assertions
/// below pin the layout the pipeline's vertex input describes, so a change here fails to compile instead of
/// drawing garbage.
struct ColorVertex {
    Vec2 position; ///< clip space, +Y up
    Color color;
};

inline constexpr int kColorVertexFloats = 6; ///< two for the position, four for the color
inline constexpr uint32_t kColorVertexPositionOffset = 0;
inline constexpr uint32_t kColorVertexColorOffset = sizeof(Vec2);

static_assert(sizeof(Vec2) == 2 * sizeof(float), "Vec2 must be two packed floats for the shader");
static_assert(sizeof(Color) == 4 * sizeof(float), "Color must be four packed floats for the shader");
static_assert(offsetof(ColorVertex, position) == kColorVertexPositionOffset);
static_assert(offsetof(ColorVertex, color) == kColorVertexColorOffset);
static_assert(sizeof(ColorVertex) == kColorVertexFloats * sizeof(float), "no padding between position and color");

/// How many vertices one frame's vertex buffer holds. D2's debug drawing needs a few; D4 sizes sprite buffers
/// by the game's maximum object count instead.
inline constexpr uint32_t kMaxColorVertices = 3 * 1024;

} // namespace dilithium
