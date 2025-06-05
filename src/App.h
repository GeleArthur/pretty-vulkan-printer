#pragma once
#include <Context/PhysicalDevice.h>
#include <Context/QueueFamilies.h>
#include <Buffer/Buffer.h>
#include <CommandBuffer/CommandPool.h>
#include <Context/Context.h>
#include <Context/Device.h>
#include <Context/Instance.h>
#include <DescriptorSets/DescriptorSets.h>
#include <Image/Image.h>
#include <Image/Sampler.h>
#include <Renderer/Renderer.h>
#include <Scene/ModelData.h>
#include <VMAAllocator/VmaAllocator.h>
#include <Window/WindowSurface.h>

namespace pvp
{
    class Swapchain;
    class App final
    {
    public:
        void run();

    private:
        Instance        m_instance{};
        WindowSurface   m_window_surface{};
        PhysicalDevice  m_physical_device;
        Device          m_device{};
        QueueFamilies   m_queue_families;
        PvpVmaAllocator m_allocator{};
        Context         m_context{};

        Swapchain* m_swapchain{};

        PvpScene*      m_scene{};
        Sampler        m_sampler{};
        Image          m_texture{};
        DescriptorSets m_descriptors{};
        uint32_t       m_double_buffer_frame{ 0 };
        CommandPool    m_cmd_pool_transfer_buffers{};
        Renderer*      m_renderer{};

        DestructorQueue m_destructor_queue;
    };
} // namespace pvp
