#pragma once

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
} // namespace pvp
