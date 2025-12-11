#pragma once

#include <GLFW/glfw3.h>

#include <string>
#include <Context/Instance.h>
namespace pvp
{
    class WindowSurfaceBuilder
    {
    public:
        explicit WindowSurfaceBuilder() = default;

        WindowSurfaceBuilder& set_window_size(int width, int height);
        WindowSurfaceBuilder& set_window_title(const std::string& title);

        void build(GLFWwindow** window);

    private:
        int         m_width{};
        int         m_height{};
        std::string m_title{ "vulkan window" };
    };
} // namespace pvp
