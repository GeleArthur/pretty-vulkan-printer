#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

#include "../DestructorQueue.h"

class VulkanInstance
{
  public:
    explicit VulkanInstance(int width,
                            int height,
                            const std::string& app_view,
                            const std::vector<std::string>& extensions_extra,
                            const std::vector<std::string>& layers_extra);

    GLFWwindow* get_window() const { return m_window; };
    VkInstance get_instance() const { return m_instance; };
    VkSurfaceKHR get_surface() const { return m_surface; };

  private:
    GLFWwindow* m_window{nullptr};
    VkInstance m_instance{nullptr};
    VkSurfaceKHR m_surface{nullptr};

    DestructorQueue m_destructing;
};
