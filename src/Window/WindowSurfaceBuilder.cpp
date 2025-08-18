#include "WindowSurfaceBuilder.h"

#include <format>
#include <stdexcept>
#include <GLFW/glfw3.h>
#include <tracy/Tracy.hpp>

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
    void WindowSurfaceBuilder::build(GLFWwindow** window)
    {
        ZoneScoped;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        // GLFWmonitor** monitors;
        // int           count;

        // monitors = glfwGetMonitors(&count);

        // int xpos, ypos;
        // glfwGetMonitorPos(monitors[0], &xpos, &ypos);

        // if (m_width == 0 || m_height == 0)
        // {
        //     const GLFWvidmode* mode = glfwGetVideoMode(monitors[0]);
        //     m_width = mode->width;
        //     m_height = mode->height;
        // }

        *window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
        glfwShowWindow(*window);

        // glfwSetWindowPos(window_surface.m_window, xpos, ypos);
        // if (auto result = glfwCreateWindowSurface(instance.get_instance(), window_surface.m_window, nullptr, &window_surface.m_surface) != VK_SUCCESS)
        // {
        //     throw std::runtime_error(std::format("Failed to create surface. {}", result));
        // }

        // glfwSetFramebufferSizeCallback(window_surface.m_window, &WindowSurface::window_resized);
    }
} // namespace pvp