#include "TextureBuilder.h"

#include "Image.h"

#include <stb_image.h>
#include <stb_image.h>
#include <PVPBuffer/Buffer.h>
#include <PVPBuffer/BufferBuilder.h>
#include <PVPVMAAllocator/VmaAllocator.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    TextureBuilder& TextureBuilder::set_path(std::filesystem::path path)
    {
        m_path = path;
        return *this;
    }
    Image TextureBuilder::build(const VkDevice device, const CommandBuffer& command_buffer) const
    {
        int      tex_width, tex_height, tex_channels;
        stbi_uc* pixels = stbi_load(m_path.generic_string().c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

        if (!pixels)
        {
            spdlog::error("{}", stbi_failure_reason());
            throw std::runtime_error("failed to load texture image!");
        }
        VkDeviceSize image_size = tex_width * tex_height * 4;

        Buffer       staging_buffer = BufferBuilder()
                                .set_size(image_size)
                                .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
                                .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
                                .build(PvpVmaAllocator::get_allocator());

        staging_buffer.input_data(pixels, image_size);
        stbi_image_free(pixels);

        Image           image_texture = Image(device,
                                    tex_width,
                                    tex_height,
                                    VK_FORMAT_R8G8B8A8_SRGB,
                                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                    VK_IMAGE_ASPECT_COLOR_BIT,
                                    VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        VkCommandBuffer cmd = command_buffer.begin_single_use_transfer_command(); // Is this correct? No sync issues?
        image_texture.transition_layout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        image_texture.copy_from_buffer(cmd, staging_buffer.get_buffer());
        image_texture.transition_layout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        command_buffer.end_single_use_transfer_command(cmd);

        staging_buffer.destroy();

        return image_texture;
    }
} // namespace pvp