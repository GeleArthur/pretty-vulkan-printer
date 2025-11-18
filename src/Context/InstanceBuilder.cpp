#include "InstanceBuilder.h"

#include <format>

#define GLFW_INCLUDE_VULKAN
#include <VulkanExternalFunctions.h>
#include <cstring>
#include <GLFW/glfw3.h>
#include <Debugger/Debugger.h>
#include <tracy/Tracy.hpp>

void pvp::InstanceBuilder::valid_extensions_check()
{
    uint32_t extension_count{};
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> extensions_available(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions_available.data());

    for (const char* extension_found : m_extensions)
    {
        if (auto extension = std::ranges::find_if(
                extensions_available,
                [&extension_found](const VkExtensionProperties& extension) {
                    return std::strcmp(extension.extensionName, extension_found);
                });
            extension == extensions_available.end())
        {
            throw std::runtime_error(std::format("Extension \"{}\" is not on your hardware :(", extension->extensionName).c_str());
        }
    }
}

pvp::InstanceBuilder& pvp::InstanceBuilder::set_app_name(const std::string& name)
{
    m_app_name = name;
    return *this;
}

pvp::InstanceBuilder& pvp::InstanceBuilder::enable_debugging(bool enabled)
{
    m_is_debugging = enabled;
    return *this;
}

void pvp::InstanceBuilder::build(Instance& instance)
{
    ZoneScoped;
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = m_app_name.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = std::format("{} Engine", m_app_name).c_str();
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    // Extensions
    uint32_t     glfw_extension_count = 0;
    char const** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    m_extensions.reserve(m_extensions.size() + glfw_extension_count);
    for (uint32_t i = 0; i < glfw_extension_count; ++i)
    {
        m_extensions.push_back(glfw_extensions[i]);
    }

    if (m_is_debugging)
    {
        m_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        m_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        m_layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    // Setup
    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledLayerCount = static_cast<uint32_t>(m_layers.size());
    create_info.ppEnabledLayerNames = m_layers.data();
    create_info.enabledExtensionCount = static_cast<uint32_t>(m_extensions.size());
    create_info.ppEnabledExtensionNames = m_extensions.data();

    // Setup debug
    if (m_is_debugging)
    {
        create_info.pNext = Debugger::get_debug_info();
    }

    valid_extensions_check();

    // Create instance
    VkResult thing = vkCreateInstance(&create_info, nullptr, &instance.m_instance);
    if (thing != VK_SUCCESS)
    {
        throw std::runtime_error(std::format(""));
    }
    instance.m_destructor_queue.add_to_queue([&] { vkDestroyInstance(instance.m_instance, nullptr); });

    VulkanInstanceExtensions::register_instance(instance.m_instance);

    // Setup debug part 2
    if (m_is_debugging)
    {
        static VkDebugUtilsMessengerEXT debug_messenger{};

        const VkDebugUtilsMessengerCreateInfoEXT* convertToLValue = Debugger::get_debug_info();
        if (VulkanInstanceExtensions::vkCreateDebugUtilsMessengerEXT(instance.m_instance, convertToLValue, nullptr, &debug_messenger) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create DebugUtilsMessengerEXT");
        }
        instance.m_destructor_queue.add_to_queue([&] { VulkanInstanceExtensions::vkDestroyDebugUtilsMessengerEXT(instance.m_instance, debug_messenger, nullptr); });
    }
}
