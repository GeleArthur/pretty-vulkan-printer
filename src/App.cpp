#include "App.h"

#include "PVPPhysicalDevice/LogicPhysicalQueueBuilder.h"

#include <GLFW/glfw3.h>
#include <PVPDescriptorSets/DescriptorLayoutBuilder.h>
#include <PVPGraphicsPipeline/GraphicsPipelineBuilder.h>
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

    // TODO: Context builder
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

    /*
    // DescriptorLayout layout{};
    //
    // DescriptorLayoutBuilder()
    //     .add_binding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
    //     .add_binding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
    //     .build(vk::Device(m_device.get_device()), layout);
    //
    // m_descriptor_pool = DescriptorPool(m_device.get_device(), {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2}}, 2);
    // m_destructor_queue.add_to_queue([&] { m_descriptor_pool.destroy(); });
    //
    // m_uniform_buffer = new UniformBuffer<ModelCameraViewData>(m_context.allocator->get_allocator());
    // m_destructor_queue.add_to_queue([&] { delete m_uniform_buffer; });
    //
    // m_pipeline_layout = PipelineLayoutBuilder()
    //                     .add_descriptor_layout(layout.get_handle())
    //                     .build(m_device.get_device());
    // m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_device.get_device(), m_pipeline_layout, nullptr); });
    //
    // TextureBuilder().set_path("resources/viking_room.png").build(m_device.get_device(), *m_command_buffer, m_texture);
    // m_destructor_queue.add_to_queue([&] {
    //     m_texture.destroy(m_context);
    // });
    //
    // SamplerBuilder().build(m_device->get_device(), m_sampler);
    // m_destructor_queue.add_to_queue([&] {
    //     m_sampler.destroy(m_device->get_device());
    // });
    //
    //
    // m_descriptors = DescriptorSetBuilder()
    //                 .set_layout(layout)
    //                 .bind_buffer(0, *m_uniform_buffer)
    //                 // .bind_image(1, m_texture, m_sampler)
    //                 .build(m_device.get_device(), m_descriptor_pool);
    //
    // m_graphics_pipeline = GraphicsPipelineBuilder()
    //                       .add_shader("shaders/shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
    //                       .add_shader("shaders/shader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    //                       .set_pipeline_layout(m_pipeline_layout)
    //                       .set_input_attribute_description(Vertex::get_attribute_descriptions())
    //                       .set_input_binding_description(Vertex::get_binding_description())
    //                       .set_color_format(std::array{m_swapchain->get_swapchain_surface_format().format})
    //                       .set_depth_format(VK_FORMAT_D32_SFLOAT_S8_UINT)
    //                       .build(m_device);
    // m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_device.get_device(), m_graphics_pipeline, nullptr); });
    //
    // m_cmd_pool_transfer_buffers = CommandPool(m_context, *m_queue_families.get_queue_family(VK_QUEUE_TRANSFER_BIT, false), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    // m_destructor_queue.add_to_queue([&] { m_cmd_pool_transfer_buffers.destroy(); });
    // VkCommandBuffer cmd = m_cmd_pool_transfer_buffers.begin_buffer();
    */

    m_scene = load_scene(m_context);
    m_destructor_queue.add_to_queue([&] { destroy_scene(m_scene); });

    m_renderer = new Renderer(m_context, *m_swapchain);
    m_destructor_queue.add_to_queue([&] { delete m_renderer; });

    // TODO: poll events on other thread
    while (!glfwWindowShouldClose(m_window_surface.get_window()))
    {
        glfwPollEvents();
        // draw_frame();
    }

    vkDeviceWaitIdle(m_device.get_device());
}

void pvp::App::draw_frame()
{
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

    const auto cmd = m_renderer->prepare_frame();
    // record_commands(cmd, m_renderer->get_current_frame_index());
    m_renderer->end_frame();
}

void pvp::App::record_commands(VkCommandBuffer graphics_command, uint32_t image_index)
{
    VkCommandBufferBeginInfo start_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vkBeginCommandBuffer(graphics_command, &start_info);
    vkEndCommandBuffer(graphics_command);

    vkBeginCommandBuffer(graphics_command, &start_info);

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

    vkEndCommandBuffer(graphics_command);
}
