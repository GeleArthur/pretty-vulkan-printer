#include "RenderInfoBuilder.h"

namespace pvp
{
    RenderInfoBuilder& RenderInfoBuilder::set_layout(VkImageLayout layout)
    {
        m_layout = layout;
        return *this;
    }
    RenderInfoBuilder& RenderInfoBuilder::add_image(Image* image)
    {
        m_images.push_back(image);
        return *this;
    }
    RenderInfo RenderInfoBuilder::build() const
    {
        constexpr VkClearValue clear_values{ 0.2f, 0.2f, 0.2f, 1.0f };

        RenderInfo info{};

        for (Image* image : m_images)
        {
            info.attachment_info.push_back(VkRenderingAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = image->get_view(),
                .imageLayout = m_layout,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = clear_values });
        }

        info.rendering_info = VkRenderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, m_images[0]->get_size() },
            .layerCount = 1,
            .colorAttachmentCount = static_cast<uint32_t>(info.attachment_info.size()),
            .pColorAttachments = info.attachment_info.data(),
        };

        return info;
    }
} // namespace pvp