#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include <globalconst.h>

namespace pvp
{
    struct DescriptorSets final
    {
        std::vector<VkDescriptorSet> sets{ MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE };
    };
} // namespace pvp
