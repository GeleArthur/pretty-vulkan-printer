#include "DescriptorLayoutBuilder.h"

#include <stdexcept>

namespace pvp
{
    DescriptorLayoutBuilder& DescriptorLayoutBuilder::add_binding(vk::DescriptorType type, vk::ShaderStageFlags stage)
    {
        bindings.push_back(vk::DescriptorSetLayoutBinding{ static_cast<uint32_t>(bindings.size() - 1), type, 1u, stage, VK_NULL_HANDLE });
        return *this;
    }

    void DescriptorLayoutBuilder::build(const vk::Device& device, vk::DescriptorSetLayout& layout)
    {
        const vk::DescriptorSetLayoutCreateInfo layout_info{
            {},
            static_cast<uint32_t>(bindings.size()),
            bindings.data(),
        };

        if (device.createDescriptorSetLayout(&layout_info, nullptr, &layout) != vk::Result::eSuccess)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }
} // namespace pvp