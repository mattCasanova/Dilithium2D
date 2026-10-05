#include "gfx/renderers/RenderCore.hpp"

#include "gfx/swapchain/FramesInFlight.hpp"
#include "gfx/swapchain/SwapchainChoices.hpp"
#include "gfx/vulkan/VkCheck.hpp"

#include <dilithium/gfx/Color.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/utilities/Assert.hpp>
#include <dilithium/utilities/Log.hpp>

#include <cstdint>
#include <limits>
#include <memory>

namespace dilithium {
namespace {

constexpr uint64_t kNoTimeout = std::numeric_limits<uint64_t>::max();

struct StageAccess {
    VkPipelineStageFlags2 stage;
    VkAccessFlags2 access;
};

void transition(VkCommandBuffer commands, VkImage image, VkImageLayout from, VkImageLayout to, StageAccess before,
                StageAccess after) {
    const VkImageMemoryBarrier2 barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = before.stage,
        .srcAccessMask = before.access,
        .dstStageMask = after.stage,
        .dstAccessMask = after.access,
        .oldLayout = from,
        .newLayout = to,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    const VkDependencyInfo dependency{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier,
    };
    vkCmdPipelineBarrier2(commands, &dependency);
}

} // namespace

RenderCore::RenderCore(const Window& window)
    : surface(instance.handle(), window), device(instance.handle(), surface.handle()),
      allocator(instance.handle(), device.getPhysical(), device.handle()),
      frames(device.handle(), device.getQueueFamily()) {
    recreateSwapchain(window.getPixelSize());
}

// NOLINTNEXTLINE(bugprone-exception-escape): it logs; std::format's bad_alloc at shutdown may end the program, rightly
RenderCore::~RenderCore() {
    // The GPU may still be using what the members are about to destroy.
    waitIdle();
}

// NOLINTNEXTLINE(bugprone-exception-escape): it logs; std::format's bad_alloc at shutdown may end the program, rightly
void RenderCore::waitIdle() const {
    // Logged rather than sent through VK_CHECK: this runs from destructors, which must not throw.
    const VkResult idle = vkDeviceWaitIdle(device.handle());
    if (idle != VK_SUCCESS) {
        logError("vkDeviceWaitIdle returned {}", describeVkResult(idle));
    }
}

FrameBegin RenderCore::beginFrame(const Window& window, Color clearColor) {
    if (frameOpen) {
        DILITHIUM_UNREACHABLE("beginFrame while a frame is open: endFrame was not called");
    }
    if (window.isMinimized()) {
        return FrameBegin::Idle;
    }
    // A size change the window has already made but whose event has not arrived yet (a resize in this same tick)
    // would otherwise be drawn at the old size and reported as suboptimal at present. Compare, don't wait.
    const PixelSize pixels = window.getPixelSize();
    const bool sizeChanged =
        swapchain && (swapchain->getExtent().width != pixels.width || swapchain->getExtent().height != pixels.height);
    if (!swapchain || swapchainStale || sizeChanged) {
        recreateSwapchain(pixels);
        if (!swapchain) {
            return FrameBegin::Idle;
        }
    }

    const FrameSlot& frame = frames.getCurrent();
    const VkDevice logical = device.handle();
    VK_CHECK(vkWaitForFences(logical, 1, &frame.inFlight, VK_TRUE, kNoTimeout));

    uint32_t imageIndex = 0;
    const VkResult acquired = vkAcquireNextImageKHR(logical, swapchain->handle(), kNoTimeout, frame.imageAvailable,
                                                    VK_NULL_HANDLE, &imageIndex);
    if (acquired == VK_ERROR_OUT_OF_DATE_KHR) {
        // Nothing was acquired: the semaphore stays unsignaled and the fence is still signaled. Rebuild, try again.
        swapchainStale = true;
        return FrameBegin::Skipped;
    }
    if (acquired == VK_SUBOPTIMAL_KHR) {
        // The image WAS acquired and its semaphore will be signaled, so this frame must still be submitted to consume
        // it. Rebuild afterwards.
        swapchainStale = true;
    } else {
        detail::checkVk(acquired, "vkAcquireNextImageKHR");
    }
    // Only now: an early return above must leave the fence signaled, or the next wait on it never returns.
    VK_CHECK(vkResetFences(logical, 1, &frame.inFlight));

    beginCommands(frame, imageIndex, clearColor);
    frameOpen = true;
    frameImageIndex = imageIndex;
    return FrameBegin::Ready;
}

void RenderCore::endFrame() {
    if (!frameOpen) {
        DILITHIUM_UNREACHABLE("endFrame without a frame open");
    }
    const FrameSlot& frame = frames.getCurrent();
    endCommands(frame, frameImageIndex);
    submit(frame, frameImageIndex);
    present(frameImageIndex);
    frames.advance();
    frameOpen = false;
}

VkFormat RenderCore::getColorFormat() const {
    if (!swapchain) {
        DILITHIUM_UNREACHABLE("getColorFormat() with no swapchain");
    }
    return swapchain->getFormat();
}

VkCommandBuffer RenderCore::getCommands() const {
    if (!frameOpen) {
        DILITHIUM_UNREACHABLE("getCommands() outside an open frame");
    }
    return frames.getCurrent().commands;
}

VkExtent2D RenderCore::getExtent() const {
    if (!frameOpen) {
        DILITHIUM_UNREACHABLE("getExtent() outside an open frame");
    }
    return swapchain->getExtent();
}

void RenderCore::recreateSwapchain(PixelSize windowPixels) {
    // Nothing may still be drawing into, or waiting on, what the old swapchain owns.
    VK_CHECK(vkDeviceWaitIdle(device.handle()));
    swapchainStale = false;

    VkSurfaceCapabilitiesKHR capabilities{};
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.getPhysical(), surface.handle(), &capabilities));
    const VkExtent2D extent = chooseExtent(capabilities, {windowPixels.width, windowPixels.height});
    if (extent.width == 0 || extent.height == 0) {
        swapchain.reset(); // no swapchain until the window has area again
        return;
    }
    const VkSwapchainKHR old = swapchain ? swapchain->handle() : VK_NULL_HANDLE;
    // The new swapchain is built from the old one, which assigning then destroys: after the new one exists.
    swapchain =
        std::make_unique<Swapchain>(device.getPhysical(), device.handle(), surface.handle(), capabilities, extent, old);
    ++swapchainBuilds;
}

