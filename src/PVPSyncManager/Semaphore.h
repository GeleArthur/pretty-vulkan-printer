#pragma once
#include <vulkan/vulkan_core.h>

struct Semaphore
{
    VkSemaphore handle = VK_NULL_HANDLE;
    VkDevice    device = VK_NULL_HANDLE;
    void        destroy() const
    {
        vkDestroySemaphore(device, handle, nullptr);
    };
};
