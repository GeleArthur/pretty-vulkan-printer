#pragma once
#include <PVPBuffer/Buffer.h>
#include <PVPImage/Image.h>
#include <PVPUniformBuffers/UniformBuffer.h>
#include <PVPWindow/WindowSurface.h>
#include <glm/fwd.hpp>

namespace pvp
{
    struct ModelCameraViewData;
    struct PvpScene;
    class GBuffer final
    {
    public:
        explicit GBuffer(const Context& context, const PvpScene& scene, const ImageInfo& image_info);
        void draw();

    private:
        void            build_pipelines();
        void            draw_albedo(VkCommandBuffer cmd);
        const Context&  m_context;
        const PvpScene& m_scene;
        const Image     m_albedo_image{};
        const ImageInfo m_image_info;

        UniformBuffer<ModelCameraViewData> m_camera_uniform;
        // const UniformBuffer<glm::mat4> m_model_uniform;

        VkPipelineLayout m_albedo_pipeline_layout{};
        VkPipeline       m_albedo_pipeline{};
    };
} // namespace pvp
