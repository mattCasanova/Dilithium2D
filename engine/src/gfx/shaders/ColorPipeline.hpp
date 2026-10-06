#pragma once

#include <dilithium/utilities/NonMovable.hpp>

#include <vulkan/vulkan.h>

namespace dilithium {

/// The graphics pipeline for `shaders/color.*`: colored triangles from `ColorVertex` buffers into a color attachment
/// of `colorFormat`, through dynamic rendering (no render pass object). Viewport and scissor are dynamic, so a resize
/// never rebuilds it; only a change of color format does. No blending (D4) and no depth yet. Not copyable or
/// movable: `DefaultRenderer` holds it in place and rebuilds it by assignment of a new one.
class ColorPipeline : NonMovable {
public:
    ColorPipeline(VkDevice newDevice, VkFormat newColorFormat);
    ~ColorPipeline();

    [[nodiscard]] VkPipeline handle() const { return pipeline; }
    [[nodiscard]] VkPipelineLayout getLayout() const { return layout; }
    [[nodiscard]] VkFormat getColorFormat() const { return colorFormat; }

private:
    VkDevice device;
    VkFormat colorFormat;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
};

} // namespace dilithium
