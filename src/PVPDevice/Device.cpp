#include "Device.h"
#include <vulkan/vulkan.h>

void pvp::Device::destroy() const
{
    vkDestroyDevice(m_device, nullptr);
}

VkDevice pvp::Device::get_device() const
{
    return m_device;
}
