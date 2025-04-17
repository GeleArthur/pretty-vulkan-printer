#pragma once

#include <vulkan/vulkan.h>

class PVPSwapchain
{
  public:
    static bool does_device_support_swapchain(VkPhysicalDevice device, VkSurfaceKHR surface);

  private:
};
