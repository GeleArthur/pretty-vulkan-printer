#pragma once

#include "Image.h"
#include "VMAAllocator/VmaAllocator.h"

#include <vulkan/vulkan.h>

namespace pvp
{
    class ImageBuilder final
    {
    public:
        ImageBuilder() = default;

        ImageBuilder& set_size(const VkExtent2D& size);
        ImageBuilder& set_format(VkFormat format);
        ImageBuilder& set_usage(VkImageUsageFlags usage);
        ImageBuilder& set_aspect_flags(VkImageAspectFlags aspect_flags);
        ImageBuilder& set_memory_usage(VmaMemoryUsage memory_usage);

        void build(const Context& context, pvp::Image& image) const;

    private:
        VkExtent2D         m_size{};
        VkFormat           m_format{ VK_FORMAT_R8G8B8_SRGB };
        VkImageUsageFlags  m_usage{ VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT };
        VkImageAspectFlags m_aspect_flags{ VK_IMAGE_ASPECT_COLOR_BIT };
        VmaMemoryUsage     m_memory_usage{ VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE };
    };
} // namespace pvp