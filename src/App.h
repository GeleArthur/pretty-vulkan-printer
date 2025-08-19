#pragma once
#include <GlfwToRender.h>
#include <VulkanApp.h>
#include <Renderer/Renderer.h>
#include <Window/WindowSurface.h>

namespace pvp
{
    class App final
    {
    public:
        void run();

    private:
        static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

        GLFWwindow*     m_window{};
        GlfwToRender    m_shared_state{};
        VulkanApp*      m_vulkan_app{};
        DestructorQueue m_destructor_queue;
    };
} // namespace pvp
