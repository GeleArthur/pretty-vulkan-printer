﻿#include "App.h"

#include "Context/LogicPhysicalQueueBuilder.h"

#include <iostream>
#include <Context/InstanceBuilder.h>
#include <DescriptorSets/DescriptorLayoutCreator.h>
#include <GLFW/glfw3.h>
#include <GraphicsPipeline/GraphicsPipelineBuilder.h>
#include <Renderer/Renderer.h>
#include <Renderer/Swapchain.h>
#include <Scene/PVPScene.h>
#include <SyncManager/FrameSyncers.h>
#include <Window/WindowSurfaceBuilder.h>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

void pvp::App::run()
{
    ZoneScoped;
    glfwInit();
    m_destructor_queue.add_to_queue([&] {
        glfwTerminate();
    });

    InstanceBuilder()
        .enable_debugging(true)
        .set_app_name("pretty vulkan printer")
        .build(m_instance);
    m_destructor_queue.add_to_queue([&] { m_instance.destroy(); });

    WindowSurfaceBuilder()
        .set_window_size(1200, 1000)
        .set_window_title("pretty vulkan printer")
        .build(m_instance, m_window_surface);
    m_destructor_queue.add_to_queue([&] { m_window_surface.destroy(m_instance); });

    // TODO: Context builder
    LogicPhysicalQueueBuilder()
        .set_extensions({ VK_EXT_SHADER_OBJECT_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME })
        .build(m_instance, m_window_surface, m_physical_device, m_device, m_queue_families);
    m_destructor_queue.add_to_queue([&] { m_device.destroy(); });

    create_allocator(m_allocator, m_instance, m_device, m_physical_device);
    m_destructor_queue.add_to_queue([&] { m_allocator.destroy(); });

    m_context.instance = &m_instance;
    m_context.physical_device = &m_physical_device;
    m_context.device = &m_device;
    m_context.allocator = &m_allocator;
    m_context.queue_families = &m_queue_families;
    m_context.window_surface = &m_window_surface;

    m_context.descriptor_creator = new DescriptorLayoutCreator(m_context);
    m_destructor_queue.add_to_queue([&] { delete m_context.descriptor_creator; });

    m_swapchain = new Swapchain(m_context, m_window_surface);
    m_destructor_queue.add_to_queue([&] { delete m_swapchain; });
    m_context.swapchain = m_swapchain;

    m_scene = new PvpScene(m_context);
    m_destructor_queue.add_to_queue([&] { delete m_scene; });

    m_renderer = new Renderer(m_context, *m_swapchain, *m_scene);
    m_destructor_queue.add_to_queue([&] { delete m_renderer; });

    std::jthread rendering_thread{ &App::run_loop, this };

    glfwSetWindowUserPointer(m_window_surface.get_window(), &m_context);

    while (!glfwWindowShouldClose(m_window_surface.get_window()))
    {
        ZoneScoped;
        glfwWaitEvents();
    }

    rendering_thread.join();
    vkDeviceWaitIdle(m_device.get_device());
}

void pvp::App::run_loop() const
{
    ZoneScoped;
    while (!glfwWindowShouldClose(m_window_surface.get_window()))
    {
        m_scene->update();
        m_renderer->draw();
        FrameMark;
    }
}
