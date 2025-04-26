#pragma once

#include <vulkan/vulkan.h>

struct Fence
{
    VkFence  handle = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE; // :(
    void     destroy() const
    {
        vkDestroyFence(device, handle, nullptr);
    };
};