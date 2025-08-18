#pragma once
#include <CommandBuffer/CommandPool.h>
#include <Context/Context.h>
#include <Context/Device.h>
#include <Context/Instance.h>
#include <Context/PhysicalDevice.h>
#include <Context/QueueFamilies.h>
#include <DescriptorSets/DescriptorSets.h>
#include <Image/Image.h>
#include <Image/Sampler.h>
#include <Renderer/Renderer.h>
#include <VMAAllocator/VmaAllocator.h>

namespace pvp
{
    struct GlfwToRender;
    class VulkanApp
    {
    public:
        explicit VulkanApp(GLFWwindow* window, GlfwToRender& gtfw_to_render);
        void run();

    private:
        Instance        m_instance{};
        PhysicalDevice  m_physical_device;
        Device          m_device{};
        QueueFamilies   m_queue_families;
        PvpVmaAllocator m_allocator{};
        Context         m_context{};
        Swapchain*      m_swapchain{};

        PvpScene*      m_scene{};
        Sampler        m_sampler{};
        Image          m_texture{};
        DescriptorSets m_descriptors{};
        uint32_t       m_double_buffer_frame{ 0 };
        CommandPool    m_cmd_pool_transfer_buffers{};
        Renderer*      m_renderer{};
        ImguiRenderer* m_imgui_renderer{};

        GlfwToRender* m_gtfw_to_render;
        VkSurfaceKHR  m_surface{};
        GLFWwindow*   m_window;

        DestructorQueue m_destructor_queue;
    };
} // namespace pvp
