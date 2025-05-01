#pragma once

#include <PVPInstance/Instance.h>
#include <PVPVMAAllocator/VmaAllocator.h>
namespace pvp
{
    struct Context
    {
        VkDevice     device;
        VmaAllocator allocator;
    };

} // namespace pvp
