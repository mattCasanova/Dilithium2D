#include "gfx/shaders/ShaderModule.hpp"

#include "gfx/vulkan/VkCheck.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <span>
#include <utility>

namespace dilithium {

ShaderModule::ShaderModule(VkDevice device, std::span<const uint32_t> spirv) : m_device(device) {
    const VkShaderModuleCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirv.size_bytes(),
        .pCode = spirv.data(),
    };
    VK_CHECK(vkCreateShaderModule(device, &info, nullptr, &m_module));
}

ShaderModule::~ShaderModule() {
    destroy();
}

ShaderModule::ShaderModule(ShaderModule&& other) noexcept
    : m_device(other.m_device), m_module(std::exchange(other.m_module, VK_NULL_HANDLE)) {}

ShaderModule& ShaderModule::operator=(ShaderModule&& other) noexcept {
    if (this != &other) {
        destroy();
        m_device = other.m_device;
        m_module = std::exchange(other.m_module, VK_NULL_HANDLE);
    }
    return *this;
}

void ShaderModule::destroy() {
    if (m_module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(m_device, m_module, nullptr);
        m_module = VK_NULL_HANDLE;
    }
}

} // namespace dilithium
