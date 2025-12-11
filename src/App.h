#pragma once
#include <DestructorQueue.h>
#include <GlfwToRender.h>
#include <VulkanApp.h>

namespace pvp
{
    class App final
    {
    public:
        void run();

    private:
        GLFWwindow*     m_window{};
        GlfwToRender    m_shared_state{};
        DestructorQueue m_destructor_queue;
    };
} // namespace pvp
