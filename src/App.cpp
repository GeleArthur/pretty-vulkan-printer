#include "App.h"

#include "Context/LogicPhysicalQueueBuilder.h"

#include <GLFW/glfw3.h>
#include <DescriptorSets/DescriptorLayoutBuilder.h>
#include <GraphicsPipeline/GraphicsPipelineBuilder.h>
#include <Context/InstanceBuilder.h>
#include <Renderer/Swapchain.h>
#include <SyncManager/FrameSyncers.h>
#include <Window/WindowSurfaceBuilder.h>
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

    m_descriptor_pool = DescriptorPool(m_device.get_device(),
                                       { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
                                         { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 },
                                         { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 10 } },
                                       10);
    m_destructor_queue.add_to_queue([&] { m_descriptor_pool.destroy(); });

    m_context.descriptor_pool = &m_descriptor_pool;
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

    m_renderer = new Renderer(m_context, *m_swapchain, m_scene);
    m_destructor_queue.add_to_queue([&] { delete m_renderer; });

    // TODO: poll events on other thread
    while (!glfwWindowShouldClose(m_window_surface.get_window()))
    {
        glfwPollEvents();
        m_renderer->draw();
    }

    vkDeviceWaitIdle(m_device.get_device());
}
