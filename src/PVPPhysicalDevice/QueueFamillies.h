#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>

namespace pvp
{
    struct QueueFamily
    {
        std::uint32_t family_index{};
        VkQueue       queue{};
    };

    struct QueueFamilies
    {
        QueueFamily graphics_family{};
        QueueFamily transfer_family{};
        QueueFamily compute_family{};
        QueueFamily present_family{};
    };
} // namespace pvp
