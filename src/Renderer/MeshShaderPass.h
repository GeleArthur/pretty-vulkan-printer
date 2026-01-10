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

    class MeshShaderPass final
    {
    public:
        explicit MeshShaderPass(const Context& context, const PvpScene& scene);
        ~MeshShaderPass() = default;
        DISABLE_COPY(MeshShaderPass);
        DISABLE_MOVE(MeshShaderPass);

        void draw(const FrameContext& cmd, uint32_t swapchain_image_index);

    private:
        void            build_pipelines();
        void            create_images();
        const Context&  m_context;
        const PvpScene& m_scene;

        VkPipelineLayout m_pipeline_layout{};
        VkPipeline       m_pipeline{};

        VkPipelineLayout m_pipeline_layout_indirect{};
        VkPipeline       m_pipeline_indirect{};

        VkPipelineLayout m_pipeline_layout_indirect_ptr{};
        VkPipeline       m_pipeline_indirect_ptr{};

        Image           m_depth_image{};
        bool            m_valid_query{};
        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
