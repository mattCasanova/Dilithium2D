#pragma once

#include <dilithium/math/Math.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

/// The vector and matrix types, as LiquidMetal2D's `TypeAliases` over `simd`: glm's, under our names. glm is the
/// engine's one public dependency, on purpose. It is built with intrinsics on (NEON on arm64, SSE on x86) and
/// Vulkan's depth range; a game that links the engine gets the same build.
namespace dilithium {

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat4 = glm::mat4;

namespace math {

/// Whether every component differs by no more than `epsilon`.
constexpr bool isNearlyEqual(Vec2 a, Vec2 b, float epsilon = kEpsilon) {
    return isNearlyEqual(a.x, b.x, epsilon) && isNearlyEqual(a.y, b.y, epsilon);
}
constexpr bool isNearlyEqual(Vec3 a, Vec3 b, float epsilon = kEpsilon) {
    return isNearlyEqual(a.x, b.x, epsilon) && isNearlyEqual(a.y, b.y, epsilon) && isNearlyEqual(a.z, b.z, epsilon);
}
constexpr bool isNearlyEqual(Vec4 a, Vec4 b, float epsilon = kEpsilon) {
    return isNearlyEqual(a.x, b.x, epsilon) && isNearlyEqual(a.y, b.y, epsilon) && isNearlyEqual(a.z, b.z, epsilon) &&
           isNearlyEqual(a.w, b.w, epsilon);
}

} // namespace math
} // namespace dilithium
