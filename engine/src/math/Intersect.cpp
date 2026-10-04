#include <dilithium/math/Intersect.hpp>
#include <dilithium/math/Math.hpp>
#include <dilithium/math/Shapes.hpp>
#include <dilithium/math/Types.hpp>

#include <glm/common.hpp>
#include <glm/geometric.hpp>

namespace dilithium::intersect {
namespace {

float lengthSquared(Vec2 v) {
    return glm::dot(v, v);
}

/// Whether `offset` (a point relative to a box's center) lies inside a box of that size.
bool insideBox(Vec2 offset, float halfWidth, float halfHeight) {
    return math::isInRange(offset.x, -halfWidth, halfWidth) && math::isInRange(offset.y, -halfHeight, halfHeight);
}

} // namespace

bool pointCircle(Vec2 point, Vec2 center, float radius) {
    return lengthSquared(point - center) <= radius * radius;
}

bool pointCircle(Vec2 point, const Circle& circle) {
    return pointCircle(point, circle.center, circle.radius);
}

bool pointBox(Vec2 point, Vec2 center, float width, float height) {
    return insideBox(point - center, width / 2.0f, height / 2.0f);
}

bool pointBox(Vec2 point, const AABB& box) {
    return pointBox(point, box.center, box.width, box.height);
}

bool pointSegment(Vec2 point, Vec2 start, Vec2 end) {
    const Vec2 line = end - start;
    const float length2 = lengthSquared(line);
    if (length2 <= 0.0f) {
        return math::isNearlyEqual(point, start); // a zero-length segment is a point
    }
    const Vec2 toPoint = point - start;
    // Off the line by more than epsilon (the cross product is the distance times the line's length)?
    const float side = math::cross(line, toPoint);
    if (side * side > math::kEpsilon * math::kEpsilon * length2) {
        return false;
    }
    // On the line: between the ends?
    const float along = glm::dot(toPoint, line);
    return along >= 0.0f && along <= length2;
}

bool pointSegment(Vec2 point, const LineSegment& segment) {
    return pointSegment(point, segment.start, segment.end);
}

bool circleCircle(Vec2 centerA, float radiusA, Vec2 centerB, float radiusB) {
    const float reach = radiusA + radiusB;
    return lengthSquared(centerA - centerB) <= reach * reach;
}

bool circleCircle(const Circle& a, const Circle& b) {
    return circleCircle(a.center, a.radius, b.center, b.radius);
}

bool circleBox(Vec2 circleCenter, float radius, Vec2 boxCenter, float width, float height) {
    const float halfWidth = width / 2.0f;
    const float halfHeight = height / 2.0f;
    const Vec2 offset = circleCenter - boxCenter;
    if (insideBox(offset, halfWidth, halfHeight)) {
        return true;
    }
    // Outside: the nearest point of the box to the circle's center decides.
    const Vec2 closest = glm::clamp(offset, Vec2{-halfWidth, -halfHeight}, Vec2{halfWidth, halfHeight});
    return lengthSquared(offset - closest) <= radius * radius;
}

bool circleBox(const Circle& circle, const AABB& box) {
    return circleBox(circle.center, circle.radius, box.center, box.width, box.height);
}

bool circleSegment(Vec2 center, float radius, Vec2 start, Vec2 end) {
    const Vec2 line = end - start;
    const Vec2 toCenter = center - start;
    const float radius2 = radius * radius;
    const float along = glm::dot(toCenter, line);
    if (along <= 0.0f) {
        return lengthSquared(toCenter) <= radius2; // nearest to the start
    }
    const float length2 = lengthSquared(line);
    if (along >= length2) {
        return lengthSquared(center - end) <= radius2; // nearest to the end
    }
    // Nearest to a point between: the perpendicular distance, squared, scaled by the line's length squared.
    const float side = math::cross(line, toCenter);
    return side * side <= radius2 * length2;
}

bool circleSegment(const Circle& circle, const LineSegment& segment) {
    return circleSegment(circle.center, circle.radius, segment.start, segment.end);
}

bool boxBox(const AABB& a, const AABB& b) {
    // Two boxes overlap when one's center is inside a box of their summed sizes around the other's center.
    return pointBox(a.center, b.center, a.width + b.width, a.height + b.height);
}

} // namespace dilithium::intersect
