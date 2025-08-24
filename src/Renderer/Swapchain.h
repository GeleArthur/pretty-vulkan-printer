#pragma once

#include "Window/WindowSurface.h"

#include <DestructorQueue.h>
#include <condition_variable>
#include <globalconst.h>
#include <vector>
#include <CommandBuffer/CommandPool.h>
#include <Events/EventListener.h>
#include <Events/Event.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    struct GlfwToRender;
    class Instance;
    class Device;

    class Swapchain
    {
    public:
        explicit Swapchain(Context& context, GlfwToRender& glfw_to_render);

        DISABLE_COPY(Swapchain);
        DISABLE_MOVE(Swapchain);

        static bool does_device_support_swapchain(VkPhysicalDevice device, VkSurfaceKHR surface);
        void        recreate_swapchain();

        [[nodiscard]] VkSurfaceFormatKHR get_swapchain_surface_format() const;
        [[nodiscard]] VkFormat           get_depth_format() const;
        [[nodiscard]] VkExtent2D         get_swapchain_extent() const;
        [[nodiscard]] int                get_min_image_count() const;
        [[nodiscard]] VkSwapchainKHR     get_swapchain() const;

        [[nodiscard]] const std::vector<VkImage>&     get_images() const;
        [[nodiscard]] const std::vector<VkImageView>& get_views() const;

        [[nodiscard]] Event<Context&, int, int>& get_on_frame_buffer_size_changed();
        [[nodiscard]] VkPresentModeKHR           get_best_present_mode() const;

    private:
        void destroy_old_swapchain();
        void create_the_swapchain();

        std::vector<VkImage>     m_swapchain_images;
        std::vector<VkImageView> m_swapchain_views;
        VkSwapchainKHR           m_swapchain{};
        VkExtent2D               m_swapchain_extent{};

        Context&           m_context;
        VkSurfaceFormatKHR m_swapchain_surface_format;
        VkFormat           m_depth_format;
        int                m_imagecount;
        CommandPool        m_command_pool;
        GlfwToRender*      m_glfw_to_render;

        Event<Context&, int, int> m_on_frame_buffer_size_changed;
        DestructorQueue           m_swap_chain_destructor;
    };
} // namespace pvp
