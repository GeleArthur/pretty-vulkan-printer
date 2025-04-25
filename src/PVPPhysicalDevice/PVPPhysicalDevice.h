#pragma once

#include <DestructorQueue.h>
#include <vulkan/vulkan.h>

#include <string>

// Could be better
struct QueueFamily
{
    uint32_t family_index {};
    VkQueue  queue {};
};

struct QueueFamilies
{
    QueueFamily graphics_family {};
    QueueFamily transfer_family {};
    QueueFamily compute_family {};
    QueueFamily present_family {};
};

namespace pvp
{
    class Instance;
    class PhysicalDevice
    {
        public:
        explicit PhysicalDevice(Instance* pvp_instance, const std::vector<std::string>& device_extensions);

        VkDevice             get_device();
        VkPhysicalDevice     get_physical_device();
        const QueueFamilies& get_queue_families() const
        {
            return m_queue_families;
        };

        private:
        VkDevice         m_device;
        VkPhysicalDevice m_physical_device {};
        QueueFamilies    m_queue_families;
        Instance*        m_instance {};
        DestructorQueue  m_destructor;
    };

} // namespace pvp
