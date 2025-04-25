#pragma once
#include <vulkan/vulkan_core.h>

struct Semaphore
{
    VkSemaphore handle = VK_NULL_HANDLE;
    void        destroy(const VkDevice& device)
    {
        vkDestroySemaphore(device, handle, nullptr);
    };
};
