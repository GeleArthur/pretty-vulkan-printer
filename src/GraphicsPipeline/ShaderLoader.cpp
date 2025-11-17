#include "ShaderLoader.h"

#include <tracy/Tracy.hpp>

#include <fstream>
#include <shaderc/shaderc.hpp>
#include <spdlog/spdlog.h>

namespace
{
    shaderc::Compiler       compiler{};
    shaderc::CompileOptions options{};
} // namespace

std::vector<char> ShaderLoader::load_file(const std::filesystem::path& path)
{
    std::ifstream     file(path, std::ios::in | std::ios::binary);
    const auto        size = std::filesystem::file_size(path);
    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    file.close();
    return buffer;
}

VkShaderModule ShaderLoader::load_shader_from_file(const VkDevice& device, const std::filesystem::path& path)
{
    ZoneScoped;

    VkShaderModuleCreateInfo create_info{};
    std::vector<char>        shader_code = load_file(path);

    std::string name = path.filename().string();

    options.SetGenerateDebugInfo();
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
    options.SetTargetSpirv(shaderc_spirv_version_1_6);

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(shader_code.data(), shader_code.size(), shaderc_glsl_infer_from_source, name.c_str(), options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        spdlog::error("Shader compilation failed: {}", result.GetErrorMessage());
        throw;
    }

    std::vector<uint32_t> spirv_code(result.cbegin(), result.cend());
    // assambly_code.resize((compliedCode.cend() - compliedCode.cbegin()));
    // std::ranges::copy(compliedCode, assambly_code.data());

    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = spirv_code.size() * sizeof(uint32_t);
    create_info.pCode = spirv_code.data();

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shader_module;
}