void RenderCore::beginCommands(const FrameSlot& frame, uint32_t imageIndex, Color clearColor) const {
    VK_CHECK(vkResetCommandPool(device.handle(), frame.pool, 0));
    const VkCommandBufferBeginInfo begin{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    VK_CHECK(vkBeginCommandBuffer(frame.commands, &begin));

    const VkImage image = swapchain->getImage(imageIndex);
    // UNDEFINED, not the old layout: the whole image is about to be cleared, so nothing in it needs keeping. The
    // source stage is the one the acquire semaphore's wait blocks, so the transition runs only after the display
    // engine has handed the image over.
    transition(frame.commands, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
               {VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_NONE},
               {VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT});

    const VkRenderingAttachmentInfo attachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = swapchain->getView(imageIndex),
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {.color = {.float32 = {clearColor.r, clearColor.g, clearColor.b, clearColor.a}}},
    };
    const VkRenderingInfo rendering{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {{0, 0}, swapchain->getExtent()},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachment,
    };
    vkCmdBeginRendering(frame.commands, &rendering);
}

void RenderCore::endCommands(const FrameSlot& frame, uint32_t imageIndex) const {
    vkCmdEndRendering(frame.commands);

    const VkImage image = swapchain->getImage(imageIndex);
    // The destination stage is the one the "rendering finished" signal covers (see submit), so the transition to
    // PRESENT_SRC is done before that semaphore tells the present to go.
    transition(frame.commands, image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
               {VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT},
               {VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_NONE});

    VK_CHECK(vkEndCommandBuffer(frame.commands));
}

void RenderCore::submit(const FrameSlot& frame, uint32_t imageIndex) const {
    const VkSemaphoreSubmitInfo waitForImage{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = frame.imageAvailable,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    };
    const VkCommandBufferSubmitInfo commands{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = frame.commands,
    };
    const VkSemaphoreSubmitInfo signalRenderFinished{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = swapchain->getRenderFinished(imageIndex),
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    };
    const VkSubmitInfo2 info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &waitForImage,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &commands,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &signalRenderFinished,
    };
    VK_CHECK(vkQueueSubmit2(device.getQueue(), 1, &info, frame.inFlight));
}

void RenderCore::present(uint32_t imageIndex) {
    const VkSemaphore renderFinished = swapchain->getRenderFinished(imageIndex);
    const VkSwapchainKHR handle = swapchain->handle();
    const VkPresentInfoKHR info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderFinished,
        .swapchainCount = 1,
        .pSwapchains = &handle,
        .pImageIndices = &imageIndex,
    };
    const VkResult presented = vkQueuePresentKHR(device.getQueue(), &info);
    if (presented == VK_ERROR_OUT_OF_DATE_KHR || presented == VK_SUBOPTIMAL_KHR) {
        swapchainStale = true; // the frame was still submitted; rebuild before the next one
        return;
    }
    detail::checkVk(presented, "vkQueuePresentKHR");
}

} // namespace dilithium
