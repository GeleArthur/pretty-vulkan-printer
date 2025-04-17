#pragma once
#include <vulkan/vulkan.h>

namespace VulkanInstanceExtensions
{
inline VkInstance my_instance;

static void register_instance(const VkInstance instance)
{
    my_instance = instance;
}

#define VK_DEFINE_INSTANCE_FUNCTION(name)                                                                              \
    auto static name(auto&&... args)                                                                                   \
    {                                                                                                                  \
        using FuncType = PFN_##name;                                                                                   \
        static FuncType function = reinterpret_cast<FuncType>(vkGetInstanceProcAddr(my_instance, #name));              \
        return function(std::forward<decltype(args)>(args)...);                                                        \
    }

VK_DEFINE_INSTANCE_FUNCTION(vkCreateDebugUtilsMessengerEXT)
VK_DEFINE_INSTANCE_FUNCTION(vkDestroyDebugUtilsMessengerEXT)

}; // namespace VulkanInstanceExtensions
