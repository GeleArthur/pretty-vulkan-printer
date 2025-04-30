#pragma once

#include <any>
#include <span>
#include <PVPCommandBuffer/CommandBuffer.h>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>
namespace pvp
{
    class Buffer final
    {
    public:
        explicit Buffer() = default;
        void destroy() const;

        void copy_from_buffer(pvp::CommandBuffer& command_buffer, Buffer& source) const;

        [[nodiscard]] const VkBuffer& get_buffer() const
        {
            return m_buffer;
        }
        [[nodiscard]] const VmaAllocation& get_allocation() const
        {
            return m_allocation;
        }
        [[nodiscard]] const VmaAllocationInfo& get_allocation_info() const
        {
            return m_allocation_info;
        }

        void set_image_data(std::span<const std::byte> input_data) const;

    private:
        friend class BufferBuilder;
        VkBuffer          m_buffer{ VK_NULL_HANDLE };
        VmaAllocation     m_allocation{ VK_NULL_HANDLE };
        VmaAllocationInfo m_allocation_info{};
    };
} // namespace pvp