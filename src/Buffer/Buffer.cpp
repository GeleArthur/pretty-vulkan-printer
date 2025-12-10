#include "Buffer.h"

#include <span>
#include <VMAAllocator/VmaAllocator.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

void pvp::Buffer::copy_from_buffer(VkCommandBuffer command_buffer, Buffer& source, const VkBufferCopy& copy_region) const
{
    vkCmdCopyBuffer(command_buffer, source.m_buffer, m_buffer, 1, &copy_region);
}
void pvp::Buffer::copy_from_buffer(VkCommandBuffer command_buffer, Buffer& source) const
{
    copy_from_buffer(command_buffer, source, VkBufferCopy{ 0, 0, get_size() });
}

void pvp::Buffer::destroy() const
{
    vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
}

void pvp::Buffer::copy_data_into_buffer(std::span<const std::byte> input_data) const
{
    memcpy(m_allocation_info.pMappedData, input_data.data(), input_data.size());
}
