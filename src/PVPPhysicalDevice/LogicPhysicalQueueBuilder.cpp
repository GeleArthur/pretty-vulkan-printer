#include "LogicPhysicalQueueBuilder.h"

#include <stdexcept>
#include <vulkan/vulkan.h>

#include <set>
#include <vector>
#include <PVPInstance/Instance.h>
#include <PVPWindow/WindowSurface.h>
#include "QueueFamillies.h"
#include "PVPDevice/Device.h"

namespace pvp
{
    LogicPhysicalQueueBuilder& LogicPhysicalQueueBuilder::set_extensions(const std::vector<const char*>& extension)
    {
        m_extensions = extension;
        return *this;
    }

    void LogicPhysicalQueueBuilder::build(const Instance& instance, const WindowSurface& window_surface, PhysicalDevice& physical_device_out, Device& device_out, QueueFamilies& queue_families_out)
    {
        m_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        auto [physical_device, queue_families] = get_best_device(instance, window_surface);

        if (physical_device == VK_NULL_HANDLE)
        {
            throw std::runtime_error("No device found that works");
        }

        physical_device_out.m_physical_device = physical_device;

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};

        std::set unique_queue_families = {
            queue_families.compute_family.family_index,
            queue_families.graphics_present_family.family_index,
            queue_families.transfer_family.family_index,
        };
        float queue_priority = 1.0f;

        for (uint32_t queue_index : unique_queue_families)
        {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_index;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(queue_create_info);
        }

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.enabledExtensionCount = static_cast<uint32_t>(m_extensions.size());
        device_create_info.ppEnabledExtensionNames = m_extensions.data();

        VkPhysicalDeviceVulkan13Features features13 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = nullptr,
            .synchronization2 = true
        };

        VkPhysicalDeviceVulkan12Features features12 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &features13
        };

        VkPhysicalDeviceVulkan11Features features11 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = &features12
        };

        VkPhysicalDeviceFeatures  device_features{};
        VkPhysicalDeviceFeatures2 features2{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &features11,
            .features = device_features
        };

        device_create_info.pNext = &features2;

        if (vkCreateDevice(physical_device, &device_create_info, nullptr, &device_out.m_device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        VkDeviceQueueInfo2 info{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queue_families_out.graphics_present_family.family_index,
            .queueIndex = 0
        };

        vkGetDeviceQueue2(device_out.get_device(), &info, &queue_families_out.graphics_present_family.queue);
        info.queueFamilyIndex = queue_families_out.compute_family.family_index;
        vkGetDeviceQueue2(device_out.get_device(), &info, &queue_families_out.compute_family.queue);
        info.queueFamilyIndex = queue_families_out.transfer_family.family_index;
        vkGetDeviceQueue2(device_out.get_device(), &info, &queue_families_out.transfer_family.queue);

        queue_families_out = queue_families;
    }

    std::tuple<VkPhysicalDevice, QueueFamilies> LogicPhysicalQueueBuilder::get_best_device(const Instance& instance, const WindowSurface& window_surface) const
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance.get_instance(), &device_count, nullptr);
        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance.get_instance(), &device_count, devices.data());

        uint32_t                                    best_score = 0;
        std::tuple<VkPhysicalDevice, QueueFamilies> best_device;

        for (const VkPhysicalDevice& physical_device : devices)
        {
            uint32_t extension_count{};
            vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
            std::vector<VkExtensionProperties> available_extensions(extension_count);
            vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());

            bool has_extension = true;
            for (const char* extension : m_extensions)
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

            auto result = get_queue_family_indices(physical_device, window_surface);
            if (std::get<0>(result) == false)
            {
                continue;
            }

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
                best_device = { physical_device, std::get<1>(result) };
                best_score = score;
            }
        }

        return best_device;
    }

    std::tuple<bool, QueueFamilies> LogicPhysicalQueueBuilder::get_queue_family_indices(const VkPhysicalDevice& physical_device, const WindowSurface& window_surface) const
    {
        QueueFamilies result{};

        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties2> queue_families(queue_family_count, { .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 });
        vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queue_family_count, queue_families.data());

        bool graphics_queue{}, compute_queue{}, transfer_queue{};
        bool success = false;

        for (int family_index = 0; family_index < queue_families.size(); ++family_index)
        {
            VkBool32 is_supporting_queue{};
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, family_index, window_surface.get_surface(), &is_supporting_queue);

            // Force support graphics and present queue. I don't want to convert between
            if (queue_families[family_index].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT && is_supporting_queue)
            {
                if (graphics_queue == false)
                {
                    result.graphics_present_family.family_index = family_index;
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

            if (graphics_queue && compute_queue && transfer_queue)
            {
                success = true;
                break;
            }
        }

        return std::tuple{ success, result };
    }
} // namespace pvp