#include "Buffer.h"

#include <iostream>
#include <PVPCommandBuffer/CommandBuffer.h>
#include <PVPVMAAllocator/VmaAllocator.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

pvp::Buffer::Buffer()
{
}

void pvp::Buffer::copy_from_buffer(pvp::CommandBuffer& command_buffer, Buffer& source) const
{
    VkCommandBuffer buffer = command_buffer.begin_single_use_transfer_command();
    {
        VkBufferCopy copy_region{};
        copy_region.size = m_allocation_info.size;
        vkCmdCopyBuffer(buffer, source.m_buffer, m_buffer, 1, &copy_region);
    }
    command_buffer.end_single_use_transfer_command(buffer);
}

void pvp::Buffer::destroy() const
{
    vmaDestroyBuffer(PvpVmaAllocator::get_allocator(), m_buffer, m_allocation);
}
void pvp::Buffer::input_data(const void* data, const size_t size) const
{
    memcpy(m_allocation_info.pMappedData, data, size);
}