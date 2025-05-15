#include "RenderInfoBuilder.h"

namespace pvp
{
    RenderInfoBuilder& RenderInfoBuilder::set_layout(VkImageLayout layout)
    {
        m_layout = layout;
        return *this;
    }
    RenderInfoBuilder& RenderInfoBuilder::add_color(Image* image)
    {
        m_colors.push_back(image);
        return *this;
    }
    RenderInfoBuilder& RenderInfoBuilder::set_depth(Image* image)
    {
        m_depth = image;
        return *this;
    }
    RenderInfoBuilder& RenderInfoBuilder::set_size(VkExtent2D size)
    {
        m_size = size;
        return *this;
    }
    RenderInfo RenderInfoBuilder::build() const
    {
        constexpr VkClearValue clear_values{ 0.2f, 0.2f, 0.2f, 1.0f };

        RenderInfo info{};
        info.rendering_info = VkRenderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, m_size },
            .layerCount = 1,
        };

        for (Image* image : m_colors)
        {
            info.attachment_info.push_back(VkRenderingAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = image->get_view(),
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = clear_values });
        }

        if (!m_colors.empty())
        {
            info.rendering_info.colorAttachmentCount = static_cast<uint32_t>(info.attachment_info.size());
            info.rendering_info.pColorAttachments = info.attachment_info.data();
        }

        constexpr VkClearValue clear_depth{ 1.0f, 0.0f };

        if (m_depth != nullptr)
        {
            info.depth_info = std::unique_ptr<VkRenderingAttachmentInfo>(new VkRenderingAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = m_depth->get_view(),
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = clear_depth });

            info.rendering_info.pDepthAttachment = info.depth_info.get();
        }

        return info;
    }
} // namespace pvp