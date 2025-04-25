#include "App.h"

#include "../build/_deps/assimp-src/code/AssetLib/3DS/3DSExporter.h"

#include <Buffer.h>
#include <Image.h>
#include <LoadModel.h>
#include <array>
#include <PVPGraphicsPipeline/GraphicsPipelineBuilder.h>
#include <PVPRenderPass/RenderPassBuilder.h>
#include <PVPSwapchain/Swapchain.h>

#include <iostream>
#include <PVPCommandBuffer/CommandBuffer.h>
#include <PVPGraphicsPipeline/DescriptorSetLayoutBuilder.h>
#include <PVPGraphicsPipeline/PVPVertex.h>
#include <PVPGraphicsPipeline/PipelineLayoutBuilder.h>
#include <PVPGraphicsPipeline/ShaderLoader.h>
#include <assimp/cimport.h>

void pvp::App::run()
{
    // TODO: Debug stuff and swapchain extensions should be moved inside as it a given that we want it.
    m_pvp_instance = new Instance(
    800,
    800,
    "pretty vulkan printer",
    true,
    { VK_EXT_DEBUG_UTILS_EXTENSION_NAME },
    { "VK_LAYER_KHRONOS_validation" });
    m_destructor_queue.add_to_queue([&] { delete m_pvp_instance; });

    m_pvp_physical_device = new PhysicalDevice(m_pvp_instance, {});
    m_destructor_queue.add_to_queue([&] { delete m_pvp_physical_device; });

    m_allocator = new PvpVmaAllocator(*m_pvp_instance, *m_pvp_physical_device);
    m_destructor_queue.add_to_queue([&] { delete m_allocator; });

    m_pvp_swapchain = new Swapchain(*m_pvp_instance, *m_pvp_physical_device);
    m_destructor_queue.add_to_queue([&] { delete m_pvp_swapchain; });

    m_pvp_render_pass = RenderPassBuilder().build(*m_pvp_swapchain, *m_pvp_physical_device);
    m_destructor_queue.add_to_queue([&] { vkDestroyRenderPass(m_pvp_physical_device->get_device(), m_pvp_render_pass, nullptr); });
    m_pvp_swapchain->create_frame_buffers(m_pvp_physical_device->get_device(), m_pvp_render_pass);

    m_sync_builder = new SyncBuilder(m_pvp_physical_device->get_device());
    m_destructor_queue.add_to_queue([&] { delete m_sync_builder; });

    m_command_buffer = new CommandBuffer(*m_pvp_physical_device);
    m_destructor_queue.add_to_queue([&] { delete m_command_buffer; });

    auto vertex_shader = ShaderLoader::load_shader_from_file(m_pvp_physical_device->get_device(), "shaders/shader.vert.spv");
    auto fragment_shader = ShaderLoader::load_shader_from_file(m_pvp_physical_device->get_device(), "shaders/shader.frag.spv");

    m_descriptor_set_layout = DescriptorSetLayoutBuilder().build(m_pvp_physical_device->get_device());
    m_destructor_queue.add_to_queue([&] { vkDestroyDescriptorSetLayout(m_pvp_physical_device->get_device(), m_descriptor_set_layout, nullptr); });

    m_pipeline_layout = PipelineLayoutBuilder().add_descriptor_layout(m_descriptor_set_layout).build(m_pvp_physical_device->get_device());
    m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_pvp_physical_device->get_device(), m_pipeline_layout, nullptr); });

    m_graphics_pipeline = GraphicsPipelineBuilder()
                          .set_render_pass(m_pvp_render_pass)
                          .add_shader(vertex_shader, VK_SHADER_STAGE_VERTEX_BIT)
                          .add_shader(fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT)
                          .set_pipeline_layout(m_pipeline_layout)
                          .set_input_atrribute_description(PvpVertex::get_attribute_descriptions())
                          .set_input_binding_description(PvpVertex::get_binding_description())
                          .build(*m_pvp_physical_device);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_pvp_physical_device->get_device(), m_graphics_pipeline, nullptr); });

    vkDestroyShaderModule(m_pvp_physical_device->get_device(), vertex_shader, nullptr);
    vkDestroyShaderModule(m_pvp_physical_device->get_device(), fragment_shader, nullptr);

    m_model.load_file(std::filesystem::absolute("resources/cube.fbx"));

    Buffer transfer_buffer = Buffer(m_model.verties.size() * sizeof(PvpVertex),
                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    memcpy(transfer_buffer.get_allocation_info().pMappedData, m_model.verties.data(), m_model.verties.size() * sizeof(PvpVertex));

    m_vertex_buffer = new Buffer(m_model.verties.size() * sizeof(PvpVertex),
                                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                 VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                                 0);
    m_destructor_queue.add_to_queue([&] { delete m_vertex_buffer; });

    transfer_buffer.copy_into_buffer(*m_command_buffer, *m_vertex_buffer);

    VkSemaphoreCreateInfo semaphore_info {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    // I AM SO DONE I WANT ANYTHING
    m_image_available_semaphore = m_sync_builder->create_semaphore();
    m_destructor_queue.add_to_queue([&] { m_image_available_semaphore.destroy(m_pvp_physical_device->get_device()); });

    m_render_finished_semaphore = m_sync_builder->create_semaphore();
    m_destructor_queue.add_to_queue([&] { m_render_finished_semaphore.destroy(m_pvp_physical_device->get_device()); });

    m_in_flight_fence = m_sync_builder->create_fence(true);
    m_destructor_queue.add_to_queue([&] { m_in_flight_fence.destroy(m_pvp_physical_device->get_device()); });

    while (!glfwWindowShouldClose(m_pvp_instance->get_window()))
    {
        glfwPollEvents();
        draw_frame();
    }

    vkDeviceWaitIdle(m_pvp_physical_device->get_device());
}
void pvp::App::draw_frame()
{
    vkWaitForFences(m_pvp_physical_device->get_device(), 1, &m_in_flight_fence.handle, VK_TRUE, UINT64_MAX);

    vkResetFences(m_pvp_physical_device->get_device(), 1, &m_in_flight_fence.handle);

    uint32_t image_index {};
    vkAcquireNextImageKHR(m_pvp_physical_device->get_device(), m_pvp_swapchain->get_swapchain(), UINT64_MAX, m_image_available_semaphore.handle, VK_NULL_HANDLE, &image_index);

    VkCommandBuffer graphics_command = m_command_buffer->get_graphics_command_buffer();

    record_commands(graphics_command, image_index);

    VkSubmitInfo submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore          wait_semaphores[] = { m_image_available_semaphore.handle };
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &graphics_command;

    VkSemaphore signal_semaphores[] = { m_render_finished_semaphore.handle };
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    if (vkQueueSubmit(m_pvp_physical_device->get_queue_families().graphics_family.queue, 1, &submit_info, m_in_flight_fence.handle) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR present_info {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swap_chains[] = { m_pvp_swapchain->get_swapchain() };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;

    vkQueuePresentKHR(m_pvp_physical_device->get_queue_families().present_family.queue, &present_info);
}
void pvp::App::record_commands(VkCommandBuffer graphics_command, uint32_t image_index)
{
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(graphics_command, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = m_pvp_render_pass;
    render_pass_info.framebuffer = m_pvp_swapchain->get_framebuffers()[image_index];
    render_pass_info.renderArea.offset = { 0, 0 };
    render_pass_info.renderArea.extent = m_pvp_swapchain->get_swapchain_extent();

    std::array<VkClearValue, 2> clear_values {};
    clear_values[0].color = { { 0.0f, 0.2f, 0.0f, 1.0f } };
    clear_values[1].depthStencil = { 1.0f, 0 };

    render_pass_info.clearValueCount = clear_values.size();
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(graphics_command, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(graphics_command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_pvp_swapchain->get_swapchain_extent().width);
    viewport.height = static_cast<float>(m_pvp_swapchain->get_swapchain_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(graphics_command, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = { 0, 0 };
    scissor.extent = m_pvp_swapchain->get_swapchain_extent();
    vkCmdSetScissor(graphics_command, 0, 1, &scissor);

    VkBuffer     vertex_buffers[] = { m_vertex_buffer->get_buffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(graphics_command, 0, 1, vertex_buffers, offsets);
    vkCmdDraw(graphics_command, m_model.verties.size(), 1, 0, 0);
    vkCmdEndRenderPass(graphics_command);

    if (vkEndCommandBuffer(graphics_command) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}
