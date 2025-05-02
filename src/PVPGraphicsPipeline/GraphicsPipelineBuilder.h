#pragma once
#include <span>
#include <PVPDevice/Device.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace pvp
{
    template <typename Con, typename Item>
    concept range_of = std::ranges::range<Con> && std::convertible_to<std::ranges::range_value_t<Con>, Item>;


    class GraphicsPipelineBuilder
    {
    public:
        VkPipeline build(pvp::Device& device);
        GraphicsPipelineBuilder& add_shader(VkShaderModule& shader, VkShaderStageFlagBits stage);

        // template<std::ranges::range U>
        GraphicsPipelineBuilder& set_input_binding_description(const range_of<VkVertexInputBindingDescription> auto& binding_description);
        GraphicsPipelineBuilder& set_input_attribute_description(const std::vector<VkVertexInputAttributeDescription>& binding_description);
        GraphicsPipelineBuilder& set_topology(VkPrimitiveTopology topology);
        GraphicsPipelineBuilder& set_pipeline_layout(VkPipelineLayout pipeline_layout);
        GraphicsPipelineBuilder& set_render_pass(VkRenderPass render_pass);

    private:
        std::vector<std::tuple<VkShaderModule, VkShaderStageFlagBits>> m_shader_stages;
        std::vector<VkVertexInputBindingDescription> m_input_binding_descriptions;
        std::vector<VkVertexInputAttributeDescription> m_input_atrribute_descriptions;
        VkPrimitiveTopology m_topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
        VkPipelineLayout m_pipeline_layout{nullptr};
        VkRenderPass m_render_pass{nullptr};
    };

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_input_binding_description(const range_of<VkVertexInputBindingDescription> auto& binding_description)
    {
        m_input_binding_descriptions.assign(binding_description.begin(), binding_description.end());
        return *this;
    }
} // namespace pvp
