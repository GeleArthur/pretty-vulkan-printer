#include "Swapchain.h"

#include <Image.h>
#include <PVPInstance/PVPInstance.h>
#include <PVPPhysicalDevice/PVPPhysicalDevice.h>

#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>

static VkSurfaceFormatKHR get_best_surface_format(const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface)
{
    uint32_t format_count {};
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

static VkPresentModeKHR get_best_present_mode(const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface)
{
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data());

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

    VkExtent2D actual_extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

    actual_extent.width = std::clamp(
    actual_extent.width,
    capabilities.minImageExtent.width,
    capabilities.maxImageExtent.width);
    actual_extent.height = std::clamp(
    actual_extent.height,
    capabilities.minImageExtent.height,
    capabilities.maxImageExtent.height);

    return actual_extent;
}

pvp::Swapchain::Swapchain(Instance& instance, PhysicalDevice& PVPdevice)
{
    const VkPhysicalDevice physical_device = PVPdevice.get_physical_device();
    const VkSurfaceKHR     surface = instance.get_surface();
    DestructorQueue        destructor_queue;

    m_swapchain_surface_format = get_best_surface_format(physical_device, surface);

    VkSurfaceCapabilitiesKHR surface_capabilities {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);

    m_swapchain_extent = get_swap_chain_extent(surface_capabilities, instance.get_window());

    VkSwapchainCreateInfoKHR create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;

    create_info.minImageCount = get_mini_image_count(surface_capabilities);
    create_info.imageFormat = m_swapchain_surface_format.format;
    create_info.imageColorSpace = m_swapchain_surface_format.colorSpace;
    create_info.imageExtent = m_swapchain_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const QueueFamilies& queue_families = PVPdevice.get_queue_families();
    if (queue_families.graphics_family.family_index != queue_families.present_family.family_index)
    {
        const uint32_t* queue_family = new uint32_t[] { queue_families.graphics_family.family_index, queue_families.present_family.family_index };
        destructor_queue.add_to_queue([&] { delete queue_family; });
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }
    create_info.preTransform = surface_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = get_best_present_mode(physical_device, surface);
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(PVPdevice.get_device(), &create_info, nullptr, &m_swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swapchain");
    }
    m_destructor_queue.add_to_queue([&] { vkDestroySwapchainKHR(PVPdevice.get_device(), m_swapchain, nullptr); });

    uint32_t image_count {};
    vkGetSwapchainImagesKHR(PVPdevice.get_device(), m_swapchain, &image_count, nullptr);
    m_swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(PVPdevice.get_device(), m_swapchain, &image_count, m_swapchain_images.data());

    m_swapchain_image_views.resize(image_count);
    for (size_t i = 0; i < m_swapchain_images.size(); ++i)
    {
        VkImageViewCreateInfo view_info {};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = m_swapchain_images[i];
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = m_swapchain_surface_format.format;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(PVPdevice.get_device(), &view_info, nullptr, &m_swapchain_image_views[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture image view!");
        }
        m_destructor_queue.add_to_queue([&, view_ptr = m_swapchain_image_views[i]] { vkDestroyImageView(PVPdevice.get_device(), view_ptr, nullptr); });
    }

    m_depth_buffer_image = new Image(PVPdevice.get_device(),
                                     m_swapchain_extent.width,
                                     m_swapchain_extent.height,
                                     VK_FORMAT_D32_SFLOAT_S8_UINT,
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                     VMA_MEMORY_USAGE_GPU_ONLY,
                                     VK_IMAGE_ASPECT_DEPTH_BIT);

    m_destructor_queue.add_to_queue([&] { delete m_depth_buffer_image; });
}

bool pvp::Swapchain::does_device_support_swapchain(const VkPhysicalDevice device, const VkSurfaceKHR surface)
{
    uint32_t format_count {};
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

    return format_count > 0 && present_mode_count > 0;
}
void pvp::Swapchain::create_frame_buffers(VkDevice device, VkRenderPass render_pass)
{
    m_framebuffers.resize(m_swapchain_image_views.size());

    for (size_t i = 0; i < m_swapchain_image_views.size(); ++i)
    {
        std::vector             attachments = { m_swapchain_image_views[i], m_depth_buffer_image->get_view() };
        VkFramebufferCreateInfo framebuffer_info {};

        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebuffer_info.pAttachments = attachments.data();
        framebuffer_info.width = m_swapchain_extent.width;
        framebuffer_info.height = m_swapchain_extent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(device, &framebuffer_info, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
        m_destructor_queue.add_to_queue([device, framebuffer = m_framebuffers[i]] { vkDestroyFramebuffer(device, framebuffer, nullptr); });
    }
}
VkSurfaceFormatKHR pvp::Swapchain::get_swapchain_surface_format()
{
    return m_swapchain_surface_format;
}
VkExtent2D pvp::Swapchain::get_swapchain_extent()
{
    return m_swapchain_extent;
}
VkSwapchainKHR pvp::Swapchain::get_swapchain()
{
    return m_swapchain;
}
const std::vector<VkFramebuffer>& pvp::Swapchain::get_framebuffers()
{
    return m_framebuffers;
}
