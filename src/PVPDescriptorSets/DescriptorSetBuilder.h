#pragma once
#include <PVPBuffer/Buffer.h>
#include <PVPImage/Image.h>
#include <span>
#include <unordered_map>
#include <vulkan/vulkan.h>

#include "DescriptorLayout.h"
#include "DescriptorPool.h"
#include "DescriptorSets.h"

#include <UniformBufferStruct.h>
#include <PVPUniformBuffers/UniformBuffer.h>

namespace pvp
{
    class DescriptorSetBuilder final
    {
    public:
        explicit DescriptorSetBuilder();
        DescriptorSetBuilder& set_layout(DescriptorLayout& layout);

        template<typename T>
        DescriptorSetBuilder& bind_buffer(uint32_t binding, const UniformBuffer<T>& buffer);
        DescriptorSetBuilder& bind_image(uint32_t binding, const Image& image, const Sampler& sampler);

        DescriptorSets build(VkDevice device, DescriptorPool pool) const;

    private:
        DescriptorLayout*                                                                                             m_descriptor_layout{};
        std::vector<std::tuple<uint32_t, std::reference_wrapper<const std::vector<Buffer>>>>                          m_buffers;
        std::vector<std::tuple<uint32_t, std::reference_wrapper<const Image>, std::reference_wrapper<const Sampler>>> m_images;
    };

    template<typename T>
    DescriptorSetBuilder& DescriptorSetBuilder::bind_buffer(uint32_t binding, const UniformBuffer<T>& buffer)
    {
        m_buffers.push_back({ binding, buffer.get_buffers() });
        return *this;
    }
} // namespace pvp
