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
        src/stb_image.h
        src/VulkanExternalFunctions.h
        src/tiny_obj_loader.h
        src/globalconst.h

        src/Debugger/Debugger.cpp
        src/Debugger/Debugger.h
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
        src/RenderPassOLD/RenderPassBuilder.cpp
        src/RenderPassOLD/RenderPassBuilder.h
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
        src/DescriptorSets/DescriptorPool.h
        src/DescriptorSets/DescriptorPool.cpp
        src/DescriptorSets/DescriptorSets.h
        src/UniformBufferStruct.h
        src/UniformBuffers/UniformBuffer.cpp
        src/UniformBuffers/UniformBuffer.h
        src/Window/WindowSurface.cpp
        src/Window/WindowSurface.h
        src/Window/WindowSurfaceBuilder.cpp
        src/Window/WindowSurfaceBuilder.h
        src/DescriptorSets/DescriptorLayoutBuilder.cpp
        src/DescriptorSets/DescriptorLayoutBuilder.h
        src/Scene/PVPScene.cpp
        src/Scene/PVPScene.h
        src/Scene/LoadModel.cpp
        src/Scene/LoadModel.h
        src/Renderer/LightPass.cpp
        src/Renderer/LightPass.h
        src/Renderer/RenderInfoBuilder.cpp
        src/Renderer/RenderInfoBuilder.h
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
