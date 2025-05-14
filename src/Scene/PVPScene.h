#pragma once
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
        explicit PvpScene(const Context& context);
        ~PvpScene();

        std::vector<Model>           models;
        UniformBuffer<SceneGlobals>* scene_globals{};
    };

} // namespace pvp