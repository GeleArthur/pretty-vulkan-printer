#include "Renderer.h"

#include <globalconst.h>
#include <stdexcept>
#include <CommandBuffer/CommandPool.h>
#include <Context/Device.h>
#include <Image/TransitionLayout.h>
#include <spdlog/spdlog.h>

#include "BlitToSwapchain.h"
#include "DepthPrePass.h"
#include "GlfwToRender.h"
#include "../ImguiRenderer.h"
#include "ToneMappingPass.h"

#include <VulkanExternalFunctions.h>
#include <imgui.h>
#include <Context/PhysicalDevice.h>
#include <DescriptorSets/DescriptorLayoutBuilder.h>
#include <Scene/PVPScene.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

pvp::Renderer::Renderer(Context& context, PvpScene& scene, ImguiRenderer& imgui_renderer)
    : m_context{ context }
    , m_scene{ scene }
    , m_depth_pre_pass{ context, scene }
    , m_geometry_draw{ context, scene, m_depth_pre_pass }
    , m_light_pass{ context, scene, m_geometry_draw, m_depth_pre_pass }
    , m_tone_mapping_pass{ context, m_light_pass }
    , m_imgui_renderer{ imgui_renderer }
    , m_blit_to_swapchain{ context, m_tone_mapping_pass.get_tone_mapped_texture() }
    , m_mesh_shader_pass{ context, scene }
{
    ZoneScoped;
    m_frame_syncers = FrameSyncers(m_context);
    m_destructor_queue.add_to_queue([&] { m_frame_syncers.destroy(m_context.device->get_device()); });

    m_cmd_pool_graphics_present = CommandPool(m_context, *context.queue_families->get_queue_family(VK_QUEUE_GRAPHICS_BIT, true), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    m_destructor_queue.add_to_queue([&] { m_cmd_pool_graphics_present.destroy(); });

    const std::vector<VkCommandBuffer> buffers = m_cmd_pool_graphics_present.allocate_buffers(max_frames_in_flight);

    for (int i = 0; i < max_frames_in_flight; ++i)
    {
        m_frame_contexts[i].buffer_index = i;
        m_frame_contexts[i].command_buffer = buffers[i];
#ifdef TRACY_ENABLE
        tracy::VkCtx* context_tracy = TracyVkContext(context.physical_device->get_physical_device(), context.device->get_device(), context.queue_families->get_queue_family(VK_QUEUE_GRAPHICS_BIT, true)->queue, m_frame_contexts[i].command_buffer);
        m_context.tracy_ctx.push_back(context_tracy);
        m_destructor_queue.add_to_queue([=] { TracyVkDestroy(context_tracy) });
        TracyVkContextName(context_tracy, "VULKAN", 1);
#endif
    }

    m_imgui_renderer.setup_vulkan_context(m_cmd_pool_graphics_present);
    m_destructor_queue.add_to_queue([&] { m_imgui_renderer.destroy_vulkan_context(); });
}

void pvp::Renderer::prepare_frame()
{
    ZoneScoped;

    ZoneNamedN(recreate_swapchain, "recreate_swapchain", true);
    if (m_context.gtfw_to_render->needs_resizing)
    {
        m_context.swapchain->recreate_swapchain();
        m_imgui_renderer.update_screen();
        m_context.gtfw_to_render->needs_resizing.store(false);
        m_frame_syncers.destroy(m_context.device->get_device());
        m_frame_syncers = FrameSyncers(m_context);
    }

    ZoneNamedN(waiting, "waiting", true);
    vkWaitForFences(m_context.device->get_device(), 1, &m_frame_syncers.in_flight_fences[m_double_buffer_frame].handle, VK_TRUE, UINT64_MAX);
    vkResetFences(m_context.device->get_device(), 1, &m_frame_syncers.in_flight_fences[m_double_buffer_frame].handle);

    ZoneNamedN(update_renderer, "update renderer", true);
    m_scene.update_render(m_frame_contexts[m_double_buffer_frame]);

    ZoneNamedN(AcquireNextImage, "Acquire Next Image", true);

    VkResult result{ VK_RESULT_MAX_ENUM };
    int      count{};
    while (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        result = vkAcquireNextImageKHR(
            m_context.device->get_device(),
            m_context.swapchain->get_swapchain(),
            UINT64_MAX,
            m_frame_syncers.acquire_semaphores.at(m_double_buffer_frame).handle,
            VK_NULL_HANDLE,
            &m_current_swapchain_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            return;
        }

        count++;
        if (count > 10)
        {
            throw std::runtime_error("failed to acquire swapchain image");
        }
    }

    ZoneNamedN(BeginCommandBuffer, "Begin Command Buffer", true);
    VkCommandBufferBeginInfo start_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vkBeginCommandBuffer(m_frame_contexts[m_double_buffer_frame].command_buffer, &start_info);
    TracyVkZone(m_context.tracy_ctx[m_double_buffer_frame], m_frame_contexts[m_double_buffer_frame].command_buffer, "Begin");

    ZoneNamedN(viewport_scisor, "VkViewport scissor", true);
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_context.swapchain->get_swapchain_extent().width);
    viewport.height = static_cast<float>(m_context.swapchain->get_swapchain_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_frame_contexts[m_double_buffer_frame].command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_context.swapchain->get_swapchain_extent();
    vkCmdSetScissor(m_frame_contexts[m_double_buffer_frame].command_buffer, 0, 1, &scissor);
}

void pvp::Renderer::draw()
{
    ZoneScoped;
    prepare_frame();
    m_depth_pre_pass.draw(m_frame_contexts[m_double_buffer_frame]);
    m_geometry_draw.draw(m_frame_contexts[m_double_buffer_frame]);
    m_light_pass.draw(m_frame_contexts[m_double_buffer_frame]);
    m_tone_mapping_pass.draw(m_frame_contexts[m_double_buffer_frame]);

    m_blit_to_swapchain.draw(m_frame_contexts[m_double_buffer_frame], m_current_swapchain_index);
    // m_mesh_shader_pass.draw(m_frame_contexts[m_double_buffer_frame], m_current_swapchain_index);
    m_imgui_renderer.draw(m_frame_contexts[m_double_buffer_frame], m_current_swapchain_index);
    TracyVkCollect(m_context.tracy_ctx[m_double_buffer_frame], m_frame_contexts[m_double_buffer_frame].command_buffer);
    end_frame();
}

void pvp::Renderer::end_frame()
{
    ZoneNamedN(end_command_buffer, "end command buffer", true);
    vkEndCommandBuffer(m_frame_contexts.at(m_double_buffer_frame).command_buffer);

    ZoneNamedN(submit_queue, "submit queue", true);
    VkSemaphoreSubmitInfo acquire_semaphores{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = m_frame_syncers.acquire_semaphores.at(m_double_buffer_frame).handle,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    };

    std::array cmd_submit_info{
        VkCommandBufferSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = m_frame_contexts.at(m_double_buffer_frame).command_buffer,
        },
        // VkCommandBufferSubmitInfo{
        // .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        // .commandBuffer = m_imgui_renderer.get_cmd(m_double_buffer_frame),
        // }
    };

    VkSemaphoreSubmitInfo semaphore_singled{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = m_frame_syncers.submit_semaphores[m_current_swapchain_index].handle,
        .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT
    };

    VkSubmitInfo2 submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &acquire_semaphores,

        .commandBufferInfoCount = cmd_submit_info.size(),
        .pCommandBufferInfos = cmd_submit_info.data(),

        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &semaphore_singled,

    };

    vkQueueSubmit2(m_cmd_pool_graphics_present.get_queue().queue,
                   1,
                   &submit_info,
                   m_frame_syncers.in_flight_fences[m_double_buffer_frame].handle);

    ZoneNamedN(present, "Queue present", true);
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &m_frame_syncers.submit_semaphores[m_current_swapchain_index].handle;

    VkSwapchainKHR swap_chains[] = { m_context.swapchain->get_swapchain() };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &m_current_swapchain_index;

    VkResult result = vkQueuePresentKHR(m_cmd_pool_graphics_present.get_queue().queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_context.gtfw_to_render->needs_resizing)
    {
        m_context.swapchain->recreate_swapchain();
        m_imgui_renderer.update_screen();
        m_context.gtfw_to_render->needs_resizing.store(false);
        m_frame_syncers.destroy(m_context.device->get_device());
        m_frame_syncers = FrameSyncers(m_context);
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_double_buffer_frame = (m_double_buffer_frame + 1) % max_frames_in_flight;
}
