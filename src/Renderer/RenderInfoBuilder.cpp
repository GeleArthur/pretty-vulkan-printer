#include "RenderInfoBuilder.h"

namespace pvp
{
    RenderInfoBuilder& RenderInfoBuilder::set_layout(VkImageLayout layout)
    {
        m_layout = layout;
        return *this;
    }
    RenderInfoBuilder& RenderInfoBuilder::set_image(Image* image)
    {
        m_image = image;
        return *this;
    }
    RenderInfo RenderInfoBuilder::build() const
    {
        constexpr VkClearValue clear_values{ 0.2f, 0.2f, 0.2f, 1.0f };

        RenderInfo info{};

        info.attachment_info = VkRenderingAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = m_image->get_view(),
            .imageLayout = m_layout,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clear_values
        };

        info.rendering_info = VkRenderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, m_image->get_size() },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &info.attachment_info,
        };

        return info;
    }
} // namespace pvp