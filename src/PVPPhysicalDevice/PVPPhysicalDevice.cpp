#include "PVPPhysicalDevice.h"
#include <PVPInstance/PVPInstance.h>
#include <PVPSwapchain/Swapchain.h>

#include <set>
#include <stdexcept>

std::tuple<bool, QueueFamilies> get_queue_family_indices(const VkPhysicalDevice physical_device, const VkSurfaceKHR surface)
{
    QueueFamilies result {};

    uint32_t      queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    bool graphics_queue {}, compute_queue {}, transfer_queue {}, surface_queue {};
    bool success = false;

    for (int family_index = 0; family_index < queue_families.size(); ++family_index)
    {
        if (queue_families[family_index].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            if (graphics_queue == false)
            {
                result.graphics_family.family_index = family_index;
                graphics_queue = true;
            }
        }
        if (queue_families[family_index].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            if (compute_queue == false)
            {
                result.compute_family.family_index = family_index;
                compute_queue = true;
            }
        }
        if (queue_families[family_index].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            if (transfer_queue == false)
            {
                result.transfer_family.family_index = family_index;
                transfer_queue = true;
            }
        }

        VkBool32 is_supporting_queue {};
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, family_index, surface, &is_supporting_queue);
        if (is_supporting_queue)
        {
            result.present_family.family_index = family_index;
            surface_queue = true;
        }

        if (graphics_queue && compute_queue && transfer_queue && surface_queue)
        {
            success = true;
            return std::tuple(success, result);
        }
    }

    return std::tuple(success, result);
}

VkPhysicalDevice get_best_device(
const std::vector<VkPhysicalDevice>& devices,
const std::vector<const char*>&      device_extensions,
const VkSurfaceKHR&                  surface)
{
    uint32_t         best_score = 0;
    VkPhysicalDevice best_device = VK_NULL_HANDLE;

    for (const VkPhysicalDevice device : devices)
    {
        // Does GPU support the extensions?
        uint32_t extension_count {};
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> available_extensions(extension_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

        bool has_extension = true;
        for (const char* extension : device_extensions)
        {
            if (std::ranges::find_if(
                available_extensions,
                [&](const VkExtensionProperties& ex) { return std::strcmp(ex.extensionName, extension); }) == available_extensions.end())
            {
                has_extension = false;
                break;
            }
        }

        if (!has_extension)
            continue;

        // Does it have a swapchain?
        if (!pvp::Swapchain::does_device_support_swapchain(device, surface))
        {
            continue;
        }

        // TODO: Make sure the device supports graphics and present queue

        // const QueueFamilyIndicesStruct& result = get_queue_family_indices(device, surface);
        // if (result.success == false)
        // {
        //     continue;
        // }

        // What is the best gpu?
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(device, &device_properties);

        uint32_t score = 1;
        if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            score += 10;
        }

        if (score > best_score)
        {
            best_device = device;
            best_score = score;
        }
    }

    return best_device;
}

pvp::PhysicalDevice::PhysicalDevice(Instance* pvp_instance, const std::vector<std::string>& device_extensions)
    : m_instance { pvp_instance }
{
    const VkInstance instance = m_instance->get_instance();

    std::vector      device_extensions_real { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    device_extensions_real.reserve(1 + device_extensions.size());
    for (std::string extension : device_extensions)
    {
        device_extensions_real.push_back(extension.c_str());
    }

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    m_physical_device = get_best_device(devices, device_extensions_real, pvp_instance->get_surface());

    if (m_physical_device == VK_NULL_HANDLE)
    {
        throw std::runtime_error("No device found that works");
    }

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos {};

    auto [success, queue_families] = get_queue_family_indices(m_physical_device, pvp_instance->get_surface());

    if (!success)
    {
        throw std::runtime_error("failed to find a suitable queue family!");
    }

    std::set<uint32_t> unique_queue_families = {
        queue_families.compute_family.family_index,
        queue_families.graphics_family.family_index,
        queue_families.transfer_family.family_index,
        queue_families.present_family.family_index
    };
    float queue_priority = 1.0f;

    for (uint32_t queue_family : unique_queue_families)
    {
        VkDeviceQueueCreateInfo queue_create_info {};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features {};

    VkDeviceCreateInfo       device_create_info {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    device_create_info.pQueueCreateInfos = queue_create_infos.data();

    device_create_info.pEnabledFeatures = &device_features;

    device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions_real.size());
    device_create_info.ppEnabledExtensionNames = device_extensions_real.data();

    if (vkCreateDevice(m_physical_device, &device_create_info, nullptr, &m_device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }
    m_destructor.add_to_queue([&] { vkDestroyDevice(m_device, nullptr); });

    vkGetDeviceQueue(m_device, queue_families.graphics_family.family_index, 0, &queue_families.graphics_family.queue);
    vkGetDeviceQueue(m_device, queue_families.compute_family.family_index, 0, &queue_families.compute_family.queue);
    vkGetDeviceQueue(m_device, queue_families.transfer_family.family_index, 0, &queue_families.transfer_family.queue);
    vkGetDeviceQueue(m_device, queue_families.present_family.family_index, 0, &queue_families.present_family.queue);

    m_queue_families = queue_families;
}

VkDevice pvp::PhysicalDevice::get_device()
{
    return m_device;
}
VkPhysicalDevice pvp::PhysicalDevice::get_physical_device()
{
    return m_physical_device;
}