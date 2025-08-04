#include "ImguiRenderer.h"

#include "Swapchain.h"

#include <Context/Context.h>
#include <Context/Device.h>
#include <Context/PhysicalDevice.h>
#include <Context/QueueFamilies.h>
#include <DescriptorSets/DescriptorLayoutCreator.h>
#include <Window/WindowSurface.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
pvp::ImguiRenderer::ImguiRenderer(const pvp::Context& context)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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
}

void pvp::ImguiRenderer::destroy()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void pvp::ImguiRenderer::render_frame(const FrameContext& frame_context)
{
    // vkCmdBeginRenderPass()
}