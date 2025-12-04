#include "PipelineLayoutBuilder.h"

#include <stdexcept>
#include <tracy/Tracy.hpp>

void PipelineLayoutBuilder::build(const VkDevice device, VkPipelineLayout& pipeline_layout) const
{
    ZoneScoped;
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(m_descriptor_layouts.size());
    pipeline_layout_info.pSetLayouts = m_descriptor_layouts.data();
    if (!m_push_constant_ranges.empty())
    {
        pipeline_layout_info.pushConstantRangeCount = static_cast<uint32_t>(m_push_constant_ranges.size());
        pipeline_layout_info.pPushConstantRanges = m_push_constant_ranges.data();
    }

    if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}
PipelineLayoutBuilder& PipelineLayoutBuilder::add_push_constant_range(const VkPushConstantRange push_constant)
{
    m_push_constant_ranges.push_back(push_constant);
    return *this;
}
PipelineLayoutBuilder& PipelineLayoutBuilder::add_descriptor_layout(const VkDescriptorSetLayout pipeline_layout)
{
    m_descriptor_layouts.push_back(pipeline_layout);
    return *this;
}