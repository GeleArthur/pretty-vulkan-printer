#pragma once
#include <utility>
#include <vulkan/vulkan.h>

namespace VulkanInstanceExtensions
{
    inline VkInstance my_instance;

    void register_instance(VkInstance instance);

#define VK_DEFINE_INSTANCE_FUNCTION(name)                                                                 \
    auto static name(auto&&... args)                                                                      \
    {                                                                                                     \
        using FuncType = PFN_##name;                                                                      \
        static FuncType function = reinterpret_cast<FuncType>(vkGetInstanceProcAddr(my_instance, #name)); \
        return function(std::forward<decltype(args)>(args)...);                                           \
    }

    VK_DEFINE_INSTANCE_FUNCTION(vkCreateDebugUtilsMessengerEXT)
    VK_DEFINE_INSTANCE_FUNCTION(vkDestroyDebugUtilsMessengerEXT)

    VK_DEFINE_INSTANCE_FUNCTION(vkSetDebugUtilsObjectNameEXT)
    VK_DEFINE_INSTANCE_FUNCTION(vkCmdBeginDebugUtilsLabelEXT)
    VK_DEFINE_INSTANCE_FUNCTION(vkCmdEndDebugUtilsLabelEXT)
    VK_DEFINE_INSTANCE_FUNCTION(vkCmdInsertDebugUtilsLabelEXT)
}; // namespace VulkanInstanceExtensions
