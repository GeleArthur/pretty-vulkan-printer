#pragma once
#include "WindowSurface.h"

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

        void build(const Instance& instance, WindowSurface& window_surface) const;

    private:
        int         m_width{ 800 };
        int         m_height{ 600 };
        std::string m_title{ "vulkan window" };
    };
} // namespace pvp
