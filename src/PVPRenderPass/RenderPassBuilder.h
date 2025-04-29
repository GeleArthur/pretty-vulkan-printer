#pragma once
#include <vulkan/vulkan_core.h>

class DestructorQueue;

// Maybe make a special renderpass for the swapchain
// Better idea use that new thing. No more render passes

namespace pvp
{
    class Device;
    class Swapchain;
    class RenderPassBuilder
    {
    public:
        // Needs swapchain for format. Could maybe be removed
        VkRenderPass build(Swapchain& swapchain, Device& device);

    private:
    };
} // namespace pvp
