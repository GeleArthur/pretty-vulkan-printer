#pragma once
#include <filesystem>
#include <span>
#include <PVPDevice/Device.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace pvp
{
    // hihi. A foot gun? Or genius?
    template<typename Con, typename Item>
    concept range_of = std::ranges::range<Con> && std::convertible_to<std::ranges::range_value_t<Con>, Item>;

    class GraphicsPipelineBuilder
    {
    public:
        GraphicsPipelineBuilder& add_shader(std::filesystem::path path, VkShaderStageFlagBits stage);
        GraphicsPipelineBuilder& set_input_binding_description(const range_of<VkVertexInputBindingDescription> auto& binding_description);
        GraphicsPipelineBuilder& set_input_attribute_description(const range_of<VkVertexInputAttributeDescription> auto& binding_description);
        GraphicsPipelineBuilder& set_topology(VkPrimitiveTopology topology);
        GraphicsPipelineBuilder& set_pipeline_layout(VkPipelineLayout pipeline_layout);
        GraphicsPipelineBuilder& set_color_format(const range_of<VkFormat> auto& formats);
        GraphicsPipelineBuilder& set_depth_format(VkFormat format);
        GraphicsPipelineBuilder& set_depth_access(VkBool32 read, VkBool32 write);

        void build(const Device& device, VkPipeline& pipeline);

    private:
        std::vector<std::tuple<std::filesystem::path, VkShaderStageFlagBits, VkShaderModule>> m_shader_stages;
        std::vector<VkVertexInputBindingDescription>                                          m_input_binding_descriptions;
        std::vector<VkVertexInputAttributeDescription>                                        m_input_attribute_descriptions;
        std::vector<VkFormat>                                                                 m_color_formats;
        VkFormat                                                                              m_depth_format{ VK_FORMAT_D32_SFLOAT_S8_UINT };
        VkPrimitiveTopology                                                                   m_topology{ VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
        VkPipelineLayout                                                                      m_pipeline_layout{ nullptr };
        VkBool32                                                                              m_read{ VK_TRUE };
        VkBool32                                                                              m_write{ VK_TRUE };
    };

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_input_binding_description(const range_of<VkVertexInputBindingDescription> auto& binding_description)
    {
        m_input_binding_descriptions.assign(binding_description.begin(), binding_description.end());
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_input_attribute_description(const range_of<VkVertexInputAttributeDescription> auto& binding_description)
    {
        m_input_attribute_descriptions.assign(binding_description.begin(), binding_description.end());
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_color_format(const range_of<VkFormat> auto& formats)
    {
        m_color_formats.assign(formats.begin(), formats.end());
        return *this;
    }
} // namespace pvp
