#pragma once
#include <vulkan/vulkan.h>

namespace pvp
{
    class PhysicalDevice
    {
    public:
        explicit PhysicalDevice() = default;

        const VkPhysicalDevice& get_physical_device();
        
    private:
        VkPhysicalDevice m_physical_device{ VK_NULL_HANDLE };
    };

} // namespace pvp
