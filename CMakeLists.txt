cmake_minimum_required(VERSION 3.25)
project(pretty-vulkan-printer VERSION 1.0 LANGUAGES CXX C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(${PROJECT_NAME}
        src/main.cpp
        src/App.cpp
        src/App.h
        src/DestructorQueue.cpp
        src/DestructorQueue.h
        src/stb_image.h
        src/VulkanExternalFunctions.h
        src/tiny_obj_loader.h
        src/globalconst.h

        src/Debugger/debugger.cpp
        src/Debugger/debugger.h
        src/Context/Instance.cpp
        src/Context/Instance.h
        src/Context/InstanceBuilder.cpp
        src/Context/InstanceBuilder.h
        src/Context/Context.h
        src/Context/PhysicalDevice.h
        src/Context/PhysicalDevice.cpp
        src/Context/LogicPhysicalQueueBuilder.h
        src/Context/LogicPhysicalQueueBuilder.cpp
        src/Context/QueueFamilies.h
        src/Context/Device.cpp
        src/Context/Device.h
        src/Context/QueueFamilies.cpp
        src/Image/Image.cpp
        src/Image/Image.h
        src/Image/Sampler.cpp
        src/Image/Sampler.h
        src/Image/TextureBuilder.cpp
        src/Image/TextureBuilder.h
        src/Image/ImageBuilder.cpp
        src/Image/ImageBuilder.h
        src/Image/SamplerBuilder.cpp
        src/Image/SamplerBuilder.h
        src/Image/TransitionLayout.cpp
        src/Image/TransitionLayout.h
        src/Renderer/Swapchain.cpp
        src/Renderer/Swapchain.h
        src/Renderer/Renderer.cpp
        src/Renderer/Renderer.h
        src/Renderer/BlitToSwapchain.cpp
        src/Renderer/BlitToSwapchain.h
        src/Renderer/GBuffer.cpp
        src/Renderer/GBuffer.h
        src/GraphicsPipeline/GraphicsPipelineBuilder.cpp
        src/GraphicsPipeline/GraphicsPipelineBuilder.h
        src/GraphicsPipeline/ShaderLoader.cpp
        src/GraphicsPipeline/ShaderLoader.h
        src/GraphicsPipeline/PipelineLayoutBuilder.cpp
        src/GraphicsPipeline/PipelineLayoutBuilder.h
        src/GraphicsPipeline/Vertex.cpp
        src/GraphicsPipeline/Vertex.h
        src/VMAAllocator/VmaAllocator.cpp
        src/VMAAllocator/VmaAllocator.cpp
        src/VMAAllocator/VmaAllocator.h
        src/Buffer/Buffer.cpp
        src/Buffer/Buffer.h
        src/Buffer/BufferBuilder.cpp
        src/Buffer/BufferBuilder.h
        src/CommandBuffer/CommandPool.cpp
        src/CommandBuffer/CommandPool.h
        src/SyncManager/SyncBuilder.cpp
        src/SyncManager/SyncBuilder.h
        src/SyncManager/Semaphore.h
        src/SyncManager/Fence.h
        src/SyncManager/FrameSyncers.cpp
        src/SyncManager/FrameSyncers.h
        src/DescriptorSets/DescriptorSetBuilder.cpp
        src/DescriptorSets/DescriptorSetBuilder.h
        src/DescriptorSets/DescriptorLayoutCreator.h
        src/DescriptorSets/DescriptorLayoutCreator.cpp
        src/DescriptorSets/DescriptorSets.h
        src/UniformBufferStruct.h
        src/UniformBuffers/UniformBuffer.cpp
        src/UniformBuffers/UniformBuffer.h
        src/Window/WindowSurfaceBuilder.cpp
        src/Window/WindowSurfaceBuilder.h
        src/DescriptorSets/DescriptorLayoutBuilder.cpp
        src/DescriptorSets/DescriptorLayoutBuilder.h
        src/Scene/PVPScene.cpp
        src/Scene/PVPScene.h
        src/Scene/ModelData.cpp
        src/Scene/ModelData.h
        src/Renderer/LightPass.cpp
        src/Renderer/LightPass.h
        src/Renderer/RenderInfoBuilder.cpp
        src/Renderer/RenderInfoBuilder.h
        src/Renderer/DepthPrePass.cpp
        src/Renderer/DepthPrePass.h
        src/Scene/Camera.cpp
        src/Scene/Camera.h
        src/Renderer/ToneMappingPass.cpp
        src/Renderer/ToneMappingPass.h
        src/VulkanExternalFunctions.cpp
        src/OverwriteNewDelete.cpp
        src/Events/EventListener.h
        src/Events/Event.h
        src/Image/StaticImage.h
        src/Image/StaticImage.h
        src/Renderer/FrameContext.h
        src/Image/StaticImage.cpp
        src/DescriptorSets/DescriptorSets.cpp
        src/ImguiRenderer.cpp
        src/ImguiRenderer.h
        src/VulkanApp.cpp
        src/VulkanApp.h
        src/GlfwToRender.h
        src/GlfwToRender.h
        src/Image/StaticCubemapImage.cpp
        src/Image/StaticCubemapImage.h
        src/Image/StaticCubemapImageBuilder.cpp
        src/Image/StaticCubemapImageBuilder.h
        src/Renderer/MeshShaderPass.cpp
        src/Renderer/MeshShaderPass.h
        src/DescriptorSets/CommonDescriptorLayouts.h
        src/Debugger/GizmosDrawer.cpp
        src/Debugger/GizmosDrawer.h
        src/Debugger/DebugVertex.h
        src/Debugger/Gizmos.h
        src/Debugger/Gizmos.cpp
)

add_subdirectory(libraries)

# needs to be different as it doesn't run when source files don't change
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink # TODO: WHEN HANDING IN CHANGE create_symlink TO copy_directory_if_different. ( I need a fallback)
        "${CMAKE_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink # TODO: WHEN HANDING IN CHANGE create_symlink TO copy_directory_if_different. ( I need a fallback)
        "${CMAKE_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
)


