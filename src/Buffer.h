#pragma once
#include <PVPCommandBuffer/CommandBuffer.h>
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

class Buffer
{
public:
    Buffer(VkDeviceSize buffer_size, VkBufferUsageFlags buffer_usage, VmaMemoryUsage usage, VmaAllocationCreateFlags flags);
    ~Buffer();
    void destroy_buffer() const;

    const VkBuffer& get_buffer() const
    {
        return m_buffer;
    };

    const VmaAllocation& get_allocation() const
    {
        return m_allocation;
    };

    const VmaAllocationInfo& get_allocation_info() const
    {
        return m_allocation_info;
    };

    void copy_into_buffer(pvp::CommandBuffer& command_buffer, Buffer& destination);

private:
    VkBuffer m_buffer;
    VmaAllocation m_allocation;
    VmaAllocationInfo m_allocation_info;
};
