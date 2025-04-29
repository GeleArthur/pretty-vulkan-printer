#pragma once
#include <span>
#include <PVPPhysicalDevice/Device.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace pvp
{
    class GraphicsPipelineBuilder
    {
    public:
        VkPipeline               build(pvp::Device& device);
        GraphicsPipelineBuilder& add_shader(VkShaderModule& shader, VkShaderStageFlagBits stage);
        GraphicsPipelineBuilder& set_input_binding_description(const std::vector<VkVertexInputBindingDescription>& binding_description);
        GraphicsPipelineBuilder& set_input_attribute_description(const std::vector<VkVertexInputAttributeDescription>& binding_description);
        GraphicsPipelineBuilder& set_topology(VkPrimitiveTopology topology);
        GraphicsPipelineBuilder& set_pipeline_layout(VkPipelineLayout pipeline_layout);
        GraphicsPipelineBuilder& set_render_pass(VkRenderPass render_pass);

    private:
        std::vector<std::tuple<VkShaderModule, VkShaderStageFlagBits>> m_shader_stages;
        std::vector<VkVertexInputBindingDescription>                   m_input_binding_descriptions;
        std::vector<VkVertexInputAttributeDescription>                 m_input_atrribute_descriptions;
        VkPrimitiveTopology                                            m_topology{ VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
        VkPipelineLayout                                               m_pipeline_layout{ nullptr };
        VkRenderPass                                                   m_render_pass{ nullptr };
    };
} // namespace pvp
