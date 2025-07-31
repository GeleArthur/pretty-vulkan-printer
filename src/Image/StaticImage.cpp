#include "StaticImage.h"
#include "TransitionLayout.h"

#include <Context/Context.h>
#include <Context/Device.h>

void pvp::StaticImage::destroy(const Context& context) const
{
    vmaDestroyImage(context.allocator->get_allocator(), m_image, m_allocation);
    vkDestroyImageView(context.device->get_device(), m_view, nullptr);
}

void pvp::StaticImage::transition_layout(VkCommandBuffer       command_buffer,
                                         VkImageLayout         new_layout,
                                         VkPipelineStageFlags2 src_stage_mask,
                                         VkPipelineStageFlags2 dst_stage_mask,
                                         VkAccessFlags2        src_access_mask,
                                         VkAccessFlags2        dst_access_mask)
{
    VkImageSubresourceRange range{
        .aspectMask = m_view_create_info.subresourceRange.aspectMask,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS
    };
    image_layout_transition(command_buffer, m_image, src_stage_mask, dst_stage_mask, src_access_mask, dst_access_mask, m_current_layout, new_layout, range);
    m_current_layout = new_layout;
}

void pvp::StaticImage::copy_from_buffer(VkCommandBuffer cmd, const Buffer& buffer) const
{
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = m_view_create_info.subresourceRange.aspectMask;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = VkExtent3D{ m_create_info.extent.width, m_create_info.extent.height, 1 };

    vkCmdCopyBufferToImage(cmd,
                           buffer.get_buffer(),
                           m_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &region);
}