#pragma once
#include "DebugVertex.h"

#include <DestructorQueue.h>
#include <globalconst.h>
#include <variant>
#include <Buffer/Buffer.h>
#include <Context/Context.h>
#include <DescriptorSets/DescriptorSets.h>
#include <glm/glm.hpp>

struct FrameContext;

namespace pvp
{
    class PvpScene;

    struct GizmosSphere
    {
        glm::vec3 position;
        float     size;
    };

    class GizmosDrawer final
    {
    public:
        explicit GizmosDrawer(Context& context, const PvpScene& scene);
        ~GizmosDrawer() = default;
        DISABLE_COPY(GizmosDrawer);
        DISABLE_MOVE(GizmosDrawer);
        void draw(const FrameContext& cmd, uint32_t swapchain_image_index);

    private:
        Context&        m_context;
        const PvpScene& m_scene;

        void build_buffers();
        void build_pipelines();

        std::vector<GizmosSphere> m_drawables;

        VkPipelineLayout m_pipeline_layout_spheres{};
        VkPipeline       m_pipeline_spheres{};

        VkPipelineLayout                         m_pipeline_layout_debug_lines{};
        VkPipeline                               m_pipeline_debug_lines{};
        std::array<Buffer, max_frames_in_flight> m_debug_lines_buffer;
        // Buffer           m_sphere_staging_buffer;
        // DescriptorSets   m_sphere_descriptor;

        DestructorQueue m_destructor_queue;
    };
} // namespace pvp
