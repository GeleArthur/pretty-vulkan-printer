#include "VmaAllocator.h"

#include "PVPDevice/Device.h"
#include "PVPInstance/Instance.h"
#include "PVPPhysicalDevice/PhysicalDevice.h"

const VmaAllocator& pvp::PvpVmaAllocator::get_allocator() const
{
    return m_allocator;
}
void pvp::PvpVmaAllocator::destroy() const
{
    vmaDestroyAllocator(m_allocator);
}

void pvp::create_allocator(PvpVmaAllocator& allocator, const pvp::Instance& instance, const pvp::Device& device, const pvp::PhysicalDevice& physical_device)
{
    VmaAllocatorCreateInfo allocator_info{};
    allocator_info.instance = instance.get_instance();
    allocator_info.physicalDevice = physical_device.get_physical_device();
    allocator_info.device = device.get_device();

    vmaCreateAllocator(&allocator_info, &allocator.m_allocator);
}
