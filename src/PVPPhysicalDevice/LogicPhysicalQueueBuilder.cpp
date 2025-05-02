#include "LogicPhysicalQueueBuilder.h"

#include <stdexcept>
#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <PVPInstance/Instance.h>
#include <PVPWindow/WindowSurface.h>
#include "QueueFamillies.h"

#include <complex.h>
#include <complex.h>

namespace pvp
{
    void LogicPhysicalQueueBuilder::build(const Instance& instance, const WindowSurface& window_surface, PhysicalDevice& physical_device, Device& device, QueueFamilies& queue_families)
    {
        m_extentions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        VkPhysicalDevice m_physical_device = get_best_device(instance, window_surface);

        if (m_physical_device == VK_NULL_HANDLE)
        {
            throw std::runtime_error("No device found that works");
        }
    }

    VkPhysicalDevice LogicPhysicalQueueBuilder::get_best_device(const Instance& instance, const WindowSurface& window_surface) const
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance.get_instance(), &device_count, nullptr);
        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance.get_instance(), &device_count, devices.data());

        uint32_t         best_score = 0;
        VkPhysicalDevice best_device = VK_NULL_HANDLE;

        for (const VkPhysicalDevice& physical_device : devices)
        {
            uint32_t extension_count{};
            vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
            std::vector<VkExtensionProperties> available_extensions(extension_count);
            vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());

            bool has_extension = true;
            for (const char* extension : m_extentions)
            {
                if (std::ranges::find_if(
                        available_extensions,
                        [&](const VkExtensionProperties& ex) {
                            return std::strcmp(ex.extensionName, extension);
                        }) == available_extensions.end())
                {
                    has_extension = false;
                    break;
                }
            }

            if (!has_extension)
                continue;

            // Supports swapchain
            uint32_t format_count{};
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, window_surface.get_surface(), &format_count, nullptr);
            uint32_t present_mode_count;
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, window_surface.get_surface(), &present_mode_count, nullptr);

            if (format_count <= 0 || present_mode_count <= 0)
                continue;

            // VkPhysicalDeviceFeatures2 features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
            // vkGetPhysicalDeviceFeatures2(physical_device, &features);

            // const QueueFamilyIndicesStruct& result = get_queue_family_indices(device, surface);
            // if (result.success == false)
            // {
            //     continue;
            // }

            // What is the best gpu?

            VkPhysicalDeviceProperties device_properties;
            vkGetPhysicalDeviceProperties(physical_device, &device_properties);

            uint32_t score = 1;
            if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                score += 10;
            }

            if (score > best_score)
            {
                best_device = physical_device;
                best_score = score;
            }
        }

        return best_device;
    }

    std::tuple<bool, QueueFamilies> LogicPhysicalQueueBuilder::get_queue_family_indices(const VkPhysicalDevice& physical_device, const WindowSurface& window_surface)
    {
        QueueFamilies result{};

        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties2> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queue_family_count, queue_families.data());

        bool graphics_queue{}, compute_queue{}, transfer_queue{}, surface_queue{};
        bool success = false;

        for (int family_index = 0; family_index < queue_families.size(); ++family_index)
        {
            if (queue_families[family_index].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                if (graphics_queue == false)
                {
                    result.graphics_family.family_index = family_index;
                    graphics_queue = true;
                }
            }
            if (queue_families[family_index].queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                if (compute_queue == false)
                {
                    result.compute_family.family_index = family_index;
                    compute_queue = true;
                }
            }
            if (queue_families[family_index].queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                if (transfer_queue == false)
                {
                    result.transfer_family.family_index = family_index;
                    transfer_queue = true;
                }
            }

            VkBool32 is_supporting_queue{};
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, family_index, window_surface.get_surface(), &is_supporting_queue);
            if (is_supporting_queue)
            {
                result.present_family.family_index = family_index;
                surface_queue = true;
            }

            if (graphics_queue && compute_queue && transfer_queue && surface_queue)
            {
                success = true;
                break;
            }
        }

        return std::tuple{ success, result };
    }
} // namespace pvp