#pragma once

#define GLFW_INCLUDE_VULKAN
#include <DestructorQueue.h>
#include <Context/Instance.h>
#include <GLFW/glfw3.h>

namespace pvp
{
    class WindowSurface final
    {
    public:
        explicit WindowSurface() = default;
        GLFWwindow*         get_window() const;
        const VkSurfaceKHR& get_surface() const;
        void                destroy(const Instance& instance) const;

    private:
        friend class WindowSurfaceBuilder;
        GLFWwindow*                                m_window{ nullptr };
        VkSurfaceKHR                               m_surface{ VK_NULL_HANDLE };
        std::vector<std::function<void(int, int)>> m_event_window_resize;
    };
} // namespace pvp
