#include "VulkanApp.h"

#include <GlfwToRender.h>
#include <ImguiRenderer.h>
#include <Context/Device.h>
#include <Context/InstanceBuilder.h>
#include <Context/LogicPhysicalQueueBuilder.h>
#include <DescriptorSets/DescriptorLayoutCreator.h>
#include <Renderer/Renderer.h>
#include <Renderer/Swapchain.h>
#include <Scene/PVPScene.h>
#include <tracy/Tracy.hpp>
#include <vulkan/vulkan.hpp>

void pvp::VulkanApp::run(GLFWwindow* window, GlfwToRender& gtfw_to_render)
{
    ZoneScoped;
    ZoneNamed(loading, "Loading");
    // TODO: Deiced to remove all the destructors and add destroy to all classes. I am feeling destroy
    Instance instance{};
    InstanceBuilder()
        .enable_debugging(true)
        .set_app_name("pretty vulkan printer")
        .build(instance);

    DestructorQueue destructor_queue;
    VkSurfaceKHR    surface{};
    if (auto result = glfwCreateWindowSurface(instance.get_instance(), window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error(std::format("Failed to create surface. {}", result));
    }
    destructor_queue.add_to_queue([&] { vkDestroySurfaceKHR(instance.get_instance(), surface, nullptr); });

    Device         device;
    PhysicalDevice physical_device;
    QueueFamilies  queue_families;
    LogicPhysicalQueueBuilder()
        .set_extensions({ VK_EXT_MESH_SHADER_EXTENSION_NAME })
        .build(instance, surface, physical_device, device, queue_families);

    PvpVmaAllocator allocator{};
    create_allocator(allocator, instance, device, physical_device);

    Context context{};
    context.instance = &instance;
    context.physical_device = &physical_device;
    context.device = &device;
    context.allocator = &allocator;
    context.queue_families = &queue_families;
    context.surface = surface;
    context.gtfw_to_render = &gtfw_to_render;

    DescriptorLayoutCreator descriptor_creator = DescriptorLayoutCreator(context);
    context.descriptor_creator = &descriptor_creator;

    Swapchain swapchain = Swapchain(context, gtfw_to_render);
    context.swapchain = &swapchain;

    PvpScene scene = PvpScene(context);
    // scene.load_scene(std::filesystem::absolute("../intelsponza/main_sponza/NewSponza_Main_glTF_003.gltf"));
    // scene.load_scene(std::filesystem::absolute("../intelsponza/pkg_a_curtains/NewSponza_Curtains_glTF.gltf"));
    // scene.load_scene(std::filesystem::absolute("resources/rossbandiger/Fixed mesh.glb"));
    scene.load_scene(std::filesystem::absolute("resources/test_triangle.glb"));
    // scene.load_scene(std::filesystem::absolute("resources/Sponza/Sponza.gltf"));

    ImguiRenderer imgui_renderer = ImguiRenderer(context, window, &gtfw_to_render);
    Renderer      renderer = Renderer(context, scene, imgui_renderer);

    ZoneNamed(running, "running");
    while (gtfw_to_render.running)
    {
        FrameMark;
        ZoneScoped;
        imgui_renderer.start_drawing();
        scene.update();
        imgui_renderer.end_drawing();
        renderer.draw();
    }

    vkDeviceWaitIdle(context.device->get_device());

    // m_destructor_queue.destroy_and_clear();
}
