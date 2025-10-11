#pragma once
#include <mutex>
#include "GLFW/glfw3.h"
#include <array>
#include <atomic>

namespace pvp
{
    struct GlfwToRender
    {
        std::mutex                     lock;
        float                          mouse_pos_x{};
        float                          mouse_pos_y{};
        std::array<bool, 5>            mouse_down{ false };
        std::array<int, GLFW_KEY_LAST> keys_pressed{};

        std::atomic<bool> needs_resizing{ false };
        int               screen_width{};
        int               screen_height{};

        std::atomic<bool> running{ true };
    };
} // namespace pvp
