#pragma once
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace pvp
{

    class Buffer;

    class BufferBuilder final
    {
    public:
        explicit BufferBuilder() = default;

        BufferBuilder& set_size(VkDeviceSize buffer_size);
        BufferBuilder& set_usage(VkBufferUsageFlags buffer_usage);
        BufferBuilder& set_memory_usage(VmaMemoryUsage usage);
        BufferBuilder& set_flags(VmaAllocationCreateFlags flags);

        Buffer build(const VmaAllocator& allocator);

    private:
        VkDeviceSize             m_buffer_size{ 0 };
        VkBufferUsageFlags       m_buffer_usage{ 0 };
        VmaMemoryUsage           m_usage{ VMA_MEMORY_USAGE_AUTO };
        VmaAllocationCreateFlags m_flags{};
    };
} // namespace pvp
