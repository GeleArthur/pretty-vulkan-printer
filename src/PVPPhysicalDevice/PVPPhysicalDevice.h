#pragma once

#include <DestructorQueue.h>
#include <vulkan/vulkan.h>

#include <string>

struct QueueFamilyIndices
{
    uint32_t graphics_family{};
    uint32_t transfer_family{};
    uint32_t compute_family{};
    uint32_t present_family{};

    bool success{};
};

const QueueFamilyIndices& get_queue_family_indices(const VkPhysicalDevice physical_device, const VkSurfaceKHR surface);

namespace pvp
{
class Instance;
class PhysicalDevice
{
  public:
    explicit PhysicalDevice(Instance* pvp_instance, const std::vector<std::string>& device_extensions);

    VkDevice get_device();
    VkPhysicalDevice get_physical_device();
    VkQueue get_graphics_queue();
    VkQueue get_compute_queue();
    VkQueue get_transfer_queue();
    VkQueue get_present_queue();

  private:
    VkDevice m_device;
    VkPhysicalDevice m_physical_device{};
    VkQueue m_graphics_queue;
    VkQueue m_compute_queue;
    VkQueue m_transfer_queue;
    VkQueue m_present_queue;
    Instance* m_instance{};
    DestructorQueue m_destructor;
};

} // namespace pvp
