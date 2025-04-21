#include "Image.h"

#include <stdexcept>
#include <PVPVMAAllocator/VmaAllocator.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    Image::Image(VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memory_usage, VkImageAspectFlags aspect_flags)
    {
        m_device = device;
        VkImageCreateInfo image_info {};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.extent.width = width;
        image_info.extent.height = height;
        image_info.extent.depth = 1;
        image_info.mipLevels = 1;
        image_info.arrayLayers = 1;
        image_info.format = format;
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_info.usage = usage;
        image_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocation_info {};
        allocation_info.usage = memory_usage;

        vmaCreateImage(PvpVmaAllocator::get_allocator(), &image_info, &allocation_info, &m_image, &m_allocation, nullptr);

        VkImageViewCreateInfo view_info {};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = m_image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = format;
        view_info.subresourceRange.aspectMask = aspect_flags;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &view_info, nullptr, &m_view) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture image view!");
        }
    }
    Image::~Image()
    {
        vmaDestroyImage(PvpVmaAllocator::get_allocator(), m_image, m_allocation);
        vkDestroyImageView(m_device, m_view, nullptr);
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
} // namespace pvp