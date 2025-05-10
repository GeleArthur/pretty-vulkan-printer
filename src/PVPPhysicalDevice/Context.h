#pragma once

#include <PVPBuffer/Buffer.h>
#include <PVPInstance/Instance.h>
#include <PVPPhysicalDevice/QueueFamilies.h>
#include <PVPVMAAllocator/VmaAllocator.h>

namespace pvp
{
    struct Context
    {
        Instance*        instance;
        PhysicalDevice*  physical_device;
        Device*          device;
        PvpVmaAllocator* allocator;
        QueueFamilies*   queue_families;
    };

    struct ImageInfo
    {
        VkFormat   depth_format;
        VkFormat   color_format;
        VkExtent2D image_size;
    };

} // namespace pvp
