#pragma once
#include <globalconst.h>
#include <PVPImage/Image.h>
#include <PVPPhysicalDevice/Context.h>
#include <vulkan/vulkan.h>

struct RenderInfo
{
    VkRenderingAttachmentInfo attachment_info;
    VkRenderingInfo           rendering_info;
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
        RenderInfoBuilder& set_image(Image* image);

        RenderInfo build() const;

    private:
        Image*        m_image;
        VkImageLayout m_layout{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    };
} // namespace pvp
