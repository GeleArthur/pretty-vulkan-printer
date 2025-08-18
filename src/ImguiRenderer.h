#pragma once
#include <vector>
#include <Window/WindowSurface.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    struct GlfwToRender;
}
struct FrameContext;
namespace pvp
{
    class CommandPool;
    struct Context;
    class ImguiRenderer final
    {
    public:
        explicit ImguiRenderer(Context& context, GLFWwindow* window, GlfwToRender* glfw_to_render);

        void setup_vulkan_context(const CommandPool& command_pool);
        void destroy_vulkan_context();

        void            draw(const FrameContext& frame_context, int swapchain_index);
        VkCommandBuffer get_cmd(int index);

    private:
        std::vector<VkCommandBuffer> m_command_buffer;
        GLFWwindow*                  m_window;
        GlfwToRender*                m_glfw_to_render;
        Context&                     m_context;
    };
} // namespace pvp
