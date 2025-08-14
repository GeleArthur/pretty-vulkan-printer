#include "RenderInfoBuilder.h"

#include <tracy/Tracy.hpp>

namespace pvp
{
    RenderInfoBuilder& RenderInfoBuilder::set_layout(VkImageLayout layout)
    {
        m_layout = layout;
        return *this;
    }
    RenderInfoBuilder& RenderInfoBuilder::add_color(VkImageView image, VkAttachmentLoadOp load, VkAttachmentStoreOp store)
    {
        ZoneScoped;
        m_colors.emplace_back(image, load, store);
        return *this;
    }
    RenderInfoBuilder& RenderInfoBuilder::set_depth(VkImageView image, VkAttachmentLoadOp load, VkAttachmentStoreOp store)
    {
        m_depth = ImageLoadStore{ image, load, store };
        return *this;
    }
    RenderInfoBuilder& RenderInfoBuilder::set_size(VkExtent2D size)
    {
        m_size = size;
        return *this;
    }
    void RenderInfoBuilder::build(RenderInfoBuilderOut& render_info) const
    {
        ZoneScoped;
        constexpr VkClearValue clear_values{ 0.2f, 0.2f, 0.2f, 1.0f };

        render_info.rendering_info = VkRenderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, m_size },
            .layerCount = 1,
        };

        render_info.attachment_info.reserve(m_colors.size());
        for (const auto& image : m_colors)
        {
            render_info.attachment_info.push_back(VkRenderingAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = std::get<0>(image),
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = std::get<1>(image),
                .storeOp = std::get<2>(image),
                .clearValue = clear_values });
        }

        if (!m_colors.empty())
        {
            render_info.rendering_info.colorAttachmentCount = static_cast<uint32_t>(render_info.attachment_info.size());
            render_info.rendering_info.pColorAttachments = render_info.attachment_info.data();
        }

        constexpr VkClearValue clear_depth{ 1.0f, 0.0f };

        if (std::get<0>(m_depth) != nullptr)
        {
            render_info.depth_info = VkRenderingAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = std::get<0>(m_depth),
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .loadOp = std::get<1>(m_depth),
                .storeOp = std::get<2>(m_depth),
                .clearValue = clear_depth
            };

            render_info.rendering_info.pDepthAttachment = &render_info.depth_info;
        }
    }
} // namespace pvp