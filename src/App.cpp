#include "App.h"

#include "Context/LogicPhysicalQueueBuilder.h"

#include <Context/InstanceBuilder.h>
#include <Debugger/Debugger.h>
#include <GLFW/glfw3.h>
#include <GraphicsPipeline/GraphicsPipelineBuilder.h>
#include <Renderer/Renderer.h>
#include <Scene/PVPScene.h>
#include <SyncManager/FrameSyncers.h>
#include <Window/WindowSurfaceBuilder.h>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

void pvp::App::run()
{
    ZoneScoped;

    glfwSetErrorCallback(Debugger::glfw_error_callback);
    glfwInit();
    m_destructor_queue.add_to_queue([&] { glfwTerminate(); });

    WindowSurfaceBuilder()
        .set_window_size(1200, 1000)
        .set_window_title("pretty vulkan printer")
        .build(&m_window);
    m_destructor_queue.add_to_queue([&] { glfwDestroyWindow(m_window); });

    glfwGetWindowSize(m_window, &m_shared_state.screen_width, &m_shared_state.screen_height);
    glfwSetWindowUserPointer(m_window, &m_shared_state);
    glfwSetFramebufferSizeCallback(m_window, &framebuffer_size_callback);
    // glfwSetMouseButtonCallback()

    m_vulkan_app = new VulkanApp(m_window, m_shared_state);
    std::jthread rendering_thread{ &VulkanApp::run, m_vulkan_app };
    m_destructor_queue.add_to_queue([&] { delete m_vulkan_app; });

    while (!glfwWindowShouldClose(m_window))
    {
        ZoneScoped;
        glfwPollEvents();
        // m_scene->update();
        // m_renderer->draw();
    }

    m_shared_state.running.store(false);

    rendering_thread.join();
    // vkDeviceWaitIdle(m_device.get_device());
}

void pvp::App::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    GlfwToRender* glfw = static_cast<GlfwToRender*>(glfwGetWindowUserPointer(window));

    {
        std::lock_guard lock(glfw->lock);
        glfw->screen_height = width;
        glfw->screen_width = height;
    }

    glfw->needs_resizing.store(true);
}

// void pvp::App::run_loop() const
// {
//     ZoneScoped;
//     tracy::SetThreadName("Renderer");
//     while (!glfwWindowShouldClose(m_window_surface.get_window()))
//     {
//
//         // m_scene->update();
//         // m_renderer->draw();
//         FrameMark;
//     }
// }
