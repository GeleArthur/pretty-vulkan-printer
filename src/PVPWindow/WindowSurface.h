#pragma once

#define GLFW_INCLUDE_VULKAN
#include <DestructorQueue.h>
#include <GLFW/glfw3.h>
#include <PVPInstance/Instance.h>

namespace pvp
{
    class WindowSurface
    {
    public:
        explicit WindowSurface() = default;
        void destroy(const Instance& instance) const;

    private:
        friend class WindowSurfaceBuilder;
        GLFWwindow*      m_window{ nullptr };
        VkSurfaceKHR     m_surface{ VK_NULL_HANDLE };
        VkPhysicalDevice m_physical_device{ VK_NULL_HANDLE };
    };
} // namespace pvp
