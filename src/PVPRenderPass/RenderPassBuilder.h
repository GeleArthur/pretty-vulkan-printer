#pragma once
#include <vulkan/vulkan_core.h>

class DestructorQueue;

namespace pvp
{
    class PhysicalDevice;
    class Swapchain;
    class RenderPassBuilder
    {
        public:
        // Needs swapchain for format. Could maybe be removed
        VkRenderPass build(Swapchain& swapchain, PhysicalDevice& device);

        private:
    };
} // namespace pvp
