#pragma once
#include "Context/PhysicalDevice.h"
#include "Renderer/Swapchain.h"

#include <Scene/LoadModel.h>
#include <CommandBuffer/CommandPool.h>
#include <DescriptorSets/DescriptorSetBuilder.h>
#include <Context/Device.h>
#include <Context/Instance.h>
#include <Renderer/Renderer.h>
#include <SyncManager/FrameSyncers.h>
#include <Window/WindowSurface.h>
#include <Scene/PVPScene.h>

namespace pvp
{
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

        // TODO: Move to renderer below

        Swapchain* m_swapchain{};

        VkPipelineLayout m_pipeline_layout{};
        VkPipeline       m_graphics_pipeline{};
        Buffer           m_vertex_buffer{};
        Buffer           m_index_buffer{};

        PvpScene                            m_scene{};
        UniformBuffer<ModelCameraViewData>* m_uniform_buffer{};
        LoadModel                           m_model{};
        Sampler                             m_sampler{};
        Image                               m_texture{};
        DescriptorSets                      m_descriptors{};
        DescriptorPool                      m_descriptor_pool{};
        uint32_t                            m_double_buffer_frame{ 0 };
        CommandPool                         m_cmd_pool_transfer_buffers{};
        Renderer*                           m_renderer{};

        DestructorQueue m_destructor_queue;
    };
} // namespace pvp
