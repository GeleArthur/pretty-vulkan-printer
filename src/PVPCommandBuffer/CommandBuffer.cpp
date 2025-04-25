#include "CommandBuffer.h"

#include <stdexcept>

namespace pvp
{
    CommandBuffer::CommandBuffer(PhysicalDevice& physical)
    {
        m_device = physical.get_device();
        VkCommandPoolCreateInfo pool_info {};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        pool_info.queueFamilyIndex = physical.get_queue_families().transfer_family.family_index;
        m_transfer_family = physical.get_queue_families().transfer_family;
        if (vkCreateCommandPool(physical.get_device(), &pool_info, nullptr, &m_single_use_transfer_command_pool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }

        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_info.queueFamilyIndex = physical.get_queue_families().graphics_family.family_index;

        if (vkCreateCommandPool(physical.get_device(), &pool_info, nullptr, &m_reset_graphics_command_pool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }

        VkCommandBufferAllocateInfo alloc_info {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = m_reset_graphics_command_pool;
        alloc_info.commandBufferCount = 1;

        vkAllocateCommandBuffers(m_device, &alloc_info, &m_graphics_command_buffer);
    }
    CommandBuffer::~CommandBuffer()
    {
        vkFreeCommandBuffers(m_device, m_reset_graphics_command_pool, 1, &m_graphics_command_buffer);
        vkDestroyCommandPool(m_device, m_single_use_transfer_command_pool, nullptr);
        vkDestroyCommandPool(m_device, m_reset_graphics_command_pool, nullptr);
    }
    VkCommandBuffer CommandBuffer::get_graphics_command_buffer() const
    {
        vkResetCommandBuffer(m_graphics_command_buffer, 0);
        return m_graphics_command_buffer;
    }

    VkCommandBuffer CommandBuffer::begin_single_use_transfer_command() const
    {
        VkCommandBufferAllocateInfo alloc_info {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = m_single_use_transfer_command_pool;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer;
        vkAllocateCommandBuffers(m_device, &alloc_info, &command_buffer);

        VkCommandBufferBeginInfo begin_info {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(command_buffer, &begin_info);

        return command_buffer;
    }
    void CommandBuffer::end_single_use_transfer_command(VkCommandBuffer command_buffer) const
    {
        vkEndCommandBuffer(command_buffer);
        VkSubmitInfo submit_info {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        vkQueueSubmit(m_transfer_family.queue, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_transfer_family.queue); // TODO: Return fence so we can go faster
        vkFreeCommandBuffers(m_device, m_single_use_transfer_command_pool, 1, &command_buffer);
    }
} // namespace pvp