#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

struct Buffer
{
    Buffer(VkDeviceSize buffer_size, VmaMemoryUsage usage, VkBufferUsageFlagBits buffer_usage);
    ~Buffer();
    void          destroy_buffer() const;

    VkBuffer      buffer;
    VmaAllocation allocation;
};
