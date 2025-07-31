﻿#pragma once

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

        [[nodiscard]] Event<Context&, int, int>& get_on_frame_buffer_size_changed();

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

        Event<Context&, int, int> m_on_frame_buffer_size_changed;
        // std::vector<std::function<void(int, int)>> m_on_frame_buffer_changed{};

        // std::condition_variable m_get_screen_size{};
        // std::mutex              m_screen_change_mutex{};
        // bool                    m_screen_updated{};
        // int                     m_frame_buffer_size_x{};
        // int                     m_frame_buffer_size_y{};

        DestructorQueue m_swap_chain_destructor;
    };
} // namespace pvp
