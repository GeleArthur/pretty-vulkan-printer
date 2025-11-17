#include "Device.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

pvp::Device::~Device()
{
    vkDestroyDevice(m_device, nullptr);
}

VkDevice pvp::Device::get_device() const
{
    return m_device;
}
