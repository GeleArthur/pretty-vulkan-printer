#pragma once

// #define VMA_STATIC_VULKAN_FUNCTIONS
// #define VMA_STATS_STRING_ENABLED 1
#include <vma/vk_mem_alloc.h>

#include <PVPInstance/Instance.h>
#include <PVPPhysicalDevice/Device.h>

class PvpVmaAllocator
{
public:
    static VmaAllocator& get_allocator();
    PvpVmaAllocator(pvp::Instance& instance, pvp::Device& physical_device);
    ~PvpVmaAllocator();
};
