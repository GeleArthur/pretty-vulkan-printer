#pragma once
#include "Camera.h"
#include "ModelData.h"

#include <filesystem>
#include <vector>
#include <Buffer/Buffer.h>
#include <Context/Context.h>
#include <UniformBuffers/UniformBuffer.h>
#include <glm/mat4x4.hpp>
#include <glm/detail/func_packing_simd.inl>

namespace pvp
{

    struct MaterialTransform
    {
        glm::mat4x4 transform;
        uint32_t    diffuse_texture_index;
        uint32_t    normal_texture_index;
        uint32_t    metalness_texture_index;
    };

    struct Model
    {
        Buffer            vertex_data;
        Buffer            index_data;
        uint32_t          index_count;
        MaterialTransform material;
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
            return m_gpu_models;
        };

        UniformBuffer<SceneGlobals>* get_scene_globals() const
        {
            return m_scene_globals_gpu;
        };

    private:
        void load_textures(std::vector<ModelData> models);

        Context&           m_context;
        std::vector<Model> m_gpu_models;
        std::vector<Image> m_gpu_textures;
        Camera             m_camera;

        UniformBuffer<SceneGlobals>* m_scene_globals_gpu{};
    };

} // namespace pvp