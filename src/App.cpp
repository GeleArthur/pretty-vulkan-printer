#include "App.h"

#include <Debugger/Debugger.h>
#include <GLFW/glfw3.h>
#include <Renderer/Renderer.h>
#include <Scene/PVPScene.h>
#include <Window/WindowSurfaceBuilder.h>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

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

    std::thread rendering_thread{ VulkanApp::run, m_window, std::ref(m_shared_state) };

    while (!glfwWindowShouldClose(m_window))
    {
        ZoneScoped;
        glfwPollEvents();
    }

    m_shared_state.running.store(false);
    rendering_thread.join();
}
