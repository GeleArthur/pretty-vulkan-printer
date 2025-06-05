#include "QueueFamilies.h"

#include "spdlog/spdlog.h"
pvp::Queue* pvp::QueueFamilies::get_queue_family(VkQueueFlagBits required_flags, VkBool32 present_required)
{
    const auto result = std::ranges::find_if(m_queues, [&](const Queue& queue) {
        return (!present_required || queue.can_present) && (queue.properties.queueFamilyProperties.queueFlags & required_flags) == required_flags;
    });

    if (result == m_queues.end())
    {
        return nullptr;
    }

    return &*result;
}