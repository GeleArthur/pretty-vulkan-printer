#pragma once
#include "Camera.h"
#include "LoadModel.h"

#include <filesystem>
#include <vector>
#include <Buffer/Buffer.h>
#include <Context/Context.h>
#include <UniformBuffers/UniformBuffer.h>
#include <glm/mat4x4.hpp>
#include <glm/detail/func_packing_simd.inl>

namespace pvp
{
    struct Model
    {
        Buffer   vertex_data;
        Buffer   index_data;
        uint32_t index_count;
    };

    struct SceneGlobals
    {
        glm::mat4x4 camera_view_projection;
        glm::vec3   lights[1];
    };

    class PvpScene
    {
    public:
        explicit PvpScene(Context& context);
        ~PvpScene();
        void update();

        std::vector<Model> get_models() const
        {
            return m_models;
        };

        UniformBuffer<SceneGlobals>* get_scene_globals() const
        {
            return m_scene_globals_gpu;
        };

    private:
        Context&           m_context;
        std::vector<Model> m_models;
        Camera             m_camera;

        std::chrono::high_resolution_clock::time_point m_last_frame_time;

        UniformBuffer<SceneGlobals>* m_scene_globals_gpu{};
    };

} // namespace pvp