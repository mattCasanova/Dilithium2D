#pragma once

#include <array>
#include <cstddef>

namespace stress {

enum class Action { None, Resize, Minimize };

/// What the stress run does to the window on one frame. `width` and `height` (in points) are set for `Resize` only.
struct Step {
    Action action = Action::None;
    int width = 0;
    int height = 0;
};

struct Size {
    int width;
    int height;
};

inline constexpr int kResizeEvery = 20;
inline constexpr int kMinimizeEvery = 97;

/// From large to tiny and wide to tall, so the swapchain is rebuilt bigger, smaller, and at odd shapes.
inline constexpr std::array<Size, 8> kSizes{{
    {1280, 720},
    {640, 360},
    {1600, 900},
    {800, 600},
    {320, 200},
    {1024, 1024},
    {1920, 1080},
    {400, 800},
}};

/// The schedule, by frame counted from 1: every 20th frame resizes to the next size in `kSizes`; every 97th
/// minimizes. Minimize wins over a resize due on the same frame. The restore is not here: minimized, the engine
/// stands still and no frame comes, so the scene asks for it when the engine reports the background state.
[[nodiscard]] constexpr Step stepFor(int frame) {
    if (frame <= 0) {
        return {};
    }
    if (frame % kMinimizeEvery == 0) {
        return {.action = Action::Minimize};
    }
    if (frame % kResizeEvery == 0) {
        const Size size = kSizes[static_cast<std::size_t>(frame / kResizeEvery) % kSizes.size()];
        return {.action = Action::Resize, .width = size.width, .height = size.height};
    }
    return {};
}

} // namespace stress
