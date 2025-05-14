#pragma once
#include <globalconst.h>
#include <vector>
#include <Image/Image.h>
#include <Context/Context.h>
#include <vulkan/vulkan.h>

struct RenderInfo
{
    std::vector<VkRenderingAttachmentInfo> attachment_info;
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
        RenderInfoBuilder& add_image(Image* image);

        RenderInfo build() const;

    private:
        std::vector<Image*> m_images;
        VkImageLayout       m_layout{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    };
} // namespace pvp
