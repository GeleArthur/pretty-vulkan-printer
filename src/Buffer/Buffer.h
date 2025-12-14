#pragma once

#include "BufferBuilder.h"

#include <DestructorQueue.h>
#include <span>
#include <Context/Context.h>
#include <VMAAllocator/VmaAllocator.h>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    class Buffer final
    {
    public:
        explicit Buffer() = default;
        void destroy() const;

        void copy_from_buffer(VkCommandBuffer command_buffer, Buffer& source, const VkBufferCopy& copy_region) const;
        void copy_from_buffer(VkCommandBuffer command_buffer, Buffer& source) const;

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

        [[nodiscard]] VkDeviceSize get_size() const
        {
            return m_buffer_size;
        }

        template<typename T>
        void copy_data_into_buffer(std::span<T> input_data) const
        {
            memcpy(m_allocation_info.pMappedData, input_data.data(), input_data.size_bytes());
        };
        template<typename T>
        void copy_data_from_tmp_buffer(Context& context, VkCommandBuffer command_buffer, std::span<T> input_data, DestructorQueue& destructor_queue) const
        {
            Buffer tmp_buffer{};
            BufferBuilder()
                .set_size(input_data.size_bytes())
                .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
                .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
                .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
                .build(context.allocator->get_allocator(), tmp_buffer);
            destructor_queue.add_to_queue([tmp_buffer] { tmp_buffer.destroy(); });

            tmp_buffer.copy_data_into_buffer(input_data);
            copy_from_buffer(command_buffer, tmp_buffer);
        };

    private:
        friend class BufferBuilder;
        VkBuffer           m_buffer{ VK_NULL_HANDLE };
        VmaAllocator       m_allocator{ VK_NULL_HANDLE };
        VmaAllocation      m_allocation{ VK_NULL_HANDLE };
        VmaAllocationInfo  m_allocation_info{};
        VkBufferCreateInfo m_create_info{};
        VkDeviceSize       m_buffer_size{};
    };
} // namespace pvp
