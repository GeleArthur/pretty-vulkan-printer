#pragma once
#include <globalconst.h>
#include <memory>
#include <tuple>
#include <vector>
#include <Image/Image.h>
#include <vulkan/vulkan.h>

struct RenderInfo
{
    std::vector<VkRenderingAttachmentInfo>     attachment_info;
    std::unique_ptr<VkRenderingAttachmentInfo> depth_info;
    VkRenderingInfo                            rendering_info;
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
        RenderInfoBuilder& add_color(Image* image, VkAttachmentLoadOp load, VkAttachmentStoreOp store);
        RenderInfoBuilder& set_depth(Image* image, VkAttachmentLoadOp load, VkAttachmentStoreOp store);
        RenderInfoBuilder& set_size(VkExtent2D size);

        RenderInfo build() const;

    private:
        using ImageLoadStore = std::tuple<Image*, VkAttachmentLoadOp, VkAttachmentStoreOp>;
        std::vector<ImageLoadStore> m_colors;
        ImageLoadStore              m_depth{};
        VkImageLayout               m_layout{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkExtent2D                  m_size{};
    };
} // namespace pvp
