#pragma once

#include <DestructorQueue.h>
#include <vulkan/vulkan.h>

#include <string>

// Could be better

namespace pvp
{
    // TODO: Is context not device
    class Instance;

    class Device
    {
    public:
        explicit Device(Instance* pvp_instance, const std::vector<std::string>& device_extensions);

        VkDevice         get_device() const;
        VkPhysicalDevice get_physical_device() const;

        const QueueFamilies& get_queue_families() const
        {
            return m_queue_families;
        };

    private:
        VkDevice         m_device;
        VkPhysicalDevice m_physical_device{};
        QueueFamilies    m_queue_families;
        Instance*        m_instance{};
        DestructorQueue  m_destructor;
    };
} // namespace pvp
