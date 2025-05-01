#pragma once
#include <PVPPhysicalDevice/Device.h>
#include <vulkan/vulkan.h>

class CommandBufferBuilder final
{
public:
    CommandBufferBuilder() = default;

private:
    VkCommandPoolCreateFlags m_flags;
    QueueFamily m_queue;
};
