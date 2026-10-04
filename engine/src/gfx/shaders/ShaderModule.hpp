#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <span>

namespace dilithium {

/// One compiled shader stage, from the SPIR-V words `dilithium_add_shaders` embedded. Only needed while a pipeline
/// is being created; the pipeline keeps its own copy of the code. Move-only.
class ShaderModule {
public:
    ShaderModule(VkDevice device, std::span<const uint32_t> spirv);
    ~ShaderModule();

    ShaderModule(const ShaderModule&) = delete;
    ShaderModule& operator=(const ShaderModule&) = delete;
    ShaderModule(ShaderModule&& other) noexcept;
    ShaderModule& operator=(ShaderModule&& other) noexcept;

    [[nodiscard]] VkShaderModule handle() const { return m_module; }

private:
    void destroy();

    VkDevice m_device = VK_NULL_HANDLE;
    VkShaderModule m_module = VK_NULL_HANDLE;
};

} // namespace dilithium
