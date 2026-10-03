#include "engine/Torture.hpp"

#include <array>
#include <cstdint>

namespace dilithium {
namespace {

struct Size {
    int width;
    int height;
};

constexpr std::array<Size, 8> kSizes{{
    {1280, 720},
    {640, 360},
    {1600, 900},
    {800, 600},
    {320, 200},
    {1024, 1024},
    {1920, 1080},
    {400, 800},
}};

} // namespace

TortureStep tortureStep(uint64_t frame) {
    if (frame == 0) {
        return {};
    }
    if (frame % kTortureMinimizeEvery == 0) {
        return {TortureAction::Minimize};
    }
    if (frame > kTortureMinimizeEvery && frame % kTortureMinimizeEvery == kTortureRestoreAfter) {
        return {TortureAction::Restore};
    }
    if (frame % kTortureResizeEvery == 0) {
        const Size size = kSizes[(frame / kTortureResizeEvery) % kSizes.size()];
        return {TortureAction::Resize, size.width, size.height};
    }
    return {};
}

} // namespace dilithium
