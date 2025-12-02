#pragma once
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
        void draw_sphere(const GizmosSphere& sphere);

    private:
        Context&        m_context;
        const PvpScene& m_scene;

        void build_buffers();
        void build_pipelines();

        std::vector<GizmosSphere> m_drawables;
        int                       m_sphere_count{}; // TODO: bad replace

        VkPipelineLayout m_pipeline_layout{};
        VkPipeline       m_pipeline{};
        Buffer           m_sphere_buffer;
        Buffer           m_sphere_staging_buffer;
        DescriptorSets   m_sphere_descriptor;

        DestructorQueue m_destructor_queue;
    };
} // namespace pvp
