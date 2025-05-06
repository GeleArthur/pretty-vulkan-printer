#include "App.h"

#include "PVPPhysicalDevice/LogicPhysicalQueueBuilder.h"

#include <GLFW/glfw3.h>
#include <PVPGraphicsPipeline/GraphicsPipelineBuilder.h>
#include <PVPGraphicsPipeline/PipelineLayoutBuilder.h>
#include <PVPGraphicsPipeline/ShaderLoader.h>
#include <PVPImage/TextureBuilder.h>
#include <PVPInstance/InstanceBuilder.h>
#include <PVPRenderer/Swapchain.h>
#include <PVPSyncManager/FrameSyncers.h>
#include <PVPWindow/WindowSurfaceBuilder.h>
#include <glm/gtx/quaternion.hpp>

void pvp::App::run()
{
    glfwInit();
    m_destructor_queue.add_to_queue([&] {
        glfwTerminate();
    });

    InstanceBuilder()
        .enable_debugging(true)
        .set_app_name("pretty vulkan printer")
        .build(m_instance);
    m_destructor_queue.add_to_queue([&] { m_instance.destroy(); });

    WindowSurfaceBuilder()
        .set_window_size(800, 600)
        .set_window_title("pretty vulkan printer")
        .build(m_instance, m_window_surface);
    m_destructor_queue.add_to_queue([&] { m_window_surface.destroy(m_instance); });

    LogicPhysicalQueueBuilder()
        .set_extensions({ VK_EXT_SHADER_OBJECT_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME })
        .build(m_instance, m_window_surface, m_physical_device, m_device, m_queue_families);
    m_destructor_queue.add_to_queue([&] { m_device.destroy(); });

    create_allocator(m_allocator, m_instance, m_device, m_physical_device);
    m_destructor_queue.add_to_queue([&] { m_allocator.destroy(); });

    m_context.instance = &m_instance;
    m_context.physical_device = &m_physical_device;
    m_context.device = &m_device;
    m_context.allocator = &m_allocator;
    m_context.queue_families = &m_queue_families;

    m_swapchain = new Swapchain(m_context, m_window_surface);
    m_destructor_queue.add_to_queue([&] { delete m_swapchain; });

    // m_sync_builder = new SyncBuilder(m_device->get_device());
    // m_destructor_queue.add_to_queue([&] {
    //     delete m_sync_builder;
    // });
    //
    DescriptorLayout layout = DescriptorLayout(
        m_device.get_device(),
        {
            VkDescriptorSetLayoutBinding{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
            // VkDescriptorSetLayoutBinding{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
        });

    m_descriptor_pool = DescriptorPool(m_device.get_device(), { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 } }, 2);
    m_destructor_queue.add_to_queue([&] { m_descriptor_pool.destroy(); });

    m_uniform_buffer = new UniformBuffer<ModelCameraViewData>(m_context.allocator->get_allocator());
    m_destructor_queue.add_to_queue([&] { delete m_uniform_buffer; });

    m_pipeline_layout = PipelineLayoutBuilder().add_descriptor_layout(layout.get_handle()).build(m_device.get_device());
    m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_device.get_device(), m_pipeline_layout, nullptr); });

    // TextureBuilder().set_path("resources/viking_room.png").build(m_device.get_device(), *m_command_buffer, m_texture);
    // m_destructor_queue.add_to_queue([&] {
    //     m_texture.destroy(m_context);
    // });

    // SamplerBuilder().build(m_device->get_device(), m_sampler);
    // m_destructor_queue.add_to_queue([&] {
    //     m_sampler.destroy(m_device->get_device());
    // });
    //

    m_descriptors = DescriptorSetBuilder()
                        .set_layout(layout)
                        .bind_buffer(0, *m_uniform_buffer)
                        // .bind_image(1, m_texture, m_sampler)
                        .build(m_device.get_device(), m_descriptor_pool);

    // Shader loading
    auto vertex_shader = ShaderLoader::load_shader_from_file(m_device.get_device(), "shaders/shader.vert.spv");
    auto fragment_shader = ShaderLoader::load_shader_from_file(m_device.get_device(), "shaders/shader.frag.spv");

    m_graphics_pipeline = GraphicsPipelineBuilder()
                              .add_shader(vertex_shader, VK_SHADER_STAGE_VERTEX_BIT)
                              .add_shader(fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT)
                              .set_pipeline_layout(m_pipeline_layout)
                              .set_input_attribute_description(Vertex::get_attribute_descriptions())
                              .set_input_binding_description(Vertex::get_binding_description())
                              .set_color_format(std::array{ m_swapchain->get_swapchain_surface_format().format })
                              .set_depth_buffer(VK_FORMAT_D32_SFLOAT_S8_UINT)
                              .build(m_device);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_device.get_device(), m_graphics_pipeline, nullptr); });

    vkDestroyShaderModule(m_device.get_device(), vertex_shader, nullptr);
    vkDestroyShaderModule(m_device.get_device(), fragment_shader, nullptr);

    m_cmd_pool_transfer_buffers = CommandPool(m_context, *m_queue_families.get_queue_family(VK_QUEUE_TRANSFER_BIT, false), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    m_destructor_queue.add_to_queue([&] { m_cmd_pool_transfer_buffers.destroy(); });
    VkCommandBuffer cmd = m_cmd_pool_transfer_buffers.begin_buffer();

    // // Model loading
    m_model.load_file(std::filesystem::absolute("resources/viking_room.obj"));

    // Vertex loading
    Buffer transfer_buffer{};
    BufferBuilder()
        .set_size(m_model.verties.size() * sizeof(Vertex))
        .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
        .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
        .build(m_allocator.get_allocator(), transfer_buffer);

    transfer_buffer.copy_data_into_buffer(std::as_bytes(std::span(m_model.verties)));

    BufferBuilder()
        .set_size(m_model.verties.size() * sizeof(Vertex))
        .set_usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        .build(m_allocator.get_allocator(), m_vertex_buffer);

    m_destructor_queue.add_to_queue([&] { m_vertex_buffer.destroy(); });

    m_vertex_buffer.copy_from_buffer(cmd, transfer_buffer);

    // Index loading
    Buffer transfer_buffer_index{};
    BufferBuilder()
        .set_size(m_model.verties.size() * sizeof(Vertex))
        .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
        .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
        .build(m_allocator.get_allocator(), transfer_buffer_index);

    transfer_buffer_index.copy_data_into_buffer(std::as_bytes(std::span(m_model.indices)));

    BufferBuilder()
        .set_size(m_model.indices.size() * sizeof(uint32_t))
        .set_usage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        .build(m_allocator.get_allocator(), m_index_buffer);
    m_destructor_queue.add_to_queue([&] { m_index_buffer.destroy(); });

    m_index_buffer.copy_from_buffer(cmd, transfer_buffer_index);

    m_cmd_pool_transfer_buffers.end_buffer(cmd);
    transfer_buffer_index.destroy();
    transfer_buffer.destroy();

    // // Frame synces
    m_frame_syncers = new FrameSyncers(m_context);
    m_destructor_queue.add_to_queue([&] {m_frame_syncers->destroy(m_device.get_device()); delete m_frame_syncers; });

    m_cmd_pool_graphics_present = CommandPool(m_context, *m_queue_families.get_queue_family(VK_QUEUE_GRAPHICS_BIT, true), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    m_cmds_graphics = (m_cmd_pool_graphics_present.allocate_buffers(MAX_FRAMES_IN_FLIGHT));

    // TODO: poll events on other thread
    while (!glfwWindowShouldClose(m_window_surface.get_window()))
    {
        glfwPollEvents();
        draw_frame();
    }

    vkDeviceWaitIdle(m_device.get_device());
}

void pvp::App::draw_frame()
{
    // TODO: Move everything into renderer
    vkWaitForFences(m_device.get_device(), 1, &m_frame_syncers->in_flight_fences[m_double_buffer_frame].handle, VK_TRUE, UINT64_MAX);
    vkResetFences(m_device.get_device(), 1, &m_frame_syncers->in_flight_fences[m_double_buffer_frame].handle);

    uint32_t image_index{};
    VkResult result = vkAcquireNextImageKHR(m_device.get_device(), m_swapchain->get_swapchain(), UINT64_MAX, m_frame_syncers->image_available_semaphores[m_double_buffer_frame].handle, VK_NULL_HANDLE, &image_index);

    // if (result == VK_ERROR_OUT_OF_DATE_KHR)
    // {
    //     m_pvp_swapchain->recreate_swapchain(*m_device, *m_command_buffer, m_pvp_render_pass);
    //     return;
    // }
    // else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    // {
    //     throw std::runtime_error("failed to acquire swap chain image!");
    // }

    // Uniform update
    {
        static auto start_time = std::chrono::high_resolution_clock::now();

        auto  current_time = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

        ModelCameraViewData ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(m_swapchain->get_swapchain_extent().width) / static_cast<float>(m_swapchain->get_swapchain_extent().height), 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        m_uniform_buffer->update(m_double_buffer_frame, ubo);
    }

    VkCommandBuffer          cmd = m_cmds_graphics[m_double_buffer_frame];
    VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    if (vkBeginCommandBuffer(cmd, &begin_info) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // Color transition
    {
        VkImageSubresourceRange range{};
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = 0;
        range.levelCount = VK_REMAINING_MIP_LEVELS;
        range.baseArrayLayer = 0;
        range.layerCount = VK_REMAINING_ARRAY_LAYERS;

        // VK_PIPELINE_STAGE_2_NONE
        VkImageMemoryBarrier2 image_memory_barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,

            .srcAccessMask = VK_ACCESS_2_NONE,
            .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m_swapchain->get_images()[m_double_buffer_frame],
            .subresourceRange = range
        };

        VkDependencyInfo dependency_info{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = 0,
            .memoryBarrierCount = 0,
            .pMemoryBarriers = VK_NULL_HANDLE,
            .bufferMemoryBarrierCount = 0,
            .pBufferMemoryBarriers = VK_NULL_HANDLE,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &image_memory_barrier
        };

        vkCmdPipelineBarrier2(cmd, &dependency_info);
    }

    VkClearValue clear_values{};
    clear_values.color = { { 0.2f, 0.2f, 0.2f, 1.0f } };
    clear_values.depthStencil = { 1.0f, 0 };

    VkRenderingAttachmentInfo color_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = m_swapchain->get_views()[m_double_buffer_frame],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = clear_values
    };

    VkRenderingInfo render_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, m_swapchain->get_swapchain_extent() },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_info,
    };

    vkCmdBeginRendering(cmd, &render_info);
    record_commands(cmd, image_index);
    vkCmdEndRendering(cmd);

    // Present transition
    {
        VkImageSubresourceRange range{};
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = 0;
        range.levelCount = VK_REMAINING_MIP_LEVELS;
        range.baseArrayLayer = 0;
        range.layerCount = VK_REMAINING_ARRAY_LAYERS;

        // VK_PIPELINE_STAGE_2_NONE
        VkImageMemoryBarrier2 image_memory_barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_NONE,

            .srcAccessMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_NONE,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m_swapchain->get_images()[m_double_buffer_frame],
            .subresourceRange = range
        };

        VkDependencyInfo dependency_info{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = 0,
            .memoryBarrierCount = 0,
            .pMemoryBarriers = VK_NULL_HANDLE,
            .bufferMemoryBarrierCount = 0,
            .pBufferMemoryBarriers = VK_NULL_HANDLE,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &image_memory_barrier
        };

        vkCmdPipelineBarrier2(cmd, &dependency_info);
    }

    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkSemaphoreSubmitInfo semaphore_submit{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = m_frame_syncers->image_available_semaphores[m_double_buffer_frame].handle,
    };

    VkCommandBufferSubmitInfo cmd_submit_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cmd,
    };


    VkSemaphoreSubmitInfo semaphore_submit_singled{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = m_frame_syncers->render_finished_semaphores[m_double_buffer_frame].handle,
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

    vkQueueSubmit2(m_cmd_pool_graphics_present.get_queue().queue, 1, )

        // VkSubmitInfo submit_info{};
        // submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        //
        // VkSemaphore          wait_semaphores[] = { m_frame_syncers->image_available_semaphores[m_double_buffer_frame].handle };
        // VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        // submit_info.waitSemaphoreCount = 1;
        // submit_info.pWaitSemaphores = wait_semaphores;
        // submit_info.pWaitDstStageMask = wait_stages;
        //
        // submit_info.commandBufferCount = 1;
        // submit_info.pCommandBuffers = &cmd;
        //
        // VkSemaphore signal_semaphores[] = { m_frame_syncers->render_finished_semaphores[m_double_buffer_frame].handle };
        // submit_info.signalSemaphoreCount = 1;
        // submit_info.pSignalSemaphores = signal_semaphores;
        //
        // if (vkQueueSubmit(m_cmd_pool_graphics_present.get_queue().queue, 1, &submit_info, m_frame_syncers->in_flight_fences[m_double_buffer_frame].handle) != VK_SUCCESS)
        // {
        //     throw std::runtime_error("failed to submit draw command buffer!");
        // }
        //
        // VkPresentInfoKHR present_info{};
        // present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        // present_info.waitSemaphoreCount = 1;
        // present_info.pWaitSemaphores = signal_semaphores;
        //
        // VkSwapchainKHR swap_chains[] = { m_swapchain->get_swapchain() };
        // present_info.swapchainCount = 1;
        // present_info.pSwapchains = swap_chains;
        // present_info.pImageIndices = &image_index;
        //
        // result = vkQueuePresentKHR(m_cmd_pool_graphics_present.get_queue().queue, &present_info);
        //
        // // if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        // // {
        // //     m_pvp_swapchain->recreate_swapchain(*m_device, *m_command_buffer, m_pvp_render_pass);
        // // }
        // // else if (result != VK_SUCCESS)
        // // {
        // //     throw std::runtime_error("failed to present swap chain image!");
        // // }
        //
        m_double_buffer_frame = (m_double_buffer_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void pvp::App::record_commands(VkCommandBuffer graphics_command, uint32_t image_index)
{
    // vkCmdBeginRenderPass(graphics_command, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(graphics_command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapchain->get_swapchain_extent().width);
    viewport.height = static_cast<float>(m_swapchain->get_swapchain_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(graphics_command, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapchain->get_swapchain_extent();
    vkCmdSetScissor(graphics_command, 0, 1, &scissor);

    VkBuffer     vertex_buffers[] = { m_vertex_buffer.get_buffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(graphics_command, 0, 1, vertex_buffers, offsets);

    vkCmdBindDescriptorSets(graphics_command,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipeline_layout,
                            0,
                            1,
                            &m_descriptors.sets[m_double_buffer_frame],
                            0,
                            nullptr);

    vkCmdBindIndexBuffer(graphics_command, m_index_buffer.get_buffer(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(graphics_command, m_model.indices.size(), 1, 0, 0, 0);
    vkCmdEndRenderPass(graphics_command);

    if (vkEndCommandBuffer(graphics_command) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}
