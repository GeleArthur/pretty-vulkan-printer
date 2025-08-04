#include "Debugger.h"

#include <VulkanExternalFunctions.h>
#include <spdlog/spdlog.h>

namespace pvp
{
    VkBool32 Debugger::debug_callback(const VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT, VkDebugUtilsMessengerCallbackDataEXT const* p_callback_data, void*)
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
    void Debugger::glfw_error_callback(int error, const char* description)
    {
        spdlog::error("{}, {}", error, description);
    }
    void Debugger::start_debug_label(VkCommandBuffer buffer, const std::string& name, glm::vec3 color)
    {
#ifdef _DEBUG
        VkDebugUtilsLabelEXT debugLabel{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
            .pLabelName = name.c_str(),
            .color = { color.x, color.y, color.z, 1.0f }
        };
        VulkanInstanceExtensions::vkCmdBeginDebugUtilsLabelEXT(buffer, &debugLabel);
#endif
    }
    void Debugger::end_debug_label(VkCommandBuffer buffer)
    {
#ifdef _DEBUG
        VulkanInstanceExtensions::vkCmdEndDebugUtilsLabelEXT(buffer);
#endif
    }
} // namespace pvp