#pragma once
#include <mutex>

namespace pvp
{
    struct GlfwToRender
    {
        std::mutex                     lock{};
        float                          mouse_pos_x{};
        float                          mouse_pos_y{};
        bool                           mouse_down[5]{ false };
        std::array<int, GLFW_KEY_LAST> keys_pressed{};

        std::atomic<bool> needs_resizing{ false };
        int               screen_width{};
        int               screen_height{};

        std::atomic<bool> running{ true };
    };
} // namespace pvp
