#include "ImageBuilder.h"

#include "Image.h"

#include <exception>

pvp::ImageBuilder& pvp::ImageBuilder::set_size(const VkExtent2D& size)
{
    m_size = size;
    return *this;
}

pvp::ImageBuilder& pvp::ImageBuilder::set_format(VkFormat format)
{
    m_format = format;
    return *this;
}

pvp::ImageBuilder& pvp::ImageBuilder::set_usage(VkImageUsageFlags usage)
{
    m_usage = usage;
    return *this;
}

pvp::ImageBuilder& pvp::ImageBuilder::set_memory_usage(VmaMemoryUsage memory_usage)
{
    m_memory_usage = memory_usage;
    return *this;
}

pvp::ImageBuilder& pvp::ImageBuilder::set_aspect_flags(VkImageAspectFlags aspect_flags)
{
    m_aspect_flags = aspect_flags;
    return *this;
}

void pvp::ImageBuilder::build(VkDevice device, VmaAllocator allocator, pvp::Image& image) const
{
    VkImageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.usage = m_usage;
    create_info.format = m_format;
    create_info.extent = VkExtent3D(m_size.width, m_size.height, 1);
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.arrayLayers = 1;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    create_info.mipLevels = 1;
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo allocation_info{};
    allocation_info.usage = m_memory_usage;

    if (vmaCreateImage(allocator, &create_info, &allocation_info, &image.m_image, &image.m_allocation, &image.m_allocation_info) != VK_SUCCESS)
    {
        throw std::exception("Failed creating image");
    }
    image.m_extent = VkExtent2D(m_size.width, m_size.height);
    image.m_debug_create_info = create_info;
    image.m_current_layout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image.m_image;
    view_info.format = m_format;
    view_info.subresourceRange.aspectMask = m_aspect_flags;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    image.m_aspect_flags = m_aspect_flags;

    if (vkCreateImageView(device, &view_info, nullptr, &image.m_view) != VK_SUCCESS)
    {
        throw std::exception("Failed creating image view");
    }
}
