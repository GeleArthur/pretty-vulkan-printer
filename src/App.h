#pragma once
#include <PVPBuffer/Buffer.h>
#include <LoadModel.h>
#include <UniformBufferStruct.h>
#include <PVPDescriptorSets/DescriptorSetBuilder.h>
#include <PVPInstance/PVPInstance.h>
#include <PVPPhysicalDevice/PVPPhysicalDevice.h>
#include <PVPSyncManager/Fence.h>
#include <PVPSyncManager/FrameSyncers.h>
#include <PVPSyncManager/Semaphore.h>
#include <PVPSyncManager/SyncBuilder.h>
#include <PVPUniformBuffers/UniformBuffer.h>
#include <PVPVMAAllocator/VmaAllocator.h>

namespace pvp
{
    class CommandBuffer;

    class Swapchain;
    class App
    {
        public:
        void run();
        void draw_frame();
        void record_commands(VkCommandBuffer graphics_command, uint32_t image_index);

        private:
        Instance*                           m_pvp_instance {};
        PhysicalDevice*                     m_pvp_physical_device {};
        Swapchain*                          m_pvp_swapchain {};
        VkRenderPass                        m_pvp_render_pass {};
        VkPipelineLayout                    m_pipeline_layout {};
        PvpVmaAllocator*                    m_allocator {};
        VkPipeline                          m_graphics_pipeline {};
        CommandBuffer*                      m_command_buffer {};
        Buffer                              m_vertex_buffer {};
        Buffer                              m_index_buffer {};
        UniformBuffer<ModelCameraViewData>* m_uniform_buffer {};
        SyncBuilder*                        m_sync_builder {};
        LoadModel                           m_model {};
        DescriptorSets                      m_descriptors {};
        DescriptorPool*                     m_descriptor_pool {};
        DestructorQueue                     m_destructor_queue;
        uint32_t                            m_double_buffer_frame { 0 };
        FrameSyncers*                       m_frame_syncers {};
    };

} // namespace pvp
