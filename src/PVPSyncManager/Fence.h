#pragma once

#include <vulkan/vulkan.h>

struct Fence
{
    VkFence handle = VK_NULL_HANDLE;

    void destroy(const VkDevice device) const
    {
        vkDestroyFence(device, handle, nullptr);
    };
};