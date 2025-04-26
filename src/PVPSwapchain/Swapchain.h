#pragma once

#include <DestructorQueue.h>
#include <PVPImage/Image.h>
#include <vulkan/vulkan.h>

#include <vector>

namespace pvp
{
    class Instance;
    class PhysicalDevice;
    class Swapchain
    {
        public:
        explicit Swapchain(Instance& instance, PhysicalDevice& PVPdevice, CommandBuffer& command_buffer);
        static bool                       does_device_support_swapchain(VkPhysicalDevice device, VkSurfaceKHR surface);
        void                              create_frame_buffers(VkDevice device, VkRenderPass render_pass);

        VkSurfaceFormatKHR                get_swapchain_surface_format();
        VkExtent2D                        get_swapchain_extent();
        VkSwapchainKHR                    get_swapchain();
        const std::vector<VkFramebuffer>& get_framebuffers();

        private:
        std::vector<VkFramebuffer> m_framebuffers;
        Image*                     m_depth_buffer_image;
        std::vector<VkImage>       m_swapchain_images;
        std::vector<VkImageView>   m_swapchain_image_views;
        VkSwapchainKHR             m_swapchain;
        VkSurfaceFormatKHR         m_swapchain_surface_format;
        VkExtent2D                 m_swapchain_extent;
        DestructorQueue            m_destructor_queue;
    };
} // namespace pvp
