#pragma once
#include <VMAAllocator/VmaAllocator.h>

namespace pvp
{
    class StaticCubemapImage final
    {
    public:
        void RenderToCubeMap();

    private:
        friend class StaticCubemapImageBuilder;

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
