#include "GraphicsPipelineBuilder.h"

#include <PVPPhysicalDevice/PVPPhysicalDevice.h>
#include <vulkan/vulkan_core.h>

#include <stdexcept>
#include <vector>

VkPipeline pvp::GraphicsPipelineBuilder::build(pvp::PhysicalDevice& device)
{
    DestructorQueue                              destructor_queue;
    std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stages;

    for (const auto& shader : m_shader_stages)
    {
        VkPipelineShaderStageCreateInfo vert_shader_stage_info {};
        vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_shader_stage_info.module = std::get<0>(shader);
        vert_shader_stage_info.stage = std::get<1>(shader);
        vert_shader_stage_info.pName = "main";
        pipeline_shader_stages.push_back(vert_shader_stage_info);
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_info {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    vertex_input_info.vertexBindingDescriptionCount = m_input_binding_descriptions.size();
    vertex_input_info.pVertexBindingDescriptions = m_input_binding_descriptions.data();
    vertex_input_info.vertexAttributeDescriptionCount = m_input_atrribute_descriptions.size();
    vertex_input_info.pVertexAttributeDescriptions = m_input_atrribute_descriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = m_topology;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewport_state {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;          // Optional
    multisampling.pSampleMask = nullptr;            // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optional

    VkPipelineColorBlendAttachmentState color_blend_attachment {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &color_blend_attachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector                      dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamic_state {};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state.pDynamicStates = dynamic_states.data();

    VkPipelineDepthStencilStateCreateInfo depthStencil {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;

    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;

    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = pipeline_shader_stages.size();
    pipelineInfo.pStages = pipeline_shader_stages.data();

    pipelineInfo.pVertexInputState = &vertex_input_info;
    pipelineInfo.pInputAssemblyState = &input_assembly;
    pipelineInfo.pViewportState = &viewport_state;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamic_state;

    pipelineInfo.layout = m_pipeline_layout;
    pipelineInfo.renderPass = m_render_pass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VkPipeline graphics_pipeline;
    if (vkCreateGraphicsPipelines(device.get_device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphics_pipeline) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    return graphics_pipeline;
}
pvp::GraphicsPipelineBuilder& pvp::GraphicsPipelineBuilder::add_shader(VkShaderModule& shader, VkShaderStageFlagBits stage)
{
    m_shader_stages.push_back(std::tuple(shader, stage));
    return *this;
}
pvp::GraphicsPipelineBuilder& pvp::GraphicsPipelineBuilder::set_input_binding_description(
const std::vector<VkVertexInputBindingDescription>& binding_description)
{
    m_input_binding_descriptions = binding_description;
    return *this;
}

pvp::GraphicsPipelineBuilder& pvp::GraphicsPipelineBuilder::set_input_atrribute_description(const std::vector<VkVertexInputAttributeDescription>& binding_description)
{
    m_input_atrribute_descriptions = binding_description;
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
pvp::GraphicsPipelineBuilder& pvp::GraphicsPipelineBuilder::set_render_pass(VkRenderPass render_pass)
{
    m_render_pass = render_pass;
    return *this;
}