#include "Window.h"
#include <GLFW/glfw3.h>

// I don't know if this is a good idea.
// Lets find out

static int window_width;
static int window_height;
static GLFWwindow* window;

void VulkanWindow::create_window(const int width, const int height)
{
    window_width = width;
    window_height = height;

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(window_width, window_height, "Vulkan", nullptr, nullptr);
}

void VulkanWindow::destroy_window()
{
    glfwDestroyWindow(window);
}

int VulkanWindow::get_width()
{
    return window_width;
}

int VulkanWindow::get_height()
{
    return window_height;
}

GLFWwindow* VulkanWindow::get_window()
{
    return window;
}
