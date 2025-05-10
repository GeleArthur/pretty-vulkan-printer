#include "Swapchain.h"

#include "GLFW/glfw3.h"
#include "PVPImage/ImageBuilder.h"
#include "PVPPhysicalDevice/PhysicalDevice.h"
#include "PVPWindow/WindowSurface.h"

#include <PVPImage/Image.h>
#include <PVPInstance/Instance.h>
#include <PVPDevice/Device.h>
#include <algorithm>
#include <array>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>

static VkSurfaceFormatKHR get_best_surface_format(const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface)
{
    uint32_t format_count{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
    std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, surface_formats.data());

    for (auto const& format : surface_formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    return surface_formats[0]; // lol
}

VkPresentModeKHR pvp::Swapchain::get_best_present_mode() const
{
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_context.physical_device->get_physical_device(), m_window_surface.get_surface(), &present_mode_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_context.physical_device->get_physical_device(), m_window_surface.get_surface(), &present_mode_count, present_modes.data());

    for (const VkPresentModeKHR& available_present_mode : present_modes)
    {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static uint32_t get_mini_image_count(const VkSurfaceCapabilitiesKHR& capabilities)
{
    uint32_t image_count = capabilities.minImageCount;
    if (capabilities.maxImageCount > 0 && capabilities.minImageCount > capabilities.maxImageCount)
    {
        image_count = capabilities.maxImageCount;
    }

    return image_count;
}

static VkExtent2D get_swap_chain_extent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    const VkExtent2D actual_extent{
        std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };

    return actual_extent;
}

static VkFormat get_depth_format()
{
    return VK_FORMAT_D32_SFLOAT; // TODO
}

pvp::Swapchain::Swapchain(Context& context, WindowSurface& surface)
    : m_window_surface(surface)
    , m_swapchain_surface_format{ get_best_surface_format(context.physical_device->get_physical_device(), surface.get_surface()) }
    , m_command_pool{ context, *context.queue_families->get_queue_family(VK_QUEUE_TRANSFER_BIT, false), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT }
    , m_context(context)
{
    m_swap_chain_destructor.add_to_queue([&] { m_command_pool.destroy(); });
    create_the_swapchain();
}

bool pvp::Swapchain::does_device_support_swapchain(const VkPhysicalDevice device, const VkSurfaceKHR surface)
{
    uint32_t format_count{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

    return format_count > 0 && present_mode_count > 0;
}

void pvp::Swapchain::recreate_swapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window_surface.get_window(), &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_window_surface.get_window(), &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_context.device->get_device());
    // destroy_old_swapchain();

    // create_the_swapchain(m_context.device, command_buffer);
    // create_frame_buffers(m_context.device.get_device(), render_pass);
}

VkSurfaceFormatKHR pvp::Swapchain::get_swapchain_surface_format() const
{
    return m_swapchain_surface_format;
}
VkFormat pvp::Swapchain::get_depth_format() const
{
    return m_depth_format;
}

VkExtent2D pvp::Swapchain::get_swapchain_extent() const
{
    return m_swapchain_extent;
}

VkSwapchainKHR pvp::Swapchain::get_swapchain() const
{
    return m_swapchain;
}
const std::vector<VkImage>& pvp::Swapchain::get_images() const
{
    return m_swapchain_images;
}
const std::vector<VkImageView>& pvp::Swapchain::get_views() const
{
    return m_swapchain_views;
}

void pvp::Swapchain::destroy_old_swapchain()
{
    m_swap_chain_destructor.destroy_and_clear();
    m_swapchain_images.clear();
    m_swapchain_views.clear();
}

void pvp::Swapchain::create_the_swapchain()
{
    VkSurfaceCapabilitiesKHR surface_capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context.physical_device->get_physical_device(), m_window_surface.get_surface(), &surface_capabilities);

    m_swapchain_extent = get_swap_chain_extent(surface_capabilities, m_window_surface.get_window());

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = m_window_surface.get_surface();
    create_info.minImageCount = get_mini_image_count(surface_capabilities);
    create_info.imageFormat = m_swapchain_surface_format.format;
    create_info.imageColorSpace = m_swapchain_surface_format.colorSpace;
    create_info.imageExtent = m_swapchain_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    Queue*     graphics = m_context.queue_families->get_queue_family(VK_QUEUE_GRAPHICS_BIT, false);
    Queue*     present = m_context.queue_families->get_queue_family(static_cast<VkQueueFlagBits>(0x00000000), true);
    std::array queue_family_indices = { graphics->family_index, present->family_index };

    if (graphics->family_index != present->family_index)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.pQueueFamilyIndices = queue_family_indices.data();
        create_info.queueFamilyIndexCount = 2;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }

    create_info.preTransform = surface_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = get_best_present_mode();
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_context.device->get_device(), &create_info, nullptr, &m_swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swapchain");
    }
    m_swap_chain_destructor.add_to_queue([&] { vkDestroySwapchainKHR(m_context.device->get_device(), m_swapchain, nullptr); });

    uint32_t image_count{};
    vkGetSwapchainImagesKHR(m_context.device->get_device(), m_swapchain, &image_count, nullptr);
    m_swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(m_context.device->get_device(), m_swapchain, &image_count, m_swapchain_images.data());

    m_swapchain_views.resize(image_count);
    for (size_t i = 0; i < m_swapchain_images.size(); ++i)
    {
        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = m_swapchain_images[i];
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = m_swapchain_surface_format.format;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_context.device->get_device(), &view_info, nullptr, &m_swapchain_views[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture image view!");
        }
        m_swap_chain_destructor.add_to_queue([&, view_ptr = m_swapchain_views[i]] { vkDestroyImageView(m_context.device->get_device(), view_ptr, nullptr); });
    }
}
