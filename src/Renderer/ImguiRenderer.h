#pragma once
#include <vector>
#include <vulkan/vulkan.h>

struct FrameContext;
namespace pvp
{
    class CommandPool;
    struct Context;
    class ImguiRenderer final
    {
    public:
        explicit ImguiRenderer(pvp::Context& context, CommandPool& command_pool);
        ~ImguiRenderer();
        void            draw(const FrameContext& frame_context, int swapchain_index);
        VkCommandBuffer get_cmd(int index);

    private:
        std::vector<VkCommandBuffer> m_command_buffer;
        Context&                     m_context;
    };
} // namespace pvp
