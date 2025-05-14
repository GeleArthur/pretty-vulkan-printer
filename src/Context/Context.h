#pragma once

#include <vulkan/vulkan.h>

namespace pvp
{
    class Swapchain;
    class Device;
    class PvpVmaAllocator;
    class PhysicalDevice;
    class Instance;
    class QueueFamilies;
    class DescriptorCreator;
    struct Context
    {
        Instance*          instance;
        PhysicalDevice*    physical_device;
        Device*            device;
        PvpVmaAllocator*   allocator;
        QueueFamilies*     queue_families;
        DescriptorCreator* descriptor_creator;
        Swapchain*         swapchain;
    };

    struct ImageInfo
    {
        VkFormat   depth_format;
        VkFormat   color_format;
        VkExtent2D image_size;
    };

} // namespace pvp
