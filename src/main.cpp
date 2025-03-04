#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <exception>
#include <iostream>
#include <vulkan/vulkan.h>
#include <algorithm>
#include <cstring>
#include <map>
#include <optional>
#include <vector>

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value();
    }
};

class HelloTraingle
{
public:
    void run()
    {
        init_vulkan();
        main_loop();
        clean_up();
    }

private:
    void init_vulkan()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        create_instance();
        setup_debug_messanger();
        pick_physics_device();
        createLogicalDevice();
    }

    void createLogicalDevice()
    {
        VkPhysicalDeviceFeatures deviceFeatures{};
        QueueFamilyIndices indices = findQueueFamilies(m_physical_device);

        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = indices.graphicsFamily.value();
        queue_create_info.queueCount = 1;
        float queuePriority = 1.0f;
        queue_create_info.pQueuePriorities = &queuePriority;

        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        create_info.pQueueCreateInfos = &queue_create_info;
        create_info.queueCreateInfoCount = 1;
        create_info.pEnabledFeatures = &deviceFeatures;
        create_info.enabledExtensionCount = 0;

        if (m_enable_validation_layers)
        {
            create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            create_info.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            create_info.enabledLayerCount = 0;
        }

        if (vkCreateDevice(m_physical_device, &create_info, nullptr, &device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    }

    void pick_physics_device()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            throw std::runtime_error("No GPU with VULKAN");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices)
        {
            if (is_device_suitable(device))
            {
                m_physical_device = device;
                break;
            }
        }

        if (m_physical_device == VK_NULL_HANDLE)
        {
            throw std::runtime_error("No device found that works");
        }
    }

    static bool is_device_suitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = findQueueFamilies(device);

        return indices.isComplete();
    }

    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices{};

        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_familys(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_familys.data());

        int i{};
        for (const auto& queue_family : queue_familys)
        {
            if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }
            if (indices.isComplete())
            {
                break;
            }
            ++i;
        }

        return indices;
    }


    void create_instance()
    {
        VkApplicationInfo info{};
        info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        info.pApplicationName = "Vulkan";
        info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        info.pEngineName = "No Engine";
        info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &info;

        auto extensions_debug = get_required_extensions();

        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions_debug.size());
        create_info.ppEnabledExtensionNames = extensions_debug.data();

        if (m_enable_validation_layers && !check_validation_layer_support())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (m_enable_validation_layers)
        {
            create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            create_info.ppEnabledLayerNames = validationLayers.data();
            populateDebugMessengerCreateInfo(debugCreateInfo);
            create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            create_info.enabledLayerCount = 0;
        }

        auto what = vkCreateInstance(&create_info, nullptr, &instance);

        if (what != VK_SUCCESS)
        {
            throw std::runtime_error("Failed");
        }


        uint32_t exection_count{};
        vkEnumerateInstanceExtensionProperties(nullptr, &exection_count, nullptr);
        std::vector<VkExtensionProperties> extensions(exection_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &exection_count, extensions.data());

        std::cout << "avaible exections:\n";
        for (const VkExtensionProperties& extension : extensions)
        {
            std::cout << '\t' << extension.extensionName << std::endl;
        }
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallBack;
    }

    void setup_debug_messanger()
    {
        if (!m_enable_validation_layers) return;

        VkDebugUtilsMessengerCreateInfoEXT create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
        create_info.pfnUserCallback = debugCallBack;
        create_info.pUserData = nullptr;

        CreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &debugMessenger);
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallBack(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    )
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    bool check_validation_layer_support() const
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> aviableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, aviableLayers.data());

        for (const auto& validation_layer : validationLayers)
        {
            bool layer_found = false;
            for (const auto& layer_properties : aviableLayers)
            {
                if (strcmp(layer_properties.layerName, validation_layer) == 0)
                {
                    layer_found = true;
                    break;
                }
            }
            if (!layer_found)
            {
                return false;
            }
        }

        return true;
    }

    std::vector<const char*> get_required_extensions()
    {
        uint32_t glfw_excention_count{};
        const char** glfw_excention;
        glfw_excention = glfwGetRequiredInstanceExtensions(&glfw_excention_count);

        std::vector<const char*> excentions(glfw_excention, glfw_excention + glfw_excention_count);
        if (m_enable_validation_layers)
        {
            excentions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return excentions;
    }

    void main_loop()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
        }
    }

    void clean_up()
    {
        vkDestroyDevice(device, nullptr);
        if (m_enable_validation_layers)
        {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    GLFWwindow* window{};
    VkInstance instance{};
    VkPhysicalDevice m_physical_device{VK_NULL_HANDLE};
    VkDevice device{};
    VkQueue graphicsQueue{};

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    VkDebugUtilsMessengerEXT debugMessenger{};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool m_enable_validation_layers = true;
#endif
};

int main()
{
    try
    {
        HelloTraingle app;
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}



