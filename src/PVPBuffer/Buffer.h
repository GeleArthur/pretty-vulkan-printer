#pragma once

#include <PVPCommandBuffer/CommandBuffer.h>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>
namespace pvp
{
    class Buffer final
    {
    public:
        explicit Buffer();

        void copy_from_buffer(pvp::CommandBuffer& command_buffer, Buffer& source) const;
        void destroy() const; // Not too happy with this as you have to remember to call destroy.

        const VkBuffer& get_buffer() const
        {
            return m_buffer;
        }
        const VmaAllocation& get_allocation() const
        {
            return m_allocation;
        }
        const VmaAllocationInfo& get_allocation_info() const
        {
            return m_allocation_info;
        }

        void input_data(const void* data, size_t size) const;

    private:
        friend class BufferBuilder;
        VkBuffer          m_buffer{ VK_NULL_HANDLE };
        VmaAllocation     m_allocation{ VK_NULL_HANDLE };
        VmaAllocationInfo m_allocation_info{};
    };
} // namespace pvp