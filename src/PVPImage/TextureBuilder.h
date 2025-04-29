#pragma once
#include "Image.h"

#include <filesystem>
#include <string_view>
#include <PVPCommandBuffer/CommandBuffer.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    class TextureBuilder
    {
        public:
        TextureBuilder() = default;
        TextureBuilder& set_path(std::filesystem::path path);
        Image           build(VkDevice device, const CommandBuffer& command_buffer) const;

        private:
        std::filesystem::path m_path;
    };
} // namespace pvp
