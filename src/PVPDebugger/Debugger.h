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

        consteval auto static get_debug_info() -> VkDebugUtilsMessengerCreateInfoEXT*
        {
            static VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
            debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debug_create_info.pfnUserCallback = debug_callback;

            return &debug_create_info;
        }
    }; // namespace Debugger
} // namespace pvp
