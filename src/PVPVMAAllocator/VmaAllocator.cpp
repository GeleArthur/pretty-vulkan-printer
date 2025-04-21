#include "VmaAllocator.h"

// bad singleton
static VmaAllocator allocator_instance;

VmaAllocator&       PvpVmaAllocator::get_allocator()
{
    return allocator_instance;
}

PvpVmaAllocator::PvpVmaAllocator(pvp::Instance& instance, pvp::PhysicalDevice& physical_device)
{
    VmaAllocatorCreateInfo allocator_info {};
    allocator_info.instance = instance.get_instance();
    allocator_info.physicalDevice = physical_device.get_physical_device();
    allocator_info.device = physical_device.get_device();

    vmaCreateAllocator(&allocator_info, &allocator_instance);
}
PvpVmaAllocator::~PvpVmaAllocator()
{
    vmaDestroyAllocator(allocator_instance);
}