#pragma once
#include "Swapchain.h"

#include <PVPImage/Image.h>
namespace pvp
{
    class RenderToSwapchain
    {
    public:
        explicit RenderToSwapchain(Swapchain* swapchain);
        void draw(VkCommandBuffer cmd, Image& image_to_blit);

    private:
        // VkImage swapchain_image;
        // VkImageView swapchain_view;
        Swapchain* m_swapchain;
    };
} // namespace pvp
