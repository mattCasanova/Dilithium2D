#include "gfx/shaders/ColorPipeline.hpp"

#include "gfx/renderers/ColorVertex.hpp"
#include "gfx/shaders/ShaderModule.hpp"
#include "gfx/vulkan/VkCheck.hpp"
#include "shaders/color_frag.hpp"
#include "shaders/color_vert.hpp"

#include <vulkan/vulkan.h>

#include <array>
#include <cstdint>

namespace dilithium {
namespace {

// Everything below is fixed for this pipeline, so it is built once, at compile time. The constructor adds only what
// varies: the shader modules, the color format, and the create call.

// One binding, one ColorVertex per vertex; the two attributes are the shader's locations 0 and 1. The offsets and
// stride are the ones ColorVertex.hpp pins with static assertions.
constexpr VkVertexInputBindingDescription kBinding{
    .binding = 0,
    .stride = sizeof(ColorVertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
};
constexpr std::array<VkVertexInputAttributeDescription, 2> kAttributes{{
    {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = kColorVertexPositionOffset},
    {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = kColorVertexColorOffset},
}};
constexpr VkPipelineVertexInputStateCreateInfo kVertexInput{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &kBinding,
    .vertexAttributeDescriptionCount = static_cast<uint32_t>(kAttributes.size()),
    .pVertexAttributeDescriptions = kAttributes.data(),
};
constexpr VkPipelineInputAssemblyStateCreateInfo kInputAssembly{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
};
// Set every frame from the swapchain's extent, so a resize never rebuilds the pipeline.
constexpr VkPipelineViewportStateCreateInfo kViewport{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .scissorCount = 1,
};
constexpr std::array<VkDynamicState, 2> kDynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
constexpr VkPipelineDynamicStateCreateInfo kDynamic{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = static_cast<uint32_t>(kDynamicStates.size()),
    .pDynamicStates = kDynamicStates.data(),
};
// No culling: in 2D a triangle's winding carries no meaning, and the negative-height viewport flips it anyway.
constexpr VkPipelineRasterizationStateCreateInfo kRasterization{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_NONE,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .lineWidth = 1.0f,
};
constexpr VkPipelineMultisampleStateCreateInfo kMultisample{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
};
// Opaque: the color written is the color drawn. Alpha blending arrives with D4's sprites.
constexpr VkPipelineColorBlendAttachmentState kBlendAttachment{
    .blendEnable = VK_FALSE,
    .colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
};
constexpr VkPipelineColorBlendStateCreateInfo kBlend{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments = &kBlendAttachment,
};

VkPipelineShaderStageCreateInfo stage(VkShaderStageFlagBits kind, const ShaderModule& module) {
    return {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = kind,
        .module = module.handle(),
        .pName = "main",
    };
}

} // namespace

ColorPipeline::ColorPipeline(VkDevice newDevice, VkFormat newColorFormat)
    : device(newDevice), colorFormat(newColorFormat) {
    // Nothing to bind yet: no descriptors, no push constants. D3's texture and D5's camera add to this.
    const VkPipelineLayoutCreateInfo layoutInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    VK_CHECK(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &layout));

    const ShaderModule vertex(device, shaders::kColorVert);
    const ShaderModule fragment(device, shaders::kColorFrag);
    const std::array<VkPipelineShaderStageCreateInfo, 2> stages{
        stage(VK_SHADER_STAGE_VERTEX_BIT, vertex),
        stage(VK_SHADER_STAGE_FRAGMENT_BIT, fragment),
    };
    // Dynamic rendering: the attachment formats stand in for a render pass.
    const VkPipelineRenderingCreateInfo rendering{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &colorFormat,
    };
    const VkGraphicsPipelineCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &rendering,
        .stageCount = static_cast<uint32_t>(stages.size()),
        .pStages = stages.data(),
        .pVertexInputState = &kVertexInput,
        .pInputAssemblyState = &kInputAssembly,
        .pViewportState = &kViewport,
        .pRasterizationState = &kRasterization,
        .pMultisampleState = &kMultisample,
        .pColorBlendState = &kBlend,
        .pDynamicState = &kDynamic,
        .layout = layout,
        .renderPass = VK_NULL_HANDLE,
    };
    const VkResult created = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline);
    if (created != VK_SUCCESS) {
        vkDestroyPipelineLayout(device, layout, nullptr); // the destructor will not run
        detail::checkVk(created, "vkCreateGraphicsPipelines");
    }
}

ColorPipeline::~ColorPipeline() {
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, layout, nullptr);
}

} // namespace dilithium
