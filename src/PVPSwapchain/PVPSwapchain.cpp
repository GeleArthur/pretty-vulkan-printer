#include "PVPSwapchain.h"
bool PVPSwapchain::does_device_support_swapchain(
    const VkPhysicalDevice device, const VkSurfaceKHR surface)
{
    uint32_t format_count{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

    return format_count > 0 && present_mode_count > 0;
}