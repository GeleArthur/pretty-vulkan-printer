#include "WindowSurface.h"

namespace pvp
{
    const GLFWwindow& WindowSurface::get_window() const
    {
        return *m_window;
    }
    const VkSurfaceKHR& WindowSurface::get_surface() const
    {
        return m_surface;
    }
    void WindowSurface::destroy(const Instance& instance) const
    {
        vkDestroySurfaceKHR(instance.get_instance(), m_surface, nullptr);
        glfwDestroyWindow(m_window);
    }
} // namespace pvp