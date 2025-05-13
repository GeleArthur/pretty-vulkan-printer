#include "Buffer.h"

#include <any>
#include <iostream>
#include <span>
#include <CommandBuffer/CommandPool.h>
#include <VMAAllocator/VmaAllocator.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

void pvp::Buffer::copy_from_buffer(VkCommandBuffer command_buffer, Buffer& source) const
{
    VkBufferCopy copy_region{};
    copy_region.size = m_allocation_info.size;
    vkCmdCopyBuffer(command_buffer, source.m_buffer, m_buffer, 1, &copy_region);
}

void pvp::Buffer::destroy() const
{
    vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
}

void pvp::Buffer::copy_data_into_buffer(std::span<const std::byte> input_data) const
{
    memcpy(m_allocation_info.pMappedData, input_data.data(), input_data.size());
}
