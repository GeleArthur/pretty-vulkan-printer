#pragma once

#include <GLFW/glfw3.h>

namespace VulkanWindow
{
    void create_window(int width, int height);
    void destroy_window();
    int get_width();
    int get_height();
    GLFWwindow* get_window();
};

