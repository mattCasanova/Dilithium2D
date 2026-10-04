#include "shaders/color_frag.hpp"
#include "shaders/color_vert.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>

namespace {

constexpr uint32_t kSpirvMagic = 0x07230203u;
constexpr uint32_t kSpirvVersion16 = 0x00010600u; // what glslc emits for --target-env=vulkan1.3
constexpr int kHeaderWords = 5;                   // magic, version, generator, bound, schema

} // namespace

TEST_CASE("the embedded shaders are SPIR-V: magic number first, a header, then code", "[shaders]") {
    STATIC_CHECK(dilithium::shaders::kColorVert[0] == kSpirvMagic);
    STATIC_CHECK(dilithium::shaders::kColorFrag[0] == kSpirvMagic);
    STATIC_CHECK(dilithium::shaders::kColorVert[1] == kSpirvVersion16);
    STATIC_CHECK(dilithium::shaders::kColorVert.size() > kHeaderWords);
    STATIC_CHECK(dilithium::shaders::kColorFrag.size() > kHeaderWords);
}

TEST_CASE("the two stages are different programs", "[shaders]") {
    STATIC_CHECK(dilithium::shaders::kColorVert.size() != dilithium::shaders::kColorFrag.size());
}
