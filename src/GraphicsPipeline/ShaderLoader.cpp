#include "ShaderLoader.h"
#include <fstream>
#include <shaderc/shaderc.hpp>
#include <spdlog/spdlog.h>

std::vector<char> ShaderLoader::load_file(const std::filesystem::path& path)
{
    std::ifstream     file(path, std::ios::in | std::ios::binary);
    const auto        size = std::filesystem::file_size(path);
    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    file.close();
    return buffer;
}

VkShaderModule ShaderLoader::load_shader_from_file(const VkDevice& device, std::filesystem::path path)
{
    VkShaderModuleCreateInfo create_info{};
    std::vector<char>        shader_code = load_file(path);

    shaderc::Compiler compiler{};

    path.replace_extension();
    auto name = path.filename().string();

    shaderc::AssemblyCompilationResult result = compiler.PreprocessGlsl(shader_code.data(), shader_code.size(), shaderc_glsl_infer_from_source, name.c_str(), {});
    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        spdlog::error(result.GetErrorMessage());
        throw;
    }

    shader_code.resize(result.cend() - result.cbegin());
    std::ranges::copy(result, shader_code.data());

    result = compiler.CompileGlslToSpvAssembly(shader_code.data(), shader_code.size(), shaderc_glsl_infer_from_source, name.c_str(), {});
    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        spdlog::error(result.GetErrorMessage().c_str());
        throw;
    }

    shader_code.resize(result.cend() - result.cbegin());
    std::ranges::copy(result, shader_code.data());

    shaderc::SpvCompilationResult compliedCode = compiler.AssembleToSpv(shader_code.data(), shader_code.size());
    if (compliedCode.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        spdlog::error(compliedCode.GetErrorMessage().c_str());
        throw;
    }

    std::vector<uint32_t> assambly_code;
    assambly_code.resize((compliedCode.cend() - compliedCode.cbegin()));
    std::ranges::copy(compliedCode, assambly_code.data());

    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = assambly_code.size() * sizeof(uint32_t);
    create_info.pCode = assambly_code.data();

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shader_module;
}