#include "BufferBuilder.h"

#include "Buffer.h"

#include <PVPVMAAllocator/VmaAllocator.h>

namespace pvp
{
    BufferBuilder& BufferBuilder::set_size(const VkDeviceSize buffer_size)
    {
        m_buffer_size = buffer_size;
        return *this;
    }
    BufferBuilder& BufferBuilder::set_usage(const VkBufferUsageFlags buffer_usage)
    {
        m_buffer_usage = buffer_usage;
        return *this;
    }
    BufferBuilder& BufferBuilder::set_memory_usage(const VmaMemoryUsage usage)
    {
        m_usage = usage;
        return *this;
    }
    BufferBuilder& BufferBuilder::set_flags(const VmaAllocationCreateFlags flags)
    {
        m_flags = flags;
        return *this;
    }
    Buffer BufferBuilder::build(const VmaAllocator& allocator)
    {
        Buffer             result;
        VkBufferCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.size = m_buffer_size;
        create_info.usage = m_buffer_usage;

        VmaAllocationCreateInfo allocation_create_info{};
        allocation_create_info.usage = m_usage;
        allocation_create_info.flags = m_flags;

        if (vmaCreateBuffer(allocator, &create_info, &allocation_create_info, &result.m_buffer, &result.m_allocation, &result.m_allocation_info) != VK_SUCCESS)
        {
            throw std::exception("Can't create buffer");
        }
        return result;
    }
} // namespace pvp