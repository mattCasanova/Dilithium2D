#include "gfx/vulkan/VkCheck.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <vector>

using dilithium::describeVkResult;

TEST_CASE("describeVkResult names the result and gives its number", "[vkcheck]") {
    CHECK(describeVkResult(VK_SUCCESS) == "VK_SUCCESS (0)");
    CHECK(describeVkResult(VK_ERROR_DEVICE_LOST) == "VK_ERROR_DEVICE_LOST (-4)");
    CHECK(describeVkResult(VK_ERROR_OUT_OF_DATE_KHR) == "VK_ERROR_OUT_OF_DATE_KHR (-1000001004)");
    CHECK(describeVkResult(VK_SUBOPTIMAL_KHR) == "VK_SUBOPTIMAL_KHR (1000001003)");
}

TEST_CASE("describeVkResult still prints a result newer than its list", "[vkcheck]") {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange): a result newer than the list is the point
    CHECK(describeVkResult(static_cast<VkResult>(1234567)) == "VkResult (1234567)");
}

TEST_CASE("VK_CHECK lets VK_SUCCESS through", "[vkcheck]") {
    VK_CHECK(VK_SUCCESS);
    SUCCEED();
}

TEST_CASE("vkEnumerate asks for the count, then fills the list", "[vkcheck]") {
    const std::vector<int> source{3, 1, 4};
    const auto items = dilithium::vkEnumerate<int>("fakeEnumerate", [&](uint32_t* count, int* out) {
        if (out == nullptr) {
            *count = static_cast<uint32_t>(source.size());
            return VK_SUCCESS;
        }
        for (uint32_t index = 0; index < *count; ++index) {
            out[index] = source[index];
        }
        return VK_SUCCESS;
    });
    CHECK(items == source);
}

TEST_CASE("vkEnumerate starts again when the list grows between the two calls", "[vkcheck]") {
    int calls = 0;
    const auto items = dilithium::vkEnumerate<int>("fakeEnumerate", [&](uint32_t* count, int* out) {
        ++calls;
        const uint32_t available = calls >= 2 ? 2 : 1; // a second item appears right after the count
        if (out == nullptr) {
            *count = available;
            return VK_SUCCESS;
        }
        for (uint32_t index = 0; index < *count; ++index) {
            out[index] = static_cast<int>(index);
        }
        return *count < available ? VK_INCOMPLETE : VK_SUCCESS;
    });
    CHECK(items == std::vector<int>{0, 1});
}

#ifndef DILITHIUM_DEBUG
TEST_CASE("VK_CHECK throws VulkanError with the result in release", "[vkcheck]") {
    try {
        VK_CHECK(VK_ERROR_OUT_OF_HOST_MEMORY);
        FAIL("VK_CHECK did not throw");
    } catch (const dilithium::VulkanError& error) {
        CHECK(error.result() == VK_ERROR_OUT_OF_HOST_MEMORY);
    }
}

TEST_CASE("VK_CHECK fails on any result but VK_SUCCESS, positive ones too", "[vkcheck]") {
    CHECK_THROWS_AS(VK_CHECK(VK_INCOMPLETE), dilithium::VulkanError);
    CHECK_THROWS_AS(VK_CHECK(VK_SUBOPTIMAL_KHR), dilithium::VulkanError);
}

#endif
