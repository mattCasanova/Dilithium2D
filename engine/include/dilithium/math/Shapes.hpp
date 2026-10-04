#pragma once

#include <dilithium/math/Types.hpp>

namespace dilithium {

/// A circle by center and radius.
struct Circle {
    Vec2 center{};
    float radius = 0.0f;
};

/// An axis-aligned box by center and full size, as LiquidMetal2D's `AABB`. Game objects are usually this shape:
/// a position and a size, no corners to keep in step.
struct AABB {
    Vec2 center{};
    float width = 0.0f;
    float height = 0.0f;

    [[nodiscard]] float halfWidth() const { return width / 2.0f; }
    [[nodiscard]] float halfHeight() const { return height / 2.0f; }
    [[nodiscard]] Vec2 min() const { return {center.x - halfWidth(), center.y - halfHeight()}; }
    [[nodiscard]] Vec2 max() const { return {center.x + halfWidth(), center.y + halfHeight()}; }
};

/// A straight line between two points, both included.
struct LineSegment {
    Vec2 start{};
    Vec2 end{};
};

} // namespace dilithium
