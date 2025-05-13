#include "LogicPhysicalQueueBuilder.h"

#include <stdexcept>
#include <vulkan/vulkan.h>

#include <set>
#include <vector>
#include <Context/Instance.h>
#include <Window/WindowSurface.h>
#include "QueueFamilies.h"
#include "Device.h"

namespace pvp
{
    static std::vector<VkQueueFamilyProperties2> get_queues(VkPhysicalDevice physical_device)
    {
        uint32_t queue_family_count{};
        vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties2> queue_families(queue_family_count, { .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 });
        vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queue_family_count, queue_families.data());
        return queue_families;
    }

    LogicPhysicalQueueBuilder& LogicPhysicalQueueBuilder::set_extensions(const std::vector<const char*>& extension)
    {
        m_extensions = extension;
        return *this;
    }

    void LogicPhysicalQueueBuilder::build(const Instance& instance, const WindowSurface& window_surface, PhysicalDevice& physical_device_out, Device& device_out, QueueFamilies& queue_families_out)
    {
        m_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        auto physical_device = get_best_device(instance, window_surface);
        if (physical_device == VK_NULL_HANDLE)
        {
            throw std::runtime_error("No device found that works");
        }

        physical_device_out.m_physical_device = physical_device;

        std::vector<VkQueueFamilyProperties2> queue_properties = get_queues(physical_device);

        float queue_priority = 1.0f;

        std::vector<VkDeviceQueueCreateInfo> all_queues;
        all_queues.reserve(queue_properties.size());

        for (int i = 0; i < queue_properties.size(); ++i)
        {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = i;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;
            all_queues.push_back(queue_create_info);
        }

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(all_queues.size());
        device_create_info.pQueueCreateInfos = all_queues.data();
        device_create_info.enabledExtensionCount = static_cast<uint32_t>(m_extensions.size());
        device_create_info.ppEnabledExtensionNames = m_extensions.data();

        VkPhysicalDeviceVulkan13Features features13 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = nullptr,
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE,
        };

        VkPhysicalDeviceVulkan12Features features12 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &features13,
            .descriptorIndexing = VK_TRUE,
            .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
            .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
            .descriptorBindingPartiallyBound = VK_TRUE,
            .descriptorBindingVariableDescriptorCount = VK_TRUE,
            .runtimeDescriptorArray = VK_TRUE,

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

        queue_families_out.m_queues.reserve(all_queues.size());

        for (uint32_t queue_family_index = 0U; queue_family_index < all_queues.size(); ++queue_family_index)
        {
            const VkQueueFamilyProperties2& queue_family_property = queue_properties[queue_family_index];

            VkBool32 present_supported{};
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_family_index, window_surface.get_surface(), &present_supported);

            VkDeviceQueueInfo2 info{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = queue_family_index,
                .queueIndex = 0
            };
            VkQueue queue;
            vkGetDeviceQueue2(device_out.get_device(), &info, &queue);

            queue_families_out.m_queues.emplace_back(queue_family_index, queue, present_supported, queue_family_property);
        }
    }

    VkPhysicalDevice LogicPhysicalQueueBuilder::get_best_device(const Instance& instance, const WindowSurface& window_surface) const
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance.get_instance(), &device_count, nullptr);
        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance.get_instance(), &device_count, devices.data());

        VkPhysicalDevice best_physical_device{ VK_NULL_HANDLE };
        uint32_t         best_score = 0;

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

            // TODO: Force features to work
            // VkPhysicalDeviceFeatures2 features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
            // vkGetPhysicalDeviceFeatures2(physical_device, &features);

            if (is_supports_all_queues(physical_device, window_surface) == false)
            {
                continue;
            }

            // Acceptable gpu. What is the best gpu?
            VkPhysicalDeviceProperties device_properties;
            vkGetPhysicalDeviceProperties(physical_device, &device_properties);

            uint32_t score = 1;
            if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                score += 10;
            }

            if (score > best_score)
            {
                best_physical_device = physical_device;
                best_score = score;
            }
        }

        return best_physical_device;
    }

    bool LogicPhysicalQueueBuilder::is_supports_all_queues(const VkPhysicalDevice& physical_device, const WindowSurface& window_surface) const
    {
        std::vector<VkQueueFamilyProperties2> queue_families = get_queues(physical_device);

        bool graphics_queue{};
        bool compute_queue{};
        bool transfer_queue{};

        for (int family_index = 0; family_index < queue_families.size(); ++family_index)
        {
            VkBool32 is_supporting_queue{};
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, family_index, window_surface.get_surface(), &is_supporting_queue);

            // Force support graphics and present queue. I don't want to convert between
            if (queue_families[family_index].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT && is_supporting_queue)
            {
                graphics_queue = true;
            }
            if (queue_families[family_index].queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                compute_queue = true;
            }
            if (queue_families[family_index].queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                transfer_queue = true;
            }

            if (graphics_queue && compute_queue && transfer_queue)
            {
                return true;
            }
        }

        return false;
    }
} // namespace pvp