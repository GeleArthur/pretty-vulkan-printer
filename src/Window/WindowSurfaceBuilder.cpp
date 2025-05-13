#include "WindowSurfaceBuilder.h"

#include <format>
#include <stdexcept>
#include <GLFW/glfw3.h>

namespace pvp
{
    WindowSurfaceBuilder& WindowSurfaceBuilder::set_window_size(int width, int height)
    {
        m_width = width;
        m_height = height;
        return *this;
    }
    WindowSurfaceBuilder& WindowSurfaceBuilder::set_window_title(const std::string& title)
    {
        m_title = title;
        return *this;
    }
    void WindowSurfaceBuilder::build(const Instance& instance, WindowSurface& window_surface) const
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window_surface.m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

        if (auto result = glfwCreateWindowSurface(instance.get_instance(), window_surface.m_window, nullptr, &window_surface.m_surface) != VK_SUCCESS)
        {
            throw std::runtime_error(std::format("Failed to create surface. {}", result));
        }
    }
} // namespace pvp