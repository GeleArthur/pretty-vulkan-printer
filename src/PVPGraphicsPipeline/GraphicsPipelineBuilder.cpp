#include "GraphicsPipelineBuilder.h"

#include "DestructorQueue.h"

#include <PVPDevice/Device.h>
#include <vulkan/vulkan_core.h>

#include <stdexcept>
#include <vector>

#include "ShaderLoader.h"

void pvp::GraphicsPipelineBuilder::build(const Device& device, VkPipeline& pipeline)
{
    DestructorQueue destructor_queue;
    std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stages;

    for (auto& shader : m_shader_stages)
    {
        std::get<2>(shader) = ShaderLoader::load_shader_from_file(device.get_device(), std::get<0>(shader));

        VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
        vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_shader_stage_info.module = std::get<2>(shader);
        vert_shader_stage_info.stage = std::get<1>(shader);
        vert_shader_stage_info.pName = "main";
        pipeline_shader_stages.push_back(vert_shader_stage_info);
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = m_input_binding_descriptions.size();
    vertex_input_info.pVertexBindingDescriptions = m_input_binding_descriptions.data();
    vertex_input_info.vertexAttributeDescriptionCount = m_input_attribute_descriptions.size();
    vertex_input_info.pVertexAttributeDescriptions = m_input_attribute_descriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = m_topology;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    std::vector dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state.pDynamicStates = dynamic_states.data();

    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = m_read;
    depth_stencil.depthWriteEnable = m_write;

    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f;
    depth_stencil.maxDepthBounds = 1.0f;

    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.front = {};
    depth_stencil.back = {};

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = pipeline_shader_stages.size();
    pipelineInfo.pStages = pipeline_shader_stages.data();

    pipelineInfo.pVertexInputState = &vertex_input_info;
    pipelineInfo.pInputAssemblyState = &input_assembly;
    pipelineInfo.pViewportState = &viewport_state;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depth_stencil;
    pipelineInfo.pColorBlendState = &color_blending;
    pipelineInfo.pDynamicState = &dynamic_state;

    pipelineInfo.layout = m_pipeline_layout;
    pipelineInfo.renderPass = VK_NULL_HANDLE;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VkPipelineRenderingCreateInfo render_target{};
    render_target.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    render_target.colorAttachmentCount = m_color_formats.size();
    render_target.pColorAttachmentFormats = m_color_formats.data();
    render_target.depthAttachmentFormat = m_depth_format;

    pipelineInfo.pNext = &render_target;

    if (vkCreateGraphicsPipelines(device.get_device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    for (auto& shader : m_shader_stages)
    {
        vkDestroyShaderModule(device.get_device(), std::get<2>(shader), nullptr);
    }
}

pvp::GraphicsPipelineBuilder& pvp::GraphicsPipelineBuilder::add_shader(std::filesystem::path path, VkShaderStageFlagBits stage)
{
    m_shader_stages.push_back(std::tuple(path, stage, VK_NULL_HANDLE));
    return *this;
}

pvp::GraphicsPipelineBuilder& pvp::GraphicsPipelineBuilder::set_topology(const VkPrimitiveTopology topology)
{
    m_topology = topology;
    return *this;
}

pvp::GraphicsPipelineBuilder& pvp::GraphicsPipelineBuilder::set_pipeline_layout(VkPipelineLayout pipeline_layout)
{
    m_pipeline_layout = pipeline_layout;
    return *this;
}

pvp::GraphicsPipelineBuilder& pvp::GraphicsPipelineBuilder::set_depth_format(VkFormat format)
{
    m_depth_format = format;
    return *this;
}

pvp::GraphicsPipelineBuilder& pvp::GraphicsPipelineBuilder::set_depth_access(VkBool32 read, VkBool32 write)
{
    m_read = read;
    m_write = write;
    return *this;
}
