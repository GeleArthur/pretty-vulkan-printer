#pragma once
#include <Buffer.h>
namespace pvp
{
    class Image final
    {
        public:
        Image(VkDevice device, uint32_t width, uint32_t height, VkFormat format,  VkImageLayout layout, VkImageUsageFlags usage, VmaMemoryUsage memory_usage, VkImageAspectFlags aspect_flags);
        ~Image();

        VkImageView   get_view() const;
        VkImage       get_image() const;
        VmaAllocation get_allocation() const;

        private:
        VmaAllocation m_allocation {};
        VkImage       m_image {};
        VkImageView   m_view {};
        VkDevice      m_device {};
    };
} // namespace pvp
