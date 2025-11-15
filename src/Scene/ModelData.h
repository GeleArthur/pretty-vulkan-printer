#pragma once
#include <filesystem>
#include <memory>
#include <meshoptimizer.h>
#include <stb_image.h>
#include <assimp/material.h>
#include <glm/mat4x4.hpp>


namespace pvp
{
    struct Context;
    class Image;
    struct Vertex;

    struct TextureData
    {
        std::string name;
        uint32_t width;
        uint32_t height;
        uint32_t channels;
        aiTextureType type;

        stbi_uc* pixels;
    };

    struct ModelData
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        glm::mat4x4 transform;
        std::string diffuse_path;
        std::string metallic_path;
        std::string normal_path;

        // MESHLETS
        std::vector<meshopt_Meshlet> meshlets;
        std::vector<uint32_t> meshlet_vertices;
        std::vector<uint32_t> meshlet_triangles;
    };

    struct LoadedScene
    {
        std::vector<ModelData> models;
        std::vector<TextureData> textures;
        TextureData cube_map;
    };

    LoadedScene load_scene_cpu(const std::filesystem::path& path);
} // namespace pvp
