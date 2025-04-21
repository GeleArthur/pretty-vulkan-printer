#include "Buffer.h"

#include <PVPVMAAllocator/VmaAllocator.h>
#include <vulkan/vulkan.h>

Buffer::Buffer(VkDeviceSize buffer_size, VmaMemoryUsage usage, VkBufferUsageFlagBits buffer_usage)
{
    VkBufferCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = buffer_size;
    create_info.usage = buffer_usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocation_create_info {};
    allocation_create_info.usage = usage;

    vmaCreateBuffer(PvpVmaAllocator::get_allocator(), &create_info, &allocation_create_info, &buffer, &allocation, nullptr);
}
Buffer::~Buffer()
{
    destroy_buffer();
}
void Buffer::destroy_buffer() const
{
    vmaDestroyBuffer(PvpVmaAllocator::get_allocator(), buffer, allocation);
}