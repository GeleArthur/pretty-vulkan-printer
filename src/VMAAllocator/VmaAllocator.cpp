#include "VmaAllocator.h"

#include "../Context/Device.h"
#include "../Context/Instance.h"
#include "../Context/PhysicalDevice.h"
#include <tracy/Tracy.hpp>

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
    ZoneScoped;
    VmaAllocatorCreateInfo allocator_info{};
    allocator_info.instance = instance.get_instance();
    allocator_info.physicalDevice = physical_device.get_physical_device();
    allocator_info.device = device.get_device();

    vmaCreateAllocator(&allocator_info, &allocator.m_allocator);
}
