#include "WindowSurface.h"

namespace pvp
{
    void WindowSurface::destroy(const Instance& instance) const
    {
        vkDestroySurfaceKHR(instance.get_instance(), m_surface, nullptr);
        glfwDestroyWindow(m_window);
    }
} // namespace pvp