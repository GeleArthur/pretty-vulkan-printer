#pragma once
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

class ShaderLoader
{
    public:
    static std::vector<char> load_file(const std::filesystem::path& path);
    static VkShaderModule    load_shader_from_file(const VkDevice& device, std::filesystem::path path);
};
