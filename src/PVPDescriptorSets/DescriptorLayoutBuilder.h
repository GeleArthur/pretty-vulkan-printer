#pragma once
#include "DescriptorLayout.h"

#include <vector>
namespace pvp
{
    class DescriptorLayoutBuilder
    {
    public:
        DescriptorLayoutBuilder& add_binding(vk::DescriptorType type, vk::ShaderStageFlags stage);
        void                     build(const vk::Device& device, vk::DescriptorSetLayout& layout);

    private:
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
    };
} // namespace pvp
