#pragma once

#include <dilithium/math/Shapes.hpp>
#include <dilithium/math/Types.hpp>

/// Yes-or-no overlap tests between the shapes, LiquidMetal2D's `Intersect`. Touching counts as a hit, everywhere.
/// Each test comes as plain values and as shapes. No contact points or normals: those are the physics rung's.
namespace dilithium::intersect {

[[nodiscard]] bool pointCircle(Vec2 point, Vec2 center, float radius);
[[nodiscard]] bool pointCircle(Vec2 point, const Circle& circle);

[[nodiscard]] bool pointBox(Vec2 point, Vec2 center, float width, float height);
[[nodiscard]] bool pointBox(Vec2 point, const AABB& box);

/// Whether the point lies on the segment, within `math::kEpsilon` of it. A zero-length segment is a point.
[[nodiscard]] bool pointSegment(Vec2 point, Vec2 start, Vec2 end);
[[nodiscard]] bool pointSegment(Vec2 point, const LineSegment& segment);

[[nodiscard]] bool circleCircle(Vec2 centerA, float radiusA, Vec2 centerB, float radiusB);
[[nodiscard]] bool circleCircle(const Circle& a, const Circle& b);

[[nodiscard]] bool circleBox(Vec2 circleCenter, float radius, Vec2 boxCenter, float width, float height);
[[nodiscard]] bool circleBox(const Circle& circle, const AABB& box);

[[nodiscard]] bool circleSegment(Vec2 center, float radius, Vec2 start, Vec2 end);
[[nodiscard]] bool circleSegment(const Circle& circle, const LineSegment& segment);

[[nodiscard]] bool boxBox(const AABB& a, const AABB& b);

} // namespace dilithium::intersect
