#pragma once
#include <globalconst.h>
#include <vector>
#include <Image/Image.h>
#include <Context/Context.h>
#include <vulkan/vulkan.h>
#include <memory>

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
        RenderInfoBuilder& add_color(Image* image);
        RenderInfoBuilder& set_depth(Image* image);
        RenderInfoBuilder& set_size(VkExtent2D size);

        RenderInfo build() const;

    private:
        std::vector<Image*> m_colors;
        Image*              m_depth{ nullptr };
        VkImageLayout       m_layout{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkExtent2D          m_size{};
    };
} // namespace pvp
