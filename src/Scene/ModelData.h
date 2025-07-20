#pragma once
#include <filesystem>
#include <memory>
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
        std::string   name;
        uint32_t      width;
        uint32_t      height;
        uint32_t      channels;
        aiTextureType type;

        stbi_uc* pixels;
    };

    struct ModelData
    {
        std::vector<Vertex>   vertices;
        std::vector<uint32_t> indices;
        glm::mat4x4           transform;
        std::string           diffuse_path;
        std::string           metallic_path;
        std::string           normal_path;
    };

    struct LoadedScene
    {
        std::vector<ModelData>   models;
        std::vector<TextureData> textures;
    };

    LoadedScene load_scene_cpu(const std::filesystem::path& path);
} // namespace pvp