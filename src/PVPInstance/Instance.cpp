#include "Instance.h"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <array>
#include <iostream>
#include <print>
#include <stdexcept>
#include <string>
#include <vector>

#include "../VulkanExternalFunctions.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
VkDebugUtilsMessageTypeFlagsEXT             messageType,
VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
void*                                       pUserData)
{
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        spdlog::error(pCallbackData->pMessage);
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        spdlog::warn(pCallbackData->pMessage);
    }
    else
    {
        spdlog::info(pCallbackData->pMessage);
    }

    return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT consteval static create_debug_info()
{
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
    debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    debug_create_info.pfnUserCallback = debugCallback;

    return debug_create_info;
}

bool static is_using_debugging(const std::vector<std::string>& extensions, const std::vector<std::string>& layers)
{
    if (std::ranges::find(layers, std::string("VK_LAYER_KHRONOS_validation")) == layers.end())
        return false;

    if (std::ranges::find(extensions, std::string(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) == extensions.end())
        return false;

    return true;
}

bool static using_valid_extensions(const std::vector<std::string>& extensions_required)
{
    uint32_t extension_count{};
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> extensions_available(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions_available.data());

    for (const std::string& extension_found : extensions_required)
    {
        if (std::ranges::find_if(
            extensions_available,
            [&extension_found](const VkExtensionProperties& extension) { return extension.extensionName == extension_found; }) == extensions_available.end())
        {
            std::cerr << "Can't find: " << extension_found << '\n';
            return false;
        }
    }

    return true;
}

pvp::Instance::Instance(
const int                       width,
const int                       height,
const std::string&              app_view,
bool                            debug,
const std::vector<std::string>& extensions_extra,
const std::vector<std::string>& layers_extra)
{
    glfwInit();
    m_destructing.add_to_queue([&] { glfwTerminate(); });

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(width, height, app_view.c_str(), nullptr, nullptr);
    m_destructing.add_to_queue([&] { glfwDestroyWindow(m_window); });

    if (using_valid_extensions(extensions_extra) == false)
    {
        throw std::runtime_error("Your GPU SUCKS!!!");
    }

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app_view.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = std::format("{} Engine", app_view).c_str();
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    // Extensions
    uint32_t     glfw_extension_count = 0;
    char const** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector extensions_to_activate(glfw_extensions, glfw_extensions + glfw_extension_count);
    extensions_to_activate.reserve(glfw_extension_count + extensions_extra.size());

    for (const std::string& extension : extensions_extra)
        extensions_to_activate.push_back(extension.c_str());

    // Layers
    std::vector<const char*> layers_to_activate;
    layers_to_activate.reserve(layers_extra.size());
    for (const std::string& to_activate : layers_extra)
        layers_to_activate.push_back(to_activate.c_str());

    // Setup
    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledLayerCount = static_cast<uint32_t>(layers_to_activate.size());
    create_info.ppEnabledLayerNames = layers_to_activate.data();
    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions_to_activate.size());
    create_info.ppEnabledExtensionNames = extensions_to_activate.data();

    // Setup debug
    VkDebugUtilsMessengerCreateInfoEXT debug_info{};
    if (is_using_debugging(extensions_extra, layers_extra))
    {
        debug_info = create_debug_info();
        create_info.pNext = &debug_info;
    }

    // Create instance
    if (vkCreateInstance(&create_info, nullptr, &m_instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create vkInstance");
    }
    m_destructing.add_to_queue([&] { vkDestroyInstance(m_instance, nullptr); });

    VulkanInstanceExtensions::register_instance(m_instance);

    // Setup debug part 2
    if (is_using_debugging(extensions_extra, layers_extra))
    {
        static VkDebugUtilsMessengerEXT debug_messenger{};
        if (VulkanInstanceExtensions::vkCreateDebugUtilsMessengerEXT(
            m_instance,
            &debug_info,
            nullptr,
            &debug_messenger) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create DebugUtilsMessengerEXT");
        }
        m_destructing.add_to_queue(
        [this] { VulkanInstanceExtensions::vkDestroyDebugUtilsMessengerEXT(m_instance, debug_messenger, nullptr); });
    }

    // Create surface
    if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("no window surface :(");
    }
    m_destructing.add_to_queue([&] { vkDestroySurfaceKHR(m_instance, m_surface, nullptr); });
}
