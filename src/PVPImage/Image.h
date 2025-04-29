#pragma once
#include "Sampler.h"

#include <PVPBuffer/Buffer.h>

namespace pvp
{
    class Image final
    {
        public:
        Image(VkDevice           device,
              uint32_t           width,
              uint32_t           height,
              VkFormat           format,
              VkImageUsageFlags  usage,
              VkImageAspectFlags aspect_flags,
              VmaMemoryUsage     memory_usage);
        ~Image();

        VkImageView              get_view() const;
        VkImage                  get_image() const;
        VmaAllocation            get_allocation() const;
        const VmaAllocationInfo& get_allocation_info() const;
        VkImageLayout            get_layout() const;

        void                     transition_layout(VkCommandBuffer& cmd, VkImageLayout new_layout);
        void                     copy_from_buffer(VkCommandBuffer& cmd, VkBuffer buffer) const;
        Sampler                  get_sampler() const;

        private:
        void              create_sampler();
        CommandBuffer*    m_command_buffer {};

        VmaAllocation     m_allocation {};
        VmaAllocationInfo m_allocation_info {};
        Sampler           m_sampler {};
        VkImage           m_image {};
        VkImageView       m_view {};
        VkImageLayout     m_old_layout { VK_IMAGE_LAYOUT_UNDEFINED };
        VkDevice          m_device {};
        uint32_t          m_width {};
        uint32_t          m_height {};
    };
} // namespace pvp
