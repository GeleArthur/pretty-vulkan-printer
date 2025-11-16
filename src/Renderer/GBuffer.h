#pragma once
#include <DestructorQueue.h>
#include <Buffer/Buffer.h>
#include <DescriptorSets/DescriptorSetBuilder.h>
#include <Image/Image.h>
#include <UniformBuffers/UniformBuffer.h>

namespace pvp
{
    class DepthPrePass;
    struct ModelCameraViewData;
    class PvpScene;
    class GBuffer final
    {
    public:
        explicit GBuffer(const Context& context, const PvpScene& scene, DepthPrePass& depth);
        void draw(const FrameContext& cmd);

        Image& get_albedo_image()
        {
            return m_albedo_image;
        };
        Image& get_normal_image()
        {
            return m_normal_image;
        };
        Image& get_metal_roughness_image()
        {
            return m_metal_roughness_image;
        };

    private:
        void            build_pipelines();
        void            create_images();
        const Context&  m_context;
        const PvpScene& m_scene;
        DepthPrePass&   m_depth_pre_pass;
        Image           m_albedo_image{};
        Image           m_normal_image{};
        Image           m_metal_roughness_image{};

        VkPipelineLayout m_pipeline_layout{};
        VkPipeline       m_albedo_pipeline{};

        VkPipelineLayout m_meshlets_pipeline_layout{};
        VkPipeline       m_meshlets_albedo_pipeline{};

        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
