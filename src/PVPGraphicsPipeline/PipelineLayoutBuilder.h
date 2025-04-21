#pragma once
#include <vector>
#include <vulkan/vulkan.h>

class PipelineLayoutBuilder
{
    public:
    VkPipelineLayout       build(VkDevice device) const;
    PipelineLayoutBuilder& add_push_constant_range(VkPushConstantRange push_constant);
    PipelineLayoutBuilder& add_descriptor_layout(VkDescriptorSetLayout pipeline_layout);

    private:
    std::vector<VkPushConstantRange>   m_push_constant_ranges;
    std::vector<VkDescriptorSetLayout> m_descriptor_layouts;
};
