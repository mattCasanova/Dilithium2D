#include "gfx/shaders/ShaderModule.hpp"

#include "gfx/vulkan/VkCheck.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <span>
#include <utility>

namespace dilithium {

ShaderModule::ShaderModule(VkDevice newDevice, std::span<const uint32_t> spirv) : device(newDevice) {
    const VkShaderModuleCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirv.size_bytes(),
        .pCode = spirv.data(),
    };
    VK_CHECK(vkCreateShaderModule(device, &info, nullptr, &module));
}

ShaderModule::~ShaderModule() {
    destroy();
}

ShaderModule::ShaderModule(ShaderModule&& other) noexcept
    : device(other.device), module(std::exchange(other.module, VK_NULL_HANDLE)) {}

ShaderModule& ShaderModule::operator=(ShaderModule&& other) noexcept {
    if (this != &other) {
        destroy();
        device = other.device;
        module = std::exchange(other.module, VK_NULL_HANDLE);
    }
    return *this;
}

void ShaderModule::destroy() {
    if (module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device, module, nullptr);
        module = VK_NULL_HANDLE;
    }
}

} // namespace dilithium
