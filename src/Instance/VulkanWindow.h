#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanInstance.h"

namespace VulkanWindow
{
void create_window(int width, int height);
void create_surface(VkInstance instance);
void destroy_window();
int get_width();
int get_height();
GLFWwindow* get_window();
}; // namespace VulkanWindow
