#include "Buffer.h"

#include <PVPCommandBuffer/CommandBuffer.h>
#include <PVPVMAAllocator/VmaAllocator.h>
#include <vulkan/vulkan.h>

Buffer::Buffer(VkDeviceSize buffer_size, VkBufferUsageFlags buffer_usage, VmaMemoryUsage usage, VmaAllocationCreateFlags flags)
{
    VkBufferCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.size = buffer_size;
    create_info.usage = buffer_usage;

    VmaAllocationCreateInfo allocation_create_info {};
    allocation_create_info.usage = usage;
    allocation_create_info.flags = flags;

    vmaCreateBuffer(PvpVmaAllocator::get_allocator(), &create_info, &allocation_create_info, &m_buffer, &m_allocation, &m_allocation_info);
}
Buffer::~Buffer()
{
    destroy_buffer();
}
void Buffer::destroy_buffer() const
{
    vmaDestroyBuffer(PvpVmaAllocator::get_allocator(), m_buffer, m_allocation);
}
void Buffer::copy_into_buffer(pvp::CommandBuffer& command_buffer, Buffer& destination)
{
    VkCommandBuffer buffer = command_buffer.begin_single_use_transfer_command();
    {
        VkBufferCopy copy_region {};
        copy_region.size = m_allocation_info.size;
        vkCmdCopyBuffer(buffer, m_buffer, destination.m_buffer, 1, &copy_region);
    }
    command_buffer.end_single_use_transfer_command(buffer);
}