#include "gfx/RenderCore.hpp"

#include "gfx/SwapchainChoices.hpp"
#include "gfx/VkCheck.hpp"
#include "platform/Window.hpp"

#include <dilithium/core/Log.hpp>

#include <cstdint>
#include <limits>

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
    : m_surface(m_instance.handle(), window.sdlWindow()), m_device(m_instance.handle(), m_surface.handle()),
      m_allocator(m_instance.handle(), m_device.physical(), m_device.handle()),
      m_frames(m_device.handle(), m_device.queueFamily()) {
    recreateSwapchain(window.pixelSize());
}

RenderCore::~RenderCore() {
    // The GPU may still be using what the members are about to destroy. A destructor must not throw, so a failure
    // here is logged rather than sent through VK_CHECK.
    const VkResult idle = vkDeviceWaitIdle(m_device.handle());
    if (idle != VK_SUCCESS) {
        logError("vkDeviceWaitIdle at shutdown returned {}", describeVkResult(idle));
    }
}

FrameOutcome RenderCore::drawFrame(const Window& window, Color color) {
    if (window.isMinimized()) {
        return FrameOutcome::Idle;
    }
    if (!m_swapchain || m_swapchainStale) {
        recreateSwapchain(window.pixelSize());
        if (!m_swapchain) {
            return FrameOutcome::Idle;
        }
    }

    const FrameSlot& frame = m_frames.current();
    const VkDevice device = m_device.handle();
    VK_CHECK(vkWaitForFences(device, 1, &frame.inFlight, VK_TRUE, kNoTimeout));

    uint32_t imageIndex = 0;
    const VkResult acquired = vkAcquireNextImageKHR(device, m_swapchain->handle(), kNoTimeout, frame.imageAvailable,
                                                    VK_NULL_HANDLE, &imageIndex);
    if (acquired == VK_ERROR_OUT_OF_DATE_KHR) {
        // Nothing was acquired: the semaphore stays unsignaled and the fence is still signaled. Rebuild, try again.
        m_swapchainStale = true;
        return FrameOutcome::Skipped;
    }
    if (acquired == VK_SUBOPTIMAL_KHR) {
        // The image WAS acquired and its semaphore will be signaled, so this frame must still be submitted to consume
        // it. Rebuild afterwards.
        m_swapchainStale = true;
    } else {
        detail::checkVk(acquired, "vkAcquireNextImageKHR");
    }
    // Only now: an early return above must leave the fence signaled, or the next wait on it never returns.
    VK_CHECK(vkResetFences(device, 1, &frame.inFlight));

    recordClear(frame, imageIndex, color);
    submit(frame, imageIndex);
    present(imageIndex);
    m_frames.advance();
    return FrameOutcome::Presented;
}

void RenderCore::recreateSwapchain(PixelSize windowPixels) {
    // Nothing may still be drawing into, or waiting on, what the old swapchain owns.
    VK_CHECK(vkDeviceWaitIdle(m_device.handle()));
    m_swapchainStale = false;

    VkSurfaceCapabilitiesKHR capabilities{};
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device.physical(), m_surface.handle(), &capabilities));
    const VkExtent2D extent = chooseExtent(capabilities, {windowPixels.width, windowPixels.height});
    if (extent.width == 0 || extent.height == 0) {
        m_swapchain.reset(); // no swapchain until the window has area again
        return;
    }
    const VkSwapchainKHR old = m_swapchain ? m_swapchain->handle() : VK_NULL_HANDLE;
    // The new swapchain is built from the old one, which assigning then destroys: after the new one exists.
    m_swapchain = std::make_unique<Swapchain>(m_device.physical(), m_device.handle(), m_surface.handle(), capabilities,
                                              extent, old);
    ++m_swapchainBuilds;
}

void RenderCore::recordClear(const FrameSlot& frame, uint32_t imageIndex, Color color) const {
    VK_CHECK(vkResetCommandPool(m_device.handle(), frame.pool, 0));
    const VkCommandBufferBeginInfo begin{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    VK_CHECK(vkBeginCommandBuffer(frame.commands, &begin));

    const VkImage image = m_swapchain->image(imageIndex);
    // UNDEFINED, not the old layout: the whole image is about to be cleared, so nothing in it needs keeping. The
    // source stage is the one the acquire semaphore's wait blocks, so the transition runs only after the display
    // engine has handed the image over.
    transition(frame.commands, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
               {VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_NONE},
               {VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT});

    const VkRenderingAttachmentInfo attachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = m_swapchain->view(imageIndex),
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {.color = {.float32 = {color.r, color.g, color.b, color.a}}},
    };
    const VkRenderingInfo rendering{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {{0, 0}, m_swapchain->extent()},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachment,
    };
    vkCmdBeginRendering(frame.commands, &rendering);
    vkCmdEndRendering(frame.commands);

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
        .semaphore = m_swapchain->renderFinished(imageIndex),
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
    VK_CHECK(vkQueueSubmit2(m_device.queue(), 1, &info, frame.inFlight));
}

void RenderCore::present(uint32_t imageIndex) {
    const VkSemaphore renderFinished = m_swapchain->renderFinished(imageIndex);
    const VkSwapchainKHR swapchain = m_swapchain->handle();
    const VkPresentInfoKHR info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderFinished,
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &imageIndex,
    };
    const VkResult presented = vkQueuePresentKHR(m_device.queue(), &info);
    if (presented == VK_ERROR_OUT_OF_DATE_KHR || presented == VK_SUBOPTIMAL_KHR) {
        m_swapchainStale = true; // the frame was still submitted; rebuild before the next one
        return;
    }
    detail::checkVk(presented, "vkQueuePresentKHR");
}

} // namespace dilithium
