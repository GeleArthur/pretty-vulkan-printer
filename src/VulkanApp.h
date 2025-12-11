#pragma once

#include <Context/Context.h>
#include <GLFW/glfw3.h>

namespace pvp
{
    namespace VulkanApp
    {
        void run(GLFWwindow* window, GlfwToRender& gtfw_to_render);
    };
} // namespace pvp
