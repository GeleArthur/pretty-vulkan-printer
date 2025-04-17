#pragma once
#include <vulkan/vulkan.h>

#define VK_DEFINE_INSTANCE_FUNCTION(name)                                                                              \
    auto name(auto&&... args)                                                                                          \
    {                                                                                                                  \
        using FuncType = PFN_##name;                                                                                   \
        static FuncType function = reinterpret_cast<FuncType>(vkGetInstanceProcAddr(m_instance, #name));               \
        return function(std::forward<decltype(args)>(args)...);                                                        \
    }

class VulkanExtraFunctionCaller final
{
  public:
    explicit VulkanExtraFunctionCaller(const VkInstance instance) : m_instance{instance} {};

    VK_DEFINE_INSTANCE_FUNCTION(vkCreateDebugUtilsMessengerEXT)
    VK_DEFINE_INSTANCE_FUNCTION(vkDestroyDebugUtilsMessengerEXT)

  private:
    VkInstance m_instance;
};
