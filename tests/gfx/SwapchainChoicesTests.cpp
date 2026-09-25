#include "gfx/SwapchainChoices.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <limits>
#include <utility>

using dilithium::chooseCompositeAlpha;
using dilithium::chooseExtent;
using dilithium::chooseImageCount;
using dilithium::choosePresentMode;
using dilithium::chooseSurfaceFormat;

namespace {

constexpr uint32_t kSwapchainDecides = std::numeric_limits<uint32_t>::max();

/// What MoltenVK 1.4.2 reports for a window on the Apple M4 Max (vulkaninfo, 2026-09-25).
VkSurfaceCapabilitiesKHR moltenVkCapabilities() {
    VkSurfaceCapabilitiesKHR capabilities{};
    capabilities.minImageCount = 2;
    capabilities.maxImageCount = 3;
    capabilities.currentExtent = {512, 512};
    capabilities.minImageExtent = {1, 1};
    capabilities.maxImageExtent = {16384, 16384};
    capabilities.maxImageArrayLayers = 1;
    capabilities.supportedTransforms = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    capabilities.currentTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    capabilities.supportedCompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
                                           VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR |
                                           VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    capabilities.supportedUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                       VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    return capabilities;
}

VkSurfaceCapabilitiesKHR swapchainDecides(VkExtent2D minExtent, VkExtent2D maxExtent) {
    VkSurfaceCapabilitiesKHR capabilities = moltenVkCapabilities();
    capabilities.currentExtent = {kSwapchainDecides, kSwapchainDecides};
    capabilities.minImageExtent = minExtent;
    capabilities.maxImageExtent = maxExtent;
    return capabilities;
}

VkSurfaceCapabilitiesKHR imageCounts(uint32_t minCount, uint32_t maxCount) {
    VkSurfaceCapabilitiesKHR capabilities = moltenVkCapabilities();
    capabilities.minImageCount = minCount;
    capabilities.maxImageCount = maxCount;
    return capabilities;
}

std::pair<uint32_t, uint32_t> sizeOf(VkExtent2D extent) {
    return {extent.width, extent.height};
}

} // namespace

TEST_CASE("chooseSurfaceFormat takes UNORM + sRGB wherever it sits in the list", "[swapchain]") {
    const std::array offered{
        VkSurfaceFormatKHR{VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT},
        VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
    };
    const auto chosen = chooseSurfaceFormat(offered);
    REQUIRE(chosen.has_value());
    CHECK(chosen->format == VK_FORMAT_B8G8R8A8_UNORM);
    CHECK(chosen->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
}

TEST_CASE("chooseSurfaceFormat falls back to the first format offered", "[swapchain]") {
    const std::array offered{
        VkSurfaceFormatKHR{VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
    };
    const auto chosen = chooseSurfaceFormat(offered);
    REQUIRE(chosen.has_value());
    CHECK(chosen->format == VK_FORMAT_R16G16B16A16_SFLOAT);
}

TEST_CASE("chooseSurfaceFormat with nothing offered is nullopt", "[swapchain]") {
    CHECK_FALSE(chooseSurfaceFormat({}).has_value());
}

TEST_CASE("choosePresentMode picks FIFO whenever it is offered", "[swapchain]") {
    const std::array offered{VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR};
    CHECK(choosePresentMode(offered) == VK_PRESENT_MODE_FIFO_KHR);
}

TEST_CASE("choosePresentMode without FIFO is nullopt", "[swapchain]") {
    const std::array offered{VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR};
    CHECK_FALSE(choosePresentMode(offered).has_value());
    CHECK_FALSE(choosePresentMode({}).has_value());
}

TEST_CASE("chooseExtent uses a fixed currentExtent over the window size", "[swapchain]") {
    CHECK(sizeOf(chooseExtent(moltenVkCapabilities(), {1280, 720})) == std::pair{512u, 512u});
}

TEST_CASE("chooseExtent clamps the window size when the swapchain decides", "[swapchain]") {
    const VkSurfaceCapabilitiesKHR capabilities = swapchainDecides({100, 100}, {4096, 2048});
    CHECK(sizeOf(chooseExtent(capabilities, {1280, 720})) == std::pair{1280u, 720u});
    CHECK(sizeOf(chooseExtent(capabilities, {50, 720})) == std::pair{100u, 720u});
    CHECK(sizeOf(chooseExtent(capabilities, {5000, 3000})) == std::pair{4096u, 2048u});
}

TEST_CASE("chooseExtent gives zero for a window with a zero side", "[swapchain]") {
    const VkSurfaceCapabilitiesKHR capabilities = swapchainDecides({1, 1}, {16384, 16384});
    CHECK(sizeOf(chooseExtent(capabilities, {0, 720})) == std::pair{0u, 0u});
    CHECK(sizeOf(chooseExtent(capabilities, {1280, 0})) == std::pair{0u, 0u});
    CHECK(sizeOf(chooseExtent(capabilities, {0, 0})) == std::pair{0u, 0u});
}

TEST_CASE("chooseImageCount asks for one more than the minimum, within the maximum", "[swapchain]") {
    CHECK(chooseImageCount(imageCounts(2, 3)) == 3);
    CHECK(chooseImageCount(imageCounts(1, 8)) == 2);
    CHECK(chooseImageCount(imageCounts(3, 3)) == 3);
}

TEST_CASE("chooseImageCount treats a maximum of 0 as no limit", "[swapchain]") {
    CHECK(chooseImageCount(imageCounts(2, 0)) == 3);
}

TEST_CASE("chooseCompositeAlpha prefers OPAQUE, else the lowest supported bit", "[swapchain]") {
    CHECK(chooseCompositeAlpha(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR | VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR |
                               VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) == VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
    CHECK(chooseCompositeAlpha(VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR | VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ==
          VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR);
    CHECK(chooseCompositeAlpha(VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR | VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) ==
          VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR);
}

TEST_CASE("chooseCompositeAlpha with no supported bit is nullopt", "[swapchain]") {
    CHECK_FALSE(chooseCompositeAlpha(0).has_value());
}

TEST_CASE("MoltenVK's real surface gets UNORM + sRGB, FIFO, 3 images and OPAQUE", "[swapchain]") {
    const VkSurfaceCapabilitiesKHR capabilities = moltenVkCapabilities();
    // The first two of the 60 formats MoltenVK offers, in its order.
    const std::array formats{
        VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
    };
    const std::array presentModes{VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR};

    const auto format = chooseSurfaceFormat(formats);
    REQUIRE(format.has_value());
    CHECK(format->format == VK_FORMAT_B8G8R8A8_UNORM);
    CHECK(choosePresentMode(presentModes) == VK_PRESENT_MODE_FIFO_KHR);
    CHECK(chooseImageCount(capabilities) == 3);
    CHECK(chooseCompositeAlpha(capabilities.supportedCompositeAlpha) == VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
}
