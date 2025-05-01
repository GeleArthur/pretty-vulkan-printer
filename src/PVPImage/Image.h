#pragma once
#include "Sampler.h"

#include <PVPBuffer/Buffer.h>

namespace pvp
{
    class Image final
    {
    public:
        Image() = default;
        void destroy(VkDevice device);

        VkImageView get_view() const;
        VkImage get_image() const;
        VmaAllocation get_allocation() const;
        const VmaAllocationInfo& get_allocation_info() const;
        VkImageLayout get_layout() const;

        void transition_layout(VkCommandBuffer& cmd, VkImageLayout new_layout);
        void copy_from_buffer(VkCommandBuffer& cmd, const Buffer& buffer) const;
        Sampler get_sampler() const;

    private:
        friend class ImageBuilder;

        VmaAllocation m_allocation{};
        VmaAllocationInfo m_allocation_info{};
        VkImageCreateInfo m_debug_create_info;
        VkImage m_image{VK_NULL_HANDLE};
        VkImageView m_view{VK_NULL_HANDLE};
        VkImageLayout m_current_layout{VK_IMAGE_LAYOUT_UNDEFINED};
        Sampler m_sampler{};
        VkExtent3D m_extent{};
    };
} // namespace pvp
