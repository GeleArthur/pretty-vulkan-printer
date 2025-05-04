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
        src/PVPSwapchain/Swapchain.cpp
        src/PVPSwapchain/Swapchain.h
        src/PVPInstance/InstanceBuilder.cpp
        src/PVPInstance/InstanceBuilder.h
        src/PVPRenderPass/RenderPassBuilder.cpp
        src/PVPRenderPass/RenderPassBuilder.h
        src/PVPGraphicsPipeline/GraphicsPipelineBuilder.cpp
        src/PVPGraphicsPipeline/GraphicsPipelineBuilder.h
        src/PVPPhysicalDevice/Context.h
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
        src/LoadModel.cpp
        src/LoadModel.h
        src/PVPCommandBuffer/CommandBuffer.cpp
        src/PVPCommandBuffer/CommandBuffer.h
        src/PVPSyncManager/SyncBuilder.cpp
        src/PVPSyncManager/SyncBuilder.h
        src/PVPSyncManager/Semaphore.h
        src/PVPSyncManager/Fence.h
        src/globalconst.h
        src/PVPDescriptorSets/DescriptorSetBuilder.cpp
        src/PVPDescriptorSets/DescriptorSetBuilder.h
        src/PVPDescriptorSets/DescriptorPool.h
        src/PVPDescriptorSets/DescriptorPool.cpp
        src/PVPDescriptorSets/DescriptorLayout.cpp
        src/PVPDescriptorSets/DescriptorLayout.h
        src/PVPDescriptorSets/DescriptorSets.h
        src/UniformBufferStruct.h
        src/PVPModels/GPUModel.cpp
        src/PVPModels/GPUModel.h
        src/PVPModels/ModelData.h
        src/PVPBuffer/BufferBuilder.cpp
        src/PVPBuffer/BufferBuilder.h
        src/PVPUniformBuffers/UniformBuffer.cpp
        src/PVPUniformBuffers/UniformBuffer.h
        src/PVPFrameContext/FrameContext.cpp
        src/PVPFrameContext/FrameContext.h
        src/PVPSyncManager/FrameSyncers.cpp
        src/PVPSyncManager/FrameSyncers.h
        src/PVPImage/Sampler.cpp
        src/PVPImage/Sampler.h
        src/PVPImage/TextureBuilder.cpp
        src/PVPImage/TextureBuilder.h
        src/PVPImage/ImageBuilder.cpp
        src/PVPImage/ImageBuilder.h
        src/PVPCommandBuffer/CommandBufferBuilder.cpp
        src/PVPCommandBuffer/CommandBufferBuilder.h
        src/PVPRenderer/PVPRenderer.cpp
        src/PVPRenderer/PVPRenderer.h
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
        src/PVPPhysicalDevice/QueueFamillies.h
        src/PVPDevice/Device.cpp
        src/PVPDevice/Device.h
)

add_subdirectory(libraries)

# needs to be different as it doesn't run when source files don't change
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} pretty-vulkan-printer-libraries)

include(shaders/compile_shaders.cmake)

#target_compile_options(${PROJECT_NAME} PRIVATE
#        $<$<CXX_COMPILER_ID:MSVC>:/W4>
#        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
#)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/.gitignore *)
