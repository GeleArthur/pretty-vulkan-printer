#pragma once
#include "PVPPhysicalDevice/QueueFamilies.h"
#include <PVPPhysicalDevice/Context.h>

namespace pvp
{
    class CommandPool final
    {
    public:
        explicit CommandPool() = default;
        explicit CommandPool(const Context& context, const Queue& queue, VkCommandPoolCreateFlags pool_flags);
        void destroy() const;

        [[nodiscard]] VkCommandBuffer begin_buffer() const;
        void                          end_buffer(VkCommandBuffer buffer, VkFence fence = VK_NULL_HANDLE) const;

        [[nodiscard]] std::vector<VkCommandBuffer> allocate_buffers(uint32_t count) const;
        [[nodiscard]] const Queue&                 get_queue() const;

    private:
        VkDevice      m_device;
        Queue         m_queue;
        VkCommandPool m_command_pool{};
    };
} // namespace pvp
