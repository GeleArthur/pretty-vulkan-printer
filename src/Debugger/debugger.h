#pragma once

#include <VulkanExternalFunctions.h>
#include <string>
#include <Context/Device.h>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>

namespace pvp
{
    namespace debugger
    {
        VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
            VkDebugUtilsMessageTypeFlagsEXT             message_type,
            VkDebugUtilsMessengerCallbackDataEXT const* p_callback_data,
            void*                                       p_user_data);

        void glfw_error_callback(int error, const char* description);

        constexpr static VkDebugUtilsMessengerCreateInfoEXT debug_create_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = 0,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debug_callback,
            .pUserData = nullptr
        };

        consteval auto static get_debug_info() -> const VkDebugUtilsMessengerCreateInfoEXT*
        {
            return &debug_create_info;
        }

        void start_debug_label(VkCommandBuffer buffer, const std::string& name, glm::vec3 color);
        void end_debug_label(VkCommandBuffer buffer);

        template<typename T>
        void add_object_name(Device* device, const T vulkan_object, const std::string& string)
        {
            VkDebugUtilsObjectNameInfoEXT info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .objectHandle = reinterpret_cast<uint64_t>(vulkan_object),
                .pObjectName = string.c_str()
            };

            if constexpr (std::is_same_v<T, VkInstance>)
            {
                info.objectType = VK_OBJECT_TYPE_INSTANCE;
            }
            else if constexpr (std::is_same_v<T, VkSemaphore>)
            {
                info.objectType = VK_OBJECT_TYPE_SEMAPHORE;
            }
            else if constexpr (std::is_same_v<T, VkBuffer>)
            {
                info.objectType = VK_OBJECT_TYPE_BUFFER;
            }
            else
            {
                info.objectType = VK_OBJECT_TYPE_UNKNOWN;
            }
            VulkanInstanceExtensions::vkSetDebugUtilsObjectNameEXT(device->get_device(), &info);
        }

    }; // namespace debugger
} // namespace pvp
