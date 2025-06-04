#include "Renderer.h"

#include <globalconst.h>
#include <stdexcept>
#include <CommandBuffer/CommandPool.h>
#include <Context/Device.h>
#include <Image/TransitionLayout.h>
#include <spdlog/spdlog.h>

#include "BlitToSwapchain.h"
#include "DepthPrePass.h"
#include "ToneMappingPass.h"

#include <VulkanExternalFunctions.h>
#include <DescriptorSets/DescriptorLayoutBuilder.h>
#include <Scene/PVPScene.h>
#include <VMAAllocator/VmaAllocator.h>

pvp::Renderer::Renderer(const Context& context, Swapchain& swapchain, const PvpScene& scene)
    : m_context{ context }
    , m_swapchain{ swapchain }
    , m_scene{ scene }
{
    m_frame_syncers = FrameSyncers(m_context);
    m_destructor_queue.add_to_queue([&] { m_frame_syncers.destroy(m_context.device->get_device()); });

    m_cmd_pool_graphics_present = CommandPool(m_context, *context.queue_families->get_queue_family(VK_QUEUE_GRAPHICS_BIT, true), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    m_destructor_queue.add_to_queue([&] { m_cmd_pool_graphics_present.destroy(); });

    m_cmds_graphics = (m_cmd_pool_graphics_present.allocate_buffers(MAX_FRAMES_IN_FLIGHT));

    m_depth_pre_pass = new DepthPrePass(m_context, m_scene);
    m_destructor_queue.add_to_queue([&] { delete m_depth_pre_pass; });

    m_geometry_draw = new GBuffer(m_context, scene, *m_depth_pre_pass);
    m_destructor_queue.add_to_queue([&] { delete m_geometry_draw; });

    m_light_pass = new LightPass(m_context, m_scene, *m_geometry_draw, *m_depth_pre_pass);
    m_destructor_queue.add_to_queue([&] { delete m_light_pass; });

    m_tone_mapping_pass = new ToneMappingPass(m_context, *m_light_pass);
    m_destructor_queue.add_to_queue([&] { delete m_tone_mapping_pass; });

    m_blit_to_swapchain = new BlitToSwapchain(m_context, swapchain, m_tone_mapping_pass->get_tone_mapped_texture());
    m_destructor_queue.add_to_queue([&] { delete m_blit_to_swapchain; });
}

void pvp::Renderer::prepare_frame()
{
    vkWaitForFences(m_context.device->get_device(), 1, &m_frame_syncers.in_flight_fences[m_double_buffer_frame].handle, VK_TRUE, UINT64_MAX);
    vkResetFences(m_context.device->get_device(), 1, &m_frame_syncers.in_flight_fences[m_double_buffer_frame].handle);

    m_scene.update_render();

    VkResult result = vkAcquireNextImageKHR(
        m_context.device->get_device(),
        m_swapchain.get_swapchain(),
        UINT64_MAX,
        m_frame_syncers.image_available_semaphores[m_double_buffer_frame].handle,
        VK_NULL_HANDLE,
        &m_current_swapchain_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // m_swapchain.recreate_swapchain();
        throw std::runtime_error("failed to present swap chain image!");
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    VkCommandBufferBeginInfo start_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vkBeginCommandBuffer(m_cmds_graphics[m_double_buffer_frame], &start_info);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapchain.get_swapchain_extent().width);
    viewport.height = static_cast<float>(m_swapchain.get_swapchain_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_cmds_graphics[m_double_buffer_frame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapchain.get_swapchain_extent();
    vkCmdSetScissor(m_cmds_graphics[m_double_buffer_frame], 0, 1, &scissor);
}

void pvp::Renderer::draw()
{
    prepare_frame();
    // VkDebugUtilsLabelEXT yea{ .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, .pLabelName = "Depth prepass", .color = { 0.0, 0.0, 0.0, 1.0 } };
    // VulkanInstanceExtensions::vkCmdBeginDebugUtilsLabelEXT(m_cmds_graphics[m_double_buffer_frame], &yea);
    m_depth_pre_pass->draw(m_cmds_graphics[m_double_buffer_frame]);
    // VulkanInstanceExtensions::vkCmdEndDebugUtilsLabelEXT(m_cmds_graphics[m_double_buffer_frame]);
    m_geometry_draw->draw(m_cmds_graphics[m_double_buffer_frame]);
    m_light_pass->draw(m_cmds_graphics[m_double_buffer_frame]);
    m_tone_mapping_pass->draw(m_cmds_graphics[m_double_buffer_frame]);
    m_blit_to_swapchain->draw(m_cmds_graphics[m_double_buffer_frame], m_current_swapchain_index);
    end_frame();
}

void pvp::Renderer::end_frame()
{
    vkEndCommandBuffer(m_cmds_graphics[m_double_buffer_frame]);

    VkSemaphoreSubmitInfo semaphore_wait_for{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = m_frame_syncers.image_available_semaphores[m_double_buffer_frame].handle,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    };

    VkCommandBufferSubmitInfo cmd_submit_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = m_cmds_graphics[m_double_buffer_frame],
    };

    VkSemaphoreSubmitInfo semaphore_singled{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = m_frame_syncers.render_finished_semaphores[m_double_buffer_frame].handle,
        .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT
    };

    VkSubmitInfo2 submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &semaphore_wait_for,

        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmd_submit_info,

        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &semaphore_singled,

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
        // m_swapchain.recreate_swapchain();
        throw std::runtime_error("failed to present swap chain image!");
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_double_buffer_frame = (m_double_buffer_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}
