#pragma once
#include "Swapchain.h"

#include <Image/Image.h>

namespace pvp
{
    class BlitToSwapchain
    {
    public:
        explicit BlitToSwapchain(const Context& context, Image& source);
        void draw(const FrameContext& cmd, uint32_t swapchain_image_index);

    private:
        Image&         m_source_image;
        const Context& m_context;
    };
} // namespace pvp
