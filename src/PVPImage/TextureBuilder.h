#pragma once
#include "Image.h"

#include <filesystem>
#include <string_view>
#include <PVPCommandBuffer/CommandPool.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    class TextureBuilder
    {
    public:
        TextureBuilder() = default;
        TextureBuilder& set_path(std::filesystem::path path);

        void build(VkDevice device, const CommandPool& command_buffer, Image& image) const;

    private:
        std::filesystem::path m_path;
    };
} // namespace pvp
