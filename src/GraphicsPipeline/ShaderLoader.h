#pragma once
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

namespace ShaderLoader
{
    void              init();
    std::vector<char> load_file(const std::filesystem::path& path);
    VkShaderModule    load_shader_from_file(const VkDevice& device, const std::filesystem::path& path);
}; // namespace ShaderLoader
