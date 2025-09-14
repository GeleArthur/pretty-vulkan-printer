#pragma once
#include "Sampler.h"
#include <vulkan/vulkan.h>

namespace pvp
{
    struct Context;
    class SamplerBuilder
    {
    public:
        explicit SamplerBuilder() = default;

        SamplerBuilder& set_filter(VkFilter filter);
        SamplerBuilder& set_mipmap(VkSamplerMipmapMode mode);
        SamplerBuilder& set_address_mode(VkSamplerAddressMode mode);

        void build(const Context& context, pvp::Sampler& sampler) const;

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
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = 4.0f,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_NEVER,
            .minLod = 0.0f,
            .maxLod = VK_LOD_CLAMP_NONE,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
            .unnormalizedCoordinates = VK_FALSE
        };
    };
} // namespace pvp
