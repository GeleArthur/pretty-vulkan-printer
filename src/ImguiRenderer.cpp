#include "ImguiRenderer.h"

#include "Renderer/RenderInfoBuilder.h"
#include "Renderer/Renderer.h"
#include "Renderer/Swapchain.h"

#include <GlfwToRender.h>
#include <Context/Context.h>
#include <Context/Device.h>
#include <Context/PhysicalDevice.h>
#include <Context/QueueFamilies.h>
#include <DescriptorSets/DescriptorLayoutCreator.h>
#include <Image/TransitionLayout.h>
#include <Window/WindowSurface.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    pvp::GlfwToRender* glfw = static_cast<pvp::GlfwToRender*>(glfwGetWindowUserPointer(window));
    {
        std::lock_guard lock(glfw->lock);
        glfw->screen_width = width;
        glfw->screen_height = height;
    }

    glfw->needs_resizing.store(true);
}
static void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    pvp::GlfwToRender* glfw = static_cast<pvp::GlfwToRender*>(glfwGetWindowUserPointer(window));
    {
        std::lock_guard lock(glfw->lock);
        glfw->mouse_pos_x = xpos;
        glfw->mouse_pos_y = ypos;
        ImGui::GetIO().AddMousePosEvent(xpos, ypos);
    }
}
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    pvp::GlfwToRender* glfw = static_cast<pvp::GlfwToRender*>(glfwGetWindowUserPointer(window));
    {
        std::lock_guard lock(glfw->lock);
        glfw->mouse_down[button] = action;
        ImGui::GetIO().AddMouseButtonEvent(button, action);
    }
}

static void char_call_back(GLFWwindow* window, unsigned int codepoint)
{
    pvp::GlfwToRender* glfw = static_cast<pvp::GlfwToRender*>(glfwGetWindowUserPointer(window));
    {
        std::lock_guard lock(glfw->lock);
        ImGui::GetIO().AddInputCharacter(codepoint);
    }
}
// ReSharper disable once CppInconsistentNaming
ImGuiKey ImGui_ImplGlfw_KeyToImGuiKey(int keycode, int scancode);

static void key_call_back(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    pvp::GlfwToRender* glfw = static_cast<pvp::GlfwToRender*>(glfwGetWindowUserPointer(window));
    {
        std::lock_guard lock(glfw->lock);
        ImGuiKey        imgui_key = ImGui_ImplGlfw_KeyToImGuiKey(key, scancode);
        ImGui::GetIO().AddKeyEvent(imgui_key, (action == GLFW_PRESS));
        glfw->keys_pressed[key] = action;
    }
}

pvp::ImguiRenderer::ImguiRenderer(Context& context, GLFWwindow* window, GlfwToRender* glfw_to_render)
    : m_window{ window }
    , m_glfw_to_render{ glfw_to_render }
    , m_context{ context }
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(m_window, false);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
}
void pvp::ImguiRenderer::setup_vulkan_context(const CommandPool& command_pool)
{
    VkFormat format = m_context.swapchain->get_swapchain_surface_format().format;

    ImGui_ImplVulkan_InitInfo info{
        .ApiVersion = VK_API_VERSION_1_3,
        .Instance = m_context.instance->get_instance(),
        .PhysicalDevice = m_context.physical_device->get_physical_device(),
        .Device = m_context.device->get_device(),
        .QueueFamily = ImGui_ImplVulkanH_SelectQueueFamilyIndex(m_context.physical_device->get_physical_device()),
        .Queue = m_context.queue_families->get_queue_family(VK_QUEUE_GRAPHICS_BIT, false)->queue,
        .DescriptorPool = m_context.descriptor_creator->get_pool(),
        .RenderPass = nullptr,
        .MinImageCount = 2,
        .ImageCount = static_cast<uint32_t>(m_context.swapchain->get_images().size()),
        .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        .PipelineCache = nullptr,
        .Subpass = 0,
        .DescriptorPoolSize = 0,
        .UseDynamicRendering = true,
        .PipelineRenderingCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .pNext = nullptr,
            .viewMask = 0,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &format,
            .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
            .stencilAttachmentFormat = VK_FORMAT_UNDEFINED },
        .Allocator = nullptr,
        .CheckVkResultFn = nullptr,
        .MinAllocationSize = 0
    };
    ImGui_ImplVulkan_Init(&info);

    m_command_buffer.resize(max_frames_in_flight);
    m_command_buffer = command_pool.allocate_buffers(max_frames_in_flight);

    {
        ImGuiIO&        io = ImGui::GetIO();
        std::lock_guard lock(m_glfw_to_render->lock);
        io.DisplaySize.x = m_glfw_to_render->screen_width;
        io.DisplaySize.y = m_glfw_to_render->screen_height;
        io.DisplayFramebufferScale.x = 1.0f;
        io.DisplayFramebufferScale.y = 1.0f;
    }

    // TODO: These functions can only be called from main thread
    glfwSetFramebufferSizeCallback(m_window, &framebuffer_size_callback);
    glfwSetCursorPosCallback(m_window, &mouse_pos_callback);
    glfwSetMouseButtonCallback(m_window, &mouse_button_callback);
    glfwSetCharCallback(m_window, &char_call_back);
    glfwSetKeyCallback(m_window, &key_call_back);
}

void pvp::ImguiRenderer::destroy_vulkan_context()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
void pvp::ImguiRenderer::update_screen()
{
    if (m_glfw_to_render->needs_resizing)
    {
        ImGuiIO& io = ImGui::GetIO();

        std::lock_guard lock(m_glfw_to_render->lock);
        io.DisplaySize.x = m_glfw_to_render->screen_width;
        io.DisplaySize.y = m_glfw_to_render->screen_height;
        io.DisplayFramebufferScale.x = 1.0f;
        io.DisplayFramebufferScale.y = 1.0f;
        m_glfw_to_render->needs_resizing = false;
    }
}
void pvp::ImguiRenderer::start_drawing()
{
    update_screen();

    // ImGui_ImplGlfw_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    {
        std::lock_guard lock(m_glfw_to_render->lock);
        ImGui::NewFrame();
    }
}
void pvp::ImguiRenderer::test_draw_demo_drawing()
{
    ImGui::ShowDemoWindow();
}
void pvp::ImguiRenderer::end_drawing()
{
    ImGui::EndFrame();
    ImGui::Render();

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void pvp::ImguiRenderer::draw(const FrameContext& frame_context, int swapchain_index)
{
    VkCommandBuffer&         cmd = m_command_buffer[frame_context.buffer_index];
    VkCommandBufferBeginInfo cmd_buffer_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(cmd, &cmd_buffer_info);

    RenderInfoBuilderOut render_color_info;
    RenderInfoBuilder()
        .add_color(m_context.swapchain->get_views()[frame_context.buffer_index], VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE)
        .set_size(m_context.swapchain->get_swapchain_extent())
        .build(render_color_info);

    vkCmdBeginRendering(cmd, &render_color_info.rendering_info);
    ImDrawData* p_draw_data = ImGui::GetDrawData();
    if (p_draw_data != nullptr)
        ImGui_ImplVulkan_RenderDrawData(p_draw_data, cmd);
    vkCmdEndRendering(cmd);

    VkImageSubresourceRange range{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS
    };

    image_layout_transition(cmd,
                            m_context.swapchain->get_images()[swapchain_index],
                            VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                            VK_ACCESS_2_TRANSFER_WRITE_BIT,
                            VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                            range);

    vkEndCommandBuffer(cmd);
}
VkCommandBuffer pvp::ImguiRenderer::get_cmd(int index)
{
    return m_command_buffer[index];
}
