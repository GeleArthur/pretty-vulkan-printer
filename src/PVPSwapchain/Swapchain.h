#pragma once

#include <DestructorQueue.h>
#include <PVPImage/Image.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace pvp
{
    class Instance;
    class Device;
    class Swapchain
    {
    public:
        explicit Swapchain(Instance& instance, Device& pvp_device, CommandBuffer& command_buffer);
        static bool does_device_support_swapchain(VkPhysicalDevice device, VkSurfaceKHR surface);
        void        create_frame_buffers(VkDevice device, VkRenderPass render_pass);
        void        recreate_swapchain(Device& device, CommandBuffer& command_buffer, VkRenderPass render_pass);

        VkSurfaceFormatKHR                get_swapchain_surface_format();
        VkExtent2D                        get_swapchain_extent();
        VkSwapchainKHR                    get_swapchain();
        const std::vector<VkFramebuffer>& get_framebuffers();

    private:
        void destroy_old_swapchain();
        void create_the_swapchain(Device& device, CommandBuffer& command_buffer);

        std::vector<VkFramebuffer> m_framebuffers;
        Image*                     m_depth_buffer_image;
        std::vector<VkImage>       m_swapchain_images;
        std::vector<VkImageView>   m_swapchain_image_views;
        VkSwapchainKHR             m_swapchain;
        VkExtent2D                 m_swapchain_extent;

        DestructorQueue m_swap_chain_destructor;

        Instance*          m_instance;
        VkSurfaceFormatKHR m_swapchain_surface_format;
    };
} // namespace pvp
