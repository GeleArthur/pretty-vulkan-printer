#pragma once
#include <array>
#include <globalconst.h>
#include <Buffer/Buffer.h>
#include <VMAAllocator/VmaAllocator.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    struct Context;
    // TODO: Copy paste of Image could be better???

    class StaticImage
    {
    public:
        void destroy(const Context& context) const;

        [[nodiscard]] VkImageView get_view() const
        {
            return m_view;
        };
        [[nodiscard]] VkImage get_image() const
        {
            return m_image;
        };

        void transition_layout(VkCommandBuffer command_buffer, VkImageLayout new_layout, VkPipelineStageFlags2 src_stage_mask, VkPipelineStageFlags2 dst_stage_mask, VkAccessFlags2 src_access_mask, VkAccessFlags2 dst_access_mask);
        void copy_from_buffer(VkCommandBuffer cmd, const Buffer& buffer) const;

    private:
        friend class ImageBuilder;

        VmaAllocation           m_allocation{};
        VmaAllocationInfo       m_allocation_info{};
        VmaAllocationCreateInfo m_allocation_create_info{};

        VkImageCreateInfo     m_create_info{};
        VkImageViewCreateInfo m_view_create_info{};

        VkImage     m_image{ VK_NULL_HANDLE };
        VkImageView m_view{ VK_NULL_HANDLE };

        VkImageLayout m_current_layout{ VK_IMAGE_LAYOUT_UNDEFINED };
    };
} // namespace pvp
