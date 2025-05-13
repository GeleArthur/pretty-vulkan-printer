#pragma once
#include <UniformBufferStruct.h>
#include <Context/Context.h>
#include <DescriptorSets/DescriptorSets.h>
#include <Image/Image.h>
#include <Scene/PVPScene.h>
#include <UniformBuffers/UniformBuffer.h>
namespace pvp
{
    class DepthPrePass
    {
        explicit DepthPrePass(const Context& context, const PvpScene& scene, const ImageInfo& image_info);
        void draw(VkCommandBuffer cmd);

        const Image& get_depth_image() const
        {
            return m_depth_image;
        };

    private:
        void            build_pipelines();
        void            create_images();
        const Context&  m_context;
        const PvpScene& m_scene;
        Image           m_depth_image{};
        const ImageInfo m_image_info;

        UniformBuffer<ModelCameraViewData> m_camera_uniform;
        DescriptorSets                     m_descriptor_binding;

        VkPipelineLayout m_albedo_pipeline_layout{};
        VkPipeline       m_albedo_pipeline{};
        VkPipeline       m_normal_pipeline{};

        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
