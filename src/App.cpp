#include "App.h"

#include <LoadModel.h>
#include <UniformBufferStruct.h>
#include <array>
#include <globalconst.h>
#include <iostream>
#include <PVPBuffer/Buffer.h>
#include <PVPBuffer/BufferBuilder.h>
#include <PVPCommandBuffer/CommandBuffer.h>
#include <PVPDescriptorSets/DescriptorLayout.h>
#include <PVPGraphicsPipeline/GraphicsPipelineBuilder.h>
#include <PVPGraphicsPipeline/Vertex.h>
#include <PVPGraphicsPipeline/PipelineLayoutBuilder.h>
#include <PVPGraphicsPipeline/ShaderLoader.h>
#include <PVPImage/TextureBuilder.h>
#include <PVPRenderPass/RenderPassBuilder.h>
#include <PVPSwapchain/Swapchain.h>
#include <PVPUniformBuffers/UniformBuffer.h>
#include <glm/gtx/quaternion.hpp>

void pvp::App::run()
{
    // TODO: Debug stuff and swapchain extensions should be moved inside as it a given that we want it.
    m_pvp_instance = new Instance(800,
                                  800,
                                  "pretty vulkan printer",
                                  true,
                                  { VK_EXT_DEBUG_UTILS_EXTENSION_NAME },
                                  { "VK_LAYER_KHRONOS_validation" });
    m_destructor_queue.add_to_queue([&] { delete m_pvp_instance; });

    m_pvp_device = new Device(m_pvp_instance, {});
    m_destructor_queue.add_to_queue([&] { delete m_pvp_device; });

    m_allocator = new PvpVmaAllocator(*m_pvp_instance, *m_pvp_device);
    m_destructor_queue.add_to_queue([&] { delete m_allocator; });

    m_command_buffer = new CommandBuffer(*m_pvp_device);
    m_destructor_queue.add_to_queue([&] { delete m_command_buffer; });

    m_pvp_swapchain = new Swapchain(*m_pvp_instance, *m_pvp_device, *m_command_buffer);
    m_destructor_queue.add_to_queue([&] { delete m_pvp_swapchain; });

    m_pvp_render_pass = RenderPassBuilder().build(*m_pvp_swapchain, *m_pvp_device);
    m_destructor_queue.add_to_queue([&] { vkDestroyRenderPass(m_pvp_device->get_device(), m_pvp_render_pass, nullptr); });
    m_pvp_swapchain->create_frame_buffers(m_pvp_device->get_device(), m_pvp_render_pass);

    m_sync_builder = new SyncBuilder(m_pvp_device->get_device());
    m_destructor_queue.add_to_queue([&] { delete m_sync_builder; });

    DescriptorLayout layout = DescriptorLayout(
        m_pvp_device->get_device(),
        {
            VkDescriptorSetLayoutBinding{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
            VkDescriptorSetLayoutBinding{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
        });

    m_pipeline_layout = PipelineLayoutBuilder().add_descriptor_layout(layout.get_handle()).build(m_pvp_device->get_device());
    m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_pvp_device->get_device(), m_pipeline_layout, nullptr); });

    m_descriptor_pool = new DescriptorPool(m_pvp_device->get_device(), { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 } }, 2);
    m_destructor_queue.add_to_queue([&] { m_descriptor_pool->destroy(); delete m_descriptor_pool; });

    m_uniform_buffer = new UniformBuffer<ModelCameraViewData>(PvpVmaAllocator::get_allocator());
    m_destructor_queue.add_to_queue([&] { delete m_uniform_buffer; });

    TextureBuilder()
        .set_path("resources/viking_room.png")
        .build(m_pvp_device->get_device(), *m_command_buffer, m_texture);
    
    m_destructor_queue.add_to_queue([&] { m_texture.destroy(m_pvp_device->get_device()); });

    m_descriptors = DescriptorSetBuilder()
                        .set_layout(layout)
                        .bind_buffer(0, *m_uniform_buffer)
                        .bind_image(1, m_texture)
                        .build(m_pvp_device->get_device(), *m_descriptor_pool);

    // Shader loading
    auto vertex_shader = ShaderLoader::load_shader_from_file(m_pvp_device->get_device(), "shaders/shader.vert.spv");
    auto fragment_shader = ShaderLoader::load_shader_from_file(m_pvp_device->get_device(), "shaders/shader.frag.spv");

    m_graphics_pipeline = GraphicsPipelineBuilder()
                              .set_render_pass(m_pvp_render_pass)
                              .add_shader(vertex_shader, VK_SHADER_STAGE_VERTEX_BIT)
                              .add_shader(fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT)
                              .set_pipeline_layout(m_pipeline_layout)
                              .set_input_attribute_description(Vertex::get_attribute_descriptions())
                              .set_input_binding_description(Vertex::get_binding_description())
                              .build(*m_pvp_device);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_pvp_device->get_device(), m_graphics_pipeline, nullptr); });

    vkDestroyShaderModule(m_pvp_device->get_device(), vertex_shader, nullptr);
    vkDestroyShaderModule(m_pvp_device->get_device(), fragment_shader, nullptr);

    // Model loading
    m_model.load_file(std::filesystem::absolute("resources/viking_room.obj"));

    // Vertex loading
    Buffer transfer_buffer{};
    BufferBuilder()
        .set_size(m_model.verties.size() * sizeof(Vertex))
        .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
        .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
        .build(PvpVmaAllocator::get_allocator(), transfer_buffer);

    transfer_buffer.set_image_data(std::as_bytes(std::span(m_model.verties)));

    BufferBuilder()
        .set_size(m_model.verties.size() * sizeof(Vertex))
        .set_usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        .build(PvpVmaAllocator::get_allocator(), m_vertex_buffer);

    m_destructor_queue.add_to_queue([&] { m_vertex_buffer.destroy(); });

    m_vertex_buffer.copy_from_buffer(*m_command_buffer, transfer_buffer);
    transfer_buffer.destroy();

    // Index loading
    Buffer transfer_buffer_index{};
    BufferBuilder()
        .set_size(m_model.verties.size() * sizeof(Vertex))
        .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
        .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
        .build(PvpVmaAllocator::get_allocator(), transfer_buffer_index);

    transfer_buffer_index.set_image_data(std::as_bytes(std::span(m_model.indices)));

    BufferBuilder()
        .set_size(m_model.indices.size() * sizeof(uint32_t))
        .set_usage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        .build(PvpVmaAllocator::get_allocator(), m_index_buffer);
    m_destructor_queue.add_to_queue([&] { m_index_buffer.destroy(); });

    m_index_buffer.copy_from_buffer(*m_command_buffer, transfer_buffer_index);
    transfer_buffer_index.destroy();

    // Frame synces
    m_frame_syncers = new FrameSyncers(*m_sync_builder);
    m_destructor_queue.add_to_queue([&] { m_frame_syncers->destroy(m_pvp_device->get_device()); delete m_frame_syncers; });

    // TODO: poll events on other thread
    while (!glfwWindowShouldClose(m_pvp_instance->get_window()))
    {
        glfwPollEvents();
        draw_frame();
    }

    vkDeviceWaitIdle(m_pvp_device->get_device());
}

