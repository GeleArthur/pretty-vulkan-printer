#pragma once
#include <span>
#include <PVPDevice/Device.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace pvp
{
    template<typename Con, typename Item>
    concept range_of = std::ranges::range<Con> && std::convertible_to<std::ranges::range_value_t<Con>, Item>;

    class GraphicsPipelineBuilder
    {
    public:
        VkPipeline               build(pvp::Device& device);
        GraphicsPipelineBuilder& add_shader(VkShaderModule& shader, VkShaderStageFlagBits stage);

        // template<std::ranges::range U>
        GraphicsPipelineBuilder& set_input_binding_description(const range_of<VkVertexInputBindingDescription> auto& binding_description);
        GraphicsPipelineBuilder& set_input_attribute_description(const std::vector<VkVertexInputAttributeDescription>& binding_description);
        GraphicsPipelineBuilder& set_topology(VkPrimitiveTopology topology);
        GraphicsPipelineBuilder& set_pipeline_layout(VkPipelineLayout pipeline_layout);
        GraphicsPipelineBuilder& set_color_format(const range_of<VkFormat> auto& formats);
        GraphicsPipelineBuilder& set_depth_buffer(VkFormat format);

    private:
        std::vector<std::tuple<VkShaderModule, VkShaderStageFlagBits>> m_shader_stages;
        std::vector<VkVertexInputBindingDescription>                   m_input_binding_descriptions;
        std::vector<VkVertexInputAttributeDescription>                 m_input_attribute_descriptions;
        std::vector<VkFormat>                                          m_color_formats;
        VkFormat                                                       m_depth_format{ VK_FORMAT_D32_SFLOAT_S8_UINT };
        VkPrimitiveTopology                                            m_topology{ VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
        VkPipelineLayout                                               m_pipeline_layout{ nullptr };
    };

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_input_binding_description(const range_of<VkVertexInputBindingDescription> auto& binding_description)
    {
        m_input_binding_descriptions.assign(binding_description.begin(), binding_description.end());
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_color_format(const range_of<VkFormat> auto& formats)
    {
        m_color_formats.assign(formats.begin(), formats.end());
        return *this;
    }
} // namespace pvp
