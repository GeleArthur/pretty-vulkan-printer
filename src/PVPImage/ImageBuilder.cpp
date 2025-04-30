#include "ImageBuilder.h"

#include "Image.h"
pvp::ImageBuilder& pvp::ImageBuilder::SetSize(const VkExtent2D& size)
{
    m_size = size;
    return *this;
}
pvp::ImageBuilder& pvp::ImageBuilder::SetFormat(VkFormat format)
{
    m_format = format;
    return *this;
}
pvp::ImageBuilder& pvp::ImageBuilder::SetUsage(VkImageUsageFlags usage)
{
    m_usage = usage;
    return *this;
}
pvp::ImageBuilder& pvp::ImageBuilder::SetMemoryUsage(VmaMemoryUsage memory_usage)
{
    m_memory_usage = memory_usage;
    return *this;
}
pvp::ImageBuilder& pvp::ImageBuilder::SetAspectFlags(VkImageAspectFlags aspect_flags)
{
    m_aspect_flags = aspect_flags;
    return *this;
}

void pvp::ImageBuilder::build(VkDevice device, VmaAllocator allocator, pvp::Image& image) const
{
    VkImageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.extent = VkExtent3D(m_size.width, m_size.height, 1);
    create_info.format = m_format;
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
    image.m_extent = VkExtent3D(m_size.width, m_size.height, 1);

    VkImageViewCreateInfo view_info{};
    view_info.image = image.m_image;
    view_info.format = m_format;
    view_info.subresourceRange.aspectMask = m_aspect_flags;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &view_info, nullptr, &image.m_view) != VK_SUCCESS)
    {
        throw std::exception("Failed creating image view");
    }
}