target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE pretty-vulkan-printer-libraries)

#target_compile_options(${PROJECT_NAME} PRIVATE
#        $<$<CXX_COMPILER_ID:MSVC>:/W4>
#        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
#)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/.gitignore *)

find_package(VLD CONFIG)
target_include_directories(${PROJECT_NAME} PRIVATE ${VLD_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${VLD_LIBRARY})

message(STATUS "DOWNLOADING BIG INTEL SPONZA SCENE")

if(NOT EXISTS $ENV{TMP}/intelsponza.zip)
    file(DOWNLOAD "https://cdrdv2.intel.com/v1/dl/getContent/830833" "$ENV{TMP}/intelsponza.zip" SHOW_PROGRESS)
endif()
if(NOT EXISTS $ENV{TMP}/intelsponza_curtains.zip)
    file(DOWNLOAD "https://cdrdv2.intel.com/v1/dl/getContent/726650" "$ENV{TMP}/intelsponza_curtains.zip" SHOW_PROGRESS)
endif()
if(NOT EXISTS $ENV{TMP}/bistro.zip)
    file(DOWNLOAD "https://developer.nvidia.com/bistro" "$ENV{TMP}/bistro.zip" SHOW_PROGRESS)
endif()

message(STATUS "EXTRACTING SPONZA SCENE")
if(EXISTS $ENV{TMP}/intelsponza.zip AND NOT EXISTS ${CMAKE_BINARY_DIR}/intelsponza/main_sponza)
    file(ARCHIVE_EXTRACT INPUT $ENV{TMP}/intelsponza.zip DESTINATION ${CMAKE_BINARY_DIR}/intelsponza VERBOSE)
endif()
if(EXISTS $ENV{TMP}/intelsponza_curtains.zip AND NOT EXISTS ${CMAKE_BINARY_DIR}/intelsponza/pkg_a_curtains)
    file(ARCHIVE_EXTRACT INPUT $ENV{TMP}/intelsponza_curtains.zip DESTINATION ${CMAKE_BINARY_DIR}/intelsponza VERBOSE)
endif()
if(EXISTS $ENV{TMP}/bistro.zip AND NOT EXISTS ${CMAKE_BINARY_DIR}/intelsponza/Bistro_v5_2)
    file(ARCHIVE_EXTRACT INPUT $ENV{TMP}/bistro.zip DESTINATION ${CMAKE_BINARY_DIR}/intelsponza VERBOSE)
endif()
