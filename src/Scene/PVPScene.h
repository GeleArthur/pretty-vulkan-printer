#pragma once
#include "LoadModel.h"

#include <filesystem>
#include <vector>
#include <PVPBuffer/Buffer.h>
#include <PVPPhysicalDevice/Context.h>
#include <glm/mat4x4.hpp>
#include <glm/detail/func_packing_simd.inl>

namespace pvp
{
    struct Model
    {
        Buffer      vertex_data;
        Buffer      index_data;
        uint32_t    index_count;
        glm::mat4x4 model_mat;
    };

    struct PvpScene
    {
        std::vector<Model> models;
        glm::mat4x4        camera_view_projection{};
    };

    PvpScene load_scene(const Context& context);
    void     destroy_scene(PvpScene& scene);

} // namespace pvp