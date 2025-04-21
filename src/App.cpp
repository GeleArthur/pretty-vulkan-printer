#include "App.h"

#include <Buffer.h>
#include <Image.h>
#include <PVPGraphicsPipeline/GraphicsPipelineBuilder.h>
#include <PVPRenderPass/RenderPassBuilder.h>
#include <PVPSwapchain/Swapchain.h>

#include <iostream>
#include <PVPGraphicsPipeline/DescriptorSetLayoutBuilder.h>
#include <PVPGraphicsPipeline/PVPVertex.h>
#include <PVPGraphicsPipeline/PipelineLayoutBuilder.h>
#include <PVPGraphicsPipeline/ShaderLoader.h>

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

    Buffer test_buffer = Buffer(30, VMA_MEMORY_USAGE_AUTO, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    // Image  test_image = Image(m_pvp_physical_device->get_device(),
    //                          1024,
    //                          1024,
    //                          VK_FORMAT_R8G8B8A8_UNORM,
    //                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    //                          VMA_MEMORY_USAGE_AUTO,
    //                          VK_IMAGE_ASPECT_COLOR_BIT);

    while (!glfwWindowShouldClose(m_pvp_instance->get_window()))
    {
        glfwPollEvents();
    }

    vkDeviceWaitIdle(m_pvp_physical_device->get_device());
}
