#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace pvp
{
    // TODO: This is bad you aren't allowed to just create a Queue from nothing
    struct Queue
    {
        uint32_t                 family_index{};
        VkQueue                  queue{ VK_NULL_HANDLE };
        VkBool32                 can_present{};
        VkQueueFamilyProperties2 properties{};
    };

    class QueueFamilies
    {
    public:
        explicit QueueFamilies() = default;
        Queue* get_queue_family(VkQueueFlagBits required_flags, VkBool32 present_required);

    private:
        friend class LogicPhysicalQueueBuilder;
        std::vector<Queue> m_queues;
    };
} // namespace pvp
