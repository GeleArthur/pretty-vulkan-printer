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
        void draw(VkCommandBuffer cmd);

        const Image& get_albedo_image() const
        {
            return m_albedo_image;
        };
        const Image& get_normal_image() const
        {
            return m_normal_image;
        };

    private:
        void            build_pipelines();
        void            create_images();
        const Context&  m_context;
        const PvpScene& m_scene;
        DepthPrePass&   m_depth_pre_pass;
        Image           m_albedo_image{};
        Image           m_normal_image{};

        VkPipelineLayout m_pipeline_layout{};
        VkPipeline       m_albedo_pipeline{};

        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
