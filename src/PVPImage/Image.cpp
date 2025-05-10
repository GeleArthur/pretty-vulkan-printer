#include "Image.h"

#include "TransitionLayout.h"
#include "PVPPhysicalDevice/Context.h"
#include <PVPDevice/Device.h>
#include <PVPVMAAllocator/VmaAllocator.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    void Image::destroy(const Context& context) const
    {
        vmaDestroyImage(context.allocator->get_allocator(), m_image, m_allocation);
        vkDestroyImageView(context.device->get_device(), m_view, nullptr);
    }

    VkImageView Image::get_view() const
    {
        return m_view;
    }

    VkImage Image::get_image() const
    {
        return m_image;
    }

    VmaAllocation Image::get_allocation() const
    {
        return m_allocation;
    }

    const VmaAllocationInfo& Image::get_allocation_info() const
    {
        return m_allocation_info;
    }

    VkImageLayout Image::get_layout() const
    {
        return m_current_layout;
    }
    VkExtent2D Image::get_size() const
    {
        return m_extent;
    }

    void Image::transition_layout(VkCommandBuffer cmd, VkImageLayout new_layout)
    {
        VkImageSubresourceRange range{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS
        };

        image_layout_transition(cmd, m_image, m_current_layout, new_layout, range);
        m_current_layout = new_layout;
    }

    void Image::transition_layout(VkCommandBuffer       command_buffer,
                                  VkImageLayout         new_layout,
                                  VkPipelineStageFlags2 src_stage_mask,
                                  VkPipelineStageFlags2 dst_stage_mask,
                                  VkAccessFlags2        src_access_mask,
                                  VkAccessFlags2        dst_access_mask)
    {
        VkImageSubresourceRange range{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS
        };
        pvp::image_layout_transition(command_buffer, m_image, src_stage_mask, dst_stage_mask, src_access_mask, dst_access_mask, m_current_layout, new_layout, range);
        m_current_layout = new_layout;
    }

    void Image::copy_from_buffer(VkCommandBuffer cmd, const Buffer& buffer) const
    {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = VkExtent3D{ m_extent.width, m_extent.height, 1 };

        vkCmdCopyBufferToImage(cmd,
                               buffer.get_buffer(),
                               m_image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1,
                               &region);
    }

} // namespace pvp
