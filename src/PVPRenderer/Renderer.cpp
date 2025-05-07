#include "Renderer.h"

#include <globalconst.h>
#include <stdexcept>
#include <PVPDevice/Device.h>

pvp::Renderer::Renderer(const Context& context, Swapchain& swapchain)
    : m_context{ context }
    , m_swapchain{ swapchain }
{
    m_frame_syncers = FrameSyncers(m_context);
    m_destructor_queue.add_to_queue([&] { m_frame_syncers.destroy(m_context.device->get_device()); });

    m_cmd_pool_graphics_present = CommandPool(m_context, *context.queue_families->get_queue_family(VK_QUEUE_GRAPHICS_BIT, true), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    m_destructor_queue.add_to_queue([&] { m_cmd_pool_graphics_present.destroy(); });

    m_cmds_graphics = (m_cmd_pool_graphics_present.allocate_buffers(MAX_FRAMES_IN_FLIGHT));
}
void pvp::Renderer::prepare_frame()
{
    // TODO: Move everything into renderer
    vkWaitForFences(m_context.device->get_device(), 1, &m_frame_syncers.in_flight_fences[m_double_buffer_frame].handle, VK_TRUE, UINT64_MAX);
    vkResetFences(m_context.device->get_device(), 1, &m_frame_syncers.in_flight_fences[m_double_buffer_frame].handle);

    VkResult result = vkAcquireNextImageKHR(
        m_context.device->get_device(),
        m_swapchain.get_swapchain(),
        UINT64_MAX,
        m_frame_syncers.image_available_semaphores[m_double_buffer_frame].handle,
        VK_NULL_HANDLE,
        &m_current_swapchain_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        m_swapchain.recreate_swapchain();
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    
}
void pvp::Renderer::end_frame()
{
    

    VkSemaphoreSubmitInfo semaphore_submit{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = m_frame_syncers.image_available_semaphores[m_double_buffer_frame].handle,
    };

    // VkCommandBufferSubmitInfo cmd_submit_info{
    //     .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
    //     .commandBuffer = cmd,
    // };
    //
    VkSemaphoreSubmitInfo semaphore_submit_singled{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = m_frame_syncers.render_finished_semaphores[m_double_buffer_frame].handle,
    };

    VkSubmitInfo2 submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &semaphore_submit,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmd_submit_info,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &semaphore_submit_singled
    };

    vkQueueSubmit2(m_cmd_pool_graphics_present.get_queue().queue, 1, &submit_info, m_frame_syncers.in_flight_fences[m_double_buffer_frame].handle);

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &m_frame_syncers.render_finished_semaphores[m_double_buffer_frame].handle;

    VkSwapchainKHR swap_chains[] = { m_swapchain.get_swapchain() };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &m_current_swapchain_index;

    VkResult result = vkQueuePresentKHR(m_cmd_pool_graphics_present.get_queue().queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        // m_pvp_swapchain->recreate_swapchain(*m_device, *m_command_buffer, m_pvp_render_pass);
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_double_buffer_frame = (m_double_buffer_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}