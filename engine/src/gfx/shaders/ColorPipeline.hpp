#pragma once

#include <vulkan/vulkan.h>

namespace dilithium {

/// The graphics pipeline for `shaders/color.*`: colored triangles from `ColorVertex` buffers into a color attachment
/// of `colorFormat`, through dynamic rendering (no render pass object). Viewport and scissor are dynamic, so a resize
/// never rebuilds it; only a change of color format does. No blending (D4) and no depth yet. Not copyable or
/// movable: `DefaultRenderer` holds it in place and rebuilds it by assignment of a new one.
class ColorPipeline {
public:
    ColorPipeline(VkDevice device, VkFormat colorFormat);
    ~ColorPipeline();

    ColorPipeline(const ColorPipeline&) = delete;
    ColorPipeline& operator=(const ColorPipeline&) = delete;
    ColorPipeline(ColorPipeline&&) = delete;
    ColorPipeline& operator=(ColorPipeline&&) = delete;

    [[nodiscard]] VkPipeline handle() const { return m_pipeline; }
    [[nodiscard]] VkPipelineLayout layout() const { return m_layout; }
    [[nodiscard]] VkFormat colorFormat() const { return m_colorFormat; }

private:
    VkDevice m_device;
    VkFormat m_colorFormat;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
};

} // namespace dilithium
