#pragma once
#include <vulkan/vulkan.h>

namespace pvp
{
    namespace Debugger
    {
        VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
            VkDebugUtilsMessageTypeFlagsEXT             message_type,
            VkDebugUtilsMessengerCallbackDataEXT const* p_callback_data,
            void*                                       p_user_data);

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
    }; // namespace Debugger
} // namespace pvp
