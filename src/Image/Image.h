#pragma once
#include "Sampler.h"

#include <Buffer/Buffer.h>

#include "../Context/Context.h"

#include <array>
#include <globalconst.h>
#include <Events/Event.h>
#include <Events/EventListener.h>

struct FrameContext;
namespace pvp
{
    class Image final
    {
    public:
        Image() = default;
        void destroy(const Context& context) const;

        DISABLE_COPY(Image);

        Image(Image&&) noexcept = default;
        Image& operator=(Image&&) noexcept = default;

        [[nodiscard]] VkImageView              get_view(const FrameContext& frame_context) const;
        [[nodiscard]] VkImageView              get_view(int index) const;
        [[nodiscard]] VkImage                  get_image(const FrameContext& frame_context) const;
        [[nodiscard]] VkImage                  get_image(int index) const;
        [[nodiscard]] VkImageLayout            get_layout(const FrameContext& frame_context) const;
        [[nodiscard]] VkImageLayout            get_layout(int index) const;
        [[nodiscard]] const VmaAllocationInfo& get_allocation_info() const;
        [[nodiscard]] VkFormat                 get_format() const;
        [[nodiscard]] VkExtent2D               get_size() const;
        [[nodiscard]] Event<>&                 get_image_invalid();

        void transition_layout(const FrameContext&   frame_context,
                               VkImageLayout         new_layout,
                               VkPipelineStageFlags2 src_stage_mask,
                               VkPipelineStageFlags2 dst_stage_mask,
                               VkAccessFlags2        src_access_mask,
                               VkAccessFlags2        dst_access_mask);

    private:
        friend class ImageBuilder;

        EventListener<Context&, int, int> m_on_image_resized{
            [this](const Context& context, int width, int height) {
                resize_image(context, width, height);
            }
        };
        void resize_image(const Context& context, int width, int height);
        void create_images(const Context& context);

        Event<> m_image_invalid{};

        VmaAllocationInfo m_allocation_info{};

        VmaAllocationCreateInfo m_allocation_create_info;
        VkImageCreateInfo       m_create_info;
        VkImageViewCreateInfo   m_view_create_info;
        std::string             m_name;

        std::array<VmaAllocation, max_frames_in_flight> m_allocation{};
        std::array<VkImage, max_frames_in_flight>       m_image{ VK_NULL_HANDLE };
        std::array<VkImageView, max_frames_in_flight>   m_view{ VK_NULL_HANDLE };

        std::array<VkImageLayout, max_frames_in_flight> m_current_layout{};
    };
} // namespace pvp