void pvp::App::draw_frame()
{
    // TODO: Move everything into renderer
    vkWaitForFences(m_pvp_device->get_device(), 1, &m_frame_syncers->in_flight_fences[m_double_buffer_frame].handle, VK_TRUE, UINT64_MAX);
    vkResetFences(m_pvp_device->get_device(), 1, &m_frame_syncers->in_flight_fences[m_double_buffer_frame].handle);

    uint32_t image_index{};
    VkResult result = vkAcquireNextImageKHR(m_pvp_device->get_device(), m_pvp_swapchain->get_swapchain(), UINT64_MAX, m_frame_syncers->image_available_semaphores[m_double_buffer_frame].handle, VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        m_pvp_swapchain->recreate_swapchain(*m_pvp_device, *m_command_buffer, m_pvp_render_pass);
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    VkCommandBuffer graphics_command = m_command_buffer->get_graphics_command_buffer(m_double_buffer_frame);

    static auto start_time = std::chrono::high_resolution_clock::now();

    auto  current_time = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

    ModelCameraViewData ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(m_pvp_swapchain->get_swapchain_extent().width) / static_cast<float>(m_pvp_swapchain->get_swapchain_extent().height), 0.1f, 10.0f);

    ubo.proj[1][1] *= -1;

    m_uniform_buffer->update(m_double_buffer_frame, ubo);

    record_commands(graphics_command, image_index);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore          wait_semaphores[] = { m_frame_syncers->image_available_semaphores[m_double_buffer_frame].handle };
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &graphics_command;

    VkSemaphore signal_semaphores[] = { m_frame_syncers->render_finished_semaphores[m_double_buffer_frame].handle };
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    if (vkQueueSubmit(m_pvp_device->get_queue_families().graphics_family.queue, 1, &submit_info, m_frame_syncers->in_flight_fences[m_double_buffer_frame].handle) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swap_chains[] = { m_pvp_swapchain->get_swapchain() };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;

    result = vkQueuePresentKHR(m_pvp_device->get_queue_families().present_family.queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        m_pvp_swapchain->recreate_swapchain(*m_pvp_device, *m_command_buffer, m_pvp_render_pass);
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_double_buffer_frame = (m_double_buffer_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void pvp::App::record_commands(VkCommandBuffer graphics_command, uint32_t image_index)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(graphics_command, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = m_pvp_render_pass;
    render_pass_info.framebuffer = m_pvp_swapchain->get_framebuffers()[image_index];
    render_pass_info.renderArea.offset = { 0, 0 };
    render_pass_info.renderArea.extent = m_pvp_swapchain->get_swapchain_extent();

    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = {
        { 0.2f, 0.2f, 0.2f, 1.0f }
    };
    clear_values[1].depthStencil = { 1.0f, 0 };

    render_pass_info.clearValueCount = clear_values.size();
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(graphics_command, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(graphics_command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_pvp_swapchain->get_swapchain_extent().width);
    viewport.height = static_cast<float>(m_pvp_swapchain->get_swapchain_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(graphics_command, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_pvp_swapchain->get_swapchain_extent();
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
