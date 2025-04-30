#include "Image.h"

#include <stdexcept>
#include <PVPVMAAllocator/VmaAllocator.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    void Image::destroy(VkDevice device)
    {
        vmaDestroyImage(PvpVmaAllocator::get_allocator(), m_image, m_allocation);
        vkDestroyImageView(device, m_view, nullptr);
        m_sampler.destroy(device);
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

    // Mmmm could this be better???
    void Image::transition_layout(VkCommandBuffer& cmd, VkImageLayout new_layout)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = m_current_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags source_stage{};
        VkPipelineStageFlags destination_stage{};

        if (m_current_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (m_current_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (m_current_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        vkCmdPipelineBarrier(cmd,
                             source_stage,
                             destination_stage,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);
        m_current_layout = new_layout;
    }
    void Image::copy_from_buffer(VkCommandBuffer& cmd, const Buffer& buffer) const
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
        region.imageExtent = m_extent;

        vkCmdCopyBufferToImage(cmd,
                               buffer.get_buffer(),
                               m_image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1,
                               &region);
    }
    Sampler Image::get_sampler() const
    {
        return m_sampler;
    }
    void Image::create_sampler()
    {
        VkSamplerCreateInfo sampler_info{};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = VK_FILTER_LINEAR;
        sampler_info.minFilter = VK_FILTER_LINEAR;
        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.anisotropyEnable = VK_FALSE;
        sampler_info.maxAnisotropy = 0.0f;
        sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        sampler_info.unnormalizedCoordinates = VK_FALSE;
        sampler_info.compareEnable = VK_FALSE;
        sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_info.minLod = 0.0f;
        sampler_info.maxLod = static_cast<float>(1);
        sampler_info.mipLodBias = 0.0f;

        if (vkCreateSampler(m_device, &sampler_info, nullptr, &m_sampler.handle) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }
} // namespace pvp