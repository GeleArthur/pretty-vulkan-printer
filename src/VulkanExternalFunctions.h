#pragma once
#include <utility>
#include <vulkan/vulkan.h>

namespace VulkanInstanceExtensions
{
    inline VkInstance my_instance;
    inline VkDevice   my_device;

    void register_instance(VkInstance instance);
    void register_device(VkDevice device);

#define VK_DEFINE_INSTANCE_FUNCTION(name)                                                                 \
    auto static name(auto&&... args)                                                                      \
    {                                                                                                     \
        using FuncType = PFN_##name;                                                                      \
        static FuncType function = reinterpret_cast<FuncType>(vkGetInstanceProcAddr(my_instance, #name)); \
        return function(std::forward<decltype(args)>(args)...);                                           \
    }

#define VK_DEFINE_DEVICE_FUNCTION(name)                                                               \
    auto static name(auto&&... args)                                                                  \
    {                                                                                                 \
        using FuncType = PFN_##name;                                                                  \
        static FuncType function = reinterpret_cast<FuncType>(vkGetDeviceProcAddr(my_device, #name)); \
        return function(std::forward<decltype(args)>(args)...);                                       \
    }

    VK_DEFINE_INSTANCE_FUNCTION(vkCreateDebugUtilsMessengerEXT)
    VK_DEFINE_INSTANCE_FUNCTION(vkDestroyDebugUtilsMessengerEXT)
    VK_DEFINE_INSTANCE_FUNCTION(vkSetDebugUtilsObjectNameEXT)
    VK_DEFINE_INSTANCE_FUNCTION(vkCmdBeginDebugUtilsLabelEXT)
    VK_DEFINE_INSTANCE_FUNCTION(vkCmdEndDebugUtilsLabelEXT)
    VK_DEFINE_INSTANCE_FUNCTION(vkCmdInsertDebugUtilsLabelEXT)
    VK_DEFINE_INSTANCE_FUNCTION(vkCmdDrawMeshTasksEXT)
    VK_DEFINE_INSTANCE_FUNCTION(vkCmdDrawMeshTasksIndirectEXT)
    VK_DEFINE_INSTANCE_FUNCTION(vkCmdDrawMeshTasksIndirectCountEXT)

    VK_DEFINE_DEVICE_FUNCTION(vkDebugMarkerSetObjectNameEXT)
    // auto static vkDebugMarkerSetObjectNameEXT(auto&&... args)
    // {
    //     using FuncType = PFN_vkDebugMarkerSetObjectNameEXT;
    //     static FuncType function = reinterpret_cast<FuncType>(vkGetDeviceProcAddr(my_device, "vkDebugMarkerSetObjectNameEXT"));
    //     if (function == nullptr)
    //     {
    //         throw;
    //     }
    //     return function(std::forward<decltype(args)>(args)...);
    // }
}; // namespace VulkanInstanceExtensions
