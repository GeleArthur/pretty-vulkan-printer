#pragma once

// #define VMA_STATIC_VULKAN_FUNCTIONS
// #define VMA_STATS_STRING_ENABLED 1
#include <vma/vk_mem_alloc.h>

#include <PVPInstance/PVPInstance.h>
#include <PVPPhysicalDevice/PVPPhysicalDevice.h>

class PvpVmaAllocator
{
    public:
    static VmaAllocator& get_allocator();
    PvpVmaAllocator(pvp::Instance& instance, pvp::PhysicalDevice& physical_device);
    ~PvpVmaAllocator();
};
