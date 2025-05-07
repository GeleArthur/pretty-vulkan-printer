#pragma once
#include "Swapchain.h"
namespace pvp
{
    class RenderToSwapchain
    {
    public:
        explicit RenderToSwapchain();
        void build_command_buffer();

    private:
        VkImage swapchain_image;
        VkImageView swapchain_view;
        Swapchain* m_swapchain;
    };
} // namespace pvp
