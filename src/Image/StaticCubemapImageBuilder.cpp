#include "StaticCubemapImageBuilder.h"

#include <VulkanExternalFunctions.h>
#include <stdexcept>
#include <Context/Device.h>
pvp::StaticCubemapImageBuilder& pvp::StaticCubemapImageBuilder::set_name(const std::string& name)
{
    m_name = name;
    return *this;
}
pvp::StaticCubemapImageBuilder& pvp::StaticCubemapImageBuilder::set_size(const VkExtent2D& size)
{
    m_size = size;
    return *this;
}
pvp::StaticCubemapImageBuilder& pvp::StaticCubemapImageBuilder::set_format(VkFormat format)
{
    m_format = format;
    return *this;
}
pvp::StaticCubemapImageBuilder& pvp::StaticCubemapImageBuilder::set_usage(VkImageUsageFlags usage)
{
    m_usage = usage;
    return *this;
}
pvp::StaticCubemapImageBuilder& pvp::StaticCubemapImageBuilder::set_aspect_flags(VkImageAspectFlags aspect_flags)
{
    m_aspect_flags = aspect_flags;
    return *this;
}
pvp::StaticCubemapImageBuilder& pvp::StaticCubemapImageBuilder::set_memory_usage(VmaMemoryUsage memory_usage)
{
    m_memory_usage = memory_usage;
    return *this;
}
void pvp::StaticCubemapImageBuilder::build(const Context& context, StaticCubemapImage& image) const
{
    VkExtent3D image_size = VkExtent3D(m_size.width, m_size.height, 1);

    VkImageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.usage = m_usage;
    create_info.format = m_format;
    create_info.extent = image_size;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.arrayLayers = 6;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    create_info.mipLevels = 1;
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;

    image.m_create_info = create_info;

    VmaAllocationCreateInfo allocation_info{};
    allocation_info.usage = m_memory_usage;
    image.m_allocation_create_info = allocation_info;

    if (vmaCreateImage(context.allocator->get_allocator(), &create_info, &allocation_info, &image.m_image, &image.m_allocation, &image.m_allocation_info) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed creating image");
    }
    image.m_current_layout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image.m_image;
    view_info.format = m_format;
    view_info.subresourceRange.aspectMask = m_aspect_flags;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    image.m_view_create_info = view_info;

    if (vkCreateImageView(context.device->get_device(), &view_info, nullptr, &image.m_view) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed creating image view");
    }

#if defined(_DEBUG)

    VkDebugUtilsObjectNameInfoEXT image_debug{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = VK_OBJECT_TYPE_IMAGE,
        .objectHandle = reinterpret_cast<uint64_t>(image.m_image),
        .pObjectName = m_name.c_str()
    };

    VulkanInstanceExtensions::vkSetDebugUtilsObjectNameEXT(context.device->get_device(), &image_debug);

#endif
}