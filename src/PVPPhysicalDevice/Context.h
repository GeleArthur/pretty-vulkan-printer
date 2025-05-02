#pragma once

#include <PVPInstance/Instance.h>
#include <PVPVMAAllocator/VmaAllocator.h>

#include "PhysicalDevice.h"
#include "QueueFamillies.h"
#include "PVPCommandBuffer/CommandBuffer.h"
#include "PVPDevice/Device.h"

namespace pvp
{
    struct Context
    {
        Device device;
        PvpVmaAllocator allocator;
        PhysicalDevice physical_device;
        QueueFamilies queue_families;
        CommandBuffer command_buffer;
    };
} // namespace pvp
