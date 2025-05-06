#pragma once
#include "PVPPhysicalDevice/PhysicalDevice.h"
#include "PVPRenderer/Swapchain.h"

#include <LoadModel.h>
#include <PVPDescriptorSets/DescriptorSetBuilder.h>
#include <PVPInstance/Instance.h>
#include <PVPDevice/Device.h>
#include <PVPSyncManager/FrameSyncers.h>
#include <PVPWindow/WindowSurface.h>

namespace pvp
{
    class App final
    {
    public:
        void run();
        void draw_frame();
        void record_commands(VkCommandBuffer graphics_command, uint32_t image_index);

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

        UniformBuffer<ModelCameraViewData>* m_uniform_buffer{};
        LoadModel                           m_model{};
        Sampler                             m_sampler{};
        Image                               m_texture{};
        DescriptorSets                      m_descriptors{};
        DescriptorPool                      m_descriptor_pool{};
        uint32_t                            m_double_buffer_frame{ 0 };
        CommandPool                         m_cmd_pool_transfer_buffers{};
        CommandPool                         m_cmd_pool_graphics_present{};
        std::vector<VkCommandBuffer>        m_cmds_graphics{};

        FrameSyncers* m_frame_syncers{};

        DestructorQueue m_destructor_queue;
    };
} // namespace pvp
