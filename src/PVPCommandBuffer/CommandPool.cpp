#include "CommandPool.h"

#include <stdexcept>
#include <PVPDevice/Device.h>

namespace pvp
{
    CommandPool::CommandPool(const Context& context, const Queue& queue, const VkCommandPoolCreateFlags pool_flags)
        : m_device(context.device->get_device())
        , m_queue(queue)
    {
        m_device = context.device->get_device();
        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = pool_flags;
        pool_info.queueFamilyIndex = queue.family_index;
        if (vkCreateCommandPool(context.device->get_device(), &pool_info, nullptr, &m_command_pool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }
    }
    void CommandPool::destroy() const
    {
        vkDestroyCommandPool(m_device, m_command_pool, nullptr);
    }
    VkCommandBuffer CommandPool::begin_buffer() const
    {
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = m_command_pool;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer;
        vkAllocateCommandBuffers(m_device, &alloc_info, &command_buffer);

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(command_buffer, &begin_info);

        return command_buffer;
    }
    void CommandPool::end_buffer(VkCommandBuffer buffer, VkFence fence) const
    {
        vkEndCommandBuffer(buffer);

        VkCommandBufferSubmitInfo command_buffer_submit{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, .commandBuffer = buffer };

        VkSubmitInfo2 submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit_info.pCommandBufferInfos = &command_buffer_submit;
        submit_info.commandBufferInfoCount = 1;

        vkQueueSubmit2(m_queue.queue, 1, &submit_info, fence);
        if (fence == VK_NULL_HANDLE)
        {
            vkQueueWaitIdle(m_queue.queue);
            vkFreeCommandBuffers(m_device, m_command_pool, 1, &buffer);
        }
    }

    std::vector<VkCommandBuffer> CommandPool::allocate_buffers(const uint32_t count) const
    {
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = m_command_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = count;

        std::vector<VkCommandBuffer> buffers(count);
        vkAllocateCommandBuffers(m_device, &alloc_info, buffers.data());

        return buffers;
    }
    const Queue& CommandPool::get_queue() const
    {
        return m_queue;
    }
} // namespace pvp
