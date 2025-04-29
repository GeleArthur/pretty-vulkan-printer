#pragma once
#include <vulkan/vulkan.h>

namespace pvp
{

    struct Sampler
    {
        VkSampler handle = VK_NULL_HANDLE;
        void      destroy(VkDevice device);
    };
} // namespace pvp
