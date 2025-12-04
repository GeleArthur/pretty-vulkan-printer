#include "ShaderLoader.h"

#include "globalconst.h"

#include <array>
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <tracy/Tracy.hpp>

#include <fstream>
#include <iostream>
#include <shaderc/shaderc.hpp>
#include <spdlog/spdlog.h>

namespace
{
    shaderc::Compiler       compiler{};
    shaderc::CompileOptions options{};
} // namespace

void test(VkDevice device)
{
    using namespace slang;
    Slang::ComPtr<slang::IGlobalSession> globalSession;

    SlangGlobalSessionDesc desc{
        .enableGLSL = true
    };
    slang::createGlobalSession(&desc, globalSession.writeRef());

    slang::TargetDesc target;
    target.format = SLANG_SPIRV;
    target.profile = globalSession->findProfile("spriv");

    std::array<const char*, 1> paths{ "shaders/" };

    // std::array<CompilerOptionEntry, 1> options = {
    //     { CompilerOptionName::EmitSpirvDirectly, { CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr } }
    // };

    slang::SessionDesc sessionDesc{
        .targets = &target,
        .targetCount = 1,
        .searchPaths = paths.data(),
        .searchPathCount = paths.size(),
        // .allowGLSLSyntax = false,
    };
    Slang::ComPtr<slang::ISession> session;
    globalSession->createSession(sessionDesc, session.writeRef());

    // SlangCompileRequest* request;
    // session->createCompileRequest(&request);

    Slang::ComPtr<slang::IBlob>
                    diagnosticsBlob;
    slang::IModule* module = session->loadModule("helloworld", diagnosticsBlob.writeRef());

    Slang::ComPtr<slang::IBlob> spirvCode;
    module->getTargetCode(0, spirvCode.writeRef(), diagnosticsBlob.writeRef());

    ;

    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = spirvCode->getBufferSize() / 4;
    create_info.pCode = static_cast<const uint32_t*>(spirvCode->getBufferPointer());

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    vkDestroyShaderModule(device, shader_module, nullptr);

    // Slang::ComPtr<IEntryPoint> computeEntryPoint;
    // module->findEntryPointByName("main", computeEntryPoint.writeRef());
    //

    // IComponentType*               components[] = { module, computeEntryPoint };
    // Slang::ComPtr<IComponentType> program;
    // session->createCompositeComponentType(components, 2, program.writeRef());
    //
    // slang::ProgramLayout* layout = program->getLayout();
    //
    // Slang::ComPtr<IComponentType> linkedProgram;
    // Slang::ComPtr<ISlangBlob>     diagnosticBlob;
    // program->link(linkedProgram.writeRef(), diagnosticBlob.writeRef()); // Slang::ComPtr<slang::I> shaderProgram;
    //
    // int                  entryPointIndex = 0; // only one entry point
    // int                  targetIndex = 0;     // only one target
    // Slang::ComPtr<IBlob> kernelBlob;
    // linkedProgram->getEntryPointCode(
    //     entryPointIndex,
    //     targetIndex,
    //     kernelBlob.writeRef(),
    //     diagnosticsBlob.writeRef());
}

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
    // test(device);
    ZoneScoped;

    std::vector<char> shader_code = load_file(path);

    std::string name = path.filename().string();

    options.SetGenerateDebugInfo();
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
    options.SetTargetSpirv(shaderc_spirv_version_1_6);

    // std::filesystem::path shader_base_path = path.parent_path();
    // options.SetIncluder(std::make_unique<NEShaderIncluder>());

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(shader_code.data(), shader_code.size(), shaderc_glsl_infer_from_source, name.c_str(), options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        spdlog::error("Shader compilation failed: {}", result.GetErrorMessage());
        throw;
    }

    std::vector<uint32_t> spirv_code(result.cbegin(), result.cend());
    // assambly_code.resize((compliedCode.cend() - compliedCode.cbegin()));
    // std::ranges::copy(compliedCode, assambly_code.data());

    VkShaderModuleCreateInfo create_info{};
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