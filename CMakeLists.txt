cmake_minimum_required(VERSION 3.25)
project(pretty-vulkan-printer VERSION 1.0 LANGUAGES CXX C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_executable(${PROJECT_NAME}
        src/main.cpp
        src/App.cpp
        src/App.h
        src/DestructorQueue.cpp
        src/DestructorQueue.h
        src/PVPInstance/Instance.cpp
        src/PVPInstance/Instance.h
        src/VulkanExternalFunctions.h
        src/stb_image.h
        src/tiny_obj_loader.h
        src/PVPImage/Image.cpp
        src/PVPImage/Image.h
        src/PVPRenderer/Swapchain.cpp
        src/PVPRenderer/Swapchain.h
        src/PVPInstance/InstanceBuilder.cpp
        src/PVPInstance/InstanceBuilder.h
        src/PVPRenderPass/RenderPassBuilder.cpp
        src/PVPRenderPass/RenderPassBuilder.h
        src/PVPGraphicsPipeline/GraphicsPipelineBuilder.cpp
        src/PVPGraphicsPipeline/GraphicsPipelineBuilder.h
        src/PVPPhysicalDevice/Context.h
        src/PVPPhysicalDevice/PhysicalDevice.h
        src/PVPPhysicalDevice/PhysicalDevice.cpp
        src/PVPGraphicsPipeline/ShaderLoader.cpp
        src/PVPGraphicsPipeline/ShaderLoader.h
        src/PVPGraphicsPipeline/PipelineLayoutBuilder.cpp
        src/PVPGraphicsPipeline/PipelineLayoutBuilder.h
        src/PVPGraphicsPipeline/Vertex.cpp
        src/PVPGraphicsPipeline/Vertex.h
        src/PVPGraphicsPipeline/DescriptorSetLayoutBuilder.cpp
        src/PVPGraphicsPipeline/DescriptorSetLayoutBuilder.h
        src/PVPVMAAllocator/VmaAllocator.cpp
        src/PVPVMAAllocator/VmaAllocator.cpp
        src/PVPVMAAllocator/VmaAllocator.h
        src/PVPBuffer/Buffer.cpp
        src/PVPBuffer/Buffer.h

        src/PVPCommandBuffer/CommandPool.cpp
        src/PVPCommandBuffer/CommandPool.h
        src/PVPSyncManager/SyncBuilder.cpp
        src/PVPSyncManager/SyncBuilder.h
        src/PVPSyncManager/Semaphore.h
        src/PVPSyncManager/Fence.h
        src/globalconst.h
        src/PVPDescriptorSets/DescriptorSetBuilder.cpp
        src/PVPDescriptorSets/DescriptorSetBuilder.h
        src/PVPDescriptorSets/DescriptorPool.h
        src/PVPDescriptorSets/DescriptorPool.cpp
        src/PVPDescriptorSets/DescriptorSets.h
        src/UniformBufferStruct.h
        src/PVPBuffer/BufferBuilder.cpp
        src/PVPBuffer/BufferBuilder.h
        src/PVPUniformBuffers/UniformBuffer.cpp
        src/PVPUniformBuffers/UniformBuffer.h
        src/PVPSyncManager/FrameSyncers.cpp
        src/PVPSyncManager/FrameSyncers.h
        src/PVPImage/Sampler.cpp
        src/PVPImage/Sampler.h
        src/PVPImage/TextureBuilder.cpp
        src/PVPImage/TextureBuilder.h
        src/PVPImage/ImageBuilder.cpp
        src/PVPImage/ImageBuilder.h
        src/PVPRenderer/Renderer.cpp
        src/PVPRenderer/Renderer.h
        src/PVPImage/SamplerBuilder.cpp
        src/PVPImage/SamplerBuilder.h
        src/PVPDebugger/Debugger.cpp
        src/PVPDebugger/Debugger.h
        src/PVPWindow/WindowSurface.cpp
        src/PVPWindow/WindowSurface.h
        src/PVPWindow/WindowSurfaceBuilder.cpp
        src/PVPWindow/WindowSurfaceBuilder.h
        src/PVPPhysicalDevice/LogicPhysicalQueueBuilder.h
        src/PVPPhysicalDevice/LogicPhysicalQueueBuilder.cpp
        src/PVPPhysicalDevice/QueueFamilies.h
        src/PVPDevice/Device.cpp
        src/PVPDevice/Device.h
        src/PVPPhysicalDevice/QueueFamilies.cpp
        src/PVPRenderer/RenderToSwapchain.cpp
        src/PVPRenderer/RenderToSwapchain.h
        src/PVPImage/TransitionLayout.cpp
        src/PVPImage/TransitionLayout.h
        src/PVPRenderer/GBuffer.cpp
        src/PVPRenderer/GBuffer.h
        src/PVPDescriptorSets/DescriptorLayoutBuilder.cpp
        src/PVPDescriptorSets/DescriptorLayoutBuilder.h
        src/Scene/PVPScene.cpp
        src/Scene/PVPScene.h
        src/Scene/LoadModel.cpp
        src/Scene/LoadModel.h
        src/PVPRenderer/LightPass.cpp
        src/PVPRenderer/LightPass.h
)

add_subdirectory(libraries)

# needs to be different as it doesn't run when source files don't change
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} pretty-vulkan-printer-libraries)

include(shaders/compile_shaders.cmake)

#target_compile_options(${PROJECT_NAME} PRIVATE
#        $<$<CXX_COMPILER_ID:MSVC>:/W4>
#        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
#)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/.gitignore *)
