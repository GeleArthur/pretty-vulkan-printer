#pragma once

#include <Context/Instance.h>
#include <Context/QueueFamilies.h>
#include <DescriptorSets/DescriptorPool.h>
#include <VMAAllocator/VmaAllocator.h>

namespace pvp
{
    struct Context
    {
        Instance*        instance;
        PhysicalDevice*  physical_device;
        Device*          device;
        PvpVmaAllocator* allocator;
        QueueFamilies*   queue_families;
        DescriptorPool*  descriptor_pool;
    };

    struct ImageInfo
    {
        VkFormat   depth_format;
        VkFormat   color_format;
        VkExtent2D image_size;
    };

} // namespace pvp
