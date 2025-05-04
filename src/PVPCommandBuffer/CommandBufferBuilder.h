#pragma once
#include "PVPPhysicalDevice/QueueFamillies.h"

#include <PVPDevice/Device.h>
#include <vulkan/vulkan.h>

class CommandBufferBuilder final
{
public:
    CommandBufferBuilder() = default;

private:
    VkCommandPoolCreateFlags m_flags;
    pvp::QueueFamily         m_queue;
};
