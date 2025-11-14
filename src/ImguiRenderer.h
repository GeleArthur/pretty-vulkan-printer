#pragma once
#include <globalconst.h>
#include <vector>
#include <Window/WindowSurface.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    class PvpScene;
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
        ~ImguiRenderer() = default;
        DISABLE_COPY(ImguiRenderer);
        DISABLE_MOVE(ImguiRenderer);

        void setup_vulkan_context(const CommandPool& command_pool);
        void destroy_vulkan_context();

        void start_drawing();
        void end_drawing();

        void draw(const FrameContext& frame_context, uint32_t swapchain_index);
        void update_screen();

    private:
        std::vector<VkCommandBuffer> m_command_buffer;
        GLFWwindow* m_window;
        GlfwToRender* m_glfw_to_render;
        Context& m_context;
    };
} // namespace pvp
