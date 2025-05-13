#pragma once

#include "Window/WindowSurface.h"

#include <DestructorQueue.h>
#include <globalconst.h>
#include <vector>
#include <CommandBuffer/CommandPool.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    class Instance;
    class Device;

    class Swapchain
    {
    public:
        explicit Swapchain(Context& context, WindowSurface& surface);

        DISABLE_COPY(Swapchain);
        DISABLE_MOVE(Swapchain);

        static bool does_device_support_swapchain(VkPhysicalDevice device, VkSurfaceKHR surface);
        void        recreate_swapchain();

        [[nodiscard]] VkSurfaceFormatKHR get_swapchain_surface_format() const;
        [[nodiscard]] VkFormat           get_depth_format() const;
        [[nodiscard]] VkExtent2D         get_swapchain_extent() const;
        [[nodiscard]] VkSwapchainKHR     get_swapchain() const;

        [[nodiscard]] const std::vector<VkImage>&     get_images() const;
        [[nodiscard]] const std::vector<VkImageView>& get_views() const;

    private:
        void             destroy_old_swapchain();
        void             create_the_swapchain();
        VkPresentModeKHR get_best_present_mode() const;

        std::vector<VkImage>     m_swapchain_images;
        std::vector<VkImageView> m_swapchain_views;
        VkSwapchainKHR           m_swapchain{};
        VkExtent2D               m_swapchain_extent{};

        WindowSurface&     m_window_surface;
        VkSurfaceFormatKHR m_swapchain_surface_format;
        VkFormat           m_depth_format;
        CommandPool        m_command_pool;
        Context&           m_context;

        DestructorQueue m_swap_chain_destructor;
    };
} // namespace pvp
