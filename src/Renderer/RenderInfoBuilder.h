#pragma once
#include <globalconst.h>
#include <memory>
#include <tuple>
#include <vector>
#include <Image/Image.h>
#include <vulkan/vulkan.h>

struct RenderInfo
{
    std::vector<VkRenderingAttachmentInfo> attachment_info;
    VkRenderingAttachmentInfo              depth_info;
    VkRenderingInfo                        rendering_info;
};

namespace pvp
{
    class RenderInfoBuilder
    {
    public:
        explicit RenderInfoBuilder() = default;
        DISABLE_COPY(RenderInfoBuilder);
        DISABLE_MOVE(RenderInfoBuilder);

        RenderInfoBuilder& set_layout(VkImageLayout layout);
        RenderInfoBuilder& add_color(VkImageView image, VkAttachmentLoadOp load, VkAttachmentStoreOp store);
        RenderInfoBuilder& set_depth(VkImageView image, VkAttachmentLoadOp load, VkAttachmentStoreOp store);
        RenderInfoBuilder& set_size(VkExtent2D size);

        void build(RenderInfo& render_info) const;

    private:
        using ImageLoadStore = std::tuple<VkImageView, VkAttachmentLoadOp, VkAttachmentStoreOp>;
        std::vector<ImageLoadStore> m_colors;
        ImageLoadStore              m_depth{};
        VkImageLayout               m_layout{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkExtent2D                  m_size{};
    };
} // namespace pvp
