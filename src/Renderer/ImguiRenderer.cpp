#include "ImguiRenderer.h"

#include "RenderInfoBuilder.h"
#include "Renderer.h"
#include "Swapchain.h"

#include <Context/Context.h>
#include <Context/Device.h>
#include <Context/PhysicalDevice.h>
#include <Context/QueueFamilies.h>
#include <DescriptorSets/DescriptorLayoutCreator.h>
#include <Image/TransitionLayout.h>
#include <Window/WindowSurface.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

pvp::ImguiRenderer::ImguiRenderer(Context& context, CommandPool& command_pool)
    : m_context{ context }
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(context.window_surface->get_window(), true);

    VkFormat format = context.swapchain->get_swapchain_surface_format().format;

    ImGui_ImplVulkan_InitInfo info{
        .ApiVersion = VK_API_VERSION_1_3,
        .Instance = context.instance->get_instance(),
        .PhysicalDevice = context.physical_device->get_physical_device(),
        .Device = context.device->get_device(),
        .QueueFamily = ImGui_ImplVulkanH_SelectQueueFamilyIndex(context.physical_device->get_physical_device()),
        .Queue = context.queue_families->get_queue_family(VK_QUEUE_GRAPHICS_BIT, false)->queue,
        .DescriptorPool = context.descriptor_creator->get_pool(),
        .RenderPass = nullptr,
        .MinImageCount = 2,
        .ImageCount = static_cast<uint32_t>(context.swapchain->get_images().size()),
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
}

pvp::ImguiRenderer::~ImguiRenderer()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
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
