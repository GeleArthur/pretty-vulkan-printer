#include "VulkanApp.h"

#include <GlfwToRender.h>
#include <ImguiRenderer.h>
#include <Context/InstanceBuilder.h>
#include <Context/LogicPhysicalQueueBuilder.h>
#include <tracy/Tracy.hpp>

pvp::VulkanApp::VulkanApp(GLFWwindow* window, GlfwToRender& gtfw_to_render)
    : m_gtfw_to_render{ &gtfw_to_render }
    , m_window{ window }
{
}
void pvp::VulkanApp::run()
{
    InstanceBuilder()
        .enable_debugging(true)
        .set_app_name("pretty vulkan printer")
        .build(m_instance);
    m_destructor_queue.add_to_queue([&] { m_instance.destroy(); });

    if (auto result = glfwCreateWindowSurface(m_instance.get_instance(), m_window, nullptr, &m_surface) != VK_SUCCESS)
    {
        throw std::runtime_error(std::format("Failed to create surface. {}", result));
    }
    m_destructor_queue.add_to_queue([&] { vkDestroySurfaceKHR(m_instance.get_instance(), m_surface, nullptr); });

    LogicPhysicalQueueBuilder()
        .set_extensions({ VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME })
        .build(m_instance, m_surface, m_physical_device, m_device, m_queue_families);
    m_destructor_queue.add_to_queue([&] { m_device.destroy(); });

    create_allocator(m_allocator, m_instance, m_device, m_physical_device);
    m_destructor_queue.add_to_queue([&] { m_allocator.destroy(); });

    m_context.instance = &m_instance;
    m_context.physical_device = &m_physical_device;
    m_context.device = &m_device;
    m_context.allocator = &m_allocator;
    m_context.queue_families = &m_queue_families;
    m_context.surface = m_surface;

    m_context.descriptor_creator = new DescriptorLayoutCreator(m_context);
    m_destructor_queue.add_to_queue([&] { delete m_context.descriptor_creator; });

    m_swapchain = new Swapchain(m_context, *m_gtfw_to_render);
    m_destructor_queue.add_to_queue([&] { delete m_swapchain; });
    m_context.swapchain = m_swapchain;

    m_scene = new PvpScene(m_context);
    m_destructor_queue.add_to_queue([&] { delete m_scene; });

    m_imgui_renderer = new ImguiRenderer(m_context, m_window, m_gtfw_to_render);
    m_destructor_queue.add_to_queue([&] { delete m_imgui_renderer; });

    m_renderer = new Renderer(m_context, *m_scene, *m_imgui_renderer);
    m_destructor_queue.add_to_queue([&] { delete m_renderer; });

    while (m_gtfw_to_render->running)
    {
        FrameMark;
        m_imgui_renderer->start_drawing();
        m_scene->update();
        m_imgui_renderer->end_drawing();
        m_renderer->draw();
    }

    vkDeviceWaitIdle(m_context.device->get_device());
}