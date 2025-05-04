#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>

namespace pvp
{
    struct QueueFamily
    {
        std::uint32_t family_index{};
        VkQueue       queue{ VK_NULL_HANDLE };
    };

    struct QueueFamilies
    {
        QueueFamily graphics_present_family{};
        QueueFamily transfer_family{};
        QueueFamily compute_family{};
    };
} // namespace pvp
