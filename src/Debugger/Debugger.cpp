#include "Debugger.h"

#include <spdlog/spdlog.h>

namespace pvp
{
    VkBool32 Debugger::debug_callback(const VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, VkDebugUtilsMessengerCallbackDataEXT const* p_callback_data, void* p_user_data)
    {
        if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            spdlog::error(p_callback_data->pMessage);
        }
        else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            spdlog::warn(p_callback_data->pMessage);
        }
        else
        {
            spdlog::info(p_callback_data->pMessage);
        }

        return VK_FALSE;
    }
} // namespace pvp