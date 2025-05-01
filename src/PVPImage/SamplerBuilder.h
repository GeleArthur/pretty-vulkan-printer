#pragma once

#include <vulkan/vulkan.h>
#include "Sampler.h"

class SamplerBuilder
{
public:
    explicit SamplerBuilder() = default;

    SamplerBuilder& set_filter(VkFilter filter);
    SamplerBuilder& set_mipmap(VkSamplerMipmapMode mode);
    SamplerBuilder& set_address_mode(VkSamplerAddressMode mode);

    void build(VkDevice device, pvp::Sampler& sampler) const;

private:
    VkSamplerCreateInfo m_sampler_info{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 0.0f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = static_cast<float>(1),
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };
};
