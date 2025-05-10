#pragma once
#include <PVPBuffer/Buffer.h>
#include <PVPDescriptorSets/DescriptorSetBuilder.h>
#include <PVPImage/Image.h>
#include <PVPUniformBuffers/UniformBuffer.h>

namespace pvp
{
    struct ModelCameraViewData;
    struct PvpScene;
    class GBuffer final
    {
    public:
        explicit GBuffer(const Context& context, const PvpScene& scene, const ImageInfo& image_info);
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
        Image           m_albedo_image{};
        Image           m_normal_image{};
        const ImageInfo m_image_info;

        UniformBuffer<ModelCameraViewData> m_camera_uniform;
        DescriptorSets                     m_descriptor_binding;

        VkPipelineLayout m_albedo_pipeline_layout{};
        VkPipeline       m_albedo_pipeline{};
        VkPipeline       m_normal_pipeline{};

        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
