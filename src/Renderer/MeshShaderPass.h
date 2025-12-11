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
        ~MeshShaderPass() = default;
        DISABLE_COPY(MeshShaderPass);
        DISABLE_MOVE(MeshShaderPass);

        void                    draw(const FrameContext& cmd, uint32_t swapchain_image_index);
        std::array<uint64_t, 2> get_invocations_count() const;

    private:
        void            build_pipelines();
        void            create_images();
        void            build_draw_calls();
        const Context&  m_context;
        const PvpScene& m_scene;

        VkPipelineLayout m_pipeline_layout{};
        VkPipeline       m_pipeline{};

        VkPipelineLayout m_pipeline_layout_indirect{};
        VkPipeline       m_pipeline_indirect{};

        Buffer         m_gpu_indirect_draw_calls;
        bool           m_use_indirect{ true };
        DescriptorSets m_indirect_descriptor;

        Image           m_depth_image{};
        VkQueryPool     m_query_pool{};
        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
