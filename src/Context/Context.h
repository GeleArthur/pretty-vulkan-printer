#pragma once

#include <array>
#include <vector>
#include <vulkan/vulkan.h>

namespace pvp
{
    struct GlfwToRender;
}
namespace tracy
{
    class VkCtx;
}
namespace pvp
{
    class WindowSurface;
    class Swapchain;
    class Device;
    class PvpVmaAllocator;
    class PhysicalDevice;
    class Instance;
    class QueueFamilies;
    class DescriptorLayoutCreator;
    struct Context
    {
        Instance*                instance;
        PhysicalDevice*          physical_device;
        Device*                  device;
        PvpVmaAllocator*         allocator;
        QueueFamilies*           queue_families;
        DescriptorLayoutCreator* descriptor_creator;
        Swapchain*               swapchain;
        VkSurfaceKHR             surface;
        // WindowSurface*           window_surface;
        GlfwToRender*           gtfw_to_render;
        std::array<uint64_t, 2> invocation_count{};

#ifdef TRACY_ENABLE
        std::vector<tracy::VkCtx*> tracy_ctx;
#endif
    };

} // namespace pvp
