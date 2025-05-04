#pragma once

#include <vulkan/vulkan.h>

namespace pvp
{
    class Device
    {
    public:
        explicit Device() = default;
        void destroy() const;

        VkDevice get_device() const;

    private:
        friend class LogicPhysicalQueueBuilder;
        VkDevice m_device;
    };
} // namespace pvp
