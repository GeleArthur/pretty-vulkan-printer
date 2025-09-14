#pragma once
#include "StaticCubemapImage.h"

#include <string>
#include <Context/Context.h>
#include <VMAAllocator/VmaAllocator.h>
#include <vulkan/vulkan.h>
namespace pvp
{
    class StaticCubemapImageBuilder final
    {
    public:
        StaticCubemapImageBuilder() = default;

        StaticCubemapImageBuilder& set_name(const std::string& name);
        StaticCubemapImageBuilder& set_size(const VkExtent2D& size);
        StaticCubemapImageBuilder& set_format(VkFormat format);
        StaticCubemapImageBuilder& set_usage(VkImageUsageFlags usage);
        StaticCubemapImageBuilder& set_aspect_flags(VkImageAspectFlags aspect_flags);
        StaticCubemapImageBuilder& set_memory_usage(VmaMemoryUsage memory_usage);

        void build(const Context& context, StaticCubemapImage& image) const;

    private:
        std::string        m_name;
        VkExtent2D         m_size{};
        VkFormat           m_format{ VK_FORMAT_R8G8B8_SRGB };
        VkImageUsageFlags  m_usage{ VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT };
        VkImageAspectFlags m_aspect_flags{ VK_IMAGE_ASPECT_COLOR_BIT };
        VmaMemoryUsage     m_memory_usage{ VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE };
    };

} // namespace pvp
