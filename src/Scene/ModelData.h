#pragma once
#include <filesystem>
#include <memory>
#include <meshoptimizer.h>
#include <stb_image.h>
#include <assimp/material.h>
#include <glm/mat4x4.hpp>
#include <vulkan/vulkan_core.h>

namespace pvp
{
    struct Context;
    class Image;
    struct Vertex;

    struct TextureData
    {
        std::string name{};
        uint32_t    width{};
        uint32_t    height{};
        VkFormat    format{};

        std::vector<uint8_t> pixels;
        bool                 generate_mip_maps{};
    };

    struct ConeBounds
    {
        glm::vec4 sphere;
        glm::vec4 cone;
        // char      cone_axis_x;
        // char      cone_axis_y;
        // char      cone_axis_z;
        // char      cone_cutoff;
    };

    struct ModelData
    {
        std::vector<Vertex>   vertices;
        std::vector<uint32_t> indices;
        glm::mat4x4           transform;
        std::string           diffuse_path;
        std::string           metallic_path;
        std::string           normal_path;

        // MESHLETS
        std::vector<meshopt_Meshlet> meshlets;
        std::vector<uint32_t>        meshlet_vertices;
        std::vector<uint8_t>         meshlet_triangles;
        std::vector<ConeBounds>      meshlet_sphere_bounds;
    };

    struct LoadedScene
    {
        std::vector<ModelData>   models;
        std::vector<TextureData> textures;
        TextureData              cube_map;
    };

    std::optional<LoadedScene> load_scene_cpu(const std::filesystem::path& path);
} // namespace pvp
