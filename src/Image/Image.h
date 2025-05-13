#pragma once
#include "Sampler.h"

#include <Buffer/Buffer.h>

#include "../Context/Context.h"

namespace pvp
{
    class Image final
    {
    public:
        Image() = default;
        void destroy(const Context& context) const;

        [[nodiscard]] VkImageView              get_view() const;
        [[nodiscard]] VkImage                  get_image() const;
        [[nodiscard]] VmaAllocation            get_allocation() const;
        [[nodiscard]] const VmaAllocationInfo& get_allocation_info() const;
        [[nodiscard]] VkImageLayout            get_layout() const;
        [[nodiscard]] VkExtent2D               get_size() const;

        void transition_layout(VkCommandBuffer cmd, VkImageLayout new_layout);
        void transition_layout(VkCommandBuffer command_buffer, VkImageLayout new_layout, VkPipelineStageFlags2 src_stage_mask, VkPipelineStageFlags2 dst_stage_mask, VkAccessFlags2 src_access_mask, VkAccessFlags2 dst_access_mask);
        void copy_from_buffer(VkCommandBuffer cmd, const Buffer& buffer) const;

    private:
        friend class ImageBuilder;

        VmaAllocation     m_allocation{};
        VmaAllocationInfo m_allocation_info{};
        VkImageCreateInfo m_debug_create_info;
        VkImage           m_image{ VK_NULL_HANDLE };
        VkImageView       m_view{ VK_NULL_HANDLE };
        VkImageLayout     m_current_layout{ VK_IMAGE_LAYOUT_UNDEFINED };
        VkExtent2D        m_extent{};
    };
} // namespace pvp
