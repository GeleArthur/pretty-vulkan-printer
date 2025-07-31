#pragma once
#include "Swapchain.h"

#include <Image/Image.h>

namespace pvp
{
    class BlitToSwapchain
    {
    public:
        explicit BlitToSwapchain(const Context& context, Swapchain& swapchain, Image& source);
        void draw(const FrameContext& cmd, uint32_t swapchain_image_index);

    private:
        Swapchain&     m_swapchain;
        Image&         m_source_image;
        const Context& m_context;
    };
} // namespace pvp
