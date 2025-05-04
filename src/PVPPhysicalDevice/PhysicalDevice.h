#pragma once
#include <vulkan/vulkan.h>

namespace pvp
{
    class PhysicalDevice final
    {
    public:
        explicit PhysicalDevice() = default;

        [[nodiscard]] const VkPhysicalDevice& get_physical_device() const;

    private:
        friend class LogicPhysicalQueueBuilder;
        VkPhysicalDevice m_physical_device{ VK_NULL_HANDLE };
    };
} // namespace pvp
