#pragma once
#include "GBuffer.h"

#include <Context/Context.h>
#include <DescriptorSets/DescriptorSets.h>
#include <GraphicsPipeline/GraphicsPipelineBuilder.h>
#include <Image/Image.h>
#include <DestructorQueue.h>

namespace pvp
{
    class ToneMappingPass;

    class MeshShaderPass
    {
    public:
        explicit MeshShaderPass(const Context& context, const PvpScene& scene);
        void draw(const FrameContext& cmd, uint32_t swapchain_image_index);
        ~MeshShaderPass() = default;
        DISABLE_COPY(MeshShaderPass);
        DISABLE_MOVE(MeshShaderPass);

    private:
        void           build_pipelines();
        void           create_images();
        const Context& m_context;

        VkPipelineLayout m_pipeline_layout{};
        VkPipeline       m_pipeline{};
        const PvpScene&  m_scene;

        Image           m_depth_image;
        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
