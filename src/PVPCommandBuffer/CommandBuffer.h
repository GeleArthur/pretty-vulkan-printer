#pragma once
#include <PVPDevice/Device.h>

namespace pvp
{
    class CommandBuffer
    {
    public:
        explicit CommandBuffer(Device& physical);
        ~CommandBuffer();

        [[nodiscard]] VkCommandBuffer get_graphics_command_buffer(uint32_t current_frame) const;
        [[nodiscard]] VkCommandBuffer begin_single_use_transfer_command() const;
        void end_single_use_transfer_command(VkCommandBuffer command_buffer) const;

    private:
        VkDevice m_device{};
        QueueFamily m_transfer_family{};
        VkCommandPool m_single_use_transfer_command_pool{};
        std::vector<VkCommandBuffer> m_graphics_command_buffer;
        VkCommandPool m_reset_graphics_command_pool{};
    };
} // namespace pvp
