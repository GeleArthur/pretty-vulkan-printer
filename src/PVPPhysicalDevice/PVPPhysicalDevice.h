#pragma once

#include <DestructorQueue.h>
#include <vulkan/vulkan.h>

#include <string>

class PVPInstance;
class PVPPhysicalDevice
{
  public:
    explicit PVPPhysicalDevice(PVPInstance* pvp_instance, const std::vector<std::string>& device_extensions);

  private:
    VkDevice m_device;
    VkPhysicalDevice m_physical_device{};
    PVPInstance* m_instance{};
    DestructorQueue m_destructor;
    VkQueue m_graphics_queue;
    VkQueue m_compute_queue;
    VkQueue m_present_queue;
